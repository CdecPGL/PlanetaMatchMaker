# Open.NAT Patches

This directory contains vendored Open.NAT code used by Planeta Match Maker clients.

## Base

- Upstream: https://github.com/lontivero/Open.NAT
- License: MIT
- Base revision: `643f04e8227fe873731b884244bcee4cc84c8d49`
- Baseline note: the code was vendored as a compile-error fixed version of the base revision above. Those pre-existing compile fixes predate this patch log.

Open.NAT is no longer maintained for this repository's needs. Keep maintenance local, minimal, and documented here.

## Repository Copies

The repository keeps two copies of this code:

- `PlanetaMatchMakerClient/Source/Library/Open.Nat`
- `PlanetaMatchMakerUnityClient/Assets/PlanetaGameLabo/MatchMaker/PlanetaMatchMakerClient/Library/Open.Nat`

Keep the two copies synchronized when changing Open.NAT code. The Unity copy must also keep `.meta` files for any added assets.

## Local Changes

### UPnP response and request hardening

UPnP is used only by clients that explicitly create NAT port mappings. The match-making server does not expose a UPnP service.

The following hardening is maintained locally:

- Validate SSDP `LOCATION` before fetching a device descriptor.
  - Only `http` URLs are accepted.
  - Userinfo is rejected.
  - The host must be a canonical IP literal. Non-canonical IPv4 forms such as octal or hexadecimal are rejected.
  - IPv4-mapped IPv6 host literals are rejected in `LOCATION` URLs.
  - The host IP must match the SSDP response source IP after normalizing IPv4-mapped response addresses and ignoring IPv6 scope IDs.
- Canonicalize UPnP `serviceType` values to the known WANIPConnection/WANPPPConnection URNs.
  - Partial `Contains` matches are not accepted.
  - The canonical URN is used for SOAP requests.
- Prevent service `controlURL` values from changing host.
  - Absolute URLs are reduced to path and query on the discovered device host.
  - Network-path URLs such as `//host/path` are rejected.
- Disable HTTP redirects for UPnP descriptor and SOAP requests.
- Parse UPnP XML through `XmlReaderSettings` with DTD processing disabled and `XmlResolver = null`.
- Reject UPnP XML responses larger than 1 MiB while reading from the HTTP response stream.
- Escape SOAP XML values before writing request bodies.

### NAT-PMP response hardening

NAT-PMP is also used by clients when Open.NAT attempts PMP discovery or port mapping.

The following hardening is maintained locally:

- Accept NAT-PMP external address discovery responses only from a searched gateway endpoint on UDP port 5351.
- Ignore NAT-PMP external address discovery responses with non-success result codes.
- Attempt NAT-PMP only from RFC1918 private IPv4 local addresses and only toward IPv4 gateway endpoints.
- Do not process unsolicited NAT-PMP gratuitous address announcements; clients use explicit request/response only.
- Store the discovered gateway address as `PmpNatDevice.HostEndPoint`.
- Accept NAT-PMP port mapping responses only when all of the following match:
  - The response source is the selected gateway endpoint on UDP port 5351.
  - The packet is exactly the 16-byte NAT-PMP port mapping response format.
  - The version and response opcode match the requested protocol.
  - The private port matches the requested mapping.
- For create responses, use the mapped external port returned by the gateway, even when it differs from the requested/suggested public port.
- For delete responses, accept the NAT-PMP success format with mapped external port `0` and lifetime `0`.
- Parse response ports as unsigned 16-bit values.
- Handle unknown NAT-PMP result codes without indexing outside the known error table.

Related tests are in `PlanetaMatchMakerClientTest/Source/OpenNatSecurityTest.cs`.
