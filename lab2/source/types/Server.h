#pragma once

class Server {
public:
    virtual ~Server() = default;

    virtual void serve() = 0;
};
