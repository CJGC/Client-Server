#include <zmqpp/zmqpp.hpp>
#include "header.h"

void listen(zmqpp::socket& s,std::string& songsPackage,std::unordered_map<std::string,std::vector<char>>& songs){
  std::unordered_map<std::string,std::vector<size_t>> clients;
  size_t sizePackage=0, i=0, songParts=0;
  zmqpp::message clientRequest, reply;
  std::string id, op, songName;
  //std::vector<char> package;
  while(true){
    s.receive(clientRequest);
    // std::cout<< clientRequest.parts();
    assert(clientRequest.parts() == 3);
    clientRequest >> id >> op >> songName;
    if(op == "list") reply << songsPackage;
    else if(op == "play")
      if(songs.find(songName) != songs.end()){
        if(clients.find(id) != clients.end()){ //client exist
          i = clients[id].at(0);
          songParts = clients[id].at(1);
          std::vector<char> package = getPackage(songs[songName],i,512000,sizePackage);
          std::cout<<sizePackage<<"\n";
          songParts--;
          reply << songParts;
          reply.add_raw(package.data(),sizePackage);
          if(songParts != 0){clients[id].at(0) = i; clients[id].at(1) = songParts;}
          else clients.erase(id);
        }
        else{ // client doesn't exist
          double parts = double(songs[songName].size()) / double(512000);
          i=0;
          songParts = std::ceil(parts);
          std::vector<char> package = getPackage(songs[songName],i,512000,sizePackage);
          if(songParts > 1){
            songParts--;
            std::vector<size_t> clientInfo(2); clientInfo.at(0) = i; clientInfo.at(1) = songParts;
            clients[id] = clientInfo;
          }
          reply << songParts;
          reply.add_raw(package.data(),sizePackage);
        }
      }
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
