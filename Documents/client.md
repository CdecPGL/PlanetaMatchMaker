# Client

## Connection Security

`MatchMakerClient.ConnectAsync` uses TLS by default. To connect to a server with a certificate whose host name is different from the address you dial, pass `MatchMakerConnectionOptions.TlsTargetHost`.

```csharp
await client.ConnectAsync(
    "127.0.0.1",
    57000,
    "player",
    new MatchMakerConnectionOptions
    {
        Mode = MatchMakerConnectionMode.Tls,
        TlsTargetHost = "match.example.com"
    });
```

Use `MatchMakerConnectionMode.Plain` only for backward compatibility or local development against a server configured with `tls.mode` set to `"plain"`.

`RemoteCertificateValidationCallback` can be set for development with self-signed certificates. Do not disable certificate validation in production.

## Port Mapping Auto Release

In .Net Framework, port mappings created in the aplication are released automatically when the application exits.

But, in .Net Core, port mappings are not released due to [Open.NAT behavior](https://github.com/lontivero/Open.NAT/issues/94).
([This page](https://stackoverflow.com/questions/44732234/why-does-the-finalize-destructor-example-not-work-in-net-core) is also helphul)

Call NatPortMappingCreator.ReleaseCreatedPortMappings method manually to ensure to release port mappings created in the application which uses .Net Core.
(Even if you don'y release them manually, port mappings will be released affer 10 minuites if NAT device suports lifetime of port mapping.)
