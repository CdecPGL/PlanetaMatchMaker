# Server Code Structure

make server simple, make client complex.

## Server

Server has below things.

- Server Data
- Server Setting
- Socket for Accept
- IO Service

Server creates server threads when it runs.

## Server Thread

Server thread creates server session when accept connection.

## Server Session

Worker uses stackful coroutine.

- Socket for Communication

## Error Disconnection

- Authentication Failer: Disconnected
- Error during handling message header: Disconnected
- Error during handling message body: Continue
