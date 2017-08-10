#include <iostream>
//#include <string>
#include <zmqpp/zmqpp.hpp>

int main(int argc, char** argv) {
	if (argc != 4) {
		std::cerr << "Should be called: " << argv[0] << " <op> operand1 operand2\n";
		return 1;
	}

	/* making socket and contex */
	zmqpp::context ctx;
	zmqpp::socket s(ctx, zmqpp::socket_type::req);

	/* binding socket with given tcp port */
	std::cout << "Connecting to tcp port 5555\n";
	s.connect("tcp://localhost:5555");

	/* sending message to server */
	std::cout << "Sending  some work!\n";
	zmqpp::message m;
	std::string operation(argv[1]);
	std::string operand1(argv[2]);
	std::string operand2(argv[3]);
	m << operation << stoi(operand1) << stoi(operand2);
	s.send(m);

	/* capturing and showing server reply */
	zmqpp::message answer;
	int a;
	s.receive(answer);
	answer >> a;
	std::cout << "Answer is " << a << std::endl;
  std::cout << "Finished\n";
	return 0;
}
