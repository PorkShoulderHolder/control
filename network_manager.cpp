//
// Created by Sam Royston on 12/18/16.
//

#include <iosfwd>
#include "network_manager.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>



void error(char *msg) {
    perror(msg);
    exit(0);
}


NetworkManager::NetworkManager() {
    /* create single socket to be shared among devices */
    if(NetworkManager::sock == 0) {
        NetworkManager::sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (NetworkManager::sock < 0) {
            error((char *) "ERROR opening socket");
        }
        std::cout << "new socket created" << std::endl;
    }
}

bool NetworkManager::send_to(char *hostname, char* msg, int port) {
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        return false;
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, (size_t)server->h_length);
    serveraddr.sin_port = htons(port);

    /* send the message to the server */
    serverlen = sizeof(serveraddr);

    int success = (int)sendto(NetworkManager::sock, msg, strlen(msg), 0,
                              (struct sockaddr *)&serveraddr, (socklen_t)serverlen);

    return success >= 0;
}

int NetworkManager::sock = 0;
