#include "tcp.h"

TCPSERVER::TCPSERVER(std::string ip_address, int port) :
    m_ip_address{ip_address},
    m_port{port}
{
}

TCPSERVER::~TCPSERVER()
{
    delete m_server_state;
}

bool TCPSERVER::start()
{
    m_server_state = tcp_server_init();
    if(m_server_state == NULL){
        return false;
    }

    std::string buffer_str{"HTTP/1.0 200 OK\r\nContent-type: application/json\r\n\r\n"};
    for(int i=0; i < buffer_str.size(); i++) {
        m_server_state->buffer_sent[i] = buffer_str[i];
    }
    m_server_state->buffer_send_len = buffer_str.size();
    return tcp_server_open(static_cast<void *>(m_server_state), m_port);
}

