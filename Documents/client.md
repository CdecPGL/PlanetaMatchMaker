# Client

## Connection Security

`MatchMakerClient.ConnectAsync` uses TLS by default. `MatchMakerAddress` is an immutable value object for host or IP address values and validates IPv4, IPv6, or host names when constructed. The existing `string` overload remains available for compatibility.

To connect to a server with a certificate whose host name is different from the address you dial, pass the TLS target host to `MatchMakerConnectionOptions`.

```csharp
await client.ConnectAsync(
    new MatchMakerAddress("127.0.0.1"),
    new MatchMakerServerPort(57000),
    new MatchMakerPlayerName("player"),
    new MatchMakerConnectionOptions(
        MatchMakerConnectionMode.Tls,
        "match.example.com"));
```

Other user-provided values also have immutable value object overloads: `MatchMakerGameId`, `MatchMakerGameVersion`, `MatchMakerRoomPassword`, `MatchMakerGameHostPort`, `MatchMakerGameHostExternalId`, and `MatchMakerSearchName`.

Use `MatchMakerConnectionMode.Plain` only for backward compatibility or local development against a server configured with `tls.mode` set to `"plain"`.

`RemoteCertificateValidationCallback` can be set for development with self-signed certificates. Do not disable certificate validation in production.

## Port Mapping Auto Release

In .Net Framework, port mappings created in the aplication are released automatically when the application exits.

But, in .Net Core, port mappings are not released due to [Open.NAT behavior](https://github.com/lontivero/Open.NAT/issues/94).
([This page](https://stackoverflow.com/questions/44732234/why-does-the-finalize-destructor-example-not-work-in-net-core) is also helphul)

Call NatPortMappingCreator.ReleaseCreatedPortMappings method manually to ensure to release port mappings created in the application which uses .Net Core.
(Even if you don'y release them manually, port mappings will be released affer 10 minuites if NAT device suports lifetime of port mapping.)
