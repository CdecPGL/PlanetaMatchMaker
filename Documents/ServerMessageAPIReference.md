# Server Message API

This is reference about the message API of the server.

## Message

Messages use a fixed-size header and body. A message may also carry one variable-length binary attachment, encoded as a sequence of fixed-size attachment chunks.

There are three types of message.

- Request: A message from a client which requires one or more replies from the server
- Notice: A message from a client which doesn't require a reply from the server
- Reply: A message from a server to response to the request message from a client

## Communication Flow

To communicate with the server, you should follow below flow.

1. [Client] Connect to the server by TCP
1. [Client/Server] Complete TLS handshake if the server and client connection mode is TLS
1. [Client] Send authentication request
1. Message roop
    1. [Client] Send requests or notices
    1. [Server] Reply to the request with one or more messages
    1. [Client] Process reply

In below situation, the server forces to close the connection immediately without any reply.

- Send invalid message type
- Send not authentication request at first time after connection
- Server internal error occured

TLS is enabled by default in the official server and client settings. Plain TCP is available only when both sides explicitly use plain connection mode.

## Message Structure

All messages consist of a header and a fixed-size body. A message whose header declares a non-zero `attachment_size` is immediately followed by attachment chunks. Each combined header and body record and each fixed-size 243-byte attachment chunk are less than or equal to 256 bytes.

Attachment chunks are subordinate records of their parent message. They do not have a `message_type`, are not dispatched independently, and are only valid while the parent message is being handled.

There are two types of header.

- Request Header: A header for request message and notice message
- Reply Header: A header for reply message

If errors occured while handling messages, the server returns only a reply message header with an error code and an `attachment_size` of zero.

## Message Header Structure

### Request/Notice Header

The size is 5 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|message_type|8 bits unsigned integer|1|A type of message.|
|attachment_size|32 bits unsigned integer|4|Size of the attachment associated with this message. Zero means that the message has no attachment.|

Options of `message_type` are below.

|Name|Category|Value|
|:---|:---|---:|
|authentication|request|0|
|create_room|request|1|
|list_room|request|2|
|join_room|request|3|
|update_room_status|notice|4|
|connection_test|request|5|
|random_match|request|6|
|keep_alive|notice|7|

### Reply Header

The size is 6 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|message_type|8 bits unsigned integer|1|A type of message.|
|error_code|8 bits unsigned integer|1|An error code.|
|attachment_size|32 bits unsigned integer|4|Size of the attachment associated with this reply. Zero means that the reply has no attachment. Current reply messages do not define attachments.|

Options of `message_type` are same as one used in request header.

Options of `error_code` are as below.

|Name|Value|Explanation|
|:---|---:|:---|
|ok|0|Request is processed successfully.|
|server_error|1|Server internal error.|
|operation_invalid|2|The operation is invalid in current state.|
|request_parameter_wrong|3|Wrong parameters which must be rejected in the client is passed for request.|
|room_not_found|4|Indicated room is not found.|
|room_password_wrong|5|Indicated password of room is not correct.|
|room_full|6|The number of player reaches limit.|
|room_permission_denied|7|Request is rejected because indicated room is the room which you are not host of or closed.|
|room_count_exceeds_limit|8|The number of room reaches limit.|
|room_connection_establish_mode_mismatch|9|Connection establish mode of the room host does not match the mode expected by the client.|
|client_already_hosting_room|10|Request is failed because the client is already hosting room.|

## Message Attachment Structure

A message may define one binary attachment. Whether the attachment is forbidden, optional, or required, and its semantic format, are defined by the parent message type. The attachment is an opaque byte sequence at the transport layer; it does not contain a generic type identifier.

The protocol maximum attachment size is 15,728,640 bytes. A message or server setting may impose a lower limit. A sender must set `attachment_size` before the fixed-size body and then send exactly `ceil(attachment_size / 240)` chunks immediately after that body.

Each attachment chunk is 243 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|sequence|16 bits unsigned integer|2|Zero-based chunk sequence number.|
|data_size|8 bits unsigned integer|1|Attachment bytes contained in this chunk. The maximum is 240.|
|data|240 elements byte array|240|Attachment data. All unused tail bytes must be zero.|

The sequence must start at zero and increase by one. `data_size` must be 240 except for the final chunk, whose expected size is derived from `attachment_size`. A missing chunk, an unexpected sequence or size, or non-zero padding makes the attachment malformed. Attachment reception uses an absolute timeout for the complete attachment rather than resetting the timeout for each chunk.

All currently defined request and notice messages except authentication forbid attachments and require `attachment_size == 0`. An unexpected attachment causes the connection to be closed. Authentication attachment requirements depend on the selected method.

## Message Body Structure

### Authentication Request

A request to authenticate.

#### Parameters

The authentication request body size is 75 bytes. Its message attachment contains the Steam ticket. The attachment is required for Steam and must not exceed either `authentication.max_credential_bytes` or the protocol maximum. The `none` development method requires an empty attachment.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|api_version|16 bits unsigned integer|2|An API version number the client requires.|
|authentication_method|8 bits unsigned integer|1|Authentication method. `0` is the development-only unauthenticated method and `1` is Steam. Other values are unsupported.|
|game_id|24 byte length UTF-8 string|24|A game ID of the client.|
|game_version|24 byte length UTF-8 string|24|A game version number of the client.|
|player_name_t|24 byte length UTF-8 string|24|A name of player. This must not be empty.|

`authentication_method` options are as below.

|Name|Value|Attachment contents|
|:---|---:|:---|
|none|0|Empty. Accepted only when the server explicitly enables unauthenticated development connections.|
|steam|1|Steam auth ticket bytes.|

#### Reply

The size is 29 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|result|8 bits unsigned integer|1|A result of authentication.|
|api_version|16 bits unsigned integer|2|An API version number of the server.|
|game_version|24 byte length UTF-8 string|24|A game version the server accepts.|
|player_tag|16 bits unsigned integer|2|A tag number of player to avoid duplication of player name.|

Options of `result` are as below.

|Name|Value|Explanation|
|:---|---:|:---|
|success|0|Authentication is succeeded.|
|api_version_mismatch|1|An API version of server is different from what the client required.|
|game_id_mismatch|2|Client game id doesn't match to the acceptable value in the server.|
|game_version_mismatch|3|Client game version doesn't match to the version the server required.|
|unsupported_authentication_method|4|The requested authentication method does not match the server setting or is unknown.|
|authentication_data_format_invalid|5|A required credential attachment is empty or malformed.|
|authentication_data_size_exceeded|6|Credential size exceeds `authentication.max_credential_bytes`.|
|authentication_data_invalid|7|Credential is invalid.|
|insecure_connection|8|Authentication over plain TCP is not allowed by server setting.|
|steam_ticket_invalid|9|Steam ticket verification failed.|
|steam_id_mismatch|10|Reserved for SteamID mismatch handling. Current authentication requests do not send a client-claimed SteamID.|
|steam_ownership_check_failed|11|Steam AppID ownership check failed.|
|steam_authentication_service_unavailable|12|Steam authentication or ownership service could not be reached or returned an unavailable response.|

Note that authentication failure are not treated as error.
If authentication is failed, the server closes the connection immediately after reply.

#### Error Codes

|Name|Condition|Continuable|
|:---|:---|:---|
|ok|The request is processed succesfully.|yes|
|request_parameter_wrong|A player name is empty.|no|
|operation_invalid|Authentication request is send more than twice.|no|

### Create Room Request

A request to create room.

#### Parameters

The size is 84 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|password|16 byte length UTF-8 string|16|A password of room you create. If this is empty, the room is created as a public room.|
|max_player_count|8 bits unsigned integer|1|A limit of player count in the room. This must not exceeds the limit which is defined in server setting.|
|connection_establish_mode|8 bits unsigned integer|1|A way how to establish P2P connection.|
|port_number|16 bits unsigned integer|2|A port number which is used for game host. 49152 to 65535 is available. This is used when `connection_establish_mode` is `builtin`.|
|external_id|64 elements byte array.|64|An id where clients connect using an external service. All zero bytes mean unspecified. If the authenticated identity has a verified external ID, the server uses it when this field is all zero, and requires an exact match when this field is nonzero.|

Options of `connection_establish_mode` are as below.

|Name|Value|Host Identifier|Explanation|
|:---|---:|:---|:---|
|builtin|0|`port_number` property|Use builtin method.|
|steam|1|Authenticated Steam external ID|Use Steam relay service. Only Steam-authenticated sessions can create Steam rooms. The room external ID is the verified SteamID64 as big-endian 8 bytes followed by zero padding.|
|others|255|`external_id` property|Use other external service.|

When `connection_establish_mode` is `others`, a nonzero request `external_id` is required unless the authenticated identity already has a verified external ID that can be used automatically.

#### Reply

The size is 4 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|room_id|32 bits unsigned integer|4|An id of the room created.|

#### Error Codes

|Name|Condition|Continuable|
|:---|:---|:---|
|ok|The request is processed succesfully.|yes|
|client_already_hosting_room|Failed to host new room because the client already hosting room.|yes|
|room_count_exceeds_limit|The number of room exceeds limit.|yes|
|request_parameter_wrong|Max player count exceeds limit. Or indicated port number is invalid.|yes|

### List Room Request

A request to get room informations which matches to requested parameters.

#### Parameters

The size is 32 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|start_index|16 bits unsigned integer|2|A start index of room data which will be replied from search results.|
|count|16 bits unsigned integer|2|The number of room data which will be replied from search results.|
|sort_kind|8 bits unsigned integer|1|A sort kind of result.|
|search_target_flags|8 bits unsigned integer|1|A flags to indicate search target.|
|search_full_name|player_full_name|26|A query to search room by the room's host player name.|

Options of `sort_kind` are as below.

|Name|Value|
|:---|---:|
|name_ascending|0|
|name_descending|1|
|create_datetime_ascending|2|
|create_datetime_descending|3|

`search_target_flags` are treated as bit flags.
Options are as below.

|Name|Value|
|:---|---:|
|public_room|1|
|private_room|2|
|open_room|4|
|closed_room|8|

`player_full_name` is 26 bytes data as below.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|name|24 byte length UTF-8 string|24|A name of player.|
|tag|16 bits unsigned integer|2|A tag of player to avoid name duplication.|

#### Reply

The body size is 216 bytes. Including the 6-byte reply header, one reply record is 222 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|total_room_count|16 bits unsigned integer|2|The number of rooms existing in the room group in the server.|
|matched_room_count|16 bits unsigned integer|2|The number of rooms which match to the query of the room.|
|reply_room_count|16 bits unsigned integer|2|The number of rooms which is included in reply messages.|
|room_info_list|A 5 elements array of room_info|210|A result room info list.|

`room_info` is 42 bytes data as below.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|room_id|32 bits unsigned integer|4|An id of the room.|
|host_player_full_name|player_full_name|26|A name of player who is hosting the room.|
|setting_flags|8 bits unsigned integer|1|A flags which indicate a setting of the room.|
|max_player_count|8 bits unsigned integer|1|Player capability of this room.|
|current_player_count|8 bits unsigned integer|1|The number of player which joins the room currently.|
|create_datetime|64 bits unsigned integer which indicates unix time|8|A datetime the room created.|
|connection_establish_mode|8 bits unsigned integer|1|A way how to establish P2P connection in the room.|

`setting_flags` are treated as bit flags.
Options are as below.

|Name|Value|
|:---|---:|
|public_room|1|
|open_room|2|

Multiple reply messages are sent if there are more rooms than rooms one reply message can send.
You can obtain the number of reply message (separation) by below expression.

```cpp
separation = floor((reply.reply_room_count + 4) / 5);
```

#### Error Codes

|Name|Condition|Continuable|
|:---|:---|:---|
|ok|The request is processed succesfully.|yes|
|request_parameter_wrong|sort_kind is invalid.|yes|

### Join Room Request

A request to get the information to join the room.

#### Parameters

The size is 21 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|room_id|32 bits unsigned integer|4|An id of the room you want to join.|
|connection_establish_mode|8 bits unsigned integer|1|An expected way how to establish P2P connection in the room.|
|password|16 byte length UTF-8 string|16|A password of the room you want to join. This is only refered when indicated room is private.|

#### Reply

The size is 82 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|game_host_endpoint|endpoint|18|An endpoint of game host which is hosting the room you want to join.|
|game_host_external_id|64 elements byte array.|64|An id to connect to the host using external service like Steam Networking. This is left justified and big endien.|

`game_host_endpoint` is 18 bytes data as below.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|ip_address|A 16 elements array of 8 bits unsigned integer|16|A IP address by big endian. In IPv4, IPv4-Mapped IPv6 is used. (example, `::ffff:192.0.0.1`)|
|port_number|16 bits unsigned integer|2|A port number.|

#### Error Codes

|Name|Condition|Continuable|
|:---|:---|:---|
|ok|The request is processed succesfully.|yes|
|room_not_found|Indicated room doesn't exist.|yes|
|room_permission_denied|Indicated room is closed.|yes|
|room_password_wrong|Indicated password is wrong.|yes|
|room_full|The number of player reaches limit.|yes|
|room_connection_establish_mode_mismatch|Connection establish mode of the room host doesn't match expected one in the client.|yes|

### Update Room Status Notice

A notice to inform new status of the room the client hosts.

#### Parameters

The size is 7 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|room_id|32 bits unsigned integer|4|An id of the room you want to join.|
|status|8 bits unsigned integer|1|A new status of the room.|
|is_current_player_count_changed|boolean|1|A flag which indicates if playr count is updated.|
|current_player_count|8 bits unsigned integer|1|A new player count.|

Options of `status` are as below.

|Name|Value|
|:---|---:|
|open|0|
|close|1|
|remove|2|

#### Error Conditions

Notice message is ignoreed if there are some errors in processing message.

|Condition|Continuable|
|:---|:---|
|Indicated room doesn't exist.|yes|
|The host of indicated room is not you.|yes|
|The number of new player count is invalid.|yes|
|The `status` parameter is invalid.|yes|

### Connection Test Request

A request to check the client is reachable from the internet.

#### Parameters

The size is 3 bytes.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|protocol|8 bits unsigned integer|1|A transport prptocol to use for connection test.|
|port_number|16 bits unsigned integer|2|A port number to use for connection test. 49152 to 65535 is available.|

Options of `prptocol` are as below.

|Name|Value|
|:---|---:|
|TCP|0|
|UDP|1|

#### Reply

The size is 1 byte.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|succeed|boolean|1|Whether the connection test is succeeded.|

#### Error Codes

|Name|Condition|Continuable|
|:---|:---|:---|
|ok|The request is processed succesfully.|yes|
|request_parameter_wrong|Indicated protocol or port number is invalid.|yes|

### Random Match Request

Not implemented now.

### Keep Alive Notice

A notice to notice alival of the client.

#### Parameters

The size is 1 byte.

|Name|Type|Size|Explanation|
|:---|:---|---:|:---|
|dummy|8 bits unsigned integer|1|Dummy data.|

#### Error Conditions

The server does nothing for this notice so there are no errors.
