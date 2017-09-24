#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <vector>
#include <cmath>
#include <iostream>
#include "timer.hh"
#include "graphReader.hh"

using namespace std;
using vec = vector<int>;

void hardrive(vec& M,const size_t& Mr,const size_t& Mc){
   /*
   This function will write the result in hardrive
   M -> Matrix, Mr -> Matrix rows, Mc -> Matrix columns
   */
   FILE *f = fopen("graph.out","w+");
   for(size_t i=0;i<Mr;i++)
      for(size_t j=0;j<Mc;j++){
         if(j+1 == Mc) fprintf(f,"%d\n",M[i*Mc + j]);
         else fprintf(f,"%d ",M[i*Mc + j]);
      }
   fclose(f);
}

void dot(vec& M,vec& MR,const size_t& Mr,const size_t& Mc,size_t ini,size_t end){
   /* Each thread will multiply a chunk of matrix M */
   for(size_t i=ini; i<=end && i<Mr; i++)
      for(size_t j=0; j<Mc; j++){
         double data = 0.0;
         for(size_t k=0; k<Mr; k++) data += M[i*Mc+k] * M[k*Mc+j];
         MR[i*Mc+j] = data;
      }
}

void graphMul(vec& M,const size_t& Mr,const size_t& Mc){
   /*
   This function will multiply a matrix by itself
   M -> Matrix, Mr -> Matrix rows, Mc -> Matrix columns
   */
   vec MR;                         // MR -> Matrix Result
   vector<thread> t;               // vector threads t
   MR.resize(M.size());            // defining MR vector size
   t.resize(8);                    // 8 threads
   size_t ti=0, i=0;               // thread index, i index
   size_t chunk = floor((double)Mr/t.size());// chunk for each thread
   size_t actThreads=0;            // active threads
   if(!chunk) chunk=1;             // if not chunk, rows are lower than 8
   int pendRows=Mr;                // pending rows to be processed|
   do{
      for(ti=0, i; ti<t.size() && pendRows > 0; ti++,i++,actThreads++){
         size_t end = (i+1)*chunk - 1;
         size_t ini = end - chunk + 1;
         t[ti] = thread(dot,ref(M),ref(MR),ref(Mr),ref(Mc),ini,end);
         pendRows -= chunk;
      }
      for(ti=0; ti<actThreads; ti++) t[ti].join();
      actThreads=0;
   }while(pendRows > 0);
   hardrive(MR,Mr,Mc);
}

int main(int argc, char const *argv[]){
   if(argc != 2){ cerr << "There should be 2 arguments!" << endl; exit(1);}
   size_t nodes, rows, cols;
   vec graph = readGraph<int>(argv[1],nodes);
   rows = cols = nodes;
   Timer timer("matrixV2.1.cpp");
   graphMul(graph,rows,cols);
   cout << "Elapsed time: " << timer.elapsed();
   return 0;
}
