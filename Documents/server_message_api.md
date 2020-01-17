# Server Message API

Message is fixed size data to communicate between server and client.

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
|player_name_t|24 byte length UTF-8 string|24|A name of player.|

#### Reply

The size is 8 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|version|16bit unsigned interger|2|An API version number of the server.|
|session_key|32bit unsigned interger|4|A generated session key.|
|player_tag|16bit unsigned interger|2|A tag number of player to avoid duplication of player name.|

#### Error Code

|Name|Condition|Continuable|
|:---|:---|:---|
|ok|The request is processed succesfully.|yes|
|api_version_mismatch|An API version of server is different from what the client required.|no|
|request_parameter_wrong|A player name is empty.|no|
|DISCONNECT|Already authenticated.|no|

## Create Room Request

### Parameters

- 

### Return

### Error Codes

- ok: Succeed
- already_hosting_room: The client is already hosting room

## Force Disconnect Conditions

The server forces to close connection immediately without any reply if wrong operation about authentication is occured.

- Send not authentication request at first time after connection
- Send wrong session key
- Send ahtnentication request more than twice
