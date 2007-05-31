/*
  The following routines are written and tested by Stefano Merler

  for
  
  Entropy management
*/

#include "func.h"
#include <math.h>

double Entropy(data,n,zero)
     double *data;
     int n;
     double zero;
{
  int i;
  double sum;
  double e;

  sum=0.0;
  for(i=0;i<n;i++)
    sum += data[i];
  if(fabs(sum-1.0) > zero)
    return(-1.0);
  else{
    e=0.0;
    for(i=0;i<n;i++)
      e += clog(data[i],zero);
  }
  return(e);
}

double clog(x,zero)
     double x;
     double zero;
{
  if(x-zero>0.0)
    return(-1.0*x*log(x)/log(2));
  else
    return(0.0);
}

void histo(data,n,h,nbin)
     double *data, *h;
     int n,nbin;
{
  int i;
  for(i=0;i<nbin;i++)
    h[i]=0.0;
  for(i=0;i<n;i++)
    h[(int) floor(data[i]*nbin)] += 1.0;

  for(i=0;i<nbin;i++)
    h[i] /= (double) n;
}


void histo1(data,n,h,nbin)
     double *data;
     int n,nbin;
     int *h;

{
  int j;
  for(j=0;j<nbin;j++)
    h[j]=0.0;
  for(j=0;j<n;j++){
    if (data[j]==1.0){
      h[nbin-1] +=1.0;
    }
    else    
      h[(int) floor(data[j]*nbin)] += 1.0; 
  }
}
