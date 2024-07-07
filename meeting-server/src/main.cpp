#include "server.h"


int main() {
    Server* server = Server::getInstance();
    server->start(8888);
    delete server;
    return 0;
}