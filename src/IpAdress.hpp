// Copyright Grama Nicolae 2020
#pragma once

#include <string>
#include "Utils.hpp"

#define uchar unsigned char

class IpAdress {
   private:
    uint adress;

   public:
    IpAdress() { adress = 0; }

    IpAdress(std::string adress) { this->adress = inet_addr(adress.c_str()); }

    IpAdress(uchar* ip_arr) {
        adress = ip_arr[3] * 16777216 + ip_arr[2] * 65536 + ip_arr[1] * 256 +
                 ip_arr[0];
    }

    IpAdress(uint adress) { this->adress = adress; }

    IpAdress(const IpAdress& other) { adress = other.adress; }

    void adressArray(uchar* ip_part) const {
        ip_part[0] = (adress & 0xFF);
        ip_part[1] = ((adress >> 8) & 0xFF);
        ip_part[2] = ((adress >> 16) & 0xFF);
        ip_part[3] = ((adress >> 24) & 0xFF);
    }

    uint getAdress() const { return adress; }

    bool operator>(const IpAdress& other) const {
        return (adress > other.adress);
    }

    bool operator<(const IpAdress& other) const {
        return (adress < other.adress);
    }

    bool operator==(const IpAdress& other) const {
        return (adress == other.adress);
    }

    friend std::ostream& operator<<(std::ostream& output, const IpAdress ip) {
        in_addr ip_addr;
        ip_addr.s_addr = ip.adress;

        output << inet_ntoa(ip_addr);
        return output;
    }
};