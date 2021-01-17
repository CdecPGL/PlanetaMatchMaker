# Server Settings

Server setting is located in `/etc/pmms/setting.json` (Linux) or `C:\pmms\setting.json` (Windows), and loaded when server starts.
Setting is written by JSON format.

## Items

|Name|Type|Default|Explanation|
|:---|:---|---:|:---|
|enable_session_key_check|boolean|true|Wheather session key check is enabled.|
|time_out_seconds|integer|300|Timeout seconds to send or receive message.|
|connection_check_time_out_seconds|integer|10|Timeout seconds in connection test request.|
|connection_check_udp_try_count|integer|3|Connection test try count in UDP.|
|ip_version|string|"v4"|IP version to use. ("v4" or "v6")|
|port|integer|57000|Port number to accept.|
|max_connection_per_thread|integer|1000|A limit of connection count in each thread.|
|thread|int|1|A number of thread to run.|
|max_room_per_room_group|integer|100|A limit of room count in each room group.|
|room_group_list|list of string|["default"]|A name list of room group.|
|max_player_per_room|integer|16|A limit of player count in each room|
|enable_console_log|boolean|true|Wheather log is outputed to console. It is recommended to set true when you use docker in order to use logging system of docker.|
|console_log_level|string|"info"|A threshold of console log by level. ("debug", "info", "warning", "error" or "fatal")|
|enable_file_log|boolean|true|Wheather log is outputed to file.|
|file_log_level|string|"info"|A threshold of file log by level. ("debug", "info", "warning", "error" or "fatal")|
|file_log_path|string|""|A path of file to ouput log. `/var/log/pmms.log` (Linux) or `C:\log\pmms.log` (Windows) are used if this setting is empty.|
