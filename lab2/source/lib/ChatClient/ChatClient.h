#pragma once
#include <messages.pb.h>

#include <thread>

class ChatClient {
    using SendPrivateMessageCallback = std::function<void(std::uint64_t, std::string_view)>;
    using SendBroadcastMessageCallback = std::function<void(std::string_view)>;
    using OnSystemLeaveCallback = std::function<void()>;

public:
    ChatClient(SendPrivateMessageCallback sendPrivateMessage, SendBroadcastMessageCallback sendBroadcastMessage,
               OnSystemLeaveCallback onSystemLeave);

    void runInteractiveWriting();
    void stopInteractiveWriting();

    void onMessage(const chat::Message& msg);

    void printMyPrivateMessage(const chat::Message& msg) const;
    void printMyBroadcastMessage(const chat::Message& msg) const;
private:
    bool parseTextCommand(std::string_view text);

    void printPrivateMessage(std::chrono::system_clock::time_point timePoint, std::uint64_t from, std::string_view msg);
    void printBroadcastMessage(std::chrono::system_clock::time_point timePoint, std::uint64_t from,
                               std::string_view msg);
    void printSystemJoinMessage(std::chrono::system_clock::time_point timePoint, std::uint64_t clientId);
    void printSystemLeaveMessage(std::chrono::system_clock::time_point timePoint, std::uint64_t clientId);
    void printSystemKillMessage(std::chrono::system_clock::time_point timePoint, std::uint64_t clientId);

    SendPrivateMessageCallback m_sendPrivateMessage;
    SendBroadcastMessageCallback m_sendBroadcastMessage;
    OnSystemLeaveCallback m_onSystemLeave;
};
