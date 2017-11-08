CC=g++ -std=c++11
ZMQ_PATH=$/usr/local
#ZMQ_PATH=/home/utp/zmq
ZMQ_INC=$(ZMQ_PATH)/include
ZMQ_LIB=$(ZMQ_PATH)/lib

all: hwprinter hwtracker

hwprinter: nodes-printer.cpp
		$(CC) -L$(ZMQ_LIB) -I$(ZMQ_INC) nodes-printer.cpp -o nodes-printer.bin -lzmq -lzmqpp

hwtracker: tracker.cpp
		$(CC) -L$(ZMQ_LIB) -I$(ZMQ_INC) tracker.cpp -o tracker.bin -lzmq -lzmqpp
