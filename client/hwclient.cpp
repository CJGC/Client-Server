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

/* needed global variables (these variables must be seen by all members)*/
list<string> userList;
list<string>::iterator currentSongItera= userList.begin();
Music music;
mutex autPlay;
condition_variable cv;
bool isPlayedByUser = false;

void autoPlay(){
	/* it will put a thread into autoplay action respecting certain rules */
	while(true){
		unique_lock<mutex> lock(autPlay);
		cv.wait(lock,[]{return isPlayedByUser;});
		while(music.getStatus() == SoundSource::Playing) this_thread::sleep_for(chrono::milliseconds(200));
		if(!isPlayedByUser) continue;
		advance(currentSongItera,1);
		if(currentSongItera == userList.end()) currentSongItera = userList.begin();
		music.openFromFile(*currentSongItera);
		music.play();
	}
}

void getPart(socket& s,string songName,unsigned int part,ofstream &ofs){
	/* it will get a given part of requested song */
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
	/* it will get song knowing the song's parts first, then requesting part by part */
	message request,answer;
	request << "meetParts" << songName << 0;
	s.send(request);
	s.receive(answer);
	size_t parts=0;
	answer >> parts;
	ofstream ofs(songName,ios::binary);
	for(size_t part=1; part<=parts; part++) getPart(s,songName,part,ofs);
	ofs.close();
}

list<string> processList(string& sList){
	/* it will turn into a true object c++ list from a concatenated string sList */
	list<string> songsList; string songName("");
	for(auto c : sList){
		if(c == '\n'){songsList.push_back(songName); songName = "";}
		else songName += c;
	}
	return songsList;
}

list<string> getList(socket &s){
	/* it will get the list of available songs from hwserver.cpp */
	message request,answer;
	string sList;
	request << "list" << "none" << 0;
	s.send(request);
	s.receive(answer);
	answer >> sList;
	return processList(sList);
}

void makeLine(string& line,string& screen,const size_t& screenSize,size_t rule){
	/* it will make a line with a rule. A rule could be the number of spaces before
	a line, like second rule in the example below (currently exist two rules) */
	size_t spaces = screenSize - line.size() - 2, makeRule = 0;
	if(rule == 1) makeRule = spaces/2;
	if(rule == 2) makeRule = 7;
	for(size_t p=1; p<=spaces+1; p++)	if(p == makeRule) screen += line; else screen += " ";
	screen += "|\n";
}

string menu(string title,list<string>& content,list<string>& availOpts){
	/* it will print a menu, with menu title, menu content  and available options for menu */
	string screen(""), option("");
	string line("--------------------------------------------------------------------------------");
	const size_t screenSize = line.size();
	screen += line+"\n|";
	makeLine(title,screen,screenSize,1);
	screen += line+"\n";
	for(auto &item : content){ screen += "|"; makeLine(item,screen,screenSize,2);}
	screen += line+"\n\n** <Available operators> **\n";
	for(auto &opt : availOpts) screen += " -> "+opt+"\n";
	screen += "\nType option: ";
	cout << screen;
	getline(cin, option);
	for(auto opt : availOpts) if(option.find(opt,0) != string::npos) return option;
	return "";
}

bool intoList(list<string>& _list,string& songName){
	/* it will check if song name is into given list */
	if(find(_list.begin(),_list.end(),songName) != _list.end()) return true;
	return false;
}

list<string>::iterator getIterator(string& songName){
	/* it will return iterator pointer of the given song name */
	return find(userList.begin(),userList.end(),songName);
}

void common(list<string>& content,list<string>& availOpts,string title,string& fpart,string& lpart){
	/* common function is common code lines that can be fit into an unique function */
	string userOpt("");
	do{
		userOpt = menu(title,content,availOpts);
		int fpos = userOpt.find(" ",0);
		if(fpos > 0) fpart = userOpt.substr(0,fpos);
		int lpos = userOpt.rfind(" ",userOpt.size()-1)+1;
		if(lpos) lpart = userOpt.substr(lpos,userOpt.size()-1);
	}while(fpart == "" && lpart == "");
}

int main(int argc, char **argv) {
	/* establishing connection */
	context ctx;
	socket s(ctx, socket_type::req);
	s.connect("tcp://localhost:5555");

	/* interacting with user */
	int menuNum = 1;
	list<string> serverMusicList = getList(s);
	thread t(autoPlay);
	do{
		switch(menuNum){
			case 1:{ // available songs menu
				list<string> availOpts{"add","goto playlist"}; // available options for first menu
				string fpart(""), lpart("");									 // first part and last part of user's string
				common(serverMusicList,availOpts,"AVAILABLE SONGS MENU",fpart,lpart);
				if(fpart == "goto" && lpart == "playlist"){menuNum = 2; continue;}
				string songName = lpart;											 // if lpart isn't playlist it will probably be a songName
				if(fpart == "add" && intoList(serverMusicList,songName) && !intoList(userList,songName)){
					getSong(s,songName);
					userList.push_back(songName);
				}
			}break;
		 	case 2:{ // user's playlist menu
				list<string> availOpts{"play","remove","stop song","next song","goto store"};
				string fpart(""), lpart("");
				common(userList,availOpts,"PLAYLIST MENU",fpart,lpart);
				if(fpart == "goto" && lpart == "store"){menuNum = 1; continue;}
				if(fpart == "stop" && lpart == "song"){isPlayedByUser = false; music.stop();continue;}
				if(fpart == "next" && lpart == "song" && !userList.empty() && isPlayedByUser){
					advance(currentSongItera,1);
					if(currentSongItera == userList.end()) currentSongItera=userList.begin();
					music.openFromFile(*currentSongItera);
					music.play();
					continue;
				}
				string songName = lpart;
				if(!intoList(userList,songName)) continue;
				if(fpart == "play"){
					currentSongItera = getIterator(songName);
					music.openFromFile(*currentSongItera);
					music.play();
					isPlayedByUser = true;
					cv.notify_one();
				}
				else if(fpart == "remove"){
					list<string>::iterator iter = getIterator(songName);
					int i = distance(userList.begin(),iter);
					int j = distance(userList.begin(),currentSongItera);
					userList.remove(songName);
					if(i == j){currentSongItera = userList.begin();isPlayedByUser=false; music.stop();}
				}
			} break;
		}
	}while(true);
	return 0;
}
