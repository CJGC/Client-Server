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
#include <cstdlib>

using namespace std;
using namespace zmqpp;
using namespace sf;

list<string> userList;

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

string getList(socket &s){
	message request,answer;
	string list;
	request << "list" << "none" << 0;
	s.send(request);
	s.receive(answer);
	answer >> list;
	return list;
}

string menu(string title,string content,vector<string>& optionsSet,string& op){
	string screen(""), option(""), stick("|"), space(" "), endLine("\n");
	string line("--------------------------------------------------------------------------------");
	size_t titleSize = title.size(), screenSize = line.size();
	size_t spaces = screenSize - titleSize - 2;
	screen += line+endLine+stick;
	for(size_t p=1; p<=spaces+1; p++)
		if(p == spaces/2) screen += title;
		else screen += space;
	screen += stick+endLine+line+content+endLine+line+endLine+endLine+"<Available operators>"+endLine;
	for(auto &opt : optionsSet) screen += "->"+opt+endLine;
	screen += endLine+"Type option: ";
	cout << screen;
	getline(cin, option);
	for(auto opt : optionsSet) if(option.find(opt,0) != string::npos){op = opt; return option;}
	return "";
}

bool intoList(string& musicList,string& songName){
	if(musicList.find(songName,0) != string::npos) return true;
	return false;
}

int main(int argc, char **argv) {
	/* establishing connection */
	context ctx;
	socket s(ctx, socket_type::req);
	s.connect("tcp://localhost:5555");

	/* interacting with user */
	bool exit = false;
	int menuNum = 1;
	string musicList = getList(s);
	do{
		system("clear");
		switch(menuNum){
			case 1:{ // available songs menu
				vector<string> optionsSet{"get","goto playlist"};
				string op(""), object(""), userOpt("");
				userOpt = menu("AVAILABLE SONGS MENU",musicList,optionsSet,op);
				cout << userOpt;
				if(userOpt == ""){ continue;}
				short int secondPartPos = userOpt.rfind(" ",userOpt.size()) + 1;
				if(secondPartPos)	object = userOpt.substr(secondPartPos,userOpt.size());
				if(object == ""){ continue;}
				if(object == "playlist"){menuNum = 2; continue;}
				if(op == "get" && intoList(musicList,object)) getSong(s,object);
			} break;
		 	case 2:{ // playlist menu
				exit = true;
			} break;
		}
	}while(!exit);
	return 0;
}
