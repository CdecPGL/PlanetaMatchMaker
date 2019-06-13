# Message Reception

## authentication_request_message

1. Check if the client version is same as the server version, and send error to the client if version is not same
1. Add or update client information to server data
1. Send authentication_reply_message to the client

## list_room_group_request_message

1. Check if the client is registered to the server, and send error to the client if the client is not registered to the server
1. Check requested room group index exists, and send error to the client if requested room group index doesn't exist
1. Send room group data by list_room_group_reply_message

## create_room_request_message

1. Check if the client is registered to the server, and send error to the client if the client is not registered to the server
1. Assign room id and add room to room list
1. Send create_room_reply_message to the client

## list_room_request_message

1. Check if the client is registered to the server, and send error to the client if the client is not registered to the server
1. Get requested room data list
1. Send room data list to the client in multiple time by list_room_reply_message

## join_room_request_message

1. Check if the client is registered to the server, and send error to the client if the client is not registered to the server
1. Check requested room group index exists, and send error to the client if requested room group index doesn't exist
1. Check requested room id exists, and send error to the client if requested room id doesn't exist
1. Check if player count of requested room reaches max, and send error to the client if it reaches max
1. Increment player count of the room and send join_room_reply_message to the client
