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
list<string>::iterator currentSongItera= userList.begin();
Music music;
mutex autPlay;
condition_variable cv;
bool stoppedByUser = false, musicIsPlaying = false;

// void autoPlay(){
// 	while(true){
// 		unique_lock<mutex> lock(autPlay);
// 		cv.wait(lock,[]{return musicIsPlaying;});
// 		Time MusicDuration = music.getDuration();
// 		// cv.wait_for(lock,MusicDuration,[]{return stoppedByUser == true;});
// 		if(stoppedByUser) cout << "stopped by user";
// 		else cout << "songs ended";
// 	}
// }

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

list<string> processList(string& sList){
	list<string> songsList; string songName("");
	for(auto c : sList){
		if(c == '\n'){songsList.push_back(songName); songName = "";}
		else songName += c;
	}
	return songsList;
}

list<string> getList(socket &s){
	message request,answer;
	string sList;
	request << "list" << "none" << 0;
	s.send(request);
	s.receive(answer);
	answer >> sList;
	return processList(sList);
}

void makeLine(string& item,string& screen,const size_t& screenSize,size_t rule){
	size_t spaces = screenSize - item.size() - 2, makeRule = 0;
	if(rule == 1) makeRule = spaces/2;
	if(rule == 2) makeRule = 7;
	for(size_t p=1; p<=spaces+1; p++)	if(p == makeRule) screen += item; else screen += " ";
	screen += "|\n";
}

string menu(string title,list<string>& content,list<string>& availOpts,string& op){
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
	for(auto opt : availOpts) if(option.find(opt,0) != string::npos){op = opt; return option;}
	return "";
}

bool intoServerList(list<string>& serverMusicList,string& songName){
	if(find(serverMusicList.begin(),serverMusicList.end(),songName) != serverMusicList.end())
		return true;
	return false;
}

bool intoUserList(string& songName){
	if(find(userList.begin(),userList.end(),songName) != userList.end()) return true;
	return false;
}

list<string>::iterator getIterator(string& songName){
	return find(userList.begin(),userList.end(),songName);
}

int main(int argc, char **argv) {
	/* establishing connection */
	context ctx;
	socket s(ctx, socket_type::req);
	s.connect("tcp://localhost:5555");
	// thread t(autoPlay);

	/* interacting with user */
	bool exit = false;
	int menuNum = 1;
	list<string> serverMusicList = getList(s);
	// this_thread::sleep_for(std::chrono::milliseconds(200));
	do{
		system("clear");
		switch(menuNum){
			case 1:{ // Available songs menu
				list<string> availOpts{"add","goto playlist"};
				string op(""), userOpt("");
				userOpt = menu("AVAILABLE SONGS MENU",serverMusicList,availOpts,op);
				if(userOpt == "") continue;
				if(userOpt == "goto playlist"){menuNum = 2; continue;}
				string songName("");
				int pos = userOpt.rfind(" ",userOpt.size()) + 1;
				if(pos)	songName = userOpt.substr(pos,userOpt.size());
				if(songName == "") continue;
				if(op == "add" && intoServerList(serverMusicList,songName) and !intoUserList(songName)){
					getSong(s,songName);
					userList.push_back(songName);
				}
			} break;
		 	case 2:{ // Playlist user menu
				list<string> availOpts{"play","stop","next","remove","goto songs menu"};
				string op(""), userOpt("");
				userOpt = menu("PLAY LIST MENU",userList,availOpts,op);
				if(userOpt == "") continue;
				if(userOpt == "stop"){music.stop();continue;}
				if(userOpt == "goto songs menu"){menuNum = 1; continue;}
				if(userOpt == "next" && !userList.empty()){
					advance(currentSongItera,1);
					if(currentSongItera == userList.end()) currentSongItera=userList.begin();
					music.openFromFile(*currentSongItera);
					music.play();
					continue;
				}
				string songName("");
				int pos = userOpt.rfind(" ",userOpt.size()-1) + 1;
				if(pos) songName = userOpt.substr(pos,userOpt.size()-1);
				if(songName == "" || !intoUserList(songName)) continue;
				if(op == "play"){
					currentSongItera = getIterator(songName);
					music.openFromFile(*currentSongItera);
					music.play();
				}
				else if(op == "remove"){
					list<string>::iterator iter = getIterator(songName);
					int i = distance(userList.begin(),iter);
					int j = distance(userList.begin(),currentSongItera);
					userList.remove(songName);
					if(i == j){currentSongItera = userList.begin(); music.stop();}
					else if(j > i) advance(currentSongItera,-1);
				}
			} break;
		}
	}while(!exit);
	return 0;
}
