#include <iostream>
#include "RoutingTable.hpp"

int main() {
    RoutingTable rt;

    rt.readTable("rtable.txt");

    std::cout << rt.getEntry(nstoi("192.168.0.0")) << "\n";
    return 0;
}
