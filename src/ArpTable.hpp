#pragma once

#include <unordered_map>
#include "MacAdress.hpp"
#include "Utils.hpp"

class ArpTable {
   private:
    std::unordered_map<int, MacAdress> table;

   public:
    ArpTable(){};

    void addEntry(int ip, MacAdress mac) { table[ip] = mac; }

    void addEntry(int ip, std::string mac) { table[ip] = MacAdress(mac); }

    void addEntry(std::string ip, std::string mac) {
        table[nstoi(ip)] = MacAdress(mac);
    }

    MacAdress getMac(int ip) { return table.find(ip)->second; }
};