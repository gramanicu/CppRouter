# CppRouter

The first homework for the Communication Protocols Course, a packet forwarding router. The problem statement can be found [here](./docs/problem_statement.pdf).

## Project Structure

- Router - This is the main module, that contains most of the routing logic. It stores the "routing table" and the "mac table".
- ArpTable - the logic behind the "mac table". It uses a unordered_map (hashmap) to provide O(1) access to the entries.
- RoutingTable - has functions to parse a route table, store, sort and retrieve entries.
- IpAdress - a wrapper for ip adresses, stored as an 32 bit unsigned int
- MacAdress - a wrapper for mac adresses, stored as an unsigned char array
- Utils - different functions used in development and by the different parts of the program
- Skel - utilitaries included with the original files. Includes interface multiplexing required to use all 4 interfaces for communication.

## Functionality

When the program is started, it will loook for "rtable.txt", the route table file. Then, it will parse it, and initialise diferent components. Then it will start listening in an infinite loop for incoming packets. Each time it will receive a packet, it will:

- check if it is an `ARP_REQUEST`. If it is for the router, it will try to send an ARP_REPLY. I wanted to add the posibility to respond to the ARP_Request even if it isn't for the router (if it has the required mac adress in it's table). The code for this still exist, but is commented.
- check if it is an `ARP_REPLY`. Check if it already had the entry in its table. If it didn't, send the packets from the queue (they will be discussed at a later step).
- check if the packet expired or has a bad checksum. If the first is true (ttl <= 1), send `ICMP_TIME_EXCEEDED` to the packet's sender.
- check if the packet is an `ICMP_ECHO` (ping) for the router. If it is, send `ICMP_ECHOREPLY`.
- decrease TTL (Time To Live).
- recompute checksum.
- find the best route to forward the packet. If it doesn't exist, send `ICMP_DEST_UNREACH`.
- get the mac adress of the next hop. If it doesn't exist at the moment, send an `ARP_REQUEST` to find it and at the packet in a queue (it will be sent when we know the MAC)
- modify all required fields and forward the packet

© 2020 Grama Nicolae
