#include <zmqpp/zmqpp.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "../lib/getMachInfo.hh"
#include "../lib/loadFiles.hh"

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
    str id, ip, port, remoteId, remoteIp, remotePort, \
                      befId, befIp, befPort, amILast;
    mutex ubd; // unbind mutex
    condition_variable cv;
    bool _exit;

  public:
    void _unbindBef(socket& cli){
      /* it will unbind this client with bef node, and will bind with other
        bef node server */
      while(true){
        unique_lock<mutex> lock(this->ubd);
        this->cv.wait(lock,[&]{return this->mustIUnbindBef;});
        if(this->_exit) break;
        cli.disconnect("tcp://"+this->befIp+":"+this->befPort);
        setBefInfo(this->befIdAux,this->befIpAux,this->befPortAux);
        cli.connect("tcp://"+this->befIp+":"+this->befPort);
        this->mustIUnbindBef = false;
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
        else
          aux += ch;
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
      setBefInfo("none","none","none");
      setRemoteInfo("none",remoteIp,remotePort);
      this->amILast = "true";
      this->_exit = false;
      this->mustIUnbindBef = false;
      setKeys(ownFiles);
    }

    void client(socket& cli,socket& printer){
      /* it will simulate a client into tracker */
      str userOp;
      cout <<"type something to connect";
      getline(cin,userOp);
      bindWithChord(cli);
      /* printing after binding node */
      message request, answer;
      str keys;
      getKeys(keys);
      request << this->id << keys ;
      printer.send(request);
      printer.receive(answer);
      cout << "type something to exit: ";
      getline(cin,userOp);
      if(this->_exit)
        return;
      unbindWithChord(cli);
    }

  private:
    void unbindWithChord(socket& cli){
      /* it will unbind with chord ring */
      message request, answer;
      str keys = "";
      getKeys(keys);
      if(this->id < this->befId) // first node with last node case
        this->amILast = "true";
      request << "setinfo" << this->remoteId << this->remoteIp \
              <<this->remotePort << this->amILast << keys;
      cli.send(request);
      cli.receive(answer);
      cli.disconnect("tcp://"+this->befIp+":"+this->befPort);
      cli.connect("tcp://"+this->remoteIp+":"+this->remotePort);
      request << "unbindbef" << this->befId << this->befIp << this->befPort \
              << " " << " ";
      cli.send(request);
      cli.receive(answer);
      cli.disconnect("tcp://"+this->remoteIp+":"+this->remotePort);
      cli.connect("tcp://"+this->ip+":"+this->port);
      request << "disconnect" << " " << " " << " " << " " << " ";
      cli.send(request);
      cli.receive(answer);
      cli.disconnect("tcp://"+this->ip+":"+this->port);
      this->cv.notify_one();
    }

    void bindWithChord(socket& cli){
      /* it will bind with chord ring */
      while(true){
        message request, answer;
        cli.connect("tcp://"+this->remoteIp+":"+this->remotePort);
        request << "getinfo" << " " << " " << " " << " " << " ";
        cli.send(request);
        cli.receive(answer);
        str remtId = "", remtNextId = "", remtNextIp = "", remtNextPort = "",\
            remtIsLast = "";
        answer >> remtId >> remtNextId >> remtNextIp >> remtNextPort \
               >> remtIsLast;
        if(this->id == remtNextId){
          this->remoteId = remtId;
          setBefInfo(remtId,this->remoteIp,this->remotePort);
          break;
        }
        if( (remtNextId == "none") \
        || (this->id > remtId && this->id < remtNextId) \
        || (remtIsLast == "true" && this->id > remtId) \
        || (remtIsLast == "true" && this->id < remtNextId)){
          request << "getkeys" << this->id << " " << " " << " " << " ";
          cli.send(request);
          cli.receive(answer);
          str keys;
          answer >> keys;
          setKeys(keys);
          if(remtIsLast == "true" && this->id > remtId){
            this->amILast = "true";
            remtIsLast = "false";
          }
          else
            this->amILast = "false";
          request << "setinfo" << this->id << this->ip << this->port \
                  << remtIsLast << " ";
          cli.send(request);
          cli.receive(answer);
          setBefInfo(remtId,this->remoteIp,this->remotePort);
          if(this->ip == remtNextIp && this->port == remtNextPort){
            this->remoteId = remtId;
            break;
          }
          setRemoteInfo(remtNextId,remtNextIp,remtNextPort);
          cli.disconnect("tcp://"+this->befIp+":"+this->befPort);
          cli.connect("tcp://"+this->remoteIp+":"+this->remotePort);
          request << "unbindbef" << this->id << this->ip << this->port \
                  << " " << " ";
          cli.send(request);
          cli.receive(answer);
          cli.disconnect("tcp://"+this->remoteIp+":"+this->remotePort);
          cli.connect("tcp://"+this->befIp+":"+this->befPort);
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

    void setBefInfo(str id,str ip,str port){
      /* it will set up bef node info */
      this->befId = id;
      this->befIp = ip;
      this->befPort = port;
    }

  /* ---------------- server side ---------------- */
  private:
    str befIdAux, befIpAux, befPortAux;
    bool mustIUnbindBef;

  public:
    void server(socket& serv){
      /* it will simulate a sever into tracker */
      serv.bind("tcp://"+this->ip+":"+this->port);
      if(this->ip == "*")
        this->ip = "localhost";
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

        else if(op == "unbindbef")
          unbindBef(reply,cId,cIp,cPort);

        else if(op == "disconnect")
          disconnect(reply);

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
      /* it will get this node and next node info for requester node */
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

    void unbindBef(message& package,str id,str ip, str port){
      /* it will request this client to disconnect with bef node */
      if(this->id != id){ // if am not alone into ring
        this->befIdAux = id;
        this->befIpAux = ip;
        this->befPortAux = port;
        package << "ok";
      }
      else
        disconnect(package); // if am alone, i will disconnect
      this->mustIUnbindBef = true;
      this->cv.notify_one();
    }

    void disconnect(message& package){
      /* it will disconnect this server */
      this->mustIUnbindBef = true;
      this->_exit = true;
      package << "ok";
    }
};

int main(int argc,const char **argv){
  str ownFiles = "", id = "";
  map<str,str> machInfo = getMachInfo();
  ownFiles = getFiles(machInfo["ip"],"/files");
  id = sha1(machInfo["mac"]+"1");
  context s_ctx, c_ctx, p_ctx;
  socket serv(s_ctx,socket_type::rep), cli(c_ctx,socket_type::req)\
         ,printer(p_ctx,socket_type::req);
  /* printing current keys domain */
  message request, answer;
  printer.connect("tcp://localhost:7777");
  request << id << ownFiles;
  printer.send(request);
  printer.receive(answer);
  /* incoming node into chord ring */
  tracker track(id,localIp,"5557",remoteIp,"5556",ownFiles);
  thread t0(&tracker::_unbindBef, &track, ref(cli));
  thread t1(&tracker::server, &track, ref(serv));
  track.client(cli,printer);
  t0.join();
  t1.join();
  printer.disconnect("tcp://localhost:7777");
  return 0;
}
