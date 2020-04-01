#pragma once

#include <string>
#include <vector>
#include "Utils.hpp"

#define uchar unsigned char

class MacAdress {
   private:
    uchar _part[6];

    static std::string readable(uchar adress) {
        uchar low, high;
        low = adress & 15;
        high = adress >> 4;
        return std::string() + (char)hexToChar(high) + (char)hexToChar(low);
    }

    static uchar hexToChar(uchar hex) {
        if (hex < 10) {
            return (uchar)(48 + hex);
        } else {
            return (uchar)(55 + hex);
        }
    }

   public:
    MacAdress() {
        for (int i = 0; i < 6; ++i) {
            _part[i] = 0;
        }
    }

    MacAdress(std::string adress) {
        std::vector<std::string> parts = split(adress, ':');
        for (int i = 0; i < 6; ++i) {
            _part[i] = parts.at(i).at(0);
            _part[i] = parts.at(i).at(1);
        }
    }

    MacAdress(uchar* adress) {
        for (int i = 0; i < 6; ++i) {
            _part[i] = adress[i];
        }
    }

    std::string part(int id) { return readable(_part[id]); }

    friend std::ostream& operator<<(std::ostream& output, const MacAdress mac) {
        for (int i = 0; i < 5; ++i) {
            output << readable(mac._part[i]) << ":";
        }
        output << readable(mac._part[5]);
        return output;
    }
};