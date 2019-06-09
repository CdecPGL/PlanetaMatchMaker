# Message Reception

## authentication_request_message

1. Check if the client version is same as the server version
1. If version is not same, send error to the client
1. Add or update client information to server data
1. Send authentication_reply_message to the client

## create_room_request_message

1. Check if the client is in the client list
1. If the client is not in the client list, send error to the client
1. Assign room id and add room to room list
1. Send create_room_reply_message to the client
