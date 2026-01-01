#pragma once
#include "Conn.h"

class Conn_Fifo : public Conn {
public:
    Conn_Fifo(bool isHost, int connId, std::string fifoReadPath, std::string fifoWritePath);

    ~Conn_Fifo() override;

    std::expected<BufferType, ConnReadError> read() override;

    ConnWriteResult write(const BufferType& buffer) override;

    IdType connId() const override { return m_connId; }

private:
    bool m_isHost;
    int m_readFd;
    int m_writeFd;
    int m_connId;
    std::string m_fifoReadPath;
    std::string m_fifoWritePath;
};
