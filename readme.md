# scylla: a minimal cli chatgpt client

scylla is a super minimal chatgpt client written in c. i named after the sea monster from greek mythology, known for its many heads - symbolizing the multiple conversational turns in a chat. .

<a href="https://i.imgur.com/gIIaMIL.png"><img src="https://i.imgur.com/gIIaMIL.png" width="60%" align="center"></a>

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
   gcc -o scylla scylla.c -lcurl -ljson-c
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

## example

start a conversation with chatgpt:

```
./scylla
user > hello, can you explain what a linked list is?
ai > certainly! i'd be happy to explain what a linked list is.

a linked list is a linear data structure used in computer science and programming. unlike arrays, which store elements in contiguous memory locations, linked lists consist of a sequence of elements, called nodes, where each node contains two main components:

1. data: this is the actual information or value stored in the node.
2. reference (or link): this is a pointer to the next node in the sequence.

key characteristics of linked lists:

1. dynamic size: linked lists can grow or shrink in size during program execution, as memory is allocated or deallocated as needed.

2. efficient insertion and deletion: adding or removing elements from the beginning or middle of the list is generally more efficient compared to arrays, as it only involves changing a few pointers.

3. non-contiguous memory: elements in a linked list are not stored in adjacent memory locations, which can be both an advantage (flexible memory usage) and a disadvantage (slower access time for specific elements).

4. sequential access: to access an element in a linked list, you typically need to traverse the list from the beginning, following the links between nodes.

there are several types of linked lists:

1. singly linked list: each node points to the next node in the sequence.
2. doubly linked list: each node has pointers to both the next and previous nodes.
3. circular linked list: the last node points back to the first node, creating a circle.

linked lists are used in various applications, such as:

- implementing other data structures like stacks and queues
- managing memory allocation in operating systems
- representing sparse matrices
- undo functionality in applications

while linked lists have some advantages, they also have drawbacks, such as slower random access compared to arrays and additional memory overhead for storing the links.

user > quit

thank you for using chatgpt-cli. goodbye!
```

## note

this is a minimal chatgpt client intended for basic usage and learning purposes. it may not support all openai api features or have robust error handling. use at your own discretion, and remember to keep your api key secure.
