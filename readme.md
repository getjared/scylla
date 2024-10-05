# scylla: a minimal cli chatgpt client

scylla is a super minimal chatgpt client written in c. i named after the sea monster from greek mythology, known for its many heads - symbolizing the multiple conversational turns in a chat. .

<a href="https://i.imgur.com/r3g38vl.mp4"><img src="https://i.imgur.com/r3g38vl.mp4" width="60%" align="center"></a>

## features

- gpt-3.5-turbo model (ill try to add 4 later on)
- sends and receives messages
- supports basic markdown rendering in the terminal
- colorful terminal output
- minimal external dependencies - only requires libcurl and json-c
- conversation export in txt, json, and markdown formats

## requirements

- gcc compiler
- libcurl library
- json-c library
- a working internet connection
- openai api key

## installation

1. clone this repository or download the `scylla.c` script.
2. compile the script:
   ```
   make
   ```
3. make sure to replace the api key in the code with your actual openai api key.

## usage

run the compiled program:

```
./scylla
```

### in-client commands

- `save`: save the conversation in txt format
- `export json`: export the conversation in json format
- `export md`: export the conversation in markdown format
- `quit`: exit the client

regular text input will be sent as a message to the chatgpt model.

## note

this is a minimal chatgpt client intended for basic usage and learning purposes. it may not support all openai api features or have robust error handling. use at your own discretion, and remember to keep your api key secure.
