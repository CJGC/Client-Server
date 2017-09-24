#include <stdio.h>
#include <stdlib.h>
#include "timer.hh"
#include <iostream>

using namespace std;

FILE * openFile(char const *fileName,FILE *f){
   /* This function will try to open a file */
   f = fopen(fileName,"r");
   if(f == NULL){printf("File '%s' doesn't exist!\n",fileName);exit(1);}
   return f;
}

double * buildMatrix(FILE *f,size_t &rows,size_t &columns){
   /* This function will build a matrix M */
   fscanf(f,"%zu",&rows);
   fscanf(f,"%zu",&columns);
   fgetc(f); /* skipping nasty character */
   double *M = (double *)malloc(rows*columns*sizeof(double));
   return M;
}

void getData(FILE *f, double *M){
   /* This function will capture data from plain text file to system memory */
   char *data = (char *)malloc(sizeof(char)), *newData = NULL,ch = ' ';
   size_t dataSize = sizeof(char), Mindex = 0;
   while(!feof(f)){
      ch = fgetc(f);
      if(ch == ' ' || ch == '\n'){
         data[dataSize-1] = '\0';
         M[Mindex] = strtof(data,NULL);
         free(data);
         data = (char *)malloc(sizeof(char));
         newData = NULL;
         dataSize = sizeof(char);
         Mindex++;
         continue;
      }
      data[dataSize-1] = ch;
      newData = (char*)realloc(data,sizeof(char));
      data = newData;
      dataSize++;
   }
   free(data);
}

void hardrive(double *M,size_t Mr,size_t Mc){
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

void mulMatrices(double *M,size_t Mr,size_t Mc){
   /*
   This function will multiply a matrix by itself
   M -> Matrix, Mr -> Matrix rows, Mc -> Matrix columns
   */
   size_t MRsize = Mr*Mc;
   double MR[MRsize]; /* MR -> Matrix Result will contain the result */

   for(size_t i=0; i<Mr; i++)
      for(size_t j=0; j<Mc; j++){
         double data = 0.0;
         for(size_t k=0; k<Mr; k++) data += M[i*Mc+k] * M[k*Mc+j];
         MR[i*Mc+j] = data;
      }
   hardrive(MR,Mr,Mc);
}

int main(int argc, char const *argv[]) {
   if(argc != 2){printf("There should be 3 arguments!\n");exit(1);}
   FILE *f=NULL; /* file pointers */
   double *M; /* matrix M */
   size_t Mr=0,Mc=0; /* matrix (rows and columns) */

   /* opening file */
   f = openFile(argv[1],f);
   /* building matrix */
   M = buildMatrix(f,Mr,Mc);
   /* getting data */
   getData(f,M);
   Timer timer("matrixV0.c");
   /* multiplying matrices */
   mulMatrices(M,Mr,Mc);
   cout<<"elapsed time = "<< timer.elapsed();
   /* freeing memory */
   free(M);
   /* closing file */
   fclose(f);
   return 0;
}
