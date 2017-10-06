#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <cmath>

#include "graphReader2.hh"
#include "duplicate.hh"
#include "timer.hh"

int main(int argc, char const **argv)
{
	_map graph;
	vec indices[2], dIndices[2];				// indices[0]= i, indices[1] = j ; duplicate Indices i j
	readGraph(argv[1], indices, graph);
	duplicate(indices,Tindices,indices[0].size());
	//quickSort(dIndices);
	return 0;
}
