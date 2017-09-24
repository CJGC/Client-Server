CC=g++ -std=c++11

all: matrixV0 matrixV1 matrixV2 matrixV2.1 

matrixV0: matrixV0.cpp
	${CC} matrixV0.cpp -o matrixV0.bin

matrixV1: matrixV1.cpp
	${CC} matrixV1.cpp -o matrixV1.bin -pthread

matrixV2: matrixV2.cpp
	${CC} matrixV2.cpp -o matrixV2.bin -pthread

matrixV2.1: matrixV2.1.cpp
	${CC} matrixV2.1.cpp -o matrixV2.1.bin -pthread
