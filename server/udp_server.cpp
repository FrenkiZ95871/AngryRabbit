#include "udp_server.h"

UdpServer::UdpServer(net::io_context &io, uint16_t port)
    : sock_(ioc, udp::endpoint(udp::v4(), port))
{
}

void UdpServer::start()
{
    doReceive();
}

void UdpServer::doReceive()
{
    auto self = shared_from_this();
    sock_.async_receive_from(
        net::buffer(recv_),
        sender_,
        [self](error_code ec, std::size_t n)
        {
            if (ec == net::error::operation_aborted)
                return;
            if (ec)
            {
                self->doReceive();
                return;
            }
            if (!ec && n >= 4)
            {
                uint32_t pid;
                std::memcpy(&pid, self->recv_.data(), 4);
                self->registerClient(pid, self->sender_);
            }
            self->doReceive();
        });
}