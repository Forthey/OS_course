#include "ChatServer.h"

#include <unordered_map>

#include "Conns/Conn.h"

class ChatServer::Impl {
    struct ClientData {
        pid_t pid;
        std::unique_ptr<Conn> client;
        std::chrono::system_clock::time_point lastMessageTime;
    };

public:
    void serve();
private:
    std::unordered_map<std::uint64_t, ClientData> clients;
};

ChatServer::ChatServer() : m_impl{std::make_unique<Impl>()} {}

ChatServer::~ChatServer() = default;
