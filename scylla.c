#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <unistd.h>
#include <ctype.h>

#define API_URL "https://api.openai.com/v1/chat/completions"
#define MAX_INPUT_SIZE 1024
#define MAX_CONVERSATION_SIZE 10000
#define MAX_RETRIES 3
#define RETRY_DELAY 5

// TODO: Replace with your actual OpenAI API key
#define API_KEY "PUT_YOUR_KEY_HERE!"

// ANSI color codes
#define COLOR_RESET   "\x1B[0m"
#define COLOR_GREEN   "\x1B[32m"
#define COLOR_BLUE    "\x1B[34m"
#define COLOR_CYAN    "\x1B[36m"
#define COLOR_YELLOW  "\x1B[33m"
#define COLOR_RED     "\x1B[31m"
#define COLOR_BOLD    "\x1B[1m"
#define COLOR_ITALIC  "\x1B[3m"

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

void print_wrapped(const char *text, int width) {
    int line_start = 0;
    int len = strlen(text);
    int in_code_block = 0;
    
    for (int i = 0; i < len; i++) {
        if (strncmp(&text[i], "```", 3) == 0) {
            in_code_block = !in_code_block;
            i += 2;
            continue;
        }
        
        if (!in_code_block && i - line_start >= width && text[i] == ' ') {
            printf("%.*s\n", i - line_start, text + line_start);
            line_start = i + 1;
        }
    }
    
    if (line_start < len) {
        printf("%s\n", text + line_start);
    }
}

void render_markdown(const char *text) {
    int in_code_block = 0;
    int in_bold = 0;
    int in_italic = 0;
    int list_level = 0;

    char *token = strtok(strdup(text), "\n");
    while (token != NULL) {
        if (strncmp(token, "```", 3) == 0) {
            in_code_block = !in_code_block;
            printf("%s\n", in_code_block ? COLOR_CYAN : COLOR_RESET);
        } else if (in_code_block) {
            printf("%s\n", token);
        } else if (token[0] == '#') {
            int level = 0;
            while (token[level] == '#' && level < 6) level++;
            printf(COLOR_BOLD "%s%s" COLOR_RESET "\n", &token[level], level > 0 ? "" : token);
        } else {
            char *p = token;
            while (*p) {
                if (*p == '*' || *p == '_') {
                    if (*(p+1) == *p) {
                        printf(in_bold ? COLOR_RESET : COLOR_BOLD);
                        in_bold = !in_bold;
                        p++;
                    } else {
                        printf(in_italic ? COLOR_RESET : COLOR_ITALIC);
                        in_italic = !in_italic;
                    }
                } else if (*p == '`') {
                    printf(COLOR_CYAN "%c" COLOR_RESET, *p);
                } else if (strncmp(p, "- ", 2) == 0 && list_level == 0) {
                    printf("  • ");
                    p++;
                    list_level = 1;
                } else {
                    putchar(*p);
                }
                p++;
            }
            printf("\n");
            list_level = 0;
        }
        token = strtok(NULL, "\n");
    }
    printf(COLOR_RESET "\n");
}

void save_conversation(const char *conversation, const char *format) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char filename[64];
    strftime(filename, sizeof(filename), "conversation_%Y%m%d_%H%M%S", tm);

    if (strcmp(format, "json") == 0) {
        strcat(filename, ".json");
        FILE *file = fopen(filename, "w");
        if (file == NULL) {
            printf(COLOR_YELLOW "Error: Unable to create file for saving conversation.\n" COLOR_RESET);
            return;
        }

        fprintf(file, "{\n  \"conversation\": [\n");
        char *token = strtok(strdup(conversation), "\n");
        int first = 1;
        while (token != NULL) {
            if (!first) fprintf(file, ",\n");
            fprintf(file, "    {\"role\": \"%s\", \"content\": \"%s\"}", 
                    strncmp(token, "user: ", 6) == 0 ? "user" : "assistant",
                    token + (strncmp(token, "user: ", 6) == 0 ? 6 : 4));
            token = strtok(NULL, "\n");
            first = 0;
        }
        fprintf(file, "\n  ]\n}");
        fclose(file);
    } else if (strcmp(format, "md") == 0) {
        strcat(filename, ".md");
        FILE *file = fopen(filename, "w");
        if (file == NULL) {
            printf(COLOR_YELLOW "Error: Unable to create file for saving conversation.\n" COLOR_RESET);
            return;
        }

        fprintf(file, "# Conversation Export\n\n");
        char *token = strtok(strdup(conversation), "\n");
        while (token != NULL) {
            if (strncmp(token, "user: ", 6) == 0) {
                fprintf(file, "## user\n\n%s\n\n", token + 6);
            } else if (strncmp(token, "ai: ", 4) == 0) {
                fprintf(file, "## ai\n\n%s\n\n", token + 4);
            }
            token = strtok(NULL, "\n");
        }
        fclose(file);
    } else {
        strcat(filename, ".txt");
        FILE *file = fopen(filename, "w");
        if (file == NULL) {
            printf(COLOR_YELLOW "Error: Unable to create file for saving conversation.\n" COLOR_RESET);
            return;
        }

        fprintf(file, "%s", conversation);
        fclose(file);
    }

    printf(COLOR_YELLOW "Conversation saved to %s\n" COLOR_RESET, filename);
}

int make_api_request(CURL *curl, const char *json_string, struct MemoryStruct *chunk, struct curl_slist *headers) {
    CURLcode res;
    int retry_count = 0;

    while (retry_count < MAX_RETRIES) {
        chunk->memory = malloc(1);
        chunk->size = 0;

        curl_easy_setopt(curl, CURLOPT_URL, API_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)chunk);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, COLOR_RED "curl_easy_perform() failed: %s\n" COLOR_RESET, curl_easy_strerror(res));
            free(chunk->memory);
            retry_count++;
            if (retry_count < MAX_RETRIES) {
                printf(COLOR_YELLOW "Retrying in %d seconds... (Attempt %d of %d)\n" COLOR_RESET, RETRY_DELAY, retry_count + 1, MAX_RETRIES);
                sleep(RETRY_DELAY);
            }
        } else {
            return 0;  // Success
        }
    }

    return 1;  // All retries failed
}

int main() {
    CURL *curl;
    struct MemoryStruct chunk;
    char input[MAX_INPUT_SIZE];
    char conversation[MAX_CONVERSATION_SIZE] = "";

    curl = curl_easy_init();
    if (curl) {
        printf(COLOR_CYAN "\n╔══════════════════════════════╗\n");
        printf("║         chatgpt-cli          ║\n");
        printf("╚══════════════════════════════╝\n\n" COLOR_RESET);
        printf("Type " COLOR_YELLOW "'save'" COLOR_RESET " to save the conversation, " COLOR_YELLOW "'export json'" COLOR_RESET " or " COLOR_YELLOW "'export md'" COLOR_RESET " to export, or " COLOR_YELLOW "'quit'" COLOR_RESET " to exit.\n\n");

        while (1) {
            printf(COLOR_GREEN "user > " COLOR_RESET);
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break;
            }
            input[strcspn(input, "\n")] = 0;  // Remove newline

            if (strcmp(input, "quit") == 0) {
                break;
            }

            if (strcmp(input, "save") == 0) {
                save_conversation(conversation, "txt");
                continue;
            }

            if (strcmp(input, "export json") == 0) {
                save_conversation(conversation, "json");
                continue;
            }

            if (strcmp(input, "export md") == 0) {
                save_conversation(conversation, "md");
                continue;
            }

            // Add user input to conversation
            strcat(conversation, "user: ");
            strcat(conversation, input);
            strcat(conversation, "\n");

            struct json_object *json_obj = json_object_new_object();
            json_object_object_add(json_obj, "model", json_object_new_string("gpt-3.5-turbo"));
            
            struct json_object *messages = json_object_new_array();
            struct json_object *message = json_object_new_object();
            json_object_object_add(message, "role", json_object_new_string("user"));
            json_object_object_add(message, "content", json_object_new_string(input));
            json_object_array_add(messages, message);
            
            json_object_object_add(json_obj, "messages", messages);

            const char *json_string = json_object_to_json_string(json_obj);

            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            char auth_header[256];
            snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", API_KEY);
            headers = curl_slist_append(headers, auth_header);

            if (make_api_request(curl, json_string, &chunk, headers) == 0) {
                struct json_object *response_json = json_tokener_parse(chunk.memory);
                if (response_json == NULL) {
                    fprintf(stderr, COLOR_RED "Failed to parse JSON response\n" COLOR_RESET);
                } else {
                    struct json_object *choices, *first_choice, *message, *content;
                    if (json_object_object_get_ex(response_json, "choices", &choices) &&
                        json_object_get_type(choices) == json_type_array &&
                        (first_choice = json_object_array_get_idx(choices, 0)) != NULL &&
                        json_object_object_get_ex(first_choice, "message", &message) &&
                        json_object_object_get_ex(message, "content", &content)) {
                        
                        const char *ai_response = json_object_get_string(content);
                        printf(COLOR_BLUE "\nai > " COLOR_RESET);
                        render_markdown(ai_response);

                        // Add AI response to conversation
                        strcat(conversation, "ai: ");
                        strcat(conversation, ai_response);
                        strcat(conversation, "\n\n");
                    } else {
                        fprintf(stderr, COLOR_RED "Failed to extract response content\n" COLOR_RESET);
                        printf("Raw API response:\n%s\n", chunk.memory);
                    }

                    json_object_put(response_json);
                }
            } else {
                printf(COLOR_RED "Failed to get a response after %d attempts. Please try again later.\n" COLOR_RESET, MAX_RETRIES);
            }

            free(chunk.memory);
            json_object_put(json_obj);
            curl_slist_free_all(headers);
        }

        curl_easy_cleanup(curl);
    }

    printf(COLOR_CYAN "\nThank you for using chatgpt-cli. Goodbye!\n" COLOR_RESET);
    return 0;
}