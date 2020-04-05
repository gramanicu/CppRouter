// Copyright Grama Nicolae 2020
#pragma once

#include <unordered_map>
#include "IpAdress.hpp"
#include "MacAdress.hpp"
#include "Utils.hpp"

class ArpTable {
   private:
    std::unordered_map<uint, MacAdress> table;
    uint size;

   public:
    ArpTable() { size = 0; };

    // Try to add the new entry (return true if the entry didn't exist)
    bool addEntry(IpAdress ip, MacAdress mac) {
        if (!hasEntry(ip)) {
            table[ip.getAdress()] = MacAdress(mac);
            size++;
            return true;
        }
        return false;
    }

    void removeEntry(IpAdress ip) {
        if (!hasEntry(ip)) {
            table.erase(ip.getAdress());
            size--;
        }
    }

    bool hasEntry(IpAdress ip) {
        auto it = table.find(ip.getAdress());
        return it != table.end();
    }

    uint getSize() const { return size; }

    MacAdress getMac(IpAdress ip) {
        auto it = table.find(ip.getAdress());
        if (it != table.end()) {
            return it->second;
        } else {
            return MacAdress();
        }
    }
};