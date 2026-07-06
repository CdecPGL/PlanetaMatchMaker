# TLS Certificate Setup

PMMS does not issue TLS certificates and does not run an ACME client. When TLS is enabled, PMMS only reads an existing certificate chain file and private key file, then accepts TLS connections with those files.

Use this document to decide where certificate files should live, how PMMS should read them, and how renewal should notify PMMS.

## Server Settings

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
- `tls.reload_on_sighup`: Reload the certificate and private key for new TLS connections when PMMS receives SIGHUP. This is supported on Linux and Unix-like platforms.

If `tls.certificate_path` or `tls.private_key_path` is omitted from the JSON setting file, PMMS uses the same directory as the loaded `setting.json`. With the standard setting path, the defaults are `/etc/pmms/server.crt` and `/etc/pmms/server.key` on Linux, or `C:\pmms\server.crt` and `C:\pmms\server.key` on Windows.

Do not commit certificate files, private keys, service account key files, or local test certificates to this repository.

## Production Certificates

For public servers, use a certificate from a trusted certificate authority. Let's Encrypt is a common choice, and its documentation recommends using an ACME client when your hosting provider does not manage certificates for you. Certbot is the recommended client for most users.

PMMS should read the certificate files produced by Certbot. Prefer referencing or mounting Certbot's managed `live` paths instead of copying certificates into the application directory:

```json
{
  "tls": {
    "mode": "tls",
    "certificate_path": "/etc/letsencrypt/live/match.example.com/fullchain.pem",
    "private_key_path": "/etc/letsencrypt/live/match.example.com/privkey.pem"
  }
}
```

### DNS-01

DNS-01 is the recommended validation method for PMMS deployments. It does not require opening inbound port 80, works for wildcard certificates, and fits Docker or VM deployments where the TLS certificate is managed separately from the PMMS process.

With the Certbot Google DNS plugin, DNS-01 validation can create and remove the required TXT records through Google Cloud DNS:

```bash
sudo certbot certonly \
  --dns-google \
  --dns-google-propagation-seconds 120 \
  -d match.example.com
```

When Certbot runs on Google Cloud, prefer Application Default Credentials through the workload's service account. If Certbot runs outside Google Cloud, provide a service account credentials file with `--dns-google-credentials` and protect that file as a secret.

### HTTP-01

HTTP-01 can be used only when opening inbound port 80 is acceptable. It is simpler for some single-server deployments, but it is not the recommended default for PMMS.

If PMMS runs on a dedicated server and no web server is already using port 80, one Certbot command is:

```bash
sudo certbot certonly --standalone -d match.example.com
```

The standalone authenticator obtains a certificate without integrating with an existing web server, but it needs to bind to port 80 for HTTP-01 validation. The domain must resolve to the server, and inbound connections to port 80 must be possible.

## Renewal and Reload

Let Certbot manage certificate renewal. PMMS reads the certificate and key at startup. It can also reload them for new TLS connections on Linux and Unix-like platforms when `tls.reload_on_sighup` is enabled:

```json
{
  "tls": {
    "mode": "tls",
    "certificate_path": "/etc/letsencrypt/live/match.example.com/fullchain.pem",
    "private_key_path": "/etc/letsencrypt/live/match.example.com/privkey.pem",
    "reload_on_sighup": true
  }
}
```

For a service manager deployment, use a Certbot deploy hook to reload or restart PMMS after a renewed certificate is installed:

```bash
sudo certbot renew --deploy-hook "systemctl reload pmms"
```

If your service unit does not support reload, restart the service instead:

```bash
sudo certbot renew --deploy-hook "systemctl restart pmms"
```

For Docker deployments with `PMMS_TLS_RELOAD_ON_SIGHUP=true`, send SIGHUP to the PMMS container:

```bash
sudo certbot renew --deploy-hook "docker kill -s HUP pmms"
```

Existing TLS connections continue using the certificate context they were created with. New TLS handshakes use the reloaded certificate after the reload succeeds. If reload fails, PMMS keeps the previous certificate context and logs the error.

## Docker

Keep Certbot outside the PMMS container. Store certificates on the host, then mount them into the PMMS container read-only.

When running with Docker and Certbot, mount `/etc/letsencrypt` read-only and override the TLS paths. This keeps Certbot's `live` and `archive` paths available inside the container:

```bash
docker run -d --name pmms \
  -p 57000:57000 \
  -e PMMS_TLS_MODE=tls \
  -e PMMS_TLS_CERTIFICATE_PATH=/etc/letsencrypt/live/match.example.com/fullchain.pem \
  -e PMMS_TLS_PRIVATE_KEY_PATH=/etc/letsencrypt/live/match.example.com/privkey.pem \
  -e PMMS_TLS_RELOAD_ON_SIGHUP=true \
  -v /etc/letsencrypt:/etc/letsencrypt:ro \
  cdec/planeta-match-maker-server:latest
```

If you want to use the default paths in `setting.json`, mount certificate files to `/etc/pmms/server.crt` and `/etc/pmms/server.key`:

```bash
docker run -d --name pmms \
  -p 57000:57000 \
  -e PMMS_TLS_MODE=tls \
  -e PMMS_TLS_RELOAD_ON_SIGHUP=true \
  -v /path/to/fullchain.pem:/etc/pmms/server.crt:ro \
  -v /path/to/privkey.pem:/etc/pmms/server.key:ro \
  cdec/planeta-match-maker-server:latest
```

Use the renewal hook from [Renewal and Reload](#renewal-and-reload) after Certbot is configured.

## Google Compute Engine VM

The Docker certificate model also works on Google Cloud Compute Engine when you deploy the PMMS server container on a VM. Use DNS-01 validation with Google Cloud DNS so the VM does not need inbound `tcp:80` for certificate issuance or renewal.

Use this deployment shape:

1. Reserve a static external IP address for the VM.
1. Manage the domain's authoritative DNS zone with Google Cloud DNS.
1. Point the DNS `A` record, for example `match.example.com`, to that static IP address.
1. Allow inbound `tcp:57000` or your configured PMMS server port.
1. Assign the VM a service account that Certbot can use to update the Cloud DNS managed zone.
1. Obtain the certificate on the VM host with the Certbot Google DNS plugin.
1. Run the PMMS container with `/etc/letsencrypt` mounted read-only.
1. Enable SIGHUP reload and use the Docker renewal hook from [Renewal and Reload](#renewal-and-reload).

Grant only the permissions required to edit the target Cloud DNS zone. Avoid project-wide `dns.admin` in production unless the project contains only this DNS zone. For least privilege, use a custom role with the Cloud DNS record change permissions required by the Certbot Google DNS plugin.

On a Debian or Ubuntu VM with Docker installed, install Certbot and the Google DNS plugin on the host:

```bash
sudo apt-get update
sudo apt-get install certbot python3-certbot-dns-google
```

Then obtain the certificate with DNS-01 validation:

```bash
sudo certbot certonly \
  --dns-google \
  --dns-google-project PROJECT_ID \
  --dns-google-propagation-seconds 120 \
  -d match.example.com
```

Run the PMMS container with the Certbot live paths:

```bash
sudo docker run -d --name pmms \
  -p 57000:57000 \
  -e PMMS_TLS_MODE=tls \
  -e PMMS_TLS_CERTIFICATE_PATH=/etc/letsencrypt/live/match.example.com/fullchain.pem \
  -e PMMS_TLS_PRIVATE_KEY_PATH=/etc/letsencrypt/live/match.example.com/privkey.pem \
  -e PMMS_TLS_RELOAD_ON_SIGHUP=true \
  -v /etc/letsencrypt:/etc/letsencrypt:ro \
  cdec/planeta-match-maker-server:latest
```

If you use Container-Optimized OS, do not rely on installing Certbot with a host package manager. Run Certbot as a separate container or use another certificate management process, persist `/etc/letsencrypt` on a mounted disk, and mount the same directory into the PMMS container read-only.

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
- [Certbot Google DNS plugin](https://certbot-dns-google.readthedocs.io/en/stable/)
- [OpenSSL req command](https://docs.openssl.org/3.0/man1/openssl-req/)
- [Compute Engine container deployment](https://docs.cloud.google.com/compute/docs/containers/deploying-containers)
- [Compute Engine static external IP addresses](https://docs.cloud.google.com/compute/docs/ip-addresses/configure-static-external-ip-address)
- [Google Cloud VPC firewall rules](https://docs.cloud.google.com/firewall/docs/using-firewalls)
