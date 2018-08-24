Chat Server
>  forked from yorickdewid/Chat-Server

=

## Add Feature

- Transplant to BSD kernel.
- Use a UDP hook to recieve message.


## Build
Run GNU make in the repository
`make`

Then start
`./chat_server`

## Features
* Accept multiple client (max 100)
* Name and rename users
* Send private messages

## Chat commands

| Command       | Parameter             |                                     |
| ------------- | --------------------- | ----------------------------------- |
| \QUIT         |                       | Leave the chatroom                  |
| \PING         |                       | Test connection, responds with PONG |
| \NAME         | [nickname]            | Change nickname                     |
| \PRIVATE      | [reference] [message] | Send private message                |
| \ACTIVE       |                       | Show active clients                 |
| \HELP         |                       | Show this help                      |

For the SSL version of the chat server check out the [Chat Server Secure](https://github.com/yorickdewid/Chat-Server-Secure "Chat Server Secure") repository.
