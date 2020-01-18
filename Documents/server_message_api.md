# Server Message API

Message is fixed size binary data to communicate between server and client.

There are three types of message.

- Request: A message from a client which requires a reply from the server
- Notice: A message from a client which doesn't require a reply from the server
- Reply: A message from a server to response to the request message from a client

## Message Structure

All messages consist of header and body.
The sizes of all messages are in less than or equals 256 bytes.

There are two types of header.

- Request Header: A header for request message and notice message
- Reply Header: A header for reply message

### Request Header

The size is 5 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|message_type|8bit unsigned interger|1|A type of message.|
|session_key|32bit unsigned interger|4|A session key which is generated when authentication.|

### Reply Header

The size is 2 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|message_type|8bit unsigned interger|1|A type of message.|
|error_code|8bit unsigned interger|1|An error code.|

## Messages

### Authentication Request

#### Parameters

The size is 26 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|version|16bit unsigned interger|2|An API version number the client requires.|
|player_name_t|24 byte length UTF-8 string|24|A name of player. This must not be empty.|

#### Reply

The size is 8 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|version|16bit unsigned interger|2|An API version number of the server.|
|session_key|32bit unsigned interger|4|A generated session key.|
|player_tag|16bit unsigned interger|2|A tag number of player to avoid duplication of player name.|

#### Error Codes

|Name|Condition|Continuable|
|:---|:---|:---|
|ok|The request is processed succesfully.|yes|
|api_version_mismatch|An API version of server is different from what the client required.|no|
|request_parameter_wrong|A player name is empty.|no|
|DISCONNECT|Already authenticated.|no|

### List Room Group Request

#### Parameters

The size is 1 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|dummy|8bit unsigned interger|1|A dummy value which is not used.|

#### Reply

The size is 245 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|room_group_count|8bit unsigned interger|1|The number of room group.|
|max_room_count_per_room_group|32bit unsigned interger|4|A limit of room count per one room group.|
|room_group_info_list|A 10 elements array of room_group_info|240|A list of room group information.|

room_group_info is 24 bytes data as below:

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|name|24 byte length UTF-8 string|24|A name of room group.|

### Error Codes

|Name|Condition|Continuable|
|:---|:---|:---|
|ok|The request is processed succesfully.|yes|

## Create Room Request

### Parameters

The size is 20 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|group_index|8bit unsigned interger|1|An index of group where you want to create room.|
|password|16 byte length UTF-8 string|16|A password of room you create. If this is empty, the room is created as a public room.|
|max_player_count|8bit unsigned interger|1|A limit of player count in the room.|
|port_number|16bit unsigned interger|2|A port number which is used for game host. 49513 to 65535 is available.|

### Reply

The size is 4 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|room_id|32bit unsigned interger|4|An id of the room created.|

### Error Codes

|Name|Condition|Continuable|
|:---|:---|:---|
|ok|The request is processed succesfully.|yes|
|client_already_hosting_room|Failed to host new room because the client already hosting room.|yes|
|room_group_not_found|Indicated room group doesn't exist.|yes|
|room_group_full|Indicated room group is full.|yes|
|request_parameter_wrong|Max player count exceeds limit. Or indicated port number is invalid.|yes|

## Force Disconnect Conditions

The server forces to close connection immediately without any reply if wrong operation about authentication is occured.

- Send not authentication request at first time after connection
- Send wrong session key
- Send ahtnentication request more than twice
