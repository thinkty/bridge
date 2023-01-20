
*Note: this is just an ongoing experiment purely out of curiosity. Idk if this is gonna work or not.*

# Bridge

**Bridge** is a simple server application to aid in peer-to-peer communication.
In certain scenarios, users are behind a NAT and depending on the [implementation](https://en.wikipedia.org/wiki/Network_address_translation#Methods_of_translation) of the NAT, it can be quite difficult to perform peer-to-peer connection.
So, Bridge is somewhat of an extension of a STUN server in a way that it allows both peers to open to each other.

However, the goal is to also function for clients behind a Symmetric NAT.
I'm currently thinking that this can be done by two phases: *registration* and *lookup*.

During the *registration* phase, the both clients connect to the server with a name to let it be reachable by others.
Then, the server will add an entry in memory by the address and name.
In the address, only the IP address is used as the port number is not-so-helpful due to NAT.
In a Symmetric NAT, the address and the port number of the external host must match the entry on the recipient.

The name is needed as a static ID in contrast to the dynamically allocated IP address.
If the name already exists by a different address, the server will respond with an error.

In the *lookup* phase, both clients query the server with the opponent's name that they know in advance to the process.
The server will return the IP address on successful lookup.
Then, the process moves to the peer as the peer-to-peer connection has to start from the peer to avoid using a relay (like TURN).

The current idea is that once the peer knows the IP address of the other peer, it sends TCP request for a given port range in a brute-force way.
It's not expected to create a connection in this step as the purpose is to add entries to allow the other peer to bypass the NAT.
The concern is that the range of the NAT port differs by vendor and in the worst case, it will have to try all 65535 ports.


An example transaction would be as 

https://www.rfc-editor.org/rfc/rfc5382#section-4.1

https://www.rfc-editor.org/rfc/rfc4787#section-4.1
