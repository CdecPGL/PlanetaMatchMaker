# Server configuration

The production image intentionally does not include `setting.json`.
Mount a complete configuration at `/etc/pmms/setting.json` when starting the container.
Production configurations must select `steam` as `authentication.method`.
