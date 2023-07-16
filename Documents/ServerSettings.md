# Server Settings

There are two ways to change server setting.

1. Locating server setting file
1. Setting environment variables

Server setting is located in `/etc/pmms/setting.json` (Linux) or `C:\pmms\setting.json` (Windows).
Setting is written by JSON format which can include comments and trailing cammma.

If specific environmet variables are defined, the value of environment variables are used even if setting file is located.

Those settings are loaded when server starts.

## Items

### `common` Section

|Name|Type|Default|Env Var|Explanation|
|:---|:---|---:|:---|:---|
|time_out_seconds|integer (1-3600)|300|PMMS_COMMON_TIME_OUT_SECONDS|Timeout seconds to send or receive message.|
|ip_version|string ("v4", "v6")|"v4"|PMMS_COMMON_IP_VERSION|IP version to use. ("v4" or "v6")|
|port|integer (0-65535)|57000|PMMS_COMMON_PORT|Port number to accept.|
|max_connection_per_thread|integer (1-65535)|1000|PMMS_COMMON_MAX_CONNECTION_PER_THREAD|A limit of connection count in each thread.|
|thread|integer (1-65535)|1|PMMS_COMMON_MAX_THREAD|A number of thread to run.|
|max_room_count|integer (1-65535)|1000|PMMS_COMMON_MAX_ROOM_COUNT|A limit of room count.|
|max_player_per_room|integer (1-255)|16|PMMS_COMMON_MAX_PLAYER_PER_ROOM|A limit of player count in each room.|

### `log` Section

|Name|Type|Default|Env Var|Explanation|
|:---|:---|---:|:---|:---|
|enable_console_log|boolean|true|PMMS_LOG_ENABLE_CONSOLE_LOG|Wheather log is outputed to console. It is recommended to set true when you use docker in order to use logging system of docker.|
|console_log_level|string ("debug", "info", "warning", "error", "fatal")|"info"|PMMS_LOG_CONSOLE_LOG_LEVEL|A threshold of console log by level.|
|enable_file_log|boolean|true|PMMS_LOG_ENABLE_FILE_LOG|Wheather log is outputed to file.|
|file_log_level|string ("debug", "info", "warning", "error", "fatal")|"info"|PMMS_LOG_FILE_LOG_LEVEL|A threshold of file log by level.|
|file_log_path|string (path)|""|PMMS_LOG_FILE_LOG_PATH|A path of file to ouput log. `/var/log/pmms.log` (Linux) or `C:\log\pmms.log` (Windows) are used if this setting is empty.|

### `connection_test` Section

|Name|Type|Default|Env Var|Explanation|
|:---|:---|---:|:---|:---|
|connection_check_tcp_time_out_seconds|integer (1-3600)|5|PMMS_CONNECTION_TEST_CONNECTION_CHECK_TCP_TIME_OUT_SECONDS|Timeout seconds in TCP connection test request.|
|connection_check_udp_time_out_seconds|integer (1-3600)|3|PMMS_CONNECTION_TEST_CONNECTION_CHECK_UDP_TIME_OUT_SECONDS|Timeout seconds in UDP connection test request.|
|connection_check_udp_try_count|integer (1-100)|3|PMMS_CONNECTION_TEST_CONNECTION_CHECK_UDP_TRY_COUNT|Connection test try count in UDP.|
