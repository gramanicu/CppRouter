#pragma once

#include <linux/ip.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#define uint uint32_t

// Split a string using the specified delimitator
std::vector<std::string> split(std::string s, char delim) {
    std::stringstream stream(s);
    std::string item;
    std::vector<std::string> tokens;
    while (getline(stream, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

// Network adress string to integer
// ex. 0.0.0.0 => 0
uint nstoi(std::string adress) {
    std::vector<std::string> ip_part = split(adress, '.');
    return stoi(ip_part.at(0)) * 16777216 + stoi(ip_part.at(1)) * 65536 +
           stoi(ip_part.at(2)) * 256 + stoi(ip_part.at(3));
}

// Integer to network adress string
// ex. 0 => 0.0.0.0
std::string itons(const int adress) {
    using namespace std;
    vector<string> ip_part(4);
    ip_part[0] = to_string((adress >> 24) & 0xFF);
    ip_part[1] = to_string((adress >> 16) & 0xFF);
    ip_part[2] = to_string((adress >> 8) & 0xFF);
    ip_part[3] = to_string(adress & 0xFF);
    return ip_part[0] + "." + ip_part[1] + "." + ip_part[2] + "." + ip_part[3];
}


class Entry {
   private:
    std::string sp;
    std::string sn;
    std::string sm;
    std::string si;

    // Convert the data from string to ints
    void parse() {
        prefix = nstoi(sp);
        next_hop = nstoi(sn);
        mask = nstoi(sm);
        interface = stoi(si);

        // Clear the memory used by the strings
        std::string().swap(sp);
        std::string().swap(sn);
        std::string().swap(sm);
        std::string().swap(si);
    }

   public:
    uint prefix;
    uint next_hop;
    uint mask;
    int interface;

    Entry() {
        prefix = 0;
        next_hop = 0;
        mask = 0;
        interface = 0;
    }

    Entry(std::string p, std::string n, std::string m, std::string i)
        : sp(p), sn(n), sm(m), si(i) {
        parse();
    }

    bool operator>(const Entry& e) const { return (mask > e.mask); }

    friend std::ostream& operator<<(std::ostream& output, const Entry entry) {
        output << itons(entry.prefix) << " " << itons(entry.next_hop) << " "
               << itons(entry.mask) << " " << std::to_string(entry.interface);
        return output;
    }

    bool isEmpty() {
        if (prefix == 0 && next_hop == 0 && mask == 0 && interface == 0) {
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

            auto it = table.find(entry.prefix);
            // Search if there is already a route to that destination
            if (it == table.end()) {
                std::vector<Entry> v;
                v.push_back(entry);
                table[entry.prefix] = v;
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