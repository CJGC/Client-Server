#include "graphReader2.hh"

int main(int argc, char const **argv)
{
	_map graph;
	vec indices[2];
	readGraph(argv[1], indices, graph);
	for(auto &i: indices[0]) cout<< i << endl;
	return 0;
}