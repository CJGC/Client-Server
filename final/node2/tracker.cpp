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
        befId, befIp, befPort, auxId, auxIp, auxPort, amILast;
    mutex ubd; // unbind mutex
    condition_variable cv;
    bool _exit, mustINotify, amIClient;

  public:
    void UpgrFinTablnotify(socket& cli){
      /* it will notify bef node, to upgrade finger table info */
      while(true){
        unique_lock<mutex> lock(this->ubd);
        this->cv.wait(lock,[&]{return this->mustINotify;});
        if(this->_exit) break;
        if(this->id < this->auxId || this->amIClient){
          message request, answer;
          request << "upgrfintabl" << this->auxId << this->auxIp \
                  << this->auxPort << " " << " ";
          cli.send(request);
          cli.receive(answer);
        }
        this->mustINotify = false;
      }
    }

  private:
    void setKeys(const str& keys){
      /* it will setup keys domain */
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
      /* it will setup remote info */
      this->remoteId = id;
      this->remoteIp = ip;
      this->remotePort = port;
    }

    void setBefInfo(str id,str ip,str port){
      /* it will setup bef node info */
      this->befId = id;
      this->befIp = ip;
      this->befPort = port;
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
      this->mustINotify = false;
      this->canIStart = false;
      this->amIClient = false;
      setKeys(ownFiles);
      buildFinTabl();
    }

    void client(socket& cli,socket& printer, socket& publ){
      /* it will simulate a client into tracker */
      publ.bind("tcp://"+this->publIp+":"+this->publPort); // binding publisher
      if(this->publIp == "*")
        this->publIp = "localhost";
      str userOp;
      cout <<"type something to bind to ring chord: ";
      getline(cin,userOp);
      bindWithChord(cli);
      this->canIStart = true;
      this->sbcv.notify_one();
      cout << "type something to continue: ";
      getline(cin,userOp);
      this->auxId = this->id;
      this->auxIp = this->publIp;
      this->auxPort = this->publPort;
      this->mustINotify = true;
      this->amIClient = true;
      this->cv.notify_one();
      /* printing after binding node */
      //message request, answer;
      //str keys;
      //getKeys(keys);
      //request << this->id << keys ;
      //printer.send(request);
      //printer.receive(answer);
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
          requestSetInfo(cli,remtIsLast,remtId);
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

    void setAmIlast(str& remtIsLast,str& remtId){
      /* it will setup am I last variable */
      if(remtIsLast == "true"){
        if(this->id > remtId){
          this->amILast = "true";
          remtIsLast = "false";
        }
        else
          this->amILast = "false";
      }
      else
        this->amILast = "false";
    }

    void requestSetInfo(socket& cli,str& remtIsLast,str& remtId){
      /* it will request to remote node upgrade info */
      message request, answer;
      setAmIlast(remtIsLast,remtId);
      request << "setinfo" << this->id << this->ip << this->port \
              << remtIsLast << " ";
      cli.send(request);
      cli.receive(answer);
      setBefInfo(remtId,this->remoteIp,this->remotePort);
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
    str befIdAux, befIpAux, befPortAux;

  public:
    void server(socket& serv,socket& cli,socket& subs){
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

        else if(op == "getpublinfo")
          getPublInfo(reply);

        else if(op == "setinfo")
          setInfo(reply,cId,cIp,cPort,last,keys);

        else if(op == "upgrfintabl"){
          upgrFinTabl(subs,cId,cIp,cPort);
          reply << "ok";
        }

        else if(op == "unbindbef")
          unbindBef(reply,cli,cId,cIp,cPort);

        else if(op == "disconnect")
          disconnect(reply);

        serv.send(reply);
      }
    }

  private:
    void setInfo(message& package,str id,str ip,str port,str last,str keys){
      /* it will setup all remote node info */
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

    void getPublInfo(message& package){
      /* it will get this node and this publisher info for requester node */
      package << this->id << this->remoteIp << this->remotePort << this->publIp\
              << this->publPort << this->amILast;
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

    void unbindBef(message& package,socket& cli,str id,str ip, str port){
      /* it will request this client to disconnect with bef node */
      if(this->id != id){ // if am not alone into ring
        cli.disconnect("tcp://"+this->befIp+":"+this->befPort);
        setBefInfo(id,ip,port);
        cli.connect("tcp://"+this->befIp+":"+this->befPort);
        package << "ok";
      }
      else
        disconnect(package); // if am alone, i will disconnect
    }

    void disconnect(message& package){
      /* it will disconnect this server */
      this->mustINotify = true;
      this->_exit = true;
      this->cv.notify_one();
      package << "ok";
    }

    /* --------------------- publisher side ----------------- */
  private:
    str publIp, publPort, publRemtIp, publRemPort;
    void publisher(socket& publ){
      /* it will publish to subscriber when this client disconnect */
      message messag;
      str ip = this->remoteIp, port = this->remotePort;
      if(this->amILast == "true"){
        ip = this->befIp;
        port = this->befPort;
      }
      messag << "external disconnect" << this->id << ip << port << this->publPort;
      publ.send(messag);
      messag << "disconnect" << this->id << " " << " " << " ";
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
      subs.subscribe(""); // default channel
      if(this->amILast != "true")
        setFinTabl(aux);
      conWithAllPubl(subs);
      conWithMyPubl(subs);
      listenPubl(subs,aux);
      disconWithMyPubl(subs);
    }

  private:
    void listenPubl(socket& subs,socket& aux){
      /* it will listen all publihers messages */
      while(true){
        message messag;
        str part1, part2, part3, part4, part5;
        subs.receive(messag);
        messag >> part1 >> part2 >> part3 >> part4 >> part5;
        if(part1 == "external disconnect" && part2 != this->id)
          upgrFinTabl(subs,aux,part2,part3,part4,part5);
        else if(part1 == "disconnect" && part2 == this->id){
          break;
        }
      }
    }

    void upgrFinTabl(socket& subs,socket& aux,str& id,str& ip,str& port, \
                     str& port2){
      /* it will disconnect with old publisher, setup finTabl with new publisher
       ip and port and finally it will connect with new publisher */
      message request, answer;
      aux.connect("tcp://"+ip+":"+port);
      request << "getpublinfo" << " " << " " << " " << " " << " ";
      aux.send(request);
      aux.receive(answer);
      str remtId = "", remtNextIp = "", remtNextPort = "", remtPublIp = "",\
          remtPublPort = "", remtIsLast, oldIp = "", oldPort = "";
      answer >> remtId >> remtNextIp >> remtNextPort >> remtPublIp \
             >> remtPublPort >> remtIsLast;
      aux.disconnect("tcp://"+ip+":"+port);

      for(auto& item: this->finTabl){
        str key = item.first;
        if(key <= id){
          str oldIpAux = item.second[0];
          str oldPortAux = item.second[1];
          if(oldPortAux == port2){
            oldIp = oldIpAux;
            oldPort = oldPortAux;
            item.second[0] = remtPublIp;
            item.second[1] = remtPublPort;
            item.second[2] = remtId;
          }
        }
      }

      subs.disconnect("tcp://"+oldIp+":"+oldPort);
      subs.connect("tcp://"+remtPublIp+":"+remtPublPort);
    }

    void upgrFinTabl(socket& subs,str& id,str& ip,str& port){
      /* it will upgrade this finger table requested by new node into ring chord
         (there is a problem here) */
      if(id > this->id){
        str oldIp = "", oldPort = "", oldId = "";
        for(auto& item : this->finTabl){
          str key = item.first;
          if(key <= id){
            str oldIpAux = item.second[0], \
                oldPortAux = item.second[1],\
                oldIdAux = item.second[2];
            if(id < oldIdAux){
              oldIp = oldIpAux;
              oldPort = oldPortAux;
              item.second[0] = ip;
              item.second[1] = port;
              item.second[2] = id;
              subs.connect("tcp://"+ip+":"+port);
            }
          }
        }

        if(oldIp != "" && oldPort != "")
          subs.disconnect("tcp://"+oldIp+":"+oldPort);
      }

       this->auxId = id;
       this->auxIp = ip;
       this->auxPort = port;
       this->mustINotify = true;
       this->amIClient = false;
       this->cv.notify_one();
    }

    void buildFinTabl(){
      /* it will build finger table */
      uint finTablDepth = log2( (double)(sizeof(uint) * 8 ) );
      for(uint i = 0; i<finTablDepth; i++){
        str key = this->id + to_string( pow(2,i) );
        key = sha1(key);
        vec second;
        second.resize(3);
        second[0] = "";
        second[1] = "";
        second[2] = "";
        this->finTabl[key] = second;
      }
    }

    void searchIntoRing(socket& aux,_map::iterator& item){
      /* it will search into chord ring for binding with */
      str key = item->first, ip = this->remoteIp, port = this->remotePort;

      while(true){
        message request, answer;
        aux.connect("tcp://"+ip+":"+port);
        request << "getpublinfo" << " " << " " << " " << " " << " ";
        aux.send(request);
        aux.receive(answer);
        str remtId = "", remtNextIp = "", remtNextPort = "", remtPublIp = "",\
            remtPublPort = "", remtIsLast;
        answer >> remtId >> remtNextIp >> remtNextPort >> remtPublIp \
               >> remtPublPort >> remtIsLast;

        if(key <= remtId || remtIsLast == "true"){
          if(remtIsLast == "true" && key > remtId)
            break;
          item->second[0] = remtPublIp;
          item->second[1] = remtPublPort;
          item->second[2] = remtId;
          break;
        }

        aux.disconnect("tcp://"+ip+":"+port);
        ip = remtNextIp;
        port = remtNextPort;
      }

      aux.disconnect("tcp://"+ip+":"+port);
    }

    void setFinTabl(socket& aux){
      /* it will setup finger table */
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
        if(ip == "" && port == "") continue;
        subs.connect("tcp://"+ip+":"+port);
      }
    }

};

int main(int argc,const char **argv){
  str ownFiles = "", id = "";
  map<str,str> machInfo = getMachInfo();
  ownFiles = getFiles(machInfo["ip"],"/files");
  id = sha1(machInfo["mac"]+"2");
  context s_ctx, c_ctx, p_ctx, a_ctx, su_ctx, pu_ctx;
  socket serv(s_ctx,socket_type::rep), cli(c_ctx,socket_type::req)\
      ,printer(p_ctx,socket_type::req), aux(a_ctx,socket_type::req)\
      ,subs(su_ctx,socket_type::subscribe), publ(pu_ctx,socket_type::publish);
  /* printing current keys domain */
  // message request, answer;
  // printer.connect("tcp://"+printerIp+":7777");
  // request << id << ownFiles;
  // printer.send(request);
  // printer.receive(answer);
  /* incoming node into chord ring */
  tracker track(id,localIp,"5557",localIp,"5558",remoteIp,"5555",ownFiles);
  thread t0(&tracker::UpgrFinTablnotify, &track, ref(cli)),\
         t1(&tracker::server, &track, ref(serv),ref(cli), ref(subs)),\
         t2(&tracker::subscriber, &track, ref(subs), ref(aux));
  track.client(cli,printer,publ);
  t0.join();
  t1.join();
  t2.join();
  // printer.disconnect("tcp://"+printerIp+":7777");
  return 0;
}
