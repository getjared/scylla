## dependencies

- `<stdio.h>`: standard input/output operations
- `<stdlib.h>`: memory allocation, random numbers, and other utility functions
- `<string.h>`: string manipulation functions
- `<time.h>`: time-related functions
- `<curl/curl.h>`: libcurl for http requests
- `<json-c/json.h>`: json-c for json parsing
- `<unistd.h>`: unix standard library for sleep function
- `<ctype.h>`: character type functions

## constants and macros

```c
#define API_URL "https://api.openai.com/v1/chat/completions"
#define MAX_INPUT_SIZE 1024
#define MAX_CONVERSATION_SIZE 10000
#define MAX_RETRIES 3
#define RETRY_DELAY 5
#define API_KEY "your_api_key_here"
```

- `API_URL`: the endpoint for openai's chat completions api
- `MAX_INPUT_SIZE`: maximum size of user input (1024 characters)
- `MAX_CONVERSATION_SIZE`: maximum size of the entire conversation (10000 characters)
- `MAX_RETRIES`: maximum number of retry attempts for api requests (3 times)
- `RETRY_DELAY`: delay between retry attempts in seconds (5 seconds)
- `API_KEY`: your openai api key (replace with your actual key)

## color macros

```c
#define COLOR_RESET   "\x1B[0m"
#define COLOR_GREEN   "\x1B[32m"
#define COLOR_BLUE    "\x1B[34m"
#define COLOR_CYAN    "\x1B[36m"
#define COLOR_YELLOW  "\x1B[33m"
#define COLOR_RED     "\x1B[31m"
#define COLOR_BOLD    "\x1B[1m"
#define COLOR_ITALIC  "\x1B[3m"
```

these macros define ansi color codes used for terminal output formatting.

## structures

### MemoryStruct

```c
struct MemoryStruct {
    char *memory;
    size_t size;
};
```

this structure is used to store the response from the api call. it contains a dynamically allocated char array (`memory`) and its size.

## functions

### WriteMemoryCallback

```c
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
```

this function is a callback used by libcurl to handle the received data from the api. it reallocates memory as needed and appends new data to the `MemoryStruct`.

### print_wrapped

```c
void print_wrapped(const char *text, int width);
```

this function prints text wrapped to a specified width, preserving code blocks.

### render_markdown

```c
void render_markdown(const char *text);
```

this function renders basic markdown formatting in the terminal, including headers, bold, italic, inline code, code blocks, and unordered lists.

### save_conversation

```c
void save_conversation(const char *conversation, const char *format);
```

this function saves the conversation in the specified format (txt, json, or md). it generates a timestamped filename and writes the conversation to the file.

### make_api_request

```c
int make_api_request(CURL *curl, const char *json_string, struct MemoryStruct *chunk, struct curl_slist *headers);
```

this function makes the api request to openai. it handles retries in case of failures and populates the `MemoryStruct` with the response.

### main

```c
int main();
```

the main function orchestrates the entire program flow:

1. initializes curl
2. enters a loop to handle user input
3. processes special commands (save, export, quit)
4. sends user input to the api
5. receives and displays ai responses
6. handles markdown rendering and conversation saving

## main loop flow

1. prompt user for input
2. check for special commands (save, export json, export md, quit)
3. if not a special command, add user input to the conversation
4. prepare json payload for api request
5. make api request
6. parse json response
7. extract ai response
8. render ai response with markdown
9. add ai response to the conversation
10. repeat from step 1

## error handling

- basic error handling for memory allocation, file operations, api requests, and json parsing
- the api request function includes a retry mechanism for handling temporary failures

## limitations

- the program has a fixed maximum input size and conversation size
- it supports only basic markdown rendering
- error handling, while present, is not exhaustive
- does not support multi-turn context in conversations (each request to the api is independent)
