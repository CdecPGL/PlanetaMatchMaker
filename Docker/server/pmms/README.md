# Server configuration

The image includes `/etc/pmms/setting.json` with Steam authentication selected.

The bundled setting intentionally omits deployment-specific values and the server refuses to start until they are
provided. Set `PMMS_AUTHENTICATION_GAME_ID`, `PMMS_AUTHENTICATION_STEAM_APP_ID`, and
`PMMS_AUTHENTICATION_STEAM_PUBLISHER_KEY` through `docker run --env-file`, or mount a complete replacement setting
file. The repository provides `Docker/server/pmms/pmms.env.example` as a template. If game version checking is enabled,
also set `PMMS_AUTHENTICATION_GAME_VERSION`.

Mount the TLS certificate chain and private key at `/etc/pmms/server.crt` and `/etc/pmms/server.key`, or override the
paths through environment variables.
