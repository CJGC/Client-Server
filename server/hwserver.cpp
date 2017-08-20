#include <zmqpp/zmqpp.hpp>
#include "header.h"

using namespace zmqpp;
using namespace std;

void listen(socket& s,string& songsPackage,string& musPath){
  message clientRequest, reply;
  unsigned int part = 0;
  string op, songName;
  while(true){
    s.receive(clientRequest);
    // assert(clientRequest.parts() == 3);
    clientRequest >> op >> songName >> part;
    if(op == "list") reply << songsPackage;
    else if(op == "getPart"){
      size_t sizePackage = 0;
      vector<char> package = getPackage(musPath,songName,512000,part,sizePackage);
      reply.add_raw(package.data(),sizePackage);
    }
    else if(op == "meetParts") reply << meetParts(musPath,songName,512000);
    else reply << "Invalid operation requested!\n";
    s.send(reply);
  }

}

int main(int argc,const char *argv[]){
  if(argc != 2){
    cerr <<"There should be two arguments: <"<< argv[0]<<"> <dir path/>\n";
    return -1;
  }
  string path = argv[1];

  /* making socket and contex */
  context ctx;
  socket s(ctx, socket_type::rep);

  /* getting songs filename */
  vector<string> files;
  if(getFiles(files,".ogg",path.c_str()) != 0){
    cerr<<"Was not possible open <"<<path<<">!";
    return -1;
  }

  /* building songs package + unordered_map that will contain songs */
  string songsPackage("");
  // unordered_map<string,vector<char>> songs;
  for(string song:files){
    songsPackage+=song+"\n";
    // songs[song] = readFileToBytes(path+song);
  }

  /* binding socket with given tcp port */
  cout << "Binding socket to tcp port 5555\n";
  s.bind("tcp://*:5555");
  cout << "Start serving requests!\n";
  /* listening client's requests */
  listen(s,songsPackage,path);
  return 0;
}
