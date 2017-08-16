#include <zmqpp/zmqpp.hpp>
#include "header.h"

void listen(zmqpp::socket& s,std::string& songsPackage,std::unordered_map<std::string,std::vector<char>>& songs){
  std::unordered_map<std::string,std::vector<size_t>> clients;
  size_t sizePackage=0, i=0, songParts=0;
  zmqpp::message clientRequest, reply;
  std::string id, op, songName, clientKey;
  std::vector<char> package;
  while(true){
    s.receive(clientRequest);
    // assert(clientRequest.parts() == 3);
    clientRequest >> id >> op >> songName;
    if(op == "list") reply << songsPackage;
    else if(op == "play")
      if(songs.find(songName) != songs.end()){
        clientKey = id + " " + songName;
        if(clients.find(clientKey) != clients.end()){ //client exist
          i = clients[clientKey].at(0);
          songParts = clients[clientKey].at(1);
          package = getPackage(songs[songName],i,512000,sizePackage);
          songParts--;
          reply << songParts;
          reply.add_raw(package.data(),sizePackage);
          if(songParts != 0){clients[clientKey].at(0) = i; clients[clientKey].at(1) = songParts;}
          else clients.erase(clientKey);
        }
        else{ // client doesn't exist
          double parts = songs[songName].size() / 512000;
          i=0;
          songParts = std::ceil(parts);
          package = getPackage(songs[songName],i,512000,sizePackage);
          if(songParts > 1){
            songParts--;
            std::vector<size_t> clientInfo(i,songParts);
            clients[clientKey] = clientInfo;
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
