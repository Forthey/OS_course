#pragma once
#include <chrono>
#include <memory>

#include "Conns/Conn_Fifo.h"
#include "Conns/Conn_Mq.h"
#include "Conns/Conn_Sock.h"

class ClientConn {
public:
    ClientConn(std::uint64_t id, pid_t pid, ClientConnType type) : id{id}, pid{pid} {
        switch (type) {
            case ClientConnType::Fifo:
                conn = std::make_unique<Conn_Fifo>(true, id, std::format("{}_{}", FIFO_TO_HOST_CHANNEL_BASE, id),
                                                   std::format("{}_{}", FIFO_TO_CLIENT_CHANNEL_BASE, id));
                break;
            case ClientConnType::MessageQueue:
                conn = std::make_unique<Conn_Mq>(true, id, std::format("{}_{}", MQ_TO_HOST_CHANNEL_BASE, id),
                                                 std::format("{}_{}", MQ_TO_CLIENT_CHANNEL_BASE, id));
                break;
            case ClientConnType::Socket:
                conn = std::make_unique<Conn_Sock>(true, id, std::format("{}_{}", SOCKET_CHANNEL_BASE, id));
                break;
        }
    }

    const std::uint64_t id{};
    const pid_t pid{};
    std::unique_ptr<Conn> conn;
    std::chrono::system_clock::time_point lastMessageTime;
};
