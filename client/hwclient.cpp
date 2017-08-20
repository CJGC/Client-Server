#include <iostream>
#include <zmqpp/zmqpp.hpp>
#include <SFML/Audio.hpp>
#include <fstream>
#include <list>
#include <iterator>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

using namespace std;
using namespace zmqpp;
using namespace sf;

void getPart(socket& s,string songName,unsigned int part,ofstream &ofs){
	message request,answer;
	request << "getPart" << songName << part;
	s.send(request);
	s.receive(answer);
	const void *data;
	size_t partSize = answer.size(0);
	answer.get(&data,0);
	ofs.write((char *)data,partSize);
}

void getSong(socket &s,string& songName){
	message request,answer;
	request <<"meetParts"<< songName << 0;
	s.send(request);
	s.receive(answer);
	size_t parts=0;
	answer >> parts;
	ofstream ofs(songName,ios::binary);
	for(size_t part=1; part<=parts; part++) getPart(s,songName,part,ofs);
	ofs.close();
}

void getList(socket &s,string& op){
	message request,answer;
	string list;
	request << op << "none" << 0;
	s.send(request);
	s.receive(answer);
	answer >> list;
	cout << list;
}

int main(int argc, char **argv) {
	/* Making some  and initialization */
	if (!(argc >= 2 && argc <= 3)) {
	  cerr << "Must be called: <" << argv[0] << "> <list>\n";
	  cerr << "or <" << argv[0] << "> <play> <file.ogg>\n";
	  return 1;
	}
	string op(argv[1]), songName("none"), id("client1");
	if (argc == 3) songName = argv[2];

	/* making socket and contex */
	context ctx;
	socket s(ctx, socket_type::req);
	/* binding socket with given tcp port */
	s.connect("tcp://localhost:5555");

	/* interacting with server */
	if(op == "list") getList(s,op);
	else if(op == "play"){
		getSong(s,songName);
	  Music music;
		if(!music.openFromFile(songName)){
			std::cerr << "There was an error opening requested song!" << '\n';
			return -1;
		}
	  music.play();
		string exit("");
		while(exit != "y"){
			cout<<"do you want to exit? (y): ";
			cin>>exit;
		}
	}
	return 0;
}
