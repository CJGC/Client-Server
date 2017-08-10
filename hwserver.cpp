#include <zmqpp/zmqpp.hpp>
#include <iostream>
//#include <string>
#include <cassert>

int main() {
  /* making socket and contex */
  zmqpp::context ctx;
  zmqpp::socket s(ctx, zmqpp::socket_type::rep);

  /* binding socket with given tcp port */
  std::cout << "Binding socket to tcp port 5555\n";
  s.bind("tcp://*:5555");
  std::cout << "Start serving requests!\n";

  while(true) {
    /* building package that will contain client message */
    zmqpp::message m;
    s.receive(m);

    /* each message has three parts: <operation|operand1|operand2> */
    assert(m.parts() == 3);

    /* processing package */
    std::string operation;
    int operand1, operand2, answer = 0;

    m >> operation >> operand1 >> operand2;
    std::cout << "Working on  " << operation << std::endl;
    if(operation == "add") answer = operand1 + operand2;
    else if(operation == "sub") answer = operand1 - operand2;
    else std::cout << "Invalid operation requested!!\n";

    /* replying to client */
    std::cout << "Sending response  " << answer << std::endl;
    zmqpp::message response;
    response << answer;
    s.send(response);
  }

  std::cout << "Finished\n";
  return 0;
}
