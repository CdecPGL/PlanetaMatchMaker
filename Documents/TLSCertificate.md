# TLS Certificate Setup

PMMS does not issue TLS certificates and does not run an ACME client. When TLS is enabled, PMMS only reads an existing certificate chain file and private key file at startup, then accepts TLS connections with those files.

Use this model for production:

```json
{
  "tls": {
    "mode": "tls",
    "certificate_path": "/etc/letsencrypt/live/match.example.com/fullchain.pem",
    "private_key_path": "/etc/letsencrypt/live/match.example.com/privkey.pem"
  }
}
```

- `tls.mode`: Use `"tls"` for encrypted transport. This is the default.
- `tls.certificate_path`: Path to a PEM certificate chain file. For Certbot, use `fullchain.pem`.
- `tls.private_key_path`: Path to a PEM private key file. For Certbot, use `privkey.pem`.

Do not commit certificate files, private keys, or local test certificates to this repository.

## Production Certificates

For public servers, use a certificate from a trusted certificate authority. Let's Encrypt is a common choice, and its documentation recommends using an ACME client when your hosting provider does not manage certificates for you. Certbot is the recommended client for most users.

If PMMS runs on a dedicated server and no web server is already using port 80, one simple Certbot command is:

```bash
sudo certbot certonly --standalone -d match.example.com
```

The standalone authenticator obtains a certificate without integrating with an existing web server, but it needs to bind to port 80 for HTTP-01 validation. The domain must resolve to the server, and inbound connections to the validation port must be possible.

After issuance, point PMMS directly at Certbot's live paths:

```json
{
  "tls": {
    "mode": "tls",
    "certificate_path": "/etc/letsencrypt/live/match.example.com/fullchain.pem",
    "private_key_path": "/etc/letsencrypt/live/match.example.com/privkey.pem"
  }
}
```

Do not copy these files into the repository or into a writable application directory unless your deployment process requires it. Prefer mounting or referencing the managed certificate paths.

## Docker

The default server setting uses these paths:

```json
{
  "tls": {
    "mode": "tls",
    "certificate_path": "/etc/pmms/tls/server.crt",
    "private_key_path": "/etc/pmms/tls/server.key"
  }
}
```

When running with Docker and Certbot, mount `/etc/letsencrypt` read-only and override the environment variables. This keeps Certbot's `live` and `archive` paths available inside the container:

```bash
docker run \
  -p 57000:57000 \
  -e PMMS_TLS_MODE=tls \
  -e PMMS_TLS_CERTIFICATE_PATH=/etc/letsencrypt/live/match.example.com/fullchain.pem \
  -e PMMS_TLS_PRIVATE_KEY_PATH=/etc/letsencrypt/live/match.example.com/privkey.pem \
  -v /etc/letsencrypt:/etc/letsencrypt:ro \
  cdec/planeta-match-maker-server:latest
```

If you want to use the default paths in `setting.json`, mount certificate files to `/etc/pmms/tls/server.crt` and `/etc/pmms/tls/server.key`:

```bash
docker run \
  -p 57000:57000 \
  -v /path/to/fullchain.pem:/etc/pmms/tls/server.crt:ro \
  -v /path/to/privkey.pem:/etc/pmms/tls/server.key:ro \
  cdec/planeta-match-maker-server:latest
```

## Renewal

Let Certbot manage certificate renewal. PMMS currently reads the certificate and key at startup; it does not reload them automatically when the files change.

Use a Certbot deploy hook to restart PMMS after a renewed certificate is installed:

```bash
sudo certbot renew --deploy-hook "systemctl restart pmms"
```

For Docker deployments, restart the container or service from the deploy hook, for example with your systemd unit, Docker Compose service, or container orchestration platform.

## Development Certificates

For local development, you can generate a self-signed certificate with OpenSSL:

```bash
mkdir -p certs
openssl req -x509 -newkey rsa:2048 -nodes -days 365 \
  -keyout certs/localhost.key \
  -out certs/localhost.crt \
  -subj "/CN=localhost" \
  -addext "subjectAltName=DNS:localhost,IP:127.0.0.1"
```

Use it only for local testing:

```json
{
  "tls": {
    "mode": "tls",
    "certificate_path": "./certs/localhost.crt",
    "private_key_path": "./certs/localhost.key"
  }
}
```

Client certificate validation should remain enabled in production. For development only, the test client provides `--accept_invalid_tls_certificate`, and the Unity component has a development-only option to accept invalid TLS certificates. Do not enable those settings for public servers.

## Plain Mode

Use `"plain"` only for backward compatibility, LAN-only checks, or local debugging:

```json
{
  "tls": {
    "mode": "plain"
  }
}
```

Plain mode sends authentication messages, room passwords, player names, and endpoint data without transport encryption.

## External TLS Termination

`external_tls_termination` is reserved for future design but is not implemented. PMMS also does not support PROXY protocol and will not accept PROXY protocol headers.

If a TLS terminator or proxy sits in front of PMMS, the server may see the proxy IP as the TCP source address. Builtin mode uses the accepted TCP source IP to build the game host endpoint, so external TLS termination can break Builtin mode unless the original client IP is preserved by some future supported mechanism.

## References

- [Let's Encrypt Getting Started](https://letsencrypt.org/getting-started/)
- [Certbot User Guide](https://eff-certbot.readthedocs.io/en/stable/using.html)
- [OpenSSL req command](https://docs.openssl.org/3.0/man1/openssl-req/)
