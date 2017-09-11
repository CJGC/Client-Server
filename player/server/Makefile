CC=g++ -std=c++11
#ZMQ_PATH=$/usr/local
ZMQ_PATH=/home/utp/zmq
ZMQ_INC=$(ZMQ_PATH)/include
ZMQ_LIB=$(ZMQ_PATH)/lib

all: hwserver

hwserver: hwserver.cpp
		$(CC) -L$(ZMQ_LIB) -I$(ZMQ_INC) hwserver.cpp -o hwserver.bin -lzmq -lzmqpp
