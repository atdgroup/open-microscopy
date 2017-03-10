#include <userint.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

#include "GCI_nrutil.h"

#define NR_END 1
#define FREE_ARG void *

// Added by P Barber to track errors
int GCI_error;

// modified by P Barber to check for errors and set var GCI_error
// 
// void nrerror(char error_text[])
// /* Numerical Recipes standard error handler */
// {
//	fprintf(stderr,"Numerical Recipes run-time error...\n");
//	fprintf(stderr,"%s\n",error_text);
//	fprintf(stderr,"...now exiting to system...\n");
//	exit(1);
// }

double *GCI_vector(long nl, long nh)
/* allocate a double vector with subscript range v[nl..nh] */
{
	double *v;

	v=(double *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(double)));
	if (!v) nrerror("allocation failure in GCI_vector()");
	return v+NR_END-nl;
}

int *GCI_ivector(long nl, long nh)
/* allocate an int vector with subscript range v[nl..nh] */
{
	int *v;

	v=(int *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(int)));
	if (!v) nrerror("allocation failure in GCI_ivector()");
	return v+NR_END-nl;
}

double **GCI_matrix(long nrl, long nrh, long ncl, long nch)
/* allocate a double matrix with subscript range m[nrl..nrh][ncl..nch] */
{
	long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
	double **m;

	/* allocate pointers to rows */
	m=(double **) malloc((size_t)((nrow+NR_END)*sizeof(double*)));
	if (!m) nrerror("allocation failure 1 in GCI_matrix()");
	m += NR_END;
	m -= nrl;

	/* allocate rows and set pointers to them */
	m[nrl]=(double *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(double)));
	if (!m[nrl]) nrerror("allocation failure 2 in GCI_matrix()");
	m[nrl] += NR_END;
	m[nrl] -= ncl;

	for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

	/* return pointer to array of pointers to rows */
	return m;
}

void GCI_free_vector(double *v, long nl, long nh)
/* free a double vector allocated with GCI_vector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

void GCI_free_ivector(int *v, long nl, long nh)
/* free an int vector allocated with GCI_ivector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

void GCI_free_matrix(double **m, long nrl, long nrh, long ncl, long nch)
/* free a double matrix allocated by matrix() */
{
	free((FREE_ARG) (m[nrl]+ncl-NR_END));
	free((FREE_ARG) (m+nrl-NR_END));
}
