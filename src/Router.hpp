// Copyright Grama Nicolae 2020
#pragma once

#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <ifaddrs.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <queue>
#include "ArpTable.hpp"
#include "RoutingTable.hpp"
#include "Skel.hpp"

#define ROUTES_FILE "rtable.txt"
#define FOREVER while (1)
#define MAX_LEN 1600
#define IP_SIZE 4 * sizeof(uchar)
#define MAC_SIZE 6 * sizeof(uchar)
#define IPH_OFFSET sizeof(ether_header)
#define ICMPH_OFFSET sizeof(ether_header) + sizeof(iphdr)
#define ARPH_OFFSET sizeof(ether_header)

// This may not be the correct length, but it works in my tests
#define ICMP_CHECK_LEN 64

#pragma region HeaderPrint
// Print the arpheader
void print_arphdr(my_arphdr *arp_hdr) {
    MacAdress smac(arp_hdr->ar_sha);
    MacAdress dmac(arp_hdr->ar_tha);
    IpAdress sip(arp_hdr->ar_sip);
    IpAdress dip(arp_hdr->ar_tip);

    std::cout << " - ARP HEADER - \n";
    if (ntohs(arp_hdr->ar_op) == ARPOP_REQUEST) {
        std::cout << "ARP REQUEST\n";
    } else if (ntohs(arp_hdr->ar_op) == ARPOP_REPLY) {
        std::cout << "ARP REPLY\n";
    }

    std::cout << "Source Mac : " << smac << "\n";
    std::cout << "Source Ip : " << sip << "\n";
    std::cout << "Destination Mac : " << dmac << "\n";
    std::cout << "Destination Ip : " << dip << "\n";
    std::cout << " - END ARP HEADER - \n\n";
}

// Print the ethernet header
void print_ethhdr(ether_header *eth_hdr) {
    MacAdress smac(eth_hdr->ether_shost);
    MacAdress dmac(eth_hdr->ether_dhost);

    std::cout << " - ETHERNET HEADER - \n";
    if (ntohs(eth_hdr->ether_type) == ETHERTYPE_ARP) {
        std::cout << "ARP EtherType\n";
    } else if (ntohs(eth_hdr->ether_type) == ETHERTYPE_IP) {
        std::cout << "IPv4 EtherType\n";
    }

    std::cout << "Source Mac :  " << smac << "\n";
    std::cout << "Destination Mac : " << dmac << "\n";
    std::cout << " - END ETHERNET HEADER - \n\n";
}

void print_iphdr(iphdr *ip_hdr) {
    IpAdress sip(ip_hdr->saddr);
    IpAdress dip(ip_hdr->daddr);

    std::cout << " - IP HEADER - \n";

    if (ip_hdr->protocol == IPPROTO_IP) {
        std::cout << "IP Protocol\n";
    } else if (ip_hdr->protocol == IPPROTO_ICMP) {
        std::cout << "ICMP Protocol\n";
    }

    std::cout << "Time To Live : " << (int)ip_hdr->ttl << "\n";
    std::cout << "Source IP : " << sip << "\n";
    std::cout << "Destination IP : " << dip << "\n";
    std::cout << " - END IP HEADER - \n\n";
}
#pragma endregion HeaderPrint

class Router {
   private:
    RoutingTable rtable;
    ArpTable atable;
    std::queue<std::pair<Entry, packet>> queue;
    std::vector<IpAdress> interfacesIP;
    std::vector<MacAdress> interfacesMAC;

    void print_queue() {
        auto aux = queue;

        std::cout << " - PACKET QUEUE - \n";
        while (!aux.empty()) {
            Entry route = aux.front().first;
            aux.pop();
            std::cout << "Required mac for ip : " << route.next_hop << "\n";
        }
        std::cout << " - END PACKET QUEUE - \n";
    }

    // Get the interface that has the specified IP (-1 if it doesn't exist)
    int getInterfaceWithIp(IpAdress ip) {
        int interface = -1;
        for (int i = 0; i < ROUTER_NUM_INTERFACES; ++i) {
            if (inet_addr(get_interface_ip(i)) == ip.getAdress()) {
                // Found the requested interface
                interface = i;
                break;
            }
        }
        return interface;
    }

    // Get the interface that has the specified MAC (-1 if it doesn't exist)
    int getInterfaceWithMac(MacAdress mac) {
        int interface = -1;
        for (int i = 0; i < ROUTER_NUM_INTERFACES; ++i) {
            if (interfacesMAC[i] == mac) {
                // Found the requested interface
                interface = i;
                break;
            }
        }
        return interface;
    }

// Outgoing request processing
#pragma region Outgoing
    void icmp_response(packet *m, uint8_t type) {
        ether_header *eth_hdr = (ether_header *)m->payload;
        iphdr *ip_hdr = (iphdr *)(m->payload + sizeof(ether_header));
        icmphdr *icmp_hdr =
            (icmphdr *)(m->payload + sizeof(ether_header) + sizeof(iphdr));

        uchar *buff = (uchar *)malloc(6 * sizeof(uchar));
        memcpy(buff, eth_hdr->ether_dhost, 6 * sizeof(uchar));
        memcpy(eth_hdr->ether_dhost, eth_hdr->ether_shost, 6 * sizeof(uchar));
        memcpy(eth_hdr->ether_shost, buff, 6 * sizeof(uchar));
        free(buff);

        icmp_hdr->code = 0;
        icmp_hdr->type = type;
        icmp_hdr->un.echo.id = htons(getpid());
        icmp_hdr->checksum = 0;
        icmp_hdr->checksum = ip_checksum(icmp_hdr, ICMP_CHECK_LEN);

        ip_hdr->protocol = IPPROTO_ICMP;
        ip_hdr->ihl = 5;
        ip_hdr->daddr = ip_hdr->saddr;
        ip_hdr->tot_len = htons(sizeof(iphdr) + sizeof(icmphdr));
        ip_hdr->saddr = inet_addr(get_interface_ip(m->interface));
        ip_hdr->ttl = 30;
        ip_hdr->check = 0;
        ip_hdr->check = ip_checksum(ip_hdr, sizeof(iphdr));

        m->len = sizeof(ether_header) + sizeof(iphdr) + sizeof(icmphdr);

        send_packet(m->interface, m);
    }

    // Broadcast an arp request to find the MAC adress of the device that has
    // the `destIp`.
    void arp_response(IpAdress destIp) {
        // Broacast the request
        for (int interface = 0; interface < ROUTER_NUM_INTERFACES;
             ++interface) {
            packet m;
            my_arphdr *arp_hdr =
                (my_arphdr *)(m.payload + sizeof(ether_header));
            ether_header *eth_hdr = (ether_header *)m.payload;

            // Set the ethernet header
            eth_hdr->ether_type = htons(ETHERTYPE_ARP);
            get_interface_mac(interface, eth_hdr->ether_shost);
            memset(eth_hdr->ether_dhost, 0xFF, 6 * sizeof(int8_t));

            // Set the arp header
            arp_hdr->ar_op = htons(ARPOP_REQUEST);
            arp_hdr->ar_hln = 6;
            arp_hdr->ar_pln = 4;
            arp_hdr->ar_hrd = htons(1);
            arp_hdr->ar_pro = htons(0x0800);

            uchar *dip, *sip;
            dip = (uchar *)malloc(4 * sizeof(uchar));
            sip = (uchar *)malloc(4 * sizeof(uchar));

            destIp.adressArray(dip);
            interfacesIP[interface].adressArray(sip);

            memcpy(arp_hdr->ar_sha, eth_hdr->ether_shost, 6 * sizeof(uchar));
            memcpy(arp_hdr->ar_tha, eth_hdr->ether_dhost, 6 * sizeof(uchar));
            memcpy(arp_hdr->ar_tip, dip, 4 * sizeof(uchar));
            memcpy(arp_hdr->ar_sip, sip, 4 * sizeof(uchar));

            free(dip);
            free(sip);

            m.len = sizeof(my_arphdr) + sizeof(ether_header);

            send_packet(interface, &m);
        }
    }

#pragma endregion Outgoing

   public:
    Router() {
        rtable.readTable(ROUTES_FILE);
        init();
        std::cout << "Interfaces:\n";
        std::cout << "r-0: " << get_interface_ip(0) << "\n";
        std::cout << "r-1: " << get_interface_ip(1) << "\n";
        std::cout << "r-2: " << get_interface_ip(2) << "\n";
        std::cout << "r-3: " << get_interface_ip(3) << "\n\n";

        for (int i = 0; i < ROUTER_NUM_INTERFACES; ++i) {
            interfacesIP.push_back(IpAdress(std::string(get_interface_ip(i))));

            uchar *mac = (uchar *)malloc(6 * sizeof(uchar));
            get_interface_mac(i, mac);
            interfacesMAC.push_back(MacAdress(mac));
            free(mac);
        }
    }

    void runServer() {
        packet m;
        int rc;
        do {
            rc = get_packet(&m);
            DIE(rc < 0, "get_message");
            ether_header *eth_hdr = (ether_header *)m.payload;
            uint16_t ethType = ntohs(eth_hdr->ether_type);

            // If there is a arp_request for the router, send back the arp_reply
            if (ethType == ETHERTYPE_ARP) {
                my_arphdr *arp_hdr = (my_arphdr *)(m.payload + ARPH_OFFSET);
                uint16_t arpType = ntohs(arp_hdr->ar_op);

                int iface = getInterfaceWithIp(arp_hdr->ar_tip);

                // Check if this is an arp_request for this router
                if (arpType == ARPOP_REQUEST) {
                    // Check if the request is for an router interface
                    if (iface != -1) {
                        arp_hdr->ar_op = htons(ARPOP_REPLY);

                        // Swap mac and ip adresses
                        swap(arp_hdr->ar_sha, arp_hdr->ar_tha, MAC_SIZE);
                        swap(arp_hdr->ar_sip, arp_hdr->ar_tip, IP_SIZE);

                        // Add the routers mac adress to the arp header
                        get_interface_mac(iface, arp_hdr->ar_sha);

                        // Set the ethernet headers values
                        memcpy(eth_hdr->ether_dhost, arp_hdr->ar_tha, MAC_SIZE);
                        memcpy(eth_hdr->ether_shost, arp_hdr->ar_sha, MAC_SIZE);

                        // Send the ARP_REPLY
                        send_packet(m.interface, &m);
                        continue;
                    }

                    // THE NEXT PART IS A REMOVED FEATURE

                    // If the router has the adress stored
                    // MacAdress mac = atable.getMac(arp_hdr->ar_tip);
                    // if (!mac.isEmpty()) {
                    //     // Send the mac from the routers database
                    //     arp_hdr->ar_op = htons(ARPOP_REPLY);

                    //     // Swap mac and ip adresses
                    //     swap(arp_hdr->ar_sha, arp_hdr->ar_tha, MAC_SIZE);
                    //     swap(arp_hdr->ar_sip, arp_hdr->ar_tip, IP_SIZE);

                    //     // Add the routers mac adress to the arp header
                    //     get_interface_mac(m.interface, eth_hdr->ether_shost);
                    //     for (int i = 0; i < 6; ++i) {
                    //         arp_hdr->ar_sha[i] = mac.charPart(i);
                    //     }

                    //     // Set the ethernet headers values
                    //     memcpy(eth_hdr->ether_dhost, arp_hdr->ar_tha,
                    //     MAC_SIZE); get_interface_mac(m.interface,
                    //     eth_hdr->ether_shost);

                    //     // Send the ARP_REPLY
                    //     send_packet(m.interface, &m);
                    //     continue;
                    // }
                } else if (arpType == ARPOP_REPLY) {
                    // Take data from this arp reply and update the table.
                    // Forward if this packet is not for this router.

                    // If this entry didn't exist, send the packets from the
                    // queue
                    if (atable.addEntry(arp_hdr->ar_sip, arp_hdr->ar_sha)) {
                        // Send packets from the queue
                        while (IpAdress(arp_hdr->ar_sip) ==
                               queue.front().first.next_hop) {
                            packet qpack = queue.front().second;
                            Entry qentry = queue.front().first;
                            ether_header *eth_hdr =
                                (struct ether_header *)qpack.payload;

                            // Set the proper ETHERNET headers for the packets
                            // in the queue

                            MacAdress nextMac = atable.getMac(qentry.next_hop);

                            get_interface_mac(qentry.interface,
                                              eth_hdr->ether_shost);
                            for (int i = 0; i < 6; ++i) {
                                eth_hdr->ether_dhost[i] = nextMac.charPart(i);
                            }

                            print_ethhdr(eth_hdr);

                            qpack.interface = qentry.interface;

                            // Forward the packet from the queue
                            send_packet(qpack.interface, &qpack);
                            queue.pop();
                        }

                        continue;
                    }
                }
            }

            iphdr *ip_hdr = (iphdr *)(m.payload + IPH_OFFSET);

            // Verify Time to Live
            if (ip_hdr->ttl <= 1) {
                // Send ICMP message and drop packet
                icmp_response(&m, ICMP_TIME_EXCEEDED);
                continue;
            }

            // Verify Checksum
            uint16_t check = ip_hdr->check;
            ip_hdr->check = 0;
            if (check != ip_checksum(ip_hdr, sizeof(iphdr))) {
                // Drop packet
                continue;
            }

            // If the destination of an IP packet is the router
            // respond if the packet is a ICMP_ECHO
            if (ethType == ETHERTYPE_IP) {
                uint16_t ipType = ip_hdr->protocol;
                int iface = getInterfaceWithIp(ip_hdr->daddr);

                if (ipType == IPPROTO_ICMP && iface != -1) {
                    icmphdr *icmp_hdr = (icmphdr *)(m.payload + ICMPH_OFFSET);

                    // Check if this packet is an ICMP_ECHO to respond to
                    if (icmp_hdr->type == ICMP_ECHO) {
                        // Set the Ethernet header values
                        swap(eth_hdr->ether_dhost, eth_hdr->ether_shost,
                             MAC_SIZE);

                        // Set the ICMP header values
                        icmp_hdr->code = 0;
                        icmp_hdr->type = ICMP_ECHOREPLY;
                        icmp_hdr->checksum = 0;
                        icmp_hdr->checksum =
                            ip_checksum(icmp_hdr, ICMP_CHECK_LEN);

                        // Set the IP header values
                        ip_hdr->daddr = ip_hdr->saddr;
                        ip_hdr->saddr = inet_addr(get_interface_ip(iface));
                        ip_hdr->ttl--;
                        ip_hdr->check = 0;
                        ip_hdr->check = ip_checksum(ip_hdr, sizeof(iphdr));

                        // Send the ICMP response
                        send_packet(m.interface, &m);
                    }
                    continue;
                }
            }

            // Decrease time to live
            ip_hdr->ttl--;

            // Recompute checksum
            ip_hdr->check = 0;
            ip_hdr->check = ip_checksum(ip_hdr, sizeof(iphdr));

            // Search for the route
            Entry route = rtable.getEntry(ip_hdr->daddr);

            // Check if a route exist
            if (route.isEmpty()) {
                // Send ICMP message and drop packet
                icmp_response(&m, ICMP_DEST_UNREACH);
                continue;
            }

            // Get MAC adress of the next hop
            MacAdress mac = atable.getMac(route.next_hop);

            if (mac.isEmpty()) {
                // If the entry doesn't exist in the arp table, send an arp
                // request and push the packet to the back of the queue
                queue.push(std::make_pair(route, m));
                arp_response(route.next_hop);
            } else {
                // Set the ethernet header of the packet
                get_interface_mac(route.interface, eth_hdr->ether_shost);
                for (int i = 0; i < 6; ++i) {
                    eth_hdr->ether_dhost[i] = mac.charPart(i);
                }

                // Forward the packet
                m.interface = route.interface;
                send_packet(m.interface, &m);
            }
        }
        FOREVER;
    }
};