#include <stdlib.h>
#include <limits>
#include <thread>
#include "graphReader2.hh"
#include "timer.hh"
#include "devStand.hh"

uint _min(node* pr,node* pc){
	/* it will find the min value between pr row and pc col */
	uint min = numeric_limits<uint>::max();
	while(pr != NULL && pc != NULL){
		if(pr->j == pc->i){
			uint val = pr->value + pc->value;
			if(val < min) min = val;
			pr = pr->right;
			pc = pc->down;
		}
		else if(pr->j > pc->i) pc = pc->down;
		else pr = pr->right;
	}
	return min;
}

void pChunk(vec* graphA,vec* graphB,vec* result,const uint& nodes,uint ini,uint end){
	/* it will process a chunk of data by a thread */
	for(uint i=ini; i<=end && i<nodes; i++)
		for(uint j=0; j<nodes; j++){
			if(i == j) continue;
			node *pr = graphA[0][i]; // pointer to row
			node *pc = graphB[1][j]; // pointer to col
			uint min = _min(pr,pc);
			if(min != numeric_limits<uint>::max()){
				node *newNod = buildNode(i,j,min);
				linkNode(result,newNod);
			}
		}
}

void multiply(vec* graphA,vec* graphB, vec* result,const uint& nodes){
	/* it will simulate an iteration of A diamond algorithm in chunks */
	uint threadsAmount = 7, ti = 0, i = 0, actiThreads = 0;
	thread t[threadsAmount];
	uint chunk = nodes/threadsAmount;
	int pendRows = nodes;
	if(!chunk) chunk = 1;

	do{
		for(ti=0, i; ti<threadsAmount && pendRows > 0; ti++,i++,actiThreads++){
			uint end = (i+1)*chunk - 1;
			uint ini = end - chunk + 1;
			t[ti] = thread(pChunk,graphA,graphB,result,ref(nodes),ini,end);
			pendRows -= chunk;
		}
		for(ti=0; ti<actiThreads; ti++) t[ti].join();
		actiThreads=0;
	}while(pendRows > 0);

}

void Adiamond(vec* graph,uint n,const uint& nodes){
	/* it will simulate A diamond algorithm using a linked lists vector */
	vec result[2];
	result[0].resize(nodes);
	result[1].resize(nodes);
	if(n == 1) return;
	if(n % 2){
		vec copiedGraph[2];
		copiedGraph[1].resize(nodes);
		copiedGraph[0].resize(nodes);
		makeCopy(copiedGraph,graph);
		multiply(graph,graph,result,nodes);
		merge(graph,result);
		result[0].clear(); result[1].clear();
		result[0].resize(nodes); result[1].resize(nodes);
		Adiamond(graph,(n-1)/2,nodes);
		multiply(graph,copiedGraph,result,nodes);
		merge(graph,result);
		destroyGraph(copiedGraph);
		return;
	}
	multiply(graph,graph,result,nodes);
	merge(graph,result);
	Adiamond(graph,n/2,nodes);
}

void exeTimes(char const** argv){
	/* it will execute Adiamond algorithm several times */
	vector<long> runTimes;
	uint times = atoi(argv[2]);
	runTimes.resize(times);

	for(uint t = times, i = 0; t > 0; t--,i++){
		vec graph[2];
		loadgraph(argv[1],graph);
		uint nodes = graph[0].size();
		Timer timer("Adiamond.cpp");
		Adiamond(graph,nodes,nodes);
		runTimes[i] = timer.elapsed();
		show(graph);
		destroyGraph(graph);
	}

	cout<<"Elapsed time (seconds) = "<< arithMean<long>(runTimes)/1000.0<<endl;
	cout<<"Standard deviation = "<< stdDev<long>(runTimes)<<endl;
}

int main(int argc, char const **argv){
	if(argc != 3){cerr << "Usage <"<<argv[0]<<"> <graph path> <times>"<<'\n'; return -1;}
	exeTimes(argv);
	return 0;
}
