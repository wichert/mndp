//
//  wire.hpp
//  mactelnet
//
//  Created by Wichert Akkerman on 26/01/16.
//  Copyright © 2016 Wichert Akkerman. All rights reserved.
//

#ifndef wire_hpp
#define wire_hpp

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>

namespace mikrotik {
    namespace internals {
        
        template<typename T>
        T fromNetwork(const char* buffer);
        
        
        template<>
        uint16_t fromNetwork(const char* buffer);
    }
    
    boost::asio::ip::udp::socket CreateSocket(boost::asio::io_service &io_service, unsigned short port);
}



#endif /* wire_hpp */
