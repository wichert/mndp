//
//  main.cpp
//  mactelnet
//
//  Created by Wichert Akkerman on 23/01/16.
//  Copyright © 2016 Wichert Akkerman. All rights reserved.
//

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
#include "../mikrotek/mndp.hpp"
#include "../mikrotek/wire.hpp"


using namespace std;
namespace logging = boost::log;
namespace po = boost::program_options;
namespace ip = boost::asio::ip;
namespace mndp = mikrotik::mndp;
namespace wire = mikrotik::mndp::wire;


void DisplayServer(std::shared_ptr<mndp::Server> server) {
    cout
        << "Platform: " << server->platform << endl
        << "Identity: " << server->identity << endl
        << "Software id: " << server->software_id << endl
        << "IPv4: " << server->ipv4 << endl
        << "IPv6: " << server->ipv6 << endl
        ;
}


int main(int argc, const char * argv[]) {
    int timeout { 10 };
    bool passive { false };
    
    po::options_description options("Command line options");
    options.add_options()
        ("help,h", "Show help message")
        ("timeout,t", po::value<int>(&timeout), "Timeout to wait for replies")
        ("passive,p", po::value<bool>(), "Do not send MNDP discovery request")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, options), vm);
    po::notify(vm);
    
    if (vm.count("help")) {
        std::cout << options << std::endl;
        return 0;
    }
              
    logging::core::get()->set_filter(logging::trivial::severity>=logging::trivial::debug);
    
    boost::asio::io_service io_service;
    auto socket = mikrotik::CreateSocket(io_service, wire::port);
    
    char buf[1500];
    ip::udp::endpoint sender_endpoint;
    
    if (timeout>=0) {
        BOOST_LOG_TRIVIAL(debug) << "Will stop after " << timeout << " seconds";
        boost::asio::steady_timer timer(io_service,
                                        std::chrono::steady_clock::now() + std::chrono::seconds(timeout));
        timer.async_wait(
                         [&](const boost::system::error_code &e) {
                             BOOST_LOG_TRIVIAL(debug) << "Stopping";
                             socket.cancel();
                         });
    }
    
    std::function<void(const boost::system::error_code &, std::size_t)> rcv_handler;
    rcv_handler = [&](const boost::system::error_code &, std::size_t len) {
        auto server = wire::parseMNDP(buf, len, sender_endpoint.address());
        if (server)
            DisplayServer(server);
        socket.async_receive_from(boost::asio::buffer(buf), sender_endpoint, rcv_handler);
    };

    socket.async_receive_from(boost::asio::buffer(buf), sender_endpoint, rcv_handler);
    
    if (!passive)
        wire::sendDiscoveryRequest(socket);

    io_service.run();
    return 0;
}
