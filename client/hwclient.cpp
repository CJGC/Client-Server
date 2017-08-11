#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <SFML/Audio.hpp>

int main(int argc, char** argv) {
	if (argc != 2){
		std::cerr << "Must be called: " << argv[0] << " file.ogg\n";
		return 1;
	}

	/* making socket and contex */
	// zmqpp::context ctx;
	// zmqpp::socket s(ctx, zmqpp::socket_type::req);

	/* binding socket with given tcp port */
	// std::cout << "Connecting to tcp port 5555\n";
	// s.connect("tcp://localhost:5555");

	std::cout<<"Simple player! \n";
	std::string fileToPlay(argv[1]);

	sf::Music music;

	if(!music.openFromFile(fileToPlay)){
		std::cerr <<"File not found or error:";
		return 1;
	}

	music.play();
	/* sending message to server */
	// std::cout << "Sending  some work!\n";
	// zmqpp::message m;

	// s.send(m);

	/* capturing and showing server reply */
	// zmqpp::message answer;
	// int a;
	// s.receive(answer);
	// answer >> a;
	// std::cout << "Answer is " << a << std::endl;
  std::cout << "Finished\n";
	return 0;
}
