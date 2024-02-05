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

    std::string header_str{"HTTP/1.0 200 OK\r\nContent-type: application/json\r\n\r\n"};
    std::string body_str{"{\"moisture\":\"5\"}"};
    for(int i=0; i < header_str.size(); i++) {
        m_server_state->buffer_sent[0][i] = header_str[i];
    }
    for(int i=0; i < body_str.size(); i++) {
        m_server_state->buffer_sent[1][i] = body_str[i];
    }
    m_server_state->buffer_send_len[0] = header_str.size();
    m_server_state->buffer_send_len[1] = body_str.size();
    m_server_state->packages_send_len = 2;

    return tcp_server_open(static_cast<void *>(m_server_state), m_port);
}

