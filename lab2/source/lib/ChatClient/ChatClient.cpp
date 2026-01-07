#include "ChatClient.h"

#include <thread>

#include "Types/Console.h"

ChatClient::ChatClient(SendPrivateMessageCallback sendPrivateMessage, SendBroadcastMessageCallback sendBroadcastMessage,
                       OnSystemLeaveCallback onSystemLeave)
    : m_sendPrivateMessage{std::move(sendPrivateMessage)}
    , m_sendBroadcastMessage{std::move(sendBroadcastMessage)}
    , m_onSystemLeave{std::move(onSystemLeave)} {}

void ChatClient::runInteractiveWriting() {
    consoleSrv().run([this](const std::string& text) {
        if (!parseTextCommand(text)) {
            consoleSrv().info("Unable to parse text command");
        }
    });
}

void ChatClient::stopInteractiveWriting() { consoleSrv().stop(); }

void ChatClient::printPrivateMessage(std::chrono::system_clock::time_point timePoint, std::uint64_t from,
                                     std::string_view msg) {
    consoleSrv().privateMsg(std::format("{:%d.%m.%Y %H:%M:%S} {} --> me: {}", timePoint, from, msg));
}

void ChatClient::printBroadcastMessage(std::chrono::system_clock::time_point timePoint, std::uint64_t from,
                                       std::string_view msg) {
    consoleSrv().broadcastMsg(std::format("{:%d.%m.%Y %H:%M:%S} {} --> all: {}", timePoint, from, msg));
}

void ChatClient::printSystemJoinMessage(std::chrono::system_clock::time_point timePoint, std::uint64_t clientId) {
    consoleSrv().info(std::format("{:%d.%m.%Y %H:%M:%S} {} joined chat", timePoint, clientId));
}

void ChatClient::printSystemLeaveMessage(std::chrono::system_clock::time_point timePoint, std::uint64_t clientId) {
    consoleSrv().info(std::format("{:%d.%m.%Y %H:%M:%S} {} left chat", timePoint, clientId));
}

void ChatClient::printSystemKillMessage(std::chrono::system_clock::time_point timePoint, std::uint64_t clientId) {
    consoleSrv().info(std::format("{:%d.%m.%Y %H:%M:%S} {} was killed due to inactivity", timePoint, clientId));
}

void ChatClient::printMyPrivateMessage(const chat::Message& msg) const {
    const auto timePoint = std::chrono::system_clock::time_point{std::chrono::seconds{msg.timestamp().seconds()}};
    consoleSrv().privateMsg(std::format("{:%d.%m.%Y %H:%M:%S} me --> {}: {}", timePoint, msg.chat_private().to_id(),
                                        msg.chat_private().message()));
}

void ChatClient::printMyBroadcastMessage(const chat::Message& msg) const {
    const auto timePoint = std::chrono::system_clock::time_point{std::chrono::seconds{msg.timestamp().seconds()}};
    consoleSrv().broadcastMsg(
        std::format("{:%d.%m.%Y %H:%M:%S} me --> all: {}", timePoint, msg.chat_broadcast().message()));
}

void ChatClient::onMessage(const chat::Message& msg) {
    const auto timePoint = std::chrono::system_clock::time_point{std::chrono::seconds{msg.timestamp().seconds()}};
    switch (msg.payload_case()) {
        case chat::Message::kChatBroadcast:
            printBroadcastMessage(timePoint, msg.chat_broadcast().from_id(), msg.chat_broadcast().message());
            break;
        case chat::Message::kChatPrivate:
            printPrivateMessage(timePoint, msg.chat_private().from_id(), msg.chat_private().message());
            break;
        case chat::Message::kSystemJoin:
            printSystemJoinMessage(timePoint, msg.system_join().client_id());
            break;
        case chat::Message::kSystemLeave:
            printSystemLeaveMessage(timePoint, msg.system_leave().client_id());
            break;
        case chat::Message::kSystemKillNotice:
            printSystemKillMessage(timePoint, msg.system_kill_notice().client_id());
            break;
        default:
            break;
    }
}

bool ChatClient::parseTextCommand(std::string_view text) {
    auto commandParts = std::views::split(text, ' ');
    std::size_t partsCount = std::ranges::distance(commandParts);

    if (partsCount == 0) {
        return false;
    }

    auto toStringView = [](auto&& sub) { return std::string_view(&*sub.begin(), std::ranges::distance(sub)); };
    auto partsIter = commandParts.begin();
    std::string_view part = toStringView(*partsIter);

    if (part == "/exit" && partsCount == 1) {
        m_onSystemLeave();
        return true;
    }

    if (part == "/b" && partsCount == 2) {
        // text
        ++partsIter;
        const auto tailBegin = (*partsIter).begin();
        std::string_view payloadText{tailBegin,
                                     static_cast<std::string_view::size_type>(std::distance(tailBegin, text.end()))};

        m_sendBroadcastMessage(payloadText);
        return true;
    }

    if (part == "/w" && partsCount == 3) {
        // id
        ++partsIter;
        part = toStringView(*partsIter);

        std::size_t id = 0;
        if (auto [_, ec] = std::from_chars(part.data(), part.data() + part.size(), id); ec != std::errc()) {
            return false;
        }

        // text
        ++partsIter;
        const auto tailBegin = (*partsIter).begin();
        std::string_view payloadText{tailBegin,
                                     static_cast<std::string_view::size_type>(std::distance(tailBegin, text.end()))};

        m_sendPrivateMessage(id, payloadText);
        return true;
    }

    return false;
}
