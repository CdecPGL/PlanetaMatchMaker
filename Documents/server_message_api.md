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
|message_type|8bit unsigned integer|1|A type of message.|
|session_key|32bit unsigned integer|4|A session key which is generated when authentication.|

### Reply Header

The size is 2 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|message_type|8bit unsigned integer|1|A type of message.|
|error_code|8bit unsigned integer|1|An error code.|

## Messages

### Authentication Request

#### Parameters

The size is 26 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|version|16bit unsigned integer|2|An API version number the client requires.|
|player_name_t|24 byte length UTF-8 string|24|A name of player. This must not be empty.|

#### Reply

The size is 8 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|version|16bit unsigned integer|2|An API version number of the server.|
|session_key|32bit unsigned integer|4|A generated session key.|
|player_tag|16bit unsigned integer|2|A tag number of player to avoid duplication of player name.|

#### Error Codes

|Name|Condition|Continuable|
|:---|:---|:---|
|ok|The request is processed succesfully.|yes|
|api_version_mismatch|An API version of server is different from what the client required.|no|
|request_parameter_wrong|A player name is empty.|no|
|DISCONNECT|Authentication request is duplicate.|no|

### List Room Group Request

#### Parameters

The size is 1 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|dummy|8bit unsigned integer|1|A dummy value which is not used.|

#### Reply

The size is 245 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|room_group_count|8bit unsigned integer|1|The number of room group.|
|max_room_count_per_room_group|32bit unsigned integer|4|A limit of room count per one room group.|
|room_group_info_list|A 10 elements array of room_group_info|240|A list of room group information.|

`room_group_info` is 24 bytes data as below:

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
|group_index|8bit unsigned integer|1|An index of group where you want to create room.|
|password|16 byte length UTF-8 string|16|A password of room you create. If this is empty, the room is created as a public room.|
|max_player_count|8bit unsigned integer|1|A limit of player count in the room. This must not exceeds the limit which is defined in server setting.|
|port_number|16bit unsigned integer|2|A port number which is used for game host. 49513 to 65535 is available.|

### Reply

The size is 4 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|room_id|32bit unsigned integer|4|An id of the room created.|

### Error Codes

|Name|Condition|Continuable|
|:---|:---|:---|
|ok|The request is processed succesfully.|yes|
|client_already_hosting_room|Failed to host new room because the client already hosting room.|yes|
|room_group_not_found|Indicated room group doesn't exist.|yes|
|room_group_full|Indicated room group is full.|yes|
|request_parameter_wrong|Max player count exceeds limit. Or indicated port number is invalid.|yes|

## List Room Request

### Parameters

The size is 31 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|group_index|8bit unsigned integer|1|An index of group where you want to list room.|
|start_index|8bit unsigned integer|1|A start index of room data which will be replied from search results.|
|count|8bit unsigned integer|1|The number of room data which will be replied from search results.|
|sort_kind|8bit unsigned integer|1|A sort kind of result.|
|search_target_flags|8bit unsigned integer|1|A flags to indicate search target.|
|search_full_name|player_full_name|26|A query to search room by the room's host player name.|

Options of `sort_kind` are as below:

|Name|Value|
|:---|---:|
|name_ascending|0|
|name_descending|1|
|create_datetime_ascending|2|
|create_datetime_descending|3|

`search_target_flags` are treated as bit flags.
Options are as below:

|Name|Value|
|:---|---:|
|public_room|1|
|private_room|2|
|open_room|4|
|closed_room|8|

`player_full_name` is 26 bytes data as below:

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|name|24 byte length UTF-8 string|24|A name of player.|
|tag|16bit unsigned integer|2|A tag of player to avoid name duplication.|

### Reply

The size is 249 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|total_room_count|8bit unsigned integer|1|The number of rooms existing in the room group in the server.|
|matched_room_count|8bit unsigned integer|1|The number of rooms which match to the query of the room.|
|reply_room_count|8bit unsigned integer|1|The number of rooms which is included in reply messages.|
|room_info_list|A 6 elements array of room_info|246|A result room info list.|

`room_info` is 41 bytes data as below:

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|room_id|32bit unsigned integer|4|An id of the room.|
|host_player_full_name|player_full_name|26|A name of player who is hosting the room.|
|setting_flags|8bit unsigned integer|1|A flags which indicate a setting of the room.|
|max_player_count|8bit unsigned integer|1|Player capability of this room.|
|create_datetime|8bit unsigned integer|1|The number of player which joins the room currently.|
|create_datetime|64bit unsigned integer which indicates unix time|8|A datetime the room created.|

`setting_flags` are treated as bit flags.
Options are as below:

|Name|Value|
|:---|---:|
|public_room|1|
|open_room|2|

Multiple reply messages are sent if there are more rooms than rooms one reply message can send.
You can obtain the number of reply message (separation) by below expression.

```cpp
separation = floor((reply.reply_room_count + 5) / 6);
```

### Error Codes

|Name|Condition|Continuable|
|:---|:---|:---|
|ok|The request is processed succesfully.|yes|
|room_group_not_found|Indicated room group doesn't exist.|yes|
|request_parameter_wrong|sort_kind is invalid.|yes|

## Join Room Request

### Parameters

The size is 21 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|group_index|8bit unsigned integer|1|An index of group where the room you want to join exists.|
|room_id|32bit unsigned integer|4|An id of the room you want to join.|
|password|16 byte length UTF-8 string|16|A password of the room you want to join. This is only refered when indicated room is private.|

### Reply

The size is 18 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|game_host_endpoint|endpoint|18|An endpoint of game host which is hosting the room you want to join.|

`game_host_endpoint` is 18 bytes data as below:

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|ip_address|A 16 elements array of 8bit unsigned integer|16|A IP address by big endian. In IPv4, IPv4-Mapped IPv6 is used. (example, `::ffff:192.0.0.1`)|
|port_number|16 bits unsigned integer|2|A port number.|

### Error Codes

|Name|Condition|Continuable|
|:---|:---|:---|
|ok|The request is processed succesfully.|yes|
|room_group_not_found|Indicated room group doesn't exist.|yes|
|room_not_found|Indicated room doesn't exist.|yes|
|room_permission_denied|Indicated room is closed.|yes|
|room_password_wrong|Indicated password is wrong.|yes|
|room_full|The number of player reaches limit.|yes|

## Force Disconnect Conditions

The server forces to close connection immediately without any reply if wrong operation about authentication is occured.

- Send not authentication request at first time after connection
- Send wrong session key
- Send ahtnentication request more than twice
