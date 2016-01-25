//
//  main.cpp
//  mactelnet
//
//  Created by Wichert Akkerman on 23/01/16.
//  Copyright © 2016 Wichert Akkerman. All rights reserved.
//

#include <iostream>
#include <chrono>
#include <arpa/inet.h>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>


namespace logging = boost::log;
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
            boost::asio::ip::address_v4 ipv4;
            boost::asio::ip::address_v6 ipv6;
            
        };
        
        namespace wire {
            struct header {
                uint8_t version;
                uint8_t ttl;
                uint16_t seq_no;
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


template<typename T>
T fromNetwork(const char* buffer);


template<>
uint16_t fromNetwork(const char* buffer) {
    auto v = *reinterpret_cast<const uint16_t*>(buffer);
    return ntohs(v);
}


void ParseMNDP(const char* buffer, size_t len, boost::asio::ip::address sender_address) {
    if (len<18) {
        BOOST_LOG_TRIVIAL(info) << "Ignoring short packet (" << len << " bytes)";
        return;
    }
    
    auto header = reinterpret_cast<const mactelnet::mndp::wire::header*>(buffer);
    size_t offset { sizeof(*header) };
    
    mactelnet::mndp::Server server;
    
    if (sender_address.is_v4())
        server.ipv4 = sender_address.to_v4();
    else if (sender_address.is_v6())
        server.ipv6 = sender_address.to_v6();
    
    while (offset + 4 < len) {
        auto attr_type = static_cast<mactelnet::mndp::wire::AttributeType>(fromNetwork<uint16_t>(buffer+offset));
        offset += 2;
        auto attr_length = fromNetwork<uint16_t>(buffer+offset);
        offset += 2;
        if (offset + attr_length > len)
            // Invalid attribute length, would overflow.
            return;
        switch (attr_type) {
            case mactelnet::mndp::wire::AttributeType::mac_address:
                if (attr_length==6)
                    memcpy(server.mac, buffer+offset, 6);
                else {
                    BOOST_LOG_TRIVIAL(warning) << "Packet from " << sender_address << " has invalid attribute length for MAC address";
                }
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
                else {
                    BOOST_LOG_TRIVIAL(warning) << "Packet from " << sender_address << " has invalid attribute length for uptime";
                }
                break;
            case mactelnet::mndp::wire::AttributeType::software_id:
                server.software_id = std::string(buffer+offset, attr_length);
                break;
            case mactelnet::mndp::wire::AttributeType::hardware:
                server.hardware = std::string(buffer+offset, attr_length);
                break;
            case mactelnet::mndp::wire::AttributeType::ipv6_address:
                if (attr_length==16) {
                    boost::asio::ip::address_v6::bytes_type bytes;
                    std::copy(buffer+offset, buffer+offset+16, bytes.begin());
                    server.ipv6 = boost::asio::ip::address_v6(bytes);
                } else {
                    BOOST_LOG_TRIVIAL(warning) << "Packet from " << sender_address << " has invalid attribute length for IPv6 address";
                }
                break;
            case mactelnet::mndp::wire::AttributeType::interface_name:
                server.interface_name = std::string(buffer+offset, attr_length);
                break;
            default:
                break;
        }
        offset += attr_length;
    }
    
    using namespace std;
    cout
        << "Platform: " << server.platform << endl
        << "Identity: " << server.identity << endl
        << "Software id: " << server.software_id << endl
        << "IPv4: " << server.ipv4 << endl
        << "IPv6: " << server.ipv6 << endl;
        ;
}


void SendDiscoveryRequest(udp::socket &socket) {
    udp::endpoint remote_endpoint(boost::asio::ip::address_v4::broadcast(), mndp_port);
    uint32_t r[1] { 0 };
    socket.send_to(boost::asio::buffer(r), remote_endpoint);
    
}


int main(int argc, const char * argv[]) {
    logging::core::get()->set_filter(logging::trivial::severity>=logging::trivial::info);
    
    boost::asio::io_service io_service;
    auto socket = CreateSocket(io_service);
    
    char buf[1500];
    udp::endpoint sender_endpoint;
    
    boost::asio::steady_timer timer(io_service,
                                    std::chrono::steady_clock::now() + std::chrono::seconds(1));
    timer.async_wait(
                     [&](const boost::system::error_code &) {
                         socket.cancel();
                     });
    
    std::function<void(const boost::system::error_code &, std::size_t)> rcv_handler;
    rcv_handler = [&](const boost::system::error_code &, std::size_t len) {
        ParseMNDP(buf, len, sender_endpoint.address());
        socket.async_receive_from(boost::asio::buffer(buf), sender_endpoint, rcv_handler);
    };

    socket.async_receive_from(boost::asio::buffer(buf), sender_endpoint, rcv_handler);
    
    SendDiscoveryRequest(socket);

    io_service.run();
    return 0;
}
