# Client

## Connection Security

`MatchMakerClient.ConnectAsync` uses TLS by default. `Host` is an immutable value object for host or IP address values and validates IPv4, IPv6, or host names when constructed.

To connect to a server with a certificate whose host name is different from the address you dial, pass the TLS target host to `ConnectionOptions`.

```csharp
await client.ConnectAsync(
    new Host("127.0.0.1"),
    new ServerPort(57000),
    new PlayerName("player"),
    AuthenticationOptions.Steam(steamAuthTicket),
    new ConnectionOptions(
        ConnectionMode.Tls,
        new Host("match.example.com")));
```

User-provided values are represented by immutable value objects: `GameId`, `GameVersion`, `Host`, `ServerPort`, `PlayerName`, `RoomPassword`, `GameHostPort`, `GameHostExternalId`, and `SearchName`.

Use `ConnectionMode.Plain` only for backward compatibility or local development against a server configured with `tls.mode` set to `"plain"`.

`RemoteCertificateValidationCallback` can be set for development with self-signed certificates. Do not disable certificate validation in production.

## Authentication

`ConnectAsync` accepts an `AuthenticationOptions` value for Steam authentication. PMMS client libraries only send credentials; they do not implement Steam ticket acquisition.
Authentication credentials are sent as the authentication message attachment. Message attachments are limited to
`ClientConstants.MaxMessageAttachmentLength` (15,728,640 bytes), which is the maximum representable by the protocol
chunk sequence field. The server's configured authentication credential limit may be lower.

```csharp
await client.ConnectAsync(
    host,
    port,
    new PlayerName("player"),
    AuthenticationOptions.Steam(steamAuthTicket));
```

For local development against a server with `authentication.method` set to `none`, use the credential-free overload or `AuthenticationOptions.None()`:

```csharp
await client.ConnectAsync(host, port, new PlayerName("player"));

await client.ConnectAsync(
    host,
    port,
    new PlayerName("player"),
    AuthenticationOptions.None());
```

This mode does not create a verified external identity and must not be used in production.

SteamID64 is not sent as a client-claimed authentication ID. The server derives the authenticated identity from the Steam verification result.

`CreateRoomWithExternalServiceAsync` accepts `GameHostExternalId? externalId = null`. `null` sends a 64-byte zero value, which means "unspecified". If the authenticated identity has a verified external ID, the server uses it automatically; if the request specifies an external ID, it must match the verified one. Steam rooms therefore normally call `CreateRoomWithSteamAsync` without passing an external ID.

## Port Mapping Auto Release

In .Net Framework, port mappings created in the aplication are released automatically when the application exits.

But, in .Net Core, port mappings are not released due to [Open.NAT behavior](https://github.com/lontivero/Open.NAT/issues/94).
([This page](https://stackoverflow.com/questions/44732234/why-does-the-finalize-destructor-example-not-work-in-net-core) is also helphul)

Call NatPortMappingCreator.ReleaseCreatedPortMappingsAsync method manually to ensure to release port mappings created in the application which uses .Net Core.
(Even if you don'y release them manually, port mappings will be released affer 10 minuites if NAT device suports lifetime of port mapping.)

Unity PlanetaMatchMakerClient releases created port mappings automatically when the component receives OnDestroy if releaseCreatedPortMappingsOnDestroy is true. This option is enabled by default.

The OnDestroy release waits up to 5 seconds. If NAT device doesn't respond in 5 seconds, application quit continues without waiting for the release to finish.

Unity may be unable to call OnDestroy on some platforms or lifecycle paths, such as mobile process kill after suspend or Web platform tab close. Call PlanetaMatchMakerClient.ReleaseCreatedPortMappingsAsync or PlanetaMatchMakerClient.ReleaseCreatedPortMappings manually before those platform-specific exits if required.
