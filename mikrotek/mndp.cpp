//
//  mndp.cpp
//  mactelnet
//
//  Created by Wichert Akkerman on 26/01/16.
//  Copyright © 2016 Wichert Akkerman. All rights reserved.
//

#include <boost/log/trivial.hpp>
#include "mndp.hpp"
#include "wire.hpp"


using namespace std;
namespace ip = boost::asio::ip;;
namespace wire = mikrotik::mndp::wire;
namespace mndp = mikrotik::mndp;
using namespace mikrotik::internals;



void wire::sendDiscoveryRequest(ip::udp::socket &socket) {
    BOOST_LOG_TRIVIAL(info) << "Sending discovery request";
    ip::udp::endpoint remote_endpoint(ip::address_v4::broadcast(), wire::port);
    uint32_t r[1] { 0 };
    socket.send_to(boost::asio::buffer(r), remote_endpoint);
    
}



shared_ptr<mndp::Server> wire::parseMNDP(const char* buffer, size_t len, ip::address sender_address) {
    if (len<18) {
        BOOST_LOG_TRIVIAL(info) << "Ignoring short packet (" << len << " bytes)";
        return nullptr;
    }
    
    auto server = make_shared<mndp::Server>();
    auto header = reinterpret_cast<const wire::header*>(buffer);
    size_t offset { sizeof(*header) };
    
    
    if (sender_address.is_v4())
        server->ipv4 = sender_address.to_v4();
    else if (sender_address.is_v6())
        server->ipv6 = sender_address.to_v6();
    
    while (offset + 4 < len) {
        auto attr_type = static_cast<AttributeType>(fromNetwork<uint16_t>(buffer+offset));
        offset += 2;
        auto attr_length = fromNetwork<uint16_t>(buffer+offset);
        offset += 2;
        if (offset + attr_length > len)
            // Invalid attribute length, would overflow.
            return nullptr;
        switch (attr_type) {
            case wire::AttributeType::mac_address:
                if (attr_length==6)
                    memcpy(server->mac, buffer+offset, 6);
                else {
                    BOOST_LOG_TRIVIAL(warning) << "Packet from " << sender_address << " has invalid attribute length for MAC address";
                }
                break;
            case wire::AttributeType::identity:
                server->identity = string(buffer+offset, attr_length);
                break;
            case wire::AttributeType::version:
                server->version = string(buffer+offset, attr_length);
                break;
            case wire::AttributeType::platform:
                server->platform = string(buffer+offset, attr_length);
                break;
            case wire::AttributeType::uptime:
                if (attr_length==4)
                    server->uptime = chrono::seconds(*reinterpret_cast<const uint32_t*>(buffer+offset));
                else {
                    BOOST_LOG_TRIVIAL(warning) << "Packet from " << sender_address << " has invalid attribute length for uptime";
                }
                break;
            case wire::AttributeType::software_id:
                server->software_id = string(buffer+offset, attr_length);
                break;
            case wire::AttributeType::hardware:
                server->hardware = string(buffer+offset, attr_length);
                break;
            case wire::AttributeType::ipv6_address:
                if (attr_length==16) {
                    ip::address_v6::bytes_type bytes;
                    copy(buffer+offset, buffer+offset+16, bytes.begin());
                    server->ipv6 = ip::address_v6(bytes);
                } else {
                    BOOST_LOG_TRIVIAL(warning) << "Packet from " << sender_address << " has invalid attribute length for IPv6 address";
                }
                break;
            case wire::AttributeType::interface_name:
                server->interface_name = string(buffer+offset, attr_length);
                break;
            default:
                break;
        }
        offset += attr_length;
    }
    
    return server;
}
