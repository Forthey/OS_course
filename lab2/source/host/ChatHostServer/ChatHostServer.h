#pragma once
#include <memory>

#include "Types/Server.h"

class ChatHostServer : public Server {
public:
    ChatHostServer();
    ~ChatHostServer() override;

    void serve() override;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};
