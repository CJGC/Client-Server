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
list<string>::iterator currentSongIndex= userList.begin();
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

string menu(string title,list<string>& content,vector<string>& availOpts,string& op){
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

list<string>::iterator intoUserList(string& songName){
	list<string>::iterator i;
	i = find(userList.begin(),userList.end(),songName);
	return i;
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
				vector<string> availOpts{"add","goto playlist"};
				string op(""), secondPart(""), userOpt("");
				userOpt = menu("AVAILABLE SONGS MENU",serverMusicList,availOpts,op);
				if(userOpt == "") continue;
				if(userOpt == "goto playlist"){menuNum = 2; continue;}
				short int secondPartPos = userOpt.rfind(" ",userOpt.size()) + 1;
				if(secondPartPos)	secondPart = userOpt.substr(secondPartPos,userOpt.size());
				if(secondPart == "") continue;
				if(op == "add" && intoServerList(serverMusicList,secondPart)){
					if(intoUserList(secondPart) != userList.end()) continue;
					getSong(s,secondPart);
					userList.push_back(secondPart);
				}
			} break;
		 	case 2:{ // Playlist user menu
				vector<string> availOpts{"play","stop","next","remove","goto songs menu"};
				string op(""), songName(""), userOpt("");
				userOpt = menu("PLAY LIST MENU",userList,availOpts,op);
				if(userOpt == "") continue;
				if(userOpt == "stop"){music.stop();continue;}
				if(userOpt == "goto songs menu"){menuNum = 1; continue;}
				if(userOpt == "next" && !userList.empty()){
					advance(currentSongIndex,1);
					if(currentSongIndex == userList.end()) currentSongIndex=userList.begin();
					music.openFromFile(*currentSongIndex);
					music.play();
					continue;
				}
				int pos = userOpt.rfind(" ",userOpt.size()-1) + 1;
				if(pos) songName = userOpt.substr(pos,userOpt.size()-1);
				if(songName == "") continue;
				if(op == "play" && !userList.empty()){
					list<string>::iterator i = intoUserList(songName);
					if(i != userList.end()) currentSongIndex = i;
					else continue;
					music.openFromFile(*currentSongIndex);
					musicIsPlaying = true;
					cv.notify_one();
					music.play();
					continue;
				}
				if(op == "remove"){
					if(currentSongIndex == intoUserList(songName)){currentSongIndex = userList.begin(); music.stop();}
					userList.remove(songName);
				}
			} break;
		}
	}while(!exit);
	system("rm *.ogg");
	return 0;
}
