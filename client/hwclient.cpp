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

class Client{
	private:
		list<string> _list;
		list<string>::iterator i;
		Music music;
		mutex autPlay;
		condition_variable cv;
		thread t;
		bool isPlaying, exit;

		void setIterator(string& songName){
			i = find(_list.begin(),_list.end(),songName);
		}

		list<string>::iterator getIterator(string& songName){
			return find(_list.begin(),_list.end(),songName);
		}

		void autoPlay(){
			while(!exit){
				unique_lock<mutex> lock(autPlay);
				cv.wait(lock,[&]{return isPlaying || exit;});
				while(music.getStatus() == SoundSource::Playing) this_thread::sleep_for(chrono::milliseconds(200));
				if(!isPlaying) continue;
				advance(i,1);
				if(i == _list.end()) i = _list.begin();
				music.openFromFile(*i);
				music.play();
			}
		}

	public:
		Client(){
			_list = {};
			i = _list.begin();
			isPlaying = false;
			exit = false;
			t = thread(&Client::autoPlay,this);
		}

		~Client(){
			isPlaying = false;
			exit = true;
			music.stop();
			cv.notify_one();
			t.join();
		}

		list<string> getList(){return _list;}

		void addSong(string& songName){_list.push_back(songName);}

		bool intoList(string& songName){
			if(find(_list.begin(),_list.end(),songName) != _list.end()) return true;
			return false;
		}

		void playSong(string& songName){
			if(!intoList(songName)) return;
			setIterator(songName);
			music.openFromFile(*i);
			music.play();
			isPlaying = true;
			cv.notify_one();
		}

		void removeSong(string& songName){
			if(!intoList(songName)) return;
			list<string>::iterator j = getIterator(songName);
			_list.remove(songName);
			if(*i == *j){
				i = _list.begin();
				isPlaying = false;
				music.stop();
			}
		}

		void nextSong(){
			if(_list.empty() || !isPlaying) return;
			advance(i,1);
			if(i == _list.end()) i = _list.begin();
			music.openFromFile(*i);
			music.play();
		}

		void stopSong(){
			isPlaying = false;
			music.stop();
		}
};

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

void common(list<string> content,list<string>& availOpts,string title,string& fpart,string& lpart){
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
	Client client;
	list<string> servMusic = getList(s);
	int num = 1;
	bool exit = false;
	//thread t(autoPlay);
	do{
		switch(num){
			case 1:{ // available songs menu
				list<string> availOpts{"add","goto playlist","exit program"}; // available options for first menu
				string fpart(""), lpart("");									 // first part and last part of user's string
				common(servMusic,availOpts,"AVAILABLE SONGS MENU",fpart,lpart);
				if(fpart == "goto" && lpart == "playlist") num = 2;
				else if(fpart == "exit" && lpart == "program") exit = true;
				else if(fpart == "add" && intoList(servMusic,lpart) && !client.intoList(lpart)){
					getSong(s,lpart);
					client.addSong(lpart);
				}
			}break;
		 	case 2:{ // user's playlist menu
				list<string> availOpts{"play","remove","stop song","next song","goto store","exit program"};
				string fpart(""), lpart("");
				common(client.getList(),availOpts,"PLAYLIST MENU",fpart,lpart);
				if(fpart == "goto" && lpart == "store") num = 1;
				else if(fpart == "exit" && lpart == "program") exit = true;
				else if(fpart == "stop" && lpart == "song")	client.stopSong();
				else if(fpart == "next" && lpart == "song")	client.nextSong();
				else if(fpart == "play") client.playSong(lpart);
				else if(fpart == "remove") client.removeSong(lpart);
			} break;
		}
	}while(!exit);
	return 0;
}
