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

### Scope reduction

The vendored code is maintained only for the current Planeta Match Maker client targets.

- Removed legacy .NET 3.5 compatibility branches and helper types.
- Removed NAT-PMP discovery and port mapping support. NAT-PMP-capable home routers are uncommon enough that the maintenance and security review cost is larger than the expected benefit for this project.

### Port mapping release reliability

Open.NAT's original shutdown release path did not await `DeletePortMapAsync` and could release the wrong mapping because it iterated a filtered snapshot but deleted by indexing into the live opened-mapping set.

The following release fixes are maintained locally:

- Release APIs are task-based so callers can wait for port mapping deletion to finish.
- Release uses a snapshot of the target mappings and deletes those exact mappings.
- Multiple mappings are deleted in parallel to avoid shutdown time increasing linearly with mapping count.
- Opened mapping registration and removal are guarded by a lock.
- Forced session mappings are included in session release, because they are created only as a fallback for routers that accept permanent leases but still need application-session cleanup.

Related tests are in `PlanetaMatchMakerClientTest/Source/OpenNatReleaseTest.cs`.

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

Related tests are in `PlanetaMatchMakerClientTest/Source/OpenNatSecurityTest.cs`.
