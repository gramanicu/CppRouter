#pragma once

#include <string>
#include "Utils.hpp"

#define uchar unsigned char

class IpAdress {
   private:
    uint adress;

   public:
    IpAdress() { adress = 0; }

    IpAdress(std::string adress) { this->adress = nstoi(adress); }

    IpAdress(uchar* ip_arr) {
        adress = ip_arr[0] * 16777216 + ip_arr[1] * 65536 + ip_arr[2] * 256 +
                 ip_arr[3];
    }

    uint getAdress() const { return adress; }

    bool operator>(const IpAdress& other) const {
        return (adress > other.adress);
    }

    bool operator==(const IpAdress& other) const {
        return (adress == other.adress);
    }

    friend std::ostream& operator<<(std::ostream& output, const IpAdress ip) {
        output << itons(ip.adress);
        return output;
    }
};