#pragma once
#include <memory>

class ChatServer {
public:
    ChatServer();
    ~ChatServer();

    void serve();

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};
