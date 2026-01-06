#pragma once
#include <messages.pb.h>

class MessageMaker {
public:
    static chat::Message makeBroadcast(std::uint64_t fromId, std::string_view text);
    static chat::Message makePrivate(std::uint64_t fromId, uint64_t toId, std::string_view text);
    static chat::Message makeSystemJoin(std::uint64_t clientId);
    static chat::Message makeSystemLeave(std::uint64_t clientId);
    static chat::Message makeSystemKillNotice(std::uint64_t clientId);
};
