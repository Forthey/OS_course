#pragma once
#include <string>

#include "Observer/Message.h"

class FileChangedInd : public Message {
public:
    enum class Action {
        CREATED,
        DELETED,
        MODIFIED,
    };

    explicit FileChangedInd(std::string fileName, Action action) : fileName(std::move(fileName)), action(action) {
    }

    std::string const fileName;
    Action action;
};

class ReloadConfigRequest : public Message {};

class StopRequest : public Message {};
