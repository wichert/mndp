//
//  main.cpp
//  mactelnet
//
//  Created by Wichert Akkerman on 09/02/16.
//  Copyright © 2016 Wichert Akkerman. All rights reserved.
//

#include <iostream>
#include <memory>
#include <string>
#include <iostream>
#include <chrono>
#include <arpa/inet.h>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/program_options.hpp>
#include "../mikrotek/telnet.hpp"
#include "../mikrotek/wire.hpp"


using namespace std;
namespace logging = boost::log;
namespace po = boost::program_options;
namespace ip = boost::asio::ip;
namespace telnet = mikrotik::telnet;
namespace wire = mikrotik::telnet::wire;


int main(int argc, const char * argv[]) {
    int timeout { 10 };
    string server;
    
    po::options_description options("Command line options");
    options.add_options()
        ("help,h", "Show help message")
        ("timeout,t", po::value<int>(&timeout), "Timeout to wait for replies")
        ("server", po::value<string>(&server), "MAC address of server")
        ;
    po::positional_options_description positional;
    positional.add("server", 1);
    
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(options).run(), vm);
    po::notify(vm);
    
    if (vm.count("help")) {
        cout << options << endl;
        return 0;
    }
    
    if (!vm.count("server")) {
        cerr << "No server provided" << endl;
        return 1;
    }
    
    logging::core::get()->set_filter(logging::trivial::severity>=logging::trivial::debug);
    
    boost::asio::io_service io_service;
    auto socket = mikrotik::CreateSocket(io_service, wire::port);

    return 0;
}
