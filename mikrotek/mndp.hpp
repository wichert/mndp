//
//  mndp.hpp
//  mactelnet
//
//  Created by Wichert Akkerman on 26/01/16.
//  Copyright © 2016 Wichert Akkerman. All rights reserved.
//

#ifndef mndp_hpp
#define mndp_hpp

#include <memory>
#include <string>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/udp.hpp>


namespace mikrotik {
    
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
            constexpr unsigned short port = 5678;

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
            
            void sendDiscoveryRequest(boost::asio::ip::udp::socket &socket);
            std::shared_ptr<Server> parseMNDP(const char* buffer, size_t len, boost::asio::ip::address sender_address);

        }
    }
}

#endif /* mndp_hpp */
