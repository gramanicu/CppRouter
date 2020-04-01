#pragma once
#include <arpa/inet.h>
#include <asm/byteorder.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define MAX_LEN 1600
#define ROUTER_NUM_INTERFACES 4

#define DIE(condition, message)                                 \
    do {                                                        \
        if ((condition)) {                                      \
            fprintf(stderr, "[%d]: %s\n", __LINE__, (message)); \
            perror("");                                         \
            exit(1);                                            \
        }                                                       \
    } while (0)

typedef struct {
    int len;
    char payload[MAX_LEN];
    int interface;
} packet;

extern int interfaces[ROUTER_NUM_INTERFACES];

int send_packet(int interface, packet *m);
int get_packet(packet *m);
char *get_interface_ip(int interface);
int get_interface_mac(int interface, uint8_t *mac);
void init();
void parse_arp_table();
int hwaddr_aton(const char *txt, uint8_t *addr);
uint16_t ip_checksum(void *vdata, size_t length);