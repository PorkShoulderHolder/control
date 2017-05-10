//
// Created by Sam Royston on 12/18/16.
//

#ifndef CONTROL_NETWORK_MANAGER_H
#define CONTROL_NETWORK_MANAGER_H

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

class NetworkManager {
static int sock;
static int backend_sock;
public:
    NetworkManager();
    bool send_udp(char *host, char *msg, int port);
    char* send_tcp(char *host, char *msg, int port, char *buf);
};


#endif //CONTROL_NETWORK_MANAGER_H
