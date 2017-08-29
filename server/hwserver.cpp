#include <zmqpp/zmqpp.hpp>
#include "header.h"

using namespace zmqpp;
using namespace std;

class Server{
  private:
    message cliReq, reply;
    unsigned int part;
    string op, song, songsPackage, dir;

    void buildSongsPackage(vector<string>& songs){
      /* it will build available songs package */
      for(string& song: songs) songsPackage+=song+"\n";
    }

  public:
    Server(string& dir){
      vector<string> songs;
      if(getSongs(songs,".ogg",dir.c_str()) != 0) exit(1);
      buildSongsPackage(songs);
      this->dir = dir;
    }

    void listen(socket& s){
      /* it will listen client's request */
      while(true){
        s.receive(cliReq);
        cliReq >> op >> song >> part;
        if(op == "getSongPart"){
          size_t partSize = 0;
          vector<char> songPart = getSongPart(dir,song,512000,part,partSize);
          reply.add_raw(songPart.data(),partSize);
        }
        else if(op == "list") reply << songsPackage;
        else if(op == "songParts") reply << songParts(dir,song,512000);
        s.send(reply);
      }
    }
};

int main(int argc,const char *argv[]){
  if(argc != 2){
    cerr <<"There should be two arguments: <"<< argv[0]<<"> <dir path/>\n";
    return -1;
  }
  string path = argv[1];
  context ctx;
  socket s(ctx, socket_type::rep);
  Server server(path);
  cout << "Binding socket to tcp port 5555\n";
  s.bind("tcp://*:5555");
  cout << "listening...!\n";
  server.listen(s);
  return 0;
}
