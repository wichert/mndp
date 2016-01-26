//
//  wire.cpp
//  mactelnet
//
//  Created by Wichert Akkerman on 26/01/16.
//  Copyright © 2016 Wichert Akkerman. All rights reserved.
//

#include <arpa/inet.h>
#include "wire.hpp"



template<>
uint16_t mikrotik::internals::fromNetwork(const char* buffer) {
    auto v = *reinterpret_cast<const uint16_t*>(buffer);
    return ntohs(v);
}
