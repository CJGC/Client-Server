#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <thread>
#include <vector>

using namespace std;
using vec = vector<double>;
// typedef const size_t

FILE * openFile(FILE *f,char const *fileName){
  /* This function will try to open a file */
  f = fopen(fileName,"r");
  if(f == NULL){printf("File '%s' doesn't exist!\n",fileName);exit(1);}
  return f;
}

void getDimensions(FILE *f,size_t &rows,size_t &columns){
  /* This function will build a matrix M */
  fscanf(f,"%zu",&rows);
  fscanf(f,"%zu",&columns);
  fgetc(f); /* skipping nasty character */
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
  FILE *f = fopen("output.txt","w+");
  for(size_t i=0;i<Mr;i++)
    for(size_t j=0;j<Mc;j++){
      if(j+1 == Mc) fprintf(f,"%.1f\n",M[i*Mc + j]);
      else fprintf(f,"%.1f ",M[i*Mc + j]);
    }
  fclose(f);
}

void threadMul(vec& M,vec& MR,const size_t& Mr,const size_t& Mc,size_t i, size_t j){
  double data = 0.0;
  for(size_t k=0; k<Mr; k++) data += M[i*Mc+k] * M[k*Mc+j];
  MR[i*Mc+j] = data;
}

void matMul(vec& M,const size_t& Mr,const size_t& Mc){
  /*
    This function will multiply a matrix by itself
    M -> Matrix, Mr -> Matrix rows, Mc -> Matrix columns
  */
  vec MR; /* MR -> Matrix Result will contain the result */
  vector<thread> t; // vector threads
  MR.resize(M.size());
  t.resize(M.size());
  size_t ti = 0;

  for(size_t i=0; i<Mr; i++)
    for(size_t j=0; j<Mc; j++, ti++){
      t[ti] = thread(threadMul,ref(M),ref(MR),ref(Mr),ref(Mc),i,j);
    }
  for(ti=0;ti<t.size();ti++) t[ti].join();
  hardrive(MR,Mr,Mc);
}

int main(int argc, char const *argv[]) {
  if(argc != 2){printf("There should be 3 arguments!\n");exit(1);}
  FILE *f=NULL; /* file pointers */
  vec M; /* matrix M */
  size_t Mr=0,Mc=0; /* matrix (rows and columns) */

  /* opening file */
  f = openFile(f,argv[1]);
  /* building matrix */
  getDimensions(f,Mr,Mc);
  /* getting data */
  getData(f,M);
  /* multiplying matrices */
  clock_t begin = clock();
  matMul(M,Mr,Mc);
  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  /* closing file */
  fclose(f);

  printf("\nTime = %f\n",time_spent);
  return 0;
}
