#include <zmqpp/zmqpp.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "getMachInfo.hh"
#include "loadFiles.hh"

using namespace zmqpp;
using namespace std;
typedef string str;
using vec = vector<str>;
using _map = map<str,vec>;

str localIp = "*", remoteIp = "localhost";

class tracker{

  /* ---- generic methods and attributes, these will be used by
          client and server side respectively --- */

  private:
    _map keys;
    str id, ip, port, remoteId, remoteIp, remotePort, amILast, befIp, befPort,\
        befIpAux, befPortAux;
    mutex ubd; // unbind mutex
    condition_variable cv;
    bool mustIUbound;

  public:
    void _unbind(socket& cli){
      /* it will unbind this client with remote server and will bind with other
      remote server */
      while(!_exit){
        unique_lock<mutex> lock(this->ubd);
        this->cv.wait(lock,[&]{return this->mustIUbound;});
        cli.disconnect("tcp://"+this->befIp+":"+this->befPort);
        setBefInfo(this->befIpAux,this->befPortAux);
        cli.connect("tcp://"+this->befIp+":"+this->befPort);
        this->mustIUbound = false;
      }
    }

  private:
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
      this->id = id;
      this->ip = ip;
      this->port = port;
      setBefInfo("none","none");
      setRemoteInfo("none",remoteIp,remotePort);
      this->amILast = "true";
      this->_exit = false;
      this->mustIUbound = false;
      setKeys(ownFiles);
    }

    void client(socket& cli,socket& serv){
      /* it will simulate a client into tracker */
      bindWithChord(cli);
      str userOp;
      cout << "type something to exit: ";
      getline(cin,userOp);
      unbindWithChord(cli);
    }

  private:
    void unbindWithChord(socket& cli){
      /* it will unbind with chord ring */
      message request, answer;
      str keys = "";
      getKeys(keys);
      request << "setinfo" << this->remoteId << this->remoteIp \
              <<this->remotePort << this->amILast << keys;
      cli.send(request);
      cli.receive(answer);
      cli.disconnect("tcp://"+this->befIp+":"+this->befPort);
      cli.connect("tcp://"+this->remoteIp+":"+this->remotePort);
      request << "unbind" <<""<< this->befIp << this->befPort << "" << "";
      cli.send(request);
      cli.receive(answer);
      cli.disconnect("tcp://"+this->remoteIp+":"+this->remotePort);
      cli.connect("tcp://"+this->ip+":"+this->port);
      request << "disconnect"<<""<<""<<""<<""<<"";
      cli.send(request);
      cli.receive(answer);
      cli.disconnect("tcp://"+this->ip+":"+this->port);
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
          setBefInfo(this->remoteIp,this->remotePort);
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

    void setBefInfo(str ip,str port){
      /* it will set up bef node info */
      this->befIp = ip;
      this->befPort = port;
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
          getInfo(reply);

        else if(op == "getkeys")
          getKeys(reply,cId);

        else if(op == "setinfo")
          setInfo(reply,cId,cIp,cPort,last,keys);

        else if(op == "unbind")
          unbind(reply,cIp,cPort);

        else if(op == "disconnect")
          disconnect(reply);

        serv.send(reply);
      }
      serv.unbind("tcp://"+this->ip+":"+this->port);
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
        _map::iterator first = this->keys.lower_bound(cId);
        _map::iterator last = this->keys.lower_bound(id);
        splitKeys(first,last,ckeys);
        package << ckeys;
        return;
      }

      if(cId > this->id){
        _map::iterator first = this->keys.lower_bound(cId);
        _map::iterator last = this->keys.end();
        splitKeys(first,last,ckeys);
      }

      if(this->amILast == "true" && cId > this->id){
        _map::iterator first = this->keys.begin();
        _map::iterator last = this->keys.lower_bound(this->id);
        splitKeys(first,last,ckeys);
      }

      package << ckeys;
    }

    void unbind(message& package,str ip, str port){
      /* it will request this client to disconnect with bef node */
      this->befIpAux = ip;
      this->befPortAux = port;
      this->mustIUbound = true;
      this->cv.notify_one();
      package << "ok";
    }

    void disconnect(message& package){
      /* it will disconnect this server */
      this->_exit = true;
      package << "ok";
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
  thread t0(&tracker::_unbind, &track, ref(cli));
  thread t1(&tracker::server, &track, ref(serv));
  track.client(cli,serv);
  t0.join();
  t1.join();
  return 0;
}
