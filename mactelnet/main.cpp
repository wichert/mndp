//
//  main.cpp
//  mactelnet
//
//  Created by Wichert Akkerman on 23/01/16.
//  Copyright © 2016 Wichert Akkerman. All rights reserved.
//

#include <iostream>
#include <boost/asio.hpp>


using boost::asio::ip::udp;

const unsigned short mndp_port = 5678;

namespace mactelnet {
    namespace ndmp {
        struct header {
            uint8_t version;
            uint8_t ttl;
            uint8_t checksum;
        };
        
        struct info {
            header header;
            unsigned char address[6];
            char identity[128];
            char version[128];
            char platform[128];
            char hardware[128];
            char softid[128];
            char ifname[128];
            unsigned int uptime;
        };
    }
}


udp::socket CreateSocket(boost::asio::io_service &io_service) {
    udp::endpoint local_endpoint(udp::v4(), mndp_port);
    
    udp::socket socket(io_service);
    socket.open(udp::v4());
    socket.set_option(boost::asio::socket_base::broadcast(true));
    socket.set_option(boost::asio::socket_base::reuse_address(true));
    socket.bind(local_endpoint);
    return socket;
}


int main(int argc, const char * argv[]) {
    boost::asio::io_service io_service;
    udp::endpoint remote_endpoint(boost::asio::ip::address_v4::broadcast(), mndp_port);
    auto socket = CreateSocket(io_service);
    
    uint32_t r[1] { 0 };
    socket.send_to(boost::asio::buffer(r), remote_endpoint);
    // insert code here...
    std::cout << "Hello, World!\n";
    return 0;
}
