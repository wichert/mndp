//
//  wire.cpp
//  mactelnet
//
//  Created by Wichert Akkerman on 26/01/16.
//  Copyright © 2016 Wichert Akkerman. All rights reserved.
//

#include <arpa/inet.h>
#include "wire.hpp"


namespace ip = boost::asio::ip;


template<>
uint16_t mikrotik::internals::fromNetwork(const char* buffer) {
    auto v = *reinterpret_cast<const uint16_t*>(buffer);
    return ntohs(v);
}


ip::udp::socket mikrotik::CreateSocket(boost::asio::io_service &io_service, unsigned short port) {
    ip::udp::endpoint local_endpoint(ip::udp::v4(), port);
    
    ip::udp::socket socket(io_service);
    socket.open(ip::udp::v4());
    socket.set_option(boost::asio::socket_base::broadcast(true));
    socket.set_option(boost::asio::socket_base::reuse_address(true));
    socket.bind(local_endpoint);
    return socket;
}