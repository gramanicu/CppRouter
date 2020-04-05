// Copyright Grama Nicolae 2020
#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
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
        prefix = IpAdress(inet_addr(sp.c_str()));
        next_hop = IpAdress(inet_addr(sn.c_str()));
        mask = IpAdress(inet_addr(sm.c_str()));
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

    Entry(const Entry& other) {
        prefix = other.prefix;
        next_hop = other.next_hop;
        mask = other.mask;
        interface = other.interface;
    }

    bool operator>(const Entry& e) const {
        if (prefix > e.prefix) {
            return true;
        } else if (prefix == e.prefix && mask > e.mask) {
            return true;
        }
        return false;
    }

    bool operator<(const Entry& e) const {
        if (prefix < e.prefix) {
            return true;
        } else if (prefix == e.prefix && mask < e.mask) {
            return true;
        }
        return false;
    }

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

    std::vector<Entry> table;

    bool isGoodEntry(int index, IpAdress ip) {
        Entry e = table[index];
        if (e.prefix.getAdress() == (ip.getAdress() & e.mask.getAdress())) {
            return true;
        }
        return false;
    }

    // Binary search function to get the best route in O(log n) time
    int binarySearch(int start, int end, IpAdress ip) {
        if (start <= end) {
            int mid = (start + end) / 2;

            if (table[mid].prefix.getAdress() !=
                (ip.getAdress() & table[mid].mask.getAdress())) {
                return binarySearch(start, mid - 1, ip);
            } else {
                return binarySearch(mid + 1, end, ip);
            }
        }

        return -1;
    }

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

            table.push_back(entry);
        }

        // O(n log n) ascending order sorting of the table, based on the
        // prefix. If they are equal, the first is the smaller mask
        std::sort(table.begin(), table.end());

        input.close();
    }

    Entry getEntry(uint dest_ip) {
        // THIS IS A BINARY SEARCH CODE. IT DIDN'T WORK WELL, SO I
        // USED A ALGORITHM THAT ACTUALLY WORKS

        // int index = binarySearch(0, table.size() - 1, dest_ip);
        // uint mask, bestMask;

        // if (index == -1) {
        //     return Entry();
        // }

        // for(int i = index; i < table.size(); ++i) {
        //     if(!isGoodEntry(i, dest_ip)) {
        //         mask = table[i].mask.getAdress();
        //     } else {
        //         mask = 0;
        //     }

        //     if(mask >= bestMask) {
        //         bestMask = mask;
        //     } else {
        //         return table[i-1];
        //     }
        // }

        uint maxMask = 0;
        for (uint i = 0; i < table.size(); ++i) {
            if (isGoodEntry(i, dest_ip)) {
                if (table[i].next_hop == dest_ip) {
                    return table[i];
                }

                if (table[i].mask.getAdress() > maxMask) {
                    maxMask = table[i].mask.getAdress();
                } else {
                    if (maxMask != 0) {
                        // Found the max, return
                        return table[i - 1];
                    }
                }
            }
        }

        return Entry();
    }

    void print() {
        for (auto& i : table) {
            std::cout << i << "\n";
        }
    }
};