//
// Created by Sam Royston on 12/18/16.
//

#include "bot.h"
#include <sys/socket.h>
#include "network_manager.h"


const char *M_COMMAND_STR[4] = { "r:::0", "l:::0", "r:::1", "l:::1"};

Bot::Bot(const char *host){
    this->port = 8888;
    this->host = (char *)host;
}

void Bot::set_target_location(cv::Point target) {

}

void Bot::apply_motor_commands(const MOTOR commands[2]) {
    NetworkManager *manager = new NetworkManager();
    for (int i = 0; i < 2; i++){
        manager->send_to(this->host, (char *)M_COMMAND_STR[commands[i]], this->port);
    }
}


