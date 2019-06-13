# Message Reception

## authentication_request_message

1. Check if the client version is same as the server version
1. If version is not same, send error to the client
1. Add or update client information to server data
1. Send authentication_reply_message to the client

## list_room_group_request_message

1. Check if the client is registered to the server
1. If the client is not registered to the server, send error to the client
1. Check requested room group index exists
1. If requested room group index doesn't exist, send error to the client
1. Send room group data by list_room_group_reply_message

## create_room_request_message

1. Check if the client is in the client list
1. If the client is not in the client list, send error to the client
1. Assign room id and add room to room list
1. Send create_room_reply_message to the client

## list_room_request_message

1. Check if the client is registered to the server
1. If the client is not registered to the server, send error to the client
1. Get requested room data list
1. Send room data list to the client in multiple time by list_room_reply_message
