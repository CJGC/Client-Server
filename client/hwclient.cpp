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

string menu(string title,list<string>& content,list<string>& availOpts){
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

void common(list<string>& content,list<string>& availOpts,string title,string& fpart,string& lpart){
	string userOpt("");
	do{
		userOpt = menu(title,content,availOpts);
		int fpos = userOpt.find(" ",0);
		if(fpos > 0) fpart = userOpt.substr(0,fpos);
		int lpos = userOpt.rfind(" ",userOpt.size()-1)+1;
		if(lpos) lpart = userOpt.substr(lpos,userOpt.size()-1);
		cout << fpart << " " << lpart;
	}while(fpart == "" && lpart == "");
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
		switch(menuNum){
			case 1:{ // Available songs menu
				list<string> availOpts{"add","goto playlist"};
				string fpart(""), lpart("");
				common(serverMusicList,availOpts,"AVAILABLE SONGS MENU",fpart,lpart);
				if(fpart == "goto" && lpart == "playlist"){menuNum = 2; continue;}
				string songName = lpart;
				if(fpart == "add" && intoServerList(serverMusicList,songName) && !intoUserList(songName)){
					getSong(s,songName);
					userList.push_back(songName);
				}
			}break;
		 	case 2:{ // Playlist user menu
				list<string> availOpts{"play","remove","stop song","next song","goto store"};
				string fpart(""), lpart("");
				common(userList,availOpts,"PLAY LIST MENU",fpart,lpart);
				if(fpart == "stop" && lpart == "song"){music.stop();continue;}
				if(fpart == "goto" && lpart == "store"){menuNum = 1; continue;}
				if(fpart == "next" && lpart == "song" && !userList.empty()){
					advance(currentSongItera,1);
					if(currentSongItera == userList.end()) currentSongItera=userList.begin();
					music.openFromFile(*currentSongItera);
					music.play();
					continue;
				}
				string songName = lpart;
				if(!intoUserList(songName)) continue;
				if(fpart == "play"){
					currentSongItera = getIterator(songName);
					music.openFromFile(*currentSongItera);
					music.play();
				}
				else if(fpart == "remove"){
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
