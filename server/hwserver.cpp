#include <zmqpp/zmqpp.hpp>
#include "header.h"

void listen(zmqpp::socket& s,std::string& songsPackage,std::unordered_map<std::string,std::vector<char>>& songs){
  while(true){
    zmqpp::message clientRequest, reply;
    std::string op, songName;
    s.receive(clientRequest);
    assert(clientRequest.parts() == 2);
    clientRequest >> op >> songName;
    if(op == "list") reply << songsPackage;
    else if(op == "play")
      if(songs.find(songName) != songs.end()) reply.add_raw(songs[songName].data(), songs[songName].size());
      else reply << "Desired song could not be found!";
    else reply << "Invalid operation requested!\n";
    s.send(reply);
  }
}

int main(int argc,const char *argv[]){
  if(argc != 2){
    std::cerr <<"There should be two arguments: <"<< argv[0]<<"> <dir path/>\n";
    return -1;
  }
  std::string path = argv[1];

  /* making socket and contex */
  zmqpp::context ctx;
  zmqpp::socket s(ctx, zmqpp::socket_type::rep);

  /* getting songs filename */
  std::vector<std::string> files;
  if(getFiles(files,".ogg",path.c_str()) != 0){
    std::cerr<<"Was not possible open <"<<path<<">!";
    return -1;
  }

  /* building songs package + unordered_map that will contain songs */
  std::string songsPackage("");
  std::unordered_map<std::string,std::vector<char>> songs;
  for(std::string song:files){
    songsPackage+=song+"\n";
    songs[song] = readFileToBytes(path+song);
  }

  /* binding socket with given tcp port */
  std::cout << "Binding socket to tcp port 5555\n";
  s.bind("tcp://*:5555");
  std::cout << "Start serving requests!\n";

  /* listening client's requests */
  listen(s,songsPackage,songs);
  return 0;
}
