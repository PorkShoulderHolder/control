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
public:
    NetworkManager();
    bool send_to(char *host, char* msg, int port);
};


#endif //CONTROL_NETWORK_MANAGER_H
