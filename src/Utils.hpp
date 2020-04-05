// Copyright Grama Nicolae 2020
#pragma once

#include <linux/if_packet.h>
#include <sys/types.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define ETH_ALEN 6

#define DIE(condition, message)                                 \
    do {                                                        \
        if ((condition)) {                                      \
            fprintf(stderr, "[%d]: %s\n", __LINE__, (message)); \
            perror("");                                         \
            exit(1);                                            \
        }                                                       \
    } while (0)

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

// Split a string using the specified delimitator
std::vector<std::string> split(std::string s, char delim) {
    std::stringstream stream(s);
    std::string item;
    std::vector<std::string> tokens;

    if (s.find(delim) == std::string::npos) {
        std::cerr << "The string doesn't contain the specified delimitator!\n";
        return tokens;
    }

    while (getline(stream, item, delim)) {
        tokens.push_back(item);
    }

    return tokens;
}

// Network adress string to integer
// ex. 0.0.0.0 => 0
uint nstoi(std::string adress) {
    std::vector<std::string> ip_part = split(adress, '.');
    return stoi(ip_part.at(3)) * 16777216 + stoi(ip_part.at(2)) * 65536 +
           stoi(ip_part.at(1)) * 256 + stoi(ip_part.at(0));
}

// Integer to network adress string
// ex. 0 => 0.0.0.0
std::string itons(const uint adress) {
    using namespace std;
    vector<string> ip_part(4);
    ip_part[0] = to_string(adress & 0xFF);
    ip_part[1] = to_string((adress >> 8) & 0xFF);
    ip_part[2] = to_string((adress >> 16) & 0xFF);
    ip_part[3] = to_string((adress >> 24) & 0xFF);
    return ip_part[3] + "." + ip_part[2] + "." + ip_part[1] + "." + ip_part[0];
}

// Swap the values at the specified pointers (of a specified size)
void swap(void* first, void* second, size_t size) {
    unsigned char* buff = (unsigned char*)malloc(size);
    memcpy(buff, first, size);
    memcpy(first, second, size);
    memcpy(second, buff, size);
    free(buff);
}