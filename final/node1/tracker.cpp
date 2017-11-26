#include <zmqpp/zmqpp.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cmath>
#include "../lib/getMachInfo.hh"
#include "../lib/loadFiles.hh"

using namespace zmqpp;
using namespace std;
typedef string str;
using vec = vector<str>;
using _map = map<str,vec>;

str localIp = "*", remoteIp = "localhost", printerIp = "localhost";

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
    tracker(str id,str ip,str port,str publIp,str publPort,str remoteIp,\
            str remotePort,str ownFiles){
      this->id = id;
      this->ip = ip;
      this->port = port;
      this->publIp = publIp;
      this->publPort = publPort;
      setBefInfo("none","none","none");
      setRemoteInfo("none",remoteIp,remotePort);
      this->amILast = "true";
      this->_exit = false;
      this->mustIUnbindBef = false;
      this->canIStart = false;
      setKeys(ownFiles);
      buildFinTabl();
    }

    void client(socket& cli,socket& printer, socket& publ){
      /* it will simulate a client into tracker */
      str userOp;
      cout <<"type something to connect";
      getline(cin,userOp);
      bindWithChord(cli);
      this->canIStart = true;
      this->sbcv.notify_one();
      /* printing after binding node */
      message request, answer;
      str keys;
      getKeys(keys);
      request << this->id << keys ;
      printer.send(request);
      printer.receive(answer);
      cout << "type something to exit: ";
      getline(cin,userOp);
      publisher(publ);
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

        cli.disconnect("tcp://"+this->remoteIp+":"+this->remotePort);
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

      while(!this->_exit){
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

    /* --------------------- publisher side ----------------- */
  private:
    str publIp, publPort;
    void publisher(socket& publ){
      /* it will publish to subscriber when this client disconnect */
      message messag;
      messag << "external disconnect" << this->id;
      publ.send(messag);
      messag << "disconnect" << " ";
      publ.send(messag);
    }

    /* --------------------- subscriber side ----------------- */
  private:
    mutex sbm; // subscriber mutex
    condition_variable sbcv; // subscriber condition variable
    _map finTabl;
    bool canIStart;

  public:
    void subscriber(socket& subs,socket& aux){
      /* main subscriber section */
      unique_lock<mutex> lock(this->sbm);
      this->sbcv.wait(lock,[&]{return this->canIStart;});
      if(this->amILast != "true")
        setFinTabl(aux);
      conWithAllPubl(subs);
      conWithMyPubl(subs);
      listenPubl(subs);
      disconWithAllPubl(subs);
      disconWithMyPubl(subs);
    }

  private:
    void listenPubl(socket& subs){
      /* it will listen all publihers messages */
      while(!this->_exit){
        message messag;
        str part1, part2;
        subs.receive(messag);
        messag >> part1 >> part2;
        if(part1 == "external disconnect"){}
        else if(part2 == "disconnect"){}
      }
    }

    void buildFinTabl(){
      /* it will build finger table */
      uint finTablDepth = log2( (double)(sizeof(uint) * 8 ) );
      for(uint i = 0; i<finTablDepth; i++){
        str key = this->id + to_string( pow(2,i) );
        key = sha1(key);
        vec second;
        second.resize(2);
        second[0] = "";
        second[1] = "";
        this->finTabl[key] = second;
      }
    }

    void searchIntoRing(socket& aux,_map::iterator& item){
      /* it will search into chord ring for binding with */
      str key = item->first, ip = this->remoteIp, port = this->remotePort;

      while(true){
        message request, answer;
        aux.connect("tcp://"+ip+":"+port);
        request << "getinfo" << " " << " " << " " << " " << " ";
        aux.send(request);
        aux.receive(answer);
        str remtId = "", remtNextId = "", remtNextIp = "", remtNextPort = "",\
            remtIsLast = "";
        answer >> remtId >> remtNextId >> remtNextIp >> remtNextPort \
               >> remtIsLast;

        if(key <= remtId || remtIsLast == "true"){
          item->second[0] = ip;
          item->second[1] = port;
          break;
        }

        aux.disconnect("tcp://"+ip+":"+port);
        ip = remtNextIp;
        port = remtNextPort;
      }

      aux.disconnect("tcp://"+ip+":"+port);
    }

    void setFinTabl(socket& aux){
      /* it will set up finger table */
      _map::iterator start = this->finTabl.begin();
      _map::iterator end = this->finTabl.end();
      for(auto& it = start; it != end; it++)
        searchIntoRing(aux,it);
    }

    void conWithMyPubl(socket& subs){
      /* it will connect with this pusblisher */
      subs.connect("tcp://"+this->publIp+":"+this->publPort);
    }

    void disconWithMyPubl(socket& subs){
      /* it will disconnect with this pusblisher */
      subs.disconnect("tcp://"+this->publIp+":"+this->publPort);
    }

    void conWithAllPubl(socket& subs){
      /* it will connect with all sockets into finger table */
      for(auto& item : this->finTabl){
        str ip = item.second[0], port = item.second[1];
        subs.connect("tcp://"+ip+":"+port);
      }
    }

    void disconWithAllPubl(socket& subs){
      /* it will disconnect with all sockets into finger table*/
      for(auto& item : this->finTabl){
        str ip = item.second[0], port = item.second[1];
        subs.disconnect("tcp://"+ip+":"+port);
      }
    }
};

int main(int argc,const char **argv){
  str ownFiles = "", id = "";
  map<str,str> machInfo = getMachInfo();
  ownFiles = getFiles(machInfo["ip"],"/files");
  id = sha1(machInfo["mac"]+"1");
  context s_ctx, c_ctx, p_ctx, a_ctx, su_ctx, pu_ctx;
  socket serv(s_ctx,socket_type::rep), cli(c_ctx,socket_type::req)\
        ,printer(p_ctx,socket_type::req), aux(a_ctx,socket_type::req)\
        ,subs(su_ctx,socket_type::subscribe), publ(pu_ctx,socket_type::publish);
  subs.subscribe(""); // default channel
  publ.bind("tcp://"+localIp+":5556"); // binding publisher
  /* printing current keys domain */
  // message request, answer;
  // printer.connect("tcp://"+printerIp+":7777");
  // request << id << ownFiles;
  // printer.send(request);
  // printer.receive(answer);
  /* incoming node into chord ring */
  tracker track(id,localIp,"5555",localIp,"5556",remoteIp,"5557",ownFiles);
  thread t0(&tracker::_unbindBef, &track, ref(cli)),\
         t1(&tracker::server, &track, ref(serv)),\
         t2(&tracker::subscriber, &track, ref(subs), ref(aux));
  track.client(cli,printer,publ);
  t0.join();
  t1.join();
  t2.join();
  // printer.disconnect("tcp://"+printerIp+":7777");
  return 0;
}
