#pragma once
#include <memory>

#include "Types/Server.h"

class ChatClientServer : public Server {
public:
    explicit ChatClientServer(pid_t serverPid);

    ~ChatClientServer() override;

    void serve() override;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};
