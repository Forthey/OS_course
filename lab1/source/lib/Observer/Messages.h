#pragma once
#include <string>
#include <filesystem>

#include "Observer/Message.h"

class FileChangedInd : public Message {
public:
    enum class Action {
        CREATED,
        DELETED,
        MODIFIED,
    };

    explicit FileChangedInd(std::filesystem::path directory, std::string fileName,
                            Action action) : directory(std::move(directory)),
                                             fileName(std::move(fileName)),
                                             action(action) {
    }

    std::filesystem::path directory;
    std::string const fileName;
    Action action;
};

class ReloadConfigRequest : public Message {
};

class StopRequest : public Message {
};
