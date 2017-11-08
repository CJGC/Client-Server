#include <zmqpp/zmqpp.hpp>
#include <string>
#include <iostream>

using namespace zmqpp;
using namespace std;
typedef string str;

int main(int argc,const char **argv){
    if(argc != 2){
      cerr << "usage "<<argv[0]<<" <string>\n";
      return -1;
    }
    // cout << sha1(argv[1]) << endl;
    //  Prepare our context and socket
    // context ctx;
    // socket sock(ctx,socket_type::req);
    //
    // cout << "Connecting to hello world server…" << std::endl;
    // sock.connect ("tcp://localhost:5555");
    //
    // //  Do 10 requests, waiting each time for a response
    // for (int request_nbr = 0; request_nbr != 10; request_nbr++) {
    //     message request;
    //     request << "node 1";
    //     sock.send (request);
    //
    //     //  Get the reply.
    //     message reply;
    //     sock.receive(reply);
    //     str rep;
    //     reply >> rep;
    //     cout << "reply " << rep << std::endl;
    // }
    return 0;
}
