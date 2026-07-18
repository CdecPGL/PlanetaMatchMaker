# Server configuration

The image includes `/etc/pmms/setting.json` with Steam authentication selected.

The bundled values are a fail-closed example. Before production use, override at least the game ID, game version,
Steam AppID, and Steam Publisher Key through the documented `PMMS_AUTHENTICATION_*` environment variables, or
mount a complete replacement setting file. The bundled Publisher Key value is an invalid placeholder and must never
be used as a real credential.

Mount the TLS certificate chain and private key at `/etc/pmms/server.crt` and `/etc/pmms/server.key`, or override the
paths through environment variables.
