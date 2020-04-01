#pragma once

#include <linux/ip.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <unordered_map>
#include "IpAdress.hpp"
#include "Utils.hpp"

#define uint uint32_t

class Entry {
   private:
    std::string sp;
    std::string sn;
    std::string sm;
    std::string si;

    // Convert the data from string to ints
    void parse() {
        prefix = IpAdress(sp);
        next_hop = IpAdress(sn);
        mask = IpAdress(sm);
        interface = stoi(si);

        // Clear the memory used by the strings
        std::string().swap(sp);
        std::string().swap(sn);
        std::string().swap(sm);
        std::string().swap(si);
    }

   public:
    IpAdress prefix;
    IpAdress next_hop;
    IpAdress mask;
    int interface;

    Entry() { interface = 0; }

    Entry(std::string p, std::string n, std::string m, std::string i)
        : sp(p), sn(n), sm(m), si(i) {
        parse();
    };

    bool operator>(const Entry& e) const { return (mask > e.mask); }

    friend std::ostream& operator<<(std::ostream& output, const Entry entry) {
        output << entry.prefix << " " << entry.next_hop << " " << entry.mask
               << " " << std::to_string(entry.interface);
        return output;
    }

    bool isEmpty() {
        if (prefix.getAdress() == 0 && next_hop.getAdress() == 0 &&
            mask.getAdress() == 0 && interface == 0) {
            return true;
        } else {
            return false;
        }
    }
};

class RoutingTable {
   private:
    int size;
    std::ifstream input;

    /*
        I'm using a map to be able to get the routes in O(1) time complexity.
        The keys will be the "prefixes", destination of the routes. Because
        they are not unique, the search in the table will return all these
        possible routes, and the router will choose the best.
    */
    std::unordered_map<uint, std::vector<Entry>> table;

   public:
    RoutingTable() { size = 0; }

    void readTable(std::string filename) {
        input.open(filename);

        std::string line;
        while (std::getline(input, line)) {
            size++;
            std::istringstream buffer(line);
            std::istream_iterator<std::string> beg(buffer), end;

            std::vector<std::string> tokens(beg, end);
            Entry entry(tokens.at(0), tokens.at(1), tokens.at(2), tokens.at(3));

            auto it = table.find(entry.prefix.getAdress());
            // Search if there is already a route to that destination
            if (it == table.end()) {
                std::vector<Entry> v;
                v.push_back(entry);
                table[entry.prefix.getAdress()] = v;
            } else {
                // If there is one, update the routes
                it->second.push_back(entry);
                std::sort(it->second.begin(), it->second.end(),
                          std::greater<Entry>());
            }
        }

        input.close();
    }

    Entry getEntry(uint dest_ip) {
        auto it = table.find(dest_ip);
        if (it == table.end()) {
            return Entry();
        } else {
            return it->second.at(0);
        }
    }
};