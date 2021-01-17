# NAT Traversal with UPnP

Planeta Match Maker supports clients connecting to game host client which is under NAT by using UPnP.

To use this feature, use `MatchMakerClient.CreateRoomWithCreatingPortMappingAsync()` method in client codes.
This method is a method to create room with NAT traversal.

This works well for clients which satisfy below conditions.

- The NAT device of private network which clients belongs to supports UPnP
- Client are in NOT multiple NAT environment

## Protocol

`MatchMakerClient.CreateRoomWithCreatingPortMappingAsync()` receives default game port, private port candidate and public port candidate as its parameter.
This method uses default game port if it is possible, otherwise try to create port mapping between a private port and public port from candidates and use mapped port.
Detailed procedure is below.

1. Connection Test for the default game port (if succeeded, move to last step)
1. Select available candidate ports from passed candidates. Used ports in the client are removed from candidates (if there are no candidates, returns error)
1. Create port mapping to host game (if failed, returns error)
1. Connection Test for the mapped port (if failed, returns error)
1. Create a room with the port which the server can reach

## Connection Test

Connection test is a test to check the port of the client which is under NAT is reachable from the server in the internet.
The procedure is different for TCP and UDP.

### TCP

In TCP, perform reachable check once because reliability of the transported data is guaranteed.

1. [Client] Check port availability
1. [Client] Request connection test to the server and create TCP listener
1. [Server] Create TCP client and try to connect to the port of the client
1. [Server] Send test message to the client
1. [Client] Reply received test message to the server
1. [Server] Wait reply from the client (if timed out, regard test as failed)
1. [Server] Check replied test message matched sent one (if not match, regard test as failed)
1. [Server] Reply connection test result

### UDP

In UDP, perform reachable check several times because reliability of the transported data is not guaranteed.
In default, the check is performed three times. This is changable in server setting.

1. [Client] Check port availability
1. [Client] Request connection test to the server create UDP listener
1. [Server] Create UDP client
1. [Server] Try below operations several times
    1. [Server] Send test message to the client
    1. [Client] Reply received test message to the server
    1. [Server] Wait reply from the client (if timed out, regard test as failed)
    1. [Server] Check replied test message matched sent one (if not match, regard test as failed)
1. [Server] Reply connection test result

## Creation of Port Mapping

Creation of Port Mapping is a process for client which is under NAT.
In this process, clients attempt to create port mapping if UPnP is available for the NAT.
If UPnP is not abvaiable, creation of port mapping is not performed and returns error.

In context of explanation related to NAT, some kind of IP address and port appear.

- Private IP Address: A client private IP address which the NAT device send data received from the internet to
- Private Port: A client port which the NAT device send data received from the internet to
- Public IP Adress: A NAT device global IP address which the NAT device receive data from internet
- Public PortA NAT device port which the NAT device receive data from internet

The procedure of port mapping creation is as below.

1. Search NAT device which supports UPnP (if there are no device, returns error)
1. Select client IP address whose network is same as the network whivh the NAT device belongs to (if there is not one IP address which matches the condition, returns error)
1. Get already created port mapping from the NAT
1. Search and try to use already created port mapping whose private IP address is my IP adsress and private port is included in candidates (if exist, following steps are skiped)
1. Select candidates of port mapping. The port pair which includes already used private ports of my IP address or public ports are removed from the candidates (if there are no candidates, returns error)
1. Craete port mapping

In this prodedure, each client under the same NAT uses different port mapping because port mappings which are used by private IP address of other clients won't be used. So multiple clients under the same NAT can create room individually.
Additionally, each client application in the same client don't use port mapping which contains already used port in othor applications, which means each client application uses different port mapping. So they also can create room individually.
