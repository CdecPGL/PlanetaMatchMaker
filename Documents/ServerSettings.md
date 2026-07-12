# Server Settings

There are two ways to change server setting.

1. Locating server setting file
1. Setting environment variables

Server setting is located in `/etc/pmms/setting.json` (Linux) or `C:\pmms\setting.json` (Windows).
Setting is written by JSON format which can include comments and trailing cammma.
The encoding of the setting file must be UTF-8 without BOM.

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

### `authentication` Section

|Name|Type|Default|Env Var|Explanation|
|:---|:---|---:|:---|:---|
|game_id|string (the length is less than 24)|""|PMMS_AUTHENTICATION_GAME_ID|A game id to accept.|
|enable_game_version_check|boolean|false|PMMS_AUTHENTICATION_ENABLE_GAME_VERSION_CHECK|Wheather game version check is enabled.|
|game_version|string (the length is less than 24)|""|PMMS_AUTHENTICATION_GAME_VERSION|A game version to accept. This setting is reffered only if enable_game_version_check is true.|
|max_credential_bytes|integer (1-15728640)|16384|PMMS_AUTHENTICATION_MAX_CREDENTIAL_BYTES|Maximum Steam ticket or OIDC token size accepted during authentication. This setting is limited by the authentication chunk sequence field.|
|timeout_seconds|integer (1-3600)|5|PMMS_AUTHENTICATION_TIMEOUT_SECONDS|Timeout for external authentication service requests.|
|clock_skew_seconds|integer (0-3600)|60|PMMS_AUTHENTICATION_CLOCK_SKEW_SECONDS|Allowed OIDC clock skew for time claim verification.|
|allow_plain_connections|boolean|false|PMMS_AUTHENTICATION_ALLOW_PLAIN_CONNECTIONS|Allow authentication over plain TCP. Keep this false in production.|
|steam.enabled|boolean|false|PMMS_AUTHENTICATION_STEAM_ENABLED|Enable Steam authentication.|
|steam.app_id|integer|0|PMMS_AUTHENTICATION_STEAM_APP_ID|Steam AppID checked by `AuthenticateUserTicket` and `CheckAppOwnership`. Required when Steam authentication is enabled.|
|steam.publisher_key|string|""|PMMS_AUTHENTICATION_STEAM_PUBLISHER_KEY|Steam Web API publisher key. This is a server secret and is never sent to clients or logs.|
|steam.identity|string|""|PMMS_AUTHENTICATION_STEAM_IDENTITY|Optional Steam ticket identity passed to `AuthenticateUserTicket`.|
|steam.authenticate_user_ticket_url|string|Steam Web API URL|PMMS_AUTHENTICATION_STEAM_AUTHENTICATE_USER_TICKET_URL|Override URL for Steam ticket verification, mainly for tests.|
|steam.check_app_ownership_url|string|Steam Web API URL|PMMS_AUTHENTICATION_STEAM_CHECK_APP_OWNERSHIP_URL|Override URL for Steam ownership verification, mainly for tests.|
|oidc.enabled|boolean|false|PMMS_AUTHENTICATION_OIDC_ENABLED|Enable OIDC JWT authentication.|
|oidc.issuer|string|""|PMMS_AUTHENTICATION_OIDC_ISSUER|Accepted OIDC issuer. Required when OIDC authentication is enabled.|
|oidc.audience|string|""|PMMS_AUTHENTICATION_OIDC_AUDIENCE|Accepted OIDC audience. Required when OIDC authentication is enabled.|
|oidc.algorithms|string array|["RS256","RS384","RS512"]|PMMS_AUTHENTICATION_OIDC_ALGORITHMS|Allowed JWT signature algorithms. Environment value is a comma-separated list.|
|oidc.discovery_url|string|""|PMMS_AUTHENTICATION_OIDC_DISCOVERY_URL|OIDC discovery document URL. Used to discover `jwks_uri` if `jwks_url` and `jwks` are not set.|
|oidc.jwks_url|string|""|PMMS_AUTHENTICATION_OIDC_JWKS_URL|JWKS URL.|
|oidc.jwks|string|""|PMMS_AUTHENTICATION_OIDC_JWKS|Inline JWKS JSON. This is useful for tests or static deployments.|
|oidc.jwks_cache_seconds|integer|3600|PMMS_AUTHENTICATION_OIDC_JWKS_CACHE_SECONDS|JWKS cache lifetime. Set 0 to disable caching.|

At least one authentication method must be enabled for clients to authenticate.
Steam authentication verifies a client-provided Steam auth ticket on the server and checks AppID ownership. Do not put the Steam publisher key in a client build.
OIDC authentication verifies JWT signature, issuer, audience, expiration, not-before, subject, and allowed algorithm by using `jwt-cpp`. PMMS does not provide OIDC login UI, PKCE, refresh token handling, or token acquisition; the game or another client-side auth library must obtain the token.

Authentication credentials should be sent over TLS in production. If `tls.mode` is `"plain"` and authentication is enabled, `allow_plain_connections` must be explicitly true or authentication fails.

Steam room external IDs are derived from the verified SteamID64 as big-endian 8 bytes followed by zero padding. OIDC room external IDs are derived from the verified `sub` claim only when its UTF-8 representation fits in 64 bytes.

### `log` Section

|Name|Type|Default|Env Var|Explanation|
|:---|:---|---:|:---|:---|
|enable_console_log|boolean|true|PMMS_LOG_ENABLE_CONSOLE_LOG|Wheather log is outputed to console. It is recommended to set true when you use docker in order to use logging system of docker.|
|console_log_level|string ("debug", "info", "warning", "error", "fatal")|"info"|PMMS_LOG_CONSOLE_LOG_LEVEL|A threshold of console log by level.|
|enable_file_log|boolean|true|PMMS_LOG_ENABLE_FILE_LOG|Wheather log is outputed to file.|
|file_log_level|string ("debug", "info", "warning", "error", "fatal")|"info"|PMMS_LOG_FILE_LOG_LEVEL|A threshold of file log by level.|
|file_log_path|string (path)|""|PMMS_LOG_FILE_LOG_PATH|A path of file to ouput log. `/var/log/pmms.log` (Linux) or `C:\log\pmms.log` (Windows) are used if this setting is empty.|

Log lines include the current thread ID as `[thread:<id>]`. Logs emitted while processing an accepted connection also include `[session:<number>]` before the client endpoint. Session numbers are process-local connection numbers that start from `1`.

### `connection_test` Section

|Name|Type|Default|Env Var|Explanation|
|:---|:---|---:|:---|:---|
|connection_check_tcp_time_out_seconds|integer (1-3600)|5|PMMS_CONNECTION_TEST_CONNECTION_CHECK_TCP_TIME_OUT_SECONDS|Timeout seconds in TCP connection test request.|
|connection_check_udp_time_out_seconds|integer (1-3600)|3|PMMS_CONNECTION_TEST_CONNECTION_CHECK_UDP_TIME_OUT_SECONDS|Timeout seconds in UDP connection test request.|
|connection_check_udp_try_count|integer (1-100)|3|PMMS_CONNECTION_TEST_CONNECTION_CHECK_UDP_TRY_COUNT|Connection test try count in UDP.|

### `tls` Section

|Name|Type|Default|Env Var|Explanation|
|:---|:---|---:|:---|:---|
|mode|string ("plain", "tls")|"tls"|PMMS_TLS_MODE|Connection security mode. Use "plain" only for backward compatibility or local development.|
|certificate_path|string (path)|`setting.json` directory + `server.crt`|PMMS_TLS_CERTIFICATE_PATH|TLS server certificate chain path. Required when `mode` is "tls".|
|private_key_path|string (path)|`setting.json` directory + `server.key`|PMMS_TLS_PRIVATE_KEY_PATH|TLS server private key path. Required when `mode` is "tls".|
|reload_on_sighup|boolean|false|PMMS_TLS_RELOAD_ON_SIGHUP|Reload TLS certificate and private key when the server receives SIGHUP. This is supported on Linux and Unix-like platforms.|

When `certificate_path` or `private_key_path` is omitted from the JSON setting file, the server uses files in the same directory as the loaded `setting.json`. For the standard setting paths, the defaults are `/etc/pmms/server.crt` and `/etc/pmms/server.key` on Linux, or `C:\pmms\server.crt` and `C:\pmms\server.key` on Windows. Environment variables still override these values.

See [TLS Certificate Setup](TLSCertificate.md) for production certificate paths, Certbot examples, Docker mounts, renewal, and development self-signed certificates.

`external_tls_termination` is reserved for future design but is not supported now. The server also does not support PROXY protocol, so it does not accept PROXY protocol headers and cannot restore the original client IP from a TLS terminator.

In Builtin mode, PMMS uses the accepted TCP connection source IP to create the game host endpoint. If TLS is terminated by an external proxy without preserving the original source IP, Builtin mode can return the proxy IP instead of the host client IP.
