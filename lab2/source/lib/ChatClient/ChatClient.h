#pragma once
#include <messages.pb.h>

#include "Conns/Conn.h"
#include "SignalHandler/ISignalHandler.h"

class ChatClient {
public:
    ChatClient(std::unique_ptr<ISignalHandler> sigHandler);
    ChatClient(std::unique_ptr<ISignalHandler> sigHandler, pid_t serverPid, ClientConnType type);
    virtual ~ChatClient() = default;

    void setConnAndId(std::unique_ptr<Conn> conn, std::uint64_t id);

    bool isRunning() const { return m_isRunning; }

    void doPollStep();

    void runInteractiveWriting();

    virtual void runReadLoop();

protected:
    virtual void onMessage(const chat::Message& msg);

    void sendPrivateMessage(const std::string& text, std::uint64_t toId);
    void sendBroadcastMessage(const std::string& text);

    void printPrivateMessage(std::chrono::system_clock::time_point timePoint, std::uint64_t from,
                             const std::string& msg);
    void printBroadcastMessage(std::chrono::system_clock::time_point timePoint, std::uint64_t from,
                               const std::string& msg);

    std::atomic<bool> m_isRunning{false};

    std::uint64_t m_id{};
    std::unique_ptr<Conn> m_conn;

    pid_t m_serverPid{};

    std::unique_ptr<ISignalHandler> m_sigHandler;
};
