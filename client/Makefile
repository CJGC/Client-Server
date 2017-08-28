CC=g++ -std=c++11
#ZMQ_PATH=/home/utp/zmq
ZMQ_PATH=$/usr/local
ZMQ_INC=$(ZM_PATH)/include
ZMQ_LIB=$(ZMQ_PATH)/lib

all: hwclient #hwserver

hwclient: hwclient.cpp
		$(CC) -L$(ZMQ_LIB) -I$(ZMQ_INC) hwclient.cpp -o hwclient.bin -lzmq -lzmqpp -pthread -lsfml-audio

# hwserver: hwserver.cpp
# 		$(CC) -L$(ZMQ_LIB) -I$(ZMQ_INC) hwserver.cpp -o hwserver.bin -lzmq -lzmqpp
