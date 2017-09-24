#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "graphReader.hh"
#include "timer.hh"

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

void graphMul(vec& M,const size_t& Mr,const size_t& Mc){
   /*
   This function will multiply a matrix by itself
   M -> Matrix, Mr -> Matrix rows, Mc -> Matrix columns
   */
   size_t MRsize = Mr*Mc;
   vec MR; /* MR -> Matrix Result will contain the result */
   MR.resize(MRsize);

   for(size_t i=0; i<Mr; i++)
      for(size_t j=0; j<Mc; j++){
         double data = 0.0;
         for(size_t k=0; k<Mr; k++) data += M[i*Mc+k] * M[k*Mc+j];
         MR[i*Mc+j] = data;
      }
   hardrive(MR,Mr,Mc);
}

int main(int argc, char const *argv[]) {
   if(argc != 2){cerr <<"There should be 2 arguments!"<<endl;exit(1);}
   size_t nodes, rows, cols;
   vec graph = readGraph<int>(argv[1],nodes);
   rows = cols = nodes;
   Timer timer("matrixV0.c");
   graphMul(graph,rows,cols);
   cout<<"Elapsed time = "<< timer.elapsed();
   return 0;
}
