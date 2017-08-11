#include <zmqpp/zmqpp.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include "header.h"

void listen(zmqpp::socket &s){
  while(true) {
    zmqpp::message requestPack, response;
    std::string operation;
    int operand1, operand2, answer = 0;
    s.receive(requestPack);

    /* each message has three parts: <operation|operand1|operand2> */
    assert(requestPack.parts() == 3);

    requestPack >> operation >> operand1 >> operand2;
    std::cout << "Working on " << operation << std::endl;
    if(operation == "add") answer = operand1 + operand2;
    else if(operation == "sub") answer = operand1 - operand2;
    else std::cout << "Invalid operation requested!!\n";

    /* replying to client */
    std::cout << "Sending response  " << answer << std::endl;
    response << answer;
    s.send(response);
  }
}

int main() {
  /* making socket and contex */
  zmqpp::context ctx;
  zmqpp::socket s(ctx, zmqpp::socket_type::rep);

  /* storing files names with desired format */
  std::vector<const char *> files;
  getFiles(files,".cpp");

  /* binding socket with given tcp port */
  std::cout << "Binding socket to tcp port 5555\n";
  s.bind("tcp://*:5555");
  std::cout << "Start serving requests!\n";

  /* listening client's requests */
  listen(s);
  return 0;
}
