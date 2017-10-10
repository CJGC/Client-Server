#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <cmath>
#include <algorithm>
#include "graphReader2.hh"
#include "duplicate.hh"
#include "timer.hh"
#include "quicksort.hh"

uint getIndex(vec& v,uint& value){
	vec::iterator it = find(v.begin(),v.end(),value);
	return it - v.begin();
}

uint minVal(_map& graph,vec* indices,vec* dIndices,uint iR,uint jR,uint iter){
	uint k = getIndex(indices[0],iR), _min = numeric_limits<uint>::max();
	for(k;indices[0][k] == indices[0][iter];k++){
		uint s = getIndex(dIndices[1],jR);
		for(s;dIndices[1][s] == indices[1][iter];s++){
			if(k > s) break;
			if(k == s){
				uint i = indices[0][k], j = indices[1][k];
				uint di = dIndices[0][s], dj = dIndices[1][s];
				string key(to_string(i)+to_string(j));
				string dKey(to_string(di)+to_string(dj));
				uint val = graph[key] + graph[dKey];
				if(val < _min) _min = val;
			}
		}
	}
	return _min;
}

void Adiamond(_map& graph,vec* indices,vec* dIndices){
	_map R;
	uint indicesSize = indices[0].size(), iterator = 0;
	while(iterator < indicesSize){
		uint iR = indices[0][iterator];
		uint jR = indices[1][iterator];
		string key(to_string(iR)+to_string(jR));
		R[key] = minVal(graph,indices,dIndices,iR,jR,iterator);
		iterator++;
	}
	for(iterator=0; iterator < indicesSize; iterator++){
		uint iR = indices[0][iterator];
		uint jR = indices[1][iterator];
		string key(to_string(iR)+to_string(jR));
		cout<< iR <<" "<< jR<<" = "<<R[key]<<endl;
	}
}

int main(int argc, char const **argv){
	_map graph;
	vec indices[2], dIndices[2]; // indices[0]=i, indices[1]=j ; d -> duplicate
	loadgraph(argv[1], indices, graph);
	duplicate(indices,dIndices,indices[0].size());
	quicksort(dIndices,0,dIndices[1].size()-1,1,0);
	fquicksort(dIndices); // final quicksort
	Adiamond(graph,indices,dIndices);
	return 0;
}
