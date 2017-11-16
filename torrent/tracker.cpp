#include <zmqpp/zmqpp.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <thread>
#include "getMachInfo.hh"
#include "loadFiles.hh"

using namespace zmqpp;
using namespace std;
typedef string str;
using vec = vector<str>;
using _map = map<str,vec>;

str localIp = "localhost", remoteIp = "localhost";

class tracker{

  /* ---- generic methods and attributes, these will be used by
          client and server side respectively --- */

  private:
    _map keys;
    str id, ip, port, remoteId, remoteIp, remotePort, amILast;

    void setKeys(const str& keys){
      /* it will set up keys domain */
      str key = "", ownerName = "", fileName = "", aux = "";

      for(char ch : keys)
        if(ch == ' ' && key == ""){
          key = aux;
          aux = "";
        }
        else if(ch == ' ' && ownerName == ""){
          ownerName = aux;
          aux = "";
        }
        else if(ch == ' ' && fileName == ""){
          fileName = aux;
          vec second;
          second.resize(2);
          second[0] = ownerName;
          second[1] = fileName;
          this->keys[key] = second;
          key = ""; ownerName = ""; fileName = ""; aux = "";
        }
        else aux += ch;
    }

    void setRemoteInfo(str id,const str& ip,const str& port){
      /* it will set up remote info */
      this->remoteId = id;
      this->remoteIp = ip;
      this->remotePort = port;
    }

  /* ---------------- client side ---------------- */

  public:
    tracker(str id,str ip,str port,str remoteIp,str remotePort,str ownFiles){
      this->ip = ip;
      this->port = port;
      this->id = id;
      setRemoteInfo("none",remoteIp,remotePort);
      this->amILast = "true";
      this->_exit = false;
      setKeys(ownFiles);
    }

    void client(socket& cli,socket& serv){
      /* it will simulate a cliento into tracker */
      bindWithChord(cli);
      str userOp;
      cout << "type something to exit: ";
      getline(cin,userOp);
      unbindWithChord(cli);
      serv.unbind("tcp://"+this->ip+":"+this->port);
    }

  private:
    void unbindWithChord(socket& cli){
      /* it will unbind with chord ring */
      message request, answer;
      str keys;
      getKeys(keys);
      request << "setinfo" << this->remoteId << this->remoteIp \
              <<this->remotePort << this->amILast << keys;
      cli.send(request);
      cli.receive(answer);
      this->_exit = true;
    }

    void bindWithChord(socket& cli){
      /* it will bind with chord ring */
      while(true){
        message request, answer;
        cli.connect("tcp://"+this->remoteIp+":"+this->remotePort);
        request << "getinfo";
        cli.send(request);
        cli.receive(answer);
        str remtId = "", remtNextId = "", remtNextIp = "", remtNextPort = "",\
            remtIsLast = "";
        answer >> remtId >> remtNextId >> remtNextIp >> remtNextPort \
               >> remtIsLast;

        if( (remtNextId == "none") \
        || (this->id > remtId && this->id < remtNextId) \
        || (remtIsLast == "true" && this->id > remtId) \
        || (remtIsLast == "true" && this->id < remtNextId)){
          request << "getKeys" << this->id << "" << "" << "" << "";
          cli.send(request);
          cli.receive(answer);
          str keys;
          answer >> keys;
          setKeys(keys);
          if(remtIsLast == "true" && this->id > remtId){
            this->amILast = "true";
            remtIsLast = "false";
          }
          else this->amILast = "false";
          request << "setinfo" << this->id << this->ip << this->port \
                  << remtIsLast << "";
          cli.send(request);
          cli.receive(answer);
          setRemoteInfo(remtNextId,remtNextIp,remtNextPort);
          break;
        }

        cli.disconnect("tpc//"+this->remoteIp+":"+this->remotePort);
        this->remoteIp = remtNextIp; this->remotePort = remtNextPort;
      }
    }

    void getKeys(str& keys){
      /* it will put keys into a formatted string for shipping */
      for(auto& item : this->keys){
        str key = item.first, ownerName = item.second[0], \
            fileName = item.second[1];
        keys += key + " " + ownerName + " " + fileName + " ";
      }
    }

  /* ---------------- server side ---------------- */
  private:
    bool _exit;

  public:
    void server(socket& serv){
      /* it will simulate a sever into tracker */
      serv.bind("tcp://"+this->ip+":"+this->port);
      message request, reply;
      str op, cId, cIp, cPort, last, keys;

      while(!_exit){
        serv.receive(request);
        request >> op >> cId >> cIp >> cPort >> last >> keys;

        if(op == "getinfo")
          this->getInfo(reply);

        else if(op == "getkeys")
          this->getKeys(reply,cId);

        else if(op == "setinfo")
          this->setInfo(reply,cId,cIp,cPort,last,keys);

        serv.send(reply);
      }

    }

  private:
    void setInfo(message& package,str id,str ip,str port,str last,str keys){
      /* it will set up all remote node info */
      setRemoteInfo(id,ip,port);
      setKeys(keys);
      this->amILast = last;
      package << "ok";
    }

    void getInfo(message& package){
      /* it will get this node and next node info for requester client */
       package << this->id << this->remoteId << this->remoteIp \
               << this->remotePort << this->amILast;
    }

    void splitKeys(_map::iterator& f, _map::iterator& l, str& ckeys){
      /* it will split keys domain */
      for(auto& it = f; it != l;){
        str key = it->first;
        str ownerName = it->second[0], fileName = it->second[1];
        ckeys += key + " " + ownerName + " " + fileName + " ";
        it = keys.erase(it);
      }
    }

    void getKeys(message& package, str cId){
      /* it will get all belonging keys for requester client */
      str ckeys = ""; // client keys

      if((this->amILast == "true" && this->id > cId && this->remoteId == "none")\
      || (this->amILast == "true" && this->id > cId && this->remoteId > cId)){
        _map::iterator first = keys.lower_bound(cId);
        _map::iterator last = keys.lower_bound(id);
        splitKeys(first,last,ckeys);
        package << ckeys;
        return;
      }

      if(cId > this->id){
        _map::iterator first = keys.lower_bound(cId);
        _map::iterator last = keys.end();
        splitKeys(first,last,ckeys);
      }

      if(this->amILast == "true" && cId > this->id){
        _map::iterator first = keys.begin();
        _map::iterator last = keys.lower_bound(this->id);
        splitKeys(first,last,ckeys);
      }

      package << ckeys;
    }
};

int main(int argc,const char **argv){
  str ownFiles = "", id = "";
  map<str,str> machInfo = getMachInfo();
  ownFiles = getFiles(machInfo["ip"],"/files");
  id = sha1(machInfo["mac"]);
  context s_ctx, c_ctx;
  socket serv(s_ctx,socket_type::rep), cli(c_ctx,socket_type::req);
  tracker track(id,localIp,"5555",remoteIp,"7777",ownFiles);
  thread t(&tracker::server, &track, ref(serv));
  track.client(cli,serv);
  t.join();
  return 0;
}
