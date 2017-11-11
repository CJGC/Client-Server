#include <zmqpp/zmqpp.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
//#include "sha1.hh"

using namespace zmqpp;
using namespace std;
typedef string str;
using vec = vector<str>;
using _map = map<str,vec>;

class tracker{

  private:
    _map keys;
    str id, port, ip, idAfter;
    bool amIFirst, amILast;

  public:
    tracker(str ip,str port,str id){
      this->ip = ip;
      this->port = port;
      this->id = id;
    }

    str getId(){ return id;}
    str getPort(){ return port;}
    str getIp(){ return ip;}
    str getIdAfter(){ return idAfter;}
    void setIdAfter(str id){ idAfter = id;}
    void setAmILast(bool value){ amILast = value;}
    void setAmIFirst(bool value){ amIFirst = value;}
};

int main(int argc,const char **argv){
    if(argc != 2){
      cerr << "usage "<<argv[0]<<" <string>\n";
      return -1;
    }
    return 0;
}
