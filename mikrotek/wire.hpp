//
//  wire.hpp
//  mactelnet
//
//  Created by Wichert Akkerman on 26/01/16.
//  Copyright © 2016 Wichert Akkerman. All rights reserved.
//

#ifndef wire_hpp
#define wire_hpp


namespace mikrotik {
    namespace internals {
        
        template<typename T>
        T fromNetwork(const char* buffer);
        
        
        template<>
        uint16_t fromNetwork(const char* buffer);
    }
}



#endif /* wire_hpp */
