#pragma once
#include <cstdint>
#include <boost/asio.hpp>
#include <memory>
#include <memory>

namespace net = boost::asio;
using udp = net::ip::udp;
using boost::system::error_code;

class UdpServer : public std::enable_shared_from_this<UdpServer>
{
public:
UdpServer(net::io_context &ioc, uint16_t port);

void start();
void registerClient(uint32_t id, udp::endpoint ep);
void unregisterClient(uint32_t id);
void broadcastSnapshot();

private:
void doReceive();

udp::socket sock_;
udp::endpoint sender_;
std::array<uint8_t, 64> recv_{};
};