#ifndef __NETWORK_TCP__
#define __NETWORK_TCP__

extern "C"
{
#include "tcp_server.h"
}

#include <string>

class TCPSERVER {
    public:
        TCPSERVER(std::string ip_address, int port);
        ~TCPSERVER();

        bool start();

    private:
        std::string m_ip_address;
        int m_port;
        TCP_SERVER_T *m_server_state;
};

#endif
