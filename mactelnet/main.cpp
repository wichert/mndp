//
//  main.cpp
//  mactelnet
//
//  Created by Wichert Akkerman on 23/01/16.
//  Copyright © 2016 Wichert Akkerman. All rights reserved.
//

#include <iostream>
#include <chrono>
#include <boost/asio.hpp>


using boost::asio::ip::udp;

const unsigned short mndp_port = 5678;

namespace mactelnet {
    namespace mndp {
        class Server {
        public:
            std::string identity;
            std::string version;
            std::string platform;
            std::string hardware;
            std::string software_id;
            std::string interface_name;
            uint8_t mac[6];
            std::chrono::seconds uptime;
            boost::asio::ip::address_v6 ipv6;
            
        };
        
        namespace wire {
            struct header {
                uint8_t version;
                uint8_t ttl;
                uint8_t checksum;
            };
            
            
            enum class AttributeType : uint16_t {
                mac_address = 0x0001,
                identity = 0x0005,
                version = 0x0007,
                platform = 0x0008,
                uptime = 0x000a,
                software_id = 0x000b,
                hardware = 0x000c,
                unpack = 0x000e,
                ipv6_address = 0x000f,
                interface_name = 0x0010,
            };
        }
        
        struct info {
            wire::header header;
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


void ParseMNDP(const char* buffer, size_t len) {
        if (len<18)
            return;
    
    size_t offset { 0 };
    auto header = reinterpret_cast<const mactelnet::mndp::wire::header*>(buffer);
    
    mactelnet::mndp::Server server;
    
    while (offset + 4 < len) {
        auto attr_type = *reinterpret_cast<const mactelnet::mndp::wire::AttributeType*>(buffer+offset);
        offset += 2;
        auto attr_length = *reinterpret_cast<const uint32_t*>(buffer+offset);
        offset += 2;
        if (offset + attr_length > len)
            // Invalid attribute length, would overflow.
            return;
        switch (attr_type) {
            case mactelnet::mndp::wire::AttributeType::mac_address:
                if (attr_length==6)
                    memcpy(server.mac, buffer+offset, 6);
                break;
            case mactelnet::mndp::wire::AttributeType::identity:
                server.identity = std::string(buffer+offset, attr_length);
                break;
            case mactelnet::mndp::wire::AttributeType::version:
                server.version = std::string(buffer+offset, attr_length);
                break;
            case mactelnet::mndp::wire::AttributeType::platform:
                server.platform = std::string(buffer+offset, attr_length);
                break;
            case mactelnet::mndp::wire::AttributeType::uptime:
                if (attr_length==4)
                    server.uptime = std::chrono::seconds(*reinterpret_cast<const uint32_t*>(buffer+offset));
                    break;
            case mactelnet::mndp::wire::AttributeType::software_id:
                server.software_id = std::string(buffer+offset, attr_length);
                break;
            case mactelnet::mndp::wire::AttributeType::hardware:
                server.hardware = std::string(buffer+offset, attr_length);
                break;
//            case mactelnet::mndp::wire::AttributeType::ipv6_address:
//                if (attr_lenth==16)
//                    server.ipv6 = boost::asio::ip::address_v6::address_v6(buffer+offset);
            case mactelnet::mndp::wire::AttributeType::interface_name:
                server.interface_name = std::string(buffer+offset, attr_length);
                break;
        }
        offset += attr_length;
    }
    
    using namespace std;
    cout
        << "Platform: " << server.platform << endl
        << "Identity: " << server.identity << endl
        << "Software id: " << server.software_id << endl;
}


int main(int argc, const char * argv[]) {
    boost::asio::io_service io_service;
    udp::endpoint remote_endpoint(boost::asio::ip::address_v4::broadcast(), mndp_port);
    auto socket = CreateSocket(io_service);
    
    uint32_t r[1] { 0 };
    socket.send_to(boost::asio::buffer(r), remote_endpoint);
    
    char buf[1500];
    udp::endpoint sender_endpoint;
    
    auto len = socket.receive_from(boost::asio::buffer(buf), sender_endpoint);
    std::cout << "Received " << len << " bytes" << std::endl;
    ParseMNDP(buf, len);
    return 0;
}
