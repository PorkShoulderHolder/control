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

#define BUFSIZE 1024


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
        std::cout << "new udp socket created" << std::endl;
    }
    if(NetworkManager::backend_sock == 0) {
        NetworkManager::backend_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (NetworkManager::backend_sock < 0) {
            error((char *) "ERROR opening socket");
        }
        std::cout << "new tcp socket created" << std::endl;
    }
}

bool send_to(char *hostname, char *msg, int port, int socket_ptr) {

}

bool NetworkManager::send_udp(char *hostname, char *msg, int port) {
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

char* NetworkManager::send_tcp(char *hostname, char *msg, int port, char* buf) {
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    static bool try_once = true;
    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stdout,"ERROR, no such host as %s\n", hostname);
        return false;
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, (size_t)server->h_length);
    serveraddr.sin_port = htons(port);

    if(try_once){
        if (connect(NetworkManager::backend_sock ,(struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
            error("ERROR connecting");
        try_once = false;
    }

    if (write(NetworkManager::backend_sock, msg, strlen(msg)) < 0)
        error("ERROR writing to socket");
    bzero(buf, BUFSIZE);
    if(read(NetworkManager::backend_sock, buf, BUFSIZE) < 0)
        error("ERROR reading from socket");

    fprintf(stdout,"--- %s\n", buf);
    return buf;
}

int NetworkManager::sock = 0;
int NetworkManager::backend_sock = 0;
