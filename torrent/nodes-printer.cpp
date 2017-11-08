#include <zmqpp/zmqpp.hpp>
#include <string>
#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep(n)    Sleep(n)
#endif

using namespace zmqpp;
using namespace std;
typedef string str;

int main () {
    //  Prepare our context and socket
    context ctx;
    socket sock(ctx, socket_type::rep);
    sock.bind ("tcp://*:5555");

    while (true) {
        message request;

        //  Wait for next request from client
        sock.receive(request);
        str node_id;
        request >> node_id;
        std::cout << node_id << std::endl;

        //  Do some 'work'
        sleep(1);

        //  Send reply back to client
        message reply;
        reply << "ok";
        sock.send(reply);
    }
    return 0;
}
