#pragma once
#include <memory>

#include "Server.h"

class ChatHostServer final : public Server {
public:
    ChatHostServer();
    ~ChatHostServer() override;

    void serve() override;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};
