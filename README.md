> Simple DNS Forwarder

> [!IMPORTANT]
> Work In Progress ...

## Todo
- [ ] concurrent connections
- [ ] mapping response to client
- [ ] caching
- [ ] filtering
- [ ] support for falling back to different dns server

## Demo
 
> DNS forwarding server
<image src="./images/server.png" width="1000">

> The client requests the DNS forwarding server (running at localhost:8989) to resolve the address of the domain name "www.google.com"
<image src="./images/client.png" width="1000">

## Quick start
- Build the project using `make`
- Run the DNS forwarding server using `./server`
- Test the server using a dummy client via `make client`

## References
- [Coding Challenges - Build Your Own DNS Forwarder](https://codingchallenges.fyi/challenges/challenge-dns-forwarder)
- [RFC 1035 Section 4.1.1 - DNS query format](https://datatracker.ietf.org/doc/html/rfc1035#section-4.1)
