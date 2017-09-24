#include <stdio.h>
#include <stdlib.h>
#include "timer.hh"
#include <thread>
#include <vector>
#include <cmath>
#include <iostream>

using namespace std;
using vec = vector<double>;

FILE * openFile(FILE *f,char const *fileName){
   /* This function will try to open a file */
   f = fopen(fileName,"r");
   if(f == NULL){printf("File '%s' doesn't exist!\n",fileName);exit(1);}
   return f;
}

void getDimensions(FILE *f,size_t &rows,size_t &columns){
   /* This function will build a matrix M */
   fscanf(f,"%zu",&rows);    // setting rows var
   fscanf(f,"%zu",&columns); // setting columns var
   fgetc(f);                 // skipping nasty character
}

void getData(FILE *f,vec& M){
   /* This function will capture data from plain text file to system memory */
   char *data = (char *)malloc(sizeof(char)), *newData = NULL,ch = ' ';
   size_t dataSize = sizeof(char);
   while(!feof(f)){
      ch = fgetc(f);
      if(ch == ' ' || ch == '\n'){
         data[dataSize-1] = '\0';
         M.push_back(strtof(data,NULL));
         free(data);
         data = (char *)malloc(sizeof(char));
         newData = NULL;
         dataSize = sizeof(char);
         continue;
      }
      data[dataSize-1] = ch;
      newData = (char*)realloc(data,sizeof(char));
      data = newData;
      dataSize++;
   }
   free(data);
}

void hardrive(vec& M,const size_t& Mr,const size_t& Mc){
   /*
   This function will write the result in hardrive
   M -> Matrix, Mr -> Matrix rows, Mc -> Matrix columns
   */
   FILE *f = fopen("M_out.txt","w+");
   for(size_t i=0;i<Mr;i++)
      for(size_t j=0;j<Mc;j++){
         if(j+1 == Mc) fprintf(f,"%.1f\n",M[i*Mc + j]);
         else fprintf(f,"%.1f ",M[i*Mc + j]);
      }
   fclose(f);
}

void threadMul(vec& M,vec& MR,const size_t& Mr,const size_t& Mc,size_t i){
   /* Each thread will multiply a chunk of matrix M */
   for(size_t j=0; j<Mc; j++){
      double data = 0.0;
      for(size_t k=0; k<Mr; k++) data += M[i*Mc+k] * M[k*Mc+j];
      MR[i*Mc+j] = data;
   }
}

void matMul(vec& M,const size_t& Mr,const size_t& Mc){
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
      t[ti] = thread(threadMul,ref(M),ref(MR),ref(Mr),ref(Mc),ti);
   
   for(ti=0; ti<t.size(); ti++) t[ti].join();
   hardrive(MR,Mr,Mc);
}

int main(int argc, char const *argv[]){
   if(argc != 2){printf("There should be 3 arguments!\n");exit(1);}
   FILE *f=NULL;               // file pointer
   vec M;                      // matrix M
   size_t Mr=0,Mc=0;           // matrix (rows and columns)
   f = openFile(f,argv[1]);    // opening file
   getDimensions(f,Mr,Mc);     // getting matrix rows and cols
   getData(f,M);               // getting disk data
   Timer timer("matrixV2.1.cpp");
   matMul(M,Mr,Mc);            // matrix multiplication
   cout << "Elapsed time: " << timer.elapsed();
   fclose(f);                  // closing file
   return 0;
}
