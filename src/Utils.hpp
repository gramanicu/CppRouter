#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define DIE(condition, message)                                 \
    do {                                                        \
        if ((condition)) {                                      \
            fprintf(stderr, "[%d]: %s\n", __LINE__, (message)); \
            perror("");                                         \
            exit(1);                                            \
        }                                                       \
    } while (0)

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
    return stoi(ip_part.at(0)) * 16777216 + stoi(ip_part.at(1)) * 65536 +
           stoi(ip_part.at(2)) * 256 + stoi(ip_part.at(3));
}

// Integer to network adress string
// ex. 0 => 0.0.0.0
std::string itons(const uint adress) {
    using namespace std;
    vector<string> ip_part(4);
    ip_part[0] = to_string((adress >> 24) & 0xFF);
    ip_part[1] = to_string((adress >> 16) & 0xFF);
    ip_part[2] = to_string((adress >> 8) & 0xFF);
    ip_part[3] = to_string(adress & 0xFF);
    return ip_part[0] + "." + ip_part[1] + "." + ip_part[2] + "." + ip_part[3];
}