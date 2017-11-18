CC=g++ -std=c++11
ZMQ_PATH=$/usr/local
#ZMQ_PATH=/home/utp/zmq
ZMQ_INC=$(ZMQ_PATH)/include
ZMQ_LIB=$(ZMQ_PATH)/lib
NODE_1_SRC_PATH=$/./node1/tracker.cpp
NODE_2_SRC_PATH=$/./node2/tracker.cpp

all: hwprinter hwnode1 hwnode2

hwprinter: nodes-printer.cpp
		$(CC) -L$(ZMQ_LIB) -I$(ZMQ_INC) nodes-printer.cpp -o nodes-printer.bin -lzmq -lzmqpp
hwnode1: $(NODE_1_SRC_PATH)
		$(CC) -L$(ZMQ_LIB) -I$(ZMQ_INC) $(NODE_1_SRC_PATH) -o "./node1/tracker.bin" -lzmq -lzmqpp -pthread
hwnode2: $(NODE_2_SRC_PATH)
		$(CC) -L$(ZMQ_LIB) -I$(ZMQ_INC) $(NODE_2_SRC_PATH) -o "./node2/tracker.bin" -lzmq -lzmqpp -pthread
