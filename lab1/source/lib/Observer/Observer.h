#pragma once
#include <memory>

#include "Message.h"

class Observer {
public:
    virtual ~Observer() = default;

    virtual void put(std::shared_ptr<Message> message) = 0;
};

