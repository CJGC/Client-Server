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

void dot(vec& M,vec& MR,const size_t& Mr,const size_t& Mc,size_t i){
   /* Each thread will multiply a chunk of matrix M */
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
   vec MR;                          // MR -> Matrix Result
   vector<thread> t;                // vector threads t
   MR.resize(M.size());             // defining MR vector size
   t.resize(Mr);                    // Mr threads
   size_t ti = 0;                   // thread index
   for(ti=0; ti<t.size(); ti++)
      t[ti] = thread(dot,ref(M),ref(MR),ref(Mr),ref(Mc),ti);
   
   for(ti=0; ti<t.size(); ti++) t[ti].join();
   hardrive(MR,Mr,Mc);
}

int main(int argc, char const *argv[]){
   if(argc != 2){ cerr<<"There should be 2 arguments!"<<endl ;exit(1);}
   size_t nodes, rows, cols;
   vec graph = readGraph<int>(argv[1],nodes);
   rows = cols = nodes;
   Timer timer("matrixV2.cpp");
   graphMul(graph,rows,cols); 
   cout << "Elapsed time: " << timer.elapsed() << endl;
   return 0;
}
