#pragma once

#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <ifaddrs.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <netinet/in.h>
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

typedef struct {
    unsigned short int ar_hrd;      /* Format of hardware address.  */
    unsigned short int ar_pro;      /* Format of protocol address.  */
    unsigned char ar_hln;           /* Length of hardware address.  */
    unsigned char ar_pln;           /* Length of protocol address.  */
    unsigned short int ar_op;       /* ARP opcode (command).  */
    unsigned char ar_sha[ETH_ALEN]; /* Sender hardware address.  */
    unsigned char ar_sip[4];        /* Sender IP address.  */
    unsigned char ar_tha[ETH_ALEN]; /* Target hardware address.  */
    unsigned char ar_tip[4];        /* Target IP address.  */
} my_arphdr;

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

class Router {
   private:
    RoutingTable rt;
    ArpTable at;
    std::queue<packet> qpacket;
    std::vector<IpAdress> interfacesIP;


    // Process an arp request and send back a responses
    void arp_request(packet *m, my_arphdr *arp_hdr) {
        MacAdress smac(arp_hdr->ar_sha);
        MacAdress dmac(arp_hdr->ar_tha);
        IpAdress sip(arp_hdr->ar_sip);
        IpAdress dip(arp_hdr->ar_tip);

        int interface = -1;
        for (int i = 0; i < ROUTER_NUM_INTERFACES; ++i) {
            if (interfacesIP[i] == dip) {
                // Found the requested interface
                interface = i;
                break;
            }
        }

        // Can send back an ARP_REPLY
        if (interface != -1) {
            // Set the art type
            arp_hdr->ar_op = htons(ARPOP_REPLY);

            // Swap the mac and ip adresses
            uchar *buff = (uchar *)malloc(6 * sizeof(uchar));
            memcpy(buff, arp_hdr->ar_sha, 6 * sizeof(uchar));
            get_interface_mac(interface, arp_hdr->ar_sha);
            memcpy(arp_hdr->ar_tha, buff, 6 * sizeof(uchar));

            memcpy(buff, arp_hdr->ar_sip, 4 * sizeof(uchar));
            memcpy(arp_hdr->ar_sip, arp_hdr->ar_tip, 4 * sizeof(uchar));
            memcpy(arp_hdr->ar_tip, buff, 4 * sizeof(uchar));

            free(buff);

            ether_header *eth_hdr = (struct ether_header *)m->payload;
            memcpy(eth_hdr->ether_dhost, arp_hdr->ar_tha, 6 * sizeof(uchar));
            memcpy(eth_hdr->ether_shost, arp_hdr->ar_sha, 6 * sizeof(uchar));

            // Send the ARP_REPLY
            send_packet(interface, m);
        }
    }

   public:
    Router() {
        rt.readTable(ROUTES_FILE);
        init();
        std::cout << "Interfaces:\n";
        std::cout << "r-0: " << get_interface_ip(0) << "\n";
        std::cout << "r-1: " << get_interface_ip(1) << "\n";
        std::cout << "r-2: " << get_interface_ip(2) << "\n";
        std::cout << "r-3: " << get_interface_ip(3) << "\n\n";

        for (int i = 0; i < ROUTER_NUM_INTERFACES; ++i) {
            interfacesIP.push_back(IpAdress(std::string(get_interface_ip(i))));
        }
    }

    void run() {
        packet m;
        int rc;

        do {
            rc = get_packet(&m);
            DIE(rc < 0, "get_message");
            ether_header *eth_hdr = (struct ether_header *)m.payload;
            uint16_t ethType = ntohs(eth_hdr->ether_type);

            switch (ethType) {
                case ETHERTYPE_ARP: {
                    my_arphdr *arp_hdr =
                        (my_arphdr *)(m.payload + sizeof(ether_header));
                    uint16_t arpType = ntohs(arp_hdr->ar_op);
                    switch (arpType) {
                        case ARPOP_REQUEST: {
                            arp_request(&m, arp_hdr);
                        } break;
                        case ARPOP_REPLY: {
                            std::cout << "ARP REPLY\n";
                        } break;
                        default:
                            continue;
                    }
                } break;
                case ETHERTYPE_IP: {
                    std::cout << "WHAT`S UP BITCHES\n";
                } break;
                default:
                    continue;
            }
        }
        FOREVER;
    }
};