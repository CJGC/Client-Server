#include <zmqpp/zmqpp.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
//#include "sha1.hh"

using namespace zmqpp;
using namespace std;
typedef string str;
using vec = vector<str>;
using _map = map<str,vec>;

class tracker{


  /* ---------------- server side ---------------- */
  private:
    str BeforeIP, BeforePort;

  public:
    tracker(str ip,str port,str id){
      this->ip = ip;
      this->port = port;
      this->id = id;
      this->AfterId = "none";
      this->AfterIp = "none";
      this->AfterPort = "none";
      this->BeforeIP = "none";
      this->BeforePort = "none";
      this->amILast = true;
      this->_exit = false;
    }

  /* ---------------- server side ---------------- */
  private:
    _map keys;
    str id, ip, port, AfterId, AfterIp, AfterPort;
    bool amILast, _exit;

    void server(socket& serv){
      /* it will simulate a sever into tracker */
      message request, reply;
      str op, idc, afIp, afPort;
      bool last;

      while(!_exit){
        serv.receive(request);
        request >> op >> idc >> afIp >> afPort >> last;

        if(op == "getinfo")
          this->getInfo(reply);

        else if(op == "getkeys")
          this->getKeys(reply,idc);

        else if(op == "setinfo")
          this->setInfo(reply,idc,afIp,afPort);

        else if(op == "setlast")
          this->setLast(reply,last);

        serv.send(reply);
      }

    }

    void setLast(message& package, bool& last){
      /* it will set up amIlast info */
      amILast = last;
      package << "ok";
    }

    void setInfo(message& package,str id,str ip,str port){
      /* it will set up all next node info */
      AfterId = id;
      AfterIp = ip;
      AfterPort = port;
      package << "ok";
    }

    void getInfo(message& package){
      /* it will get this node and next node info for requester client */
      str info = id + " " + AfterId + " " + AfterIp + " " + AfterPort;
      package << info;
    }

    void splitKeys(_map::iterator& f, _map::iterator& l, str& ckeys){
      /* it will split keys domain */
      for(auto& it = f; it != l;){
        str key = it->first;
        str ownerName = it->second[0], fileName = it->second[1];
        ckeys += key + " " + ownerName + " " + fileName + "\n";
        it = keys.erase(it);
      }
    }

    void getKeys(message& package, str idc){
      /* it will get all belonging keys for requester client */
      str ckeys = ""; // client keys

      if(amILast && id > idc && AfterId == "none"  || \
        amILast && id > idc && AfterId > idc){
        _map::iterator first = keys.lower_bound(idc);
        _map::iterator last = keys.lower_bound(id);
        splitKeys(first,last,ckeys);
        package << ckeys;
        return;
      }

      if(idc > id){
        _map::iterator first = keys.lower_bound(idc);
        _map::iterator last = keys.end();
        splitKeys(first,last,ckeys);
      }

      if(amILast && idc > id){
        _map::iterator first = keys.begin();
        _map::iterator last = keys.lower_bound(id);
        splitKeys(first,last,ckeys);
      }

      package << ckeys;
    }
};

int main(int argc,const char **argv){
    if(argc != 2){
      cerr << "usage "<<argv[0]<<" <string>\n";
      return -1;
    }
    return 0;
}
