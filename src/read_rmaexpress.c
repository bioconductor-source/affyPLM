/************************************************************************
 **
 ** file: read_rmaexpress.c
 **
 ** aim: read RMAExpress computed expression values
 **      
 **
 ** Copyright (C) 2006 Ben Bolstad
 **
 ** created by: B. M. Bolstad   <bmb@bmbolstad.com>
 ** created on: Apr 2, 2006
 **
 ** last modified: January 17, 2003
 **
 ** License: GPL V2 or later
 **
 **
 ** Specific Modification History
 ** Apr 2, 2006 - Initial version
 **
 **
 ***************************************************************************/


#include <R.h>
#include <Rdefines.h>
#include <Rmath.h>
#include <Rinternals.h>

#include "stdlib.h"
#include "stdio.h"







static size_t fread_int32(int *destination, int n, FILE *instream){

  size_t result;
  
  result = fread(destination,sizeof(int),n,instream);

#ifdef WORDS_BIGENDIAN 
  while( n-- > 0 ){
  /* bit flip since all Affymetrix binary files are little endian */
    
    *destination=(((*destination>>24)&0xff) | ((*destination&0xff)<<24) |
		  ((*destination>>8)&0xff00) | ((*destination&0xff00)<<8));  
    destination++;
  }
#endif
  return result;
}



static void swap_float_8(double *destination){

  unsigned char *cptr;
  unsigned char tmp;


  cptr = (unsigned char *)destination;
  tmp = cptr[0];
  cptr[0] = cptr[7];
  cptr[7] = tmp;
  tmp = cptr[1];
  cptr[1] = cptr[6];
  cptr[6] = tmp;
  tmp = cptr[2];
  cptr[2] = cptr[5];
  cptr[5] =tmp;
  tmp = cptr[3];
  cptr[3] = cptr[4];
  cptr[4] = tmp;
  



}









static size_t fread_float64(double *destination, int n, FILE *instream){

  size_t result;

  
  result = fread(destination,sizeof(double),n,instream);


#ifdef WORDS_BIGENDIAN 
  while( n-- > 0 ){
    swap_float_8(destination);
    destination++;
  }
#endif
  
  return result;
}

static size_t fread_char(char *destination, int n, FILE *instream){

  int i=0;
  size_t result;
  
  result = fread(destination,sizeof(char),n,instream);
  
#ifdef WORDS_BIGENDIAN 
  /* Probably don't need to do anything for characters */

#endif

  return result;

}



SEXP read_rmaexpress(SEXP R_filename){

  SEXP intensity,colnames,rownames,dimnames;
  char *filename;
  const char *mode = "rb";
  
  char *buffer;  /* temporary buffer for storing strings */

  char *header;
  char *rmaexpressversion;
  char *cdfname;

  FILE *currentFile;
  
  double *DataMatrix;
 
  int version_number;
  int n_arrays;
  int n_probesets;

  int str_length;

  int i,j;

  filename = CHAR(STRING_ELT(R_filename,0));
  currentFile = fopen(filename,mode);

  if (currentFile == NULL){
    error("Could not open file %s", filename);
  } 

 
  

  fread_int32(&str_length,1,currentFile);

  header = Calloc(str_length,char);
  
  fread_char(header,str_length,currentFile);

  
  if (strcmp(header,"RMAExpressionValues") !=0){
    Free(header);
    error("This file does not look like it contains RMAExpress computed expression values\n");
  }
  Free(header);
  
  fread_int32(&version_number,1,currentFile);

  if (version_number != 1){
    error("This version of this file format not recognized.\n");
  }

  fread_int32(&str_length,1,currentFile);

  rmaexpressversion = Calloc(str_length,char);
  fread_char(rmaexpressversion,str_length,currentFile);
  Free(rmaexpressversion);

  fread_int32(&str_length,1,currentFile);
  cdfname = Calloc(str_length,char);
  fread_char(cdfname,str_length,currentFile);
  Free(cdfname);



  fread_int32(&n_arrays,1,currentFile);
  fread_int32(&n_probesets,1,currentFile);
  

  PROTECT(colnames = allocVector(STRSXP,n_arrays));
  for (i =0; i < n_arrays; i++){
    fread_int32(&str_length,1,currentFile);
    buffer= Calloc(str_length,char);
    fread_char(buffer,str_length,currentFile);
    SET_VECTOR_ELT(colnames,i,mkChar(buffer));
    Free(buffer);
  }

  PROTECT(rownames = allocVector(STRSXP,n_probesets));
  for (j =0; j < n_probesets; j++){
    fread_int32(&str_length,1,currentFile);
    buffer= Calloc(str_length,char);
    fread_char(buffer,str_length,currentFile);
    SET_VECTOR_ELT(rownames,j,mkChar(buffer));
    Free(buffer);
  }
  




  
  PROTECT(intensity = allocMatrix(REALSXP, n_probesets, n_arrays));
  
  PROTECT(dimnames = allocVector(VECSXP,2));
  SET_VECTOR_ELT(dimnames,0,rownames);
  SET_VECTOR_ELT(dimnames,1,colnames);

  setAttrib(intensity, R_DimNamesSymbol, dimnames);
  UNPROTECT(1);


  DataMatrix = NUMERIC_POINTER(intensity);

  fread_float64(&DataMatrix[0],  n_probesets* n_arrays,currentFile);

  UNPROTECT(3);
  return intensity;

}



SEXP read_rmaexpress_header(SEXP R_filename){


  
  SEXP returnvalue,colnames,rownames,dimnames;
  SEXP tmpsxp;

  char *filename;
  const char *mode = "rb";
  
  char *buffer;  /* temporary buffer for storing strings */

  char *header;
  char *rmaexpressversion;
  char *cdfname;

  FILE *currentFile;
  

  int version_number;
  int n_arrays;
  int n_probesets;

  int str_length;

  int i,j;

  filename = CHAR(STRING_ELT(R_filename,0));
  currentFile = fopen(filename,mode);

  if (currentFile == NULL){
    error("Could not open file %s", filename);
  } 
  
  fread_int32(&str_length,1,currentFile);

  header = Calloc(str_length,char);
  
  fread_char(header,str_length,currentFile);

 

  if (strcmp(header,"RMAExpressionValues") !=0){
    Free(header);
    error("This file does not look like it contains RMAExpress computed expression values\n");
  }
  Free(header);
  
  fread_int32(&version_number,1,currentFile);

  if (version_number != 1){
    error("This version of this file format not recognized.\n");
  }

  fread_int32(&str_length,1,currentFile);

  rmaexpressversion = Calloc(str_length,char);
  fread_char(rmaexpressversion,str_length,currentFile);
  
  
  fread_int32(&str_length,1,currentFile);
  cdfname = Calloc(str_length,char);
  fread_char(cdfname,str_length,currentFile);


  fread_int32(&n_arrays,1,currentFile);
  fread_int32(&n_probesets,1,currentFile);
  

  PROTECT(colnames = allocVector(STRSXP,n_arrays));
  for (i =0; i < n_arrays; i++){
    fread_int32(&str_length,1,currentFile);
    buffer= Calloc(str_length,char);
    fread_char(buffer,str_length,currentFile);
    SET_VECTOR_ELT(colnames,i,mkChar(buffer));
    Free(buffer);
  }

  PROTECT(rownames = allocVector(STRSXP,n_probesets));
  for (j =0; j < n_probesets; j++){
    fread_int32(&str_length,1,currentFile);
    buffer= Calloc(str_length,char);
    fread_char(buffer,str_length,currentFile);
    SET_VECTOR_ELT(rownames,j,mkChar(buffer));
    Free(buffer);
  }
  
  PROTECT(returnvalue = allocVector(VECSXP,4));

  PROTECT(tmpsxp=allocVector(STRSXP,1));
  SET_VECTOR_ELT(tmpsxp,0,mkChar(rmaexpressversion));
  Free(rmaexpressversion);
  SET_VECTOR_ELT(returnvalue,0,tmpsxp);
  UNPROTECT(1);
   
  PROTECT(tmpsxp=allocVector(STRSXP,1));
  SET_VECTOR_ELT(tmpsxp,0,mkChar(cdfname));
  Free(cdfname);
  SET_VECTOR_ELT(returnvalue,1,tmpsxp);
  UNPROTECT(1);
  
  SET_VECTOR_ELT(returnvalue,2,colnames);
  SET_VECTOR_ELT(returnvalue,3,rownames);
    





  UNPROTECT(3);
  return returnvalue;

}

