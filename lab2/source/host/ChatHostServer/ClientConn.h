#pragma once
#include <chrono>
#include <memory>

#include "Conns/Conn_Fifo.h"
#include "Conns/Conn_Mq.h"
#include "Conns/Conn_Sock.h"
#include "Timer/Timer.h"

class ClientConn {
    using OnInactiveCallback = std::function<void(std::uint64_t)>;

public:
    ClientConn(std::uint64_t id, pid_t pid, ClientConnType type, OnInactiveCallback onInactiveCb)
        : id{id}
        , pid{pid}
        , inactiveTimer{std::chrono::minutes{1}, [this] { onInactive(); }}
        , onInactiveCb{std::move(onInactiveCb)} {
        switch (type) {
            case ClientConnType::Fifo:
                conn = std::make_unique<Conn_Fifo>(
                    true, id, std::format("{}/{}_{}", std::getenv("HOME"), FIFO_TO_HOST_CHANNEL_BASE, id),
                    std::format("{}/{}_{}", std::getenv("HOME"), FIFO_TO_CLIENT_CHANNEL_BASE, id));
                break;
            case ClientConnType::MessageQueue:
                conn = std::make_unique<Conn_Mq>(true, id, std::format("{}_{}", MQ_TO_HOST_CHANNEL_BASE, id),
                                                 std::format("{}_{}", MQ_TO_CLIENT_CHANNEL_BASE, id));
                break;
            case ClientConnType::Socket:
                conn = std::make_unique<Conn_Sock>(
                    true, id, std::format("{}/{}_{}", std::getenv("HOME"), SOCKET_CHANNEL_BASE, id));
                break;
        }

        inactiveTimer.start();
    }

    const std::uint64_t id{};
    const pid_t pid{};
    std::unique_ptr<Conn> conn;
    std::chrono::system_clock::time_point lastMessageTime;
    Timer inactiveTimer;
    OnInactiveCallback onInactiveCb;

private:
    void onInactive() const { onInactiveCb(id); }
};
