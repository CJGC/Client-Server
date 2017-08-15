#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <SFML/Audio.hpp>
#include <fstream>

int main(int argc, char** argv) {
	/* Making some  and initialization */
	if(!(argc >= 2 && argc <= 3)){
		std::cerr<<"Must be called: <"<< argv[0] <<"> <list>\n";
		std::cerr<<"or <"<<argv[0]<<"> <play> <file.ogg>\n";
		return 1;
	}
	std::string op(argv[1]), songName("none"), result;
	if(argc == 3)	songName = argv[2];

	/* making socket and contex */
	zmqpp::context ctx;
	zmqpp::socket s(ctx, zmqpp::socket_type::req);

	/* binding socket with given tcp port */
	zmqpp::message m, answer;

	/* interacting with server */
	s.connect("tcp://localhost:5555");
	m << op << songName;
	s.send(m);
	s.receive(answer);
	// openfrommemory
	/* processing server reply */
	if(op == "play"){
		const void *data;
		answer.get(&data,0);
		size_t size = answer.size(0);
		std::cout<<size;
		std::ofstream ofs(songName, std::ios::binary);
		ofs.write((char*)data, size);
		ofs.close();
		sf::Music music;
		if(!music.openFromFile(songName)){
			std::cerr <<"File not found or error:";
			return 1;
		}
		music.play();
		std::string exit("no");
		while(exit != "y"){
			std::cout<<"do you want to exit? (y)";
			std::getline(std::cin,exit);
		}
		// music.stop();
		return 0;
	}
	answer >> result;
	std::cout << result;
	return 0;
}
