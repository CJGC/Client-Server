#include <zmqpp/zmqpp.hpp>
#include <string>
#include <iostream>

using namespace zmqpp;
using namespace std;
typedef string str;

int main () {
    //  Prepare our context and socket
    context ctx;
    socket sock(ctx, socket_type::rep);
    sock.bind ("tcp://*:7777");

    while (true) {
        message request, reply;
        sock.receive(request);
        str id, keysdomain;
        request >> id >> keysdomain;
        cout <<"Node id = "<<id<< "\nKeys domain = "<<keysdomain<<endl<<endl;
        reply << "ok";
        sock.send(reply);
    }
    return 0;
}
