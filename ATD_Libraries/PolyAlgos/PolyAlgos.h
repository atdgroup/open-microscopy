#ifndef __WPD_POLY__
#define __WPD_POLY__


/* Polynomial fitting error codes */
#define POLY_ERR_COINCIDENT_WELL_COORD		-6
#define POLY_ERR_ILLEGAL_WELL_COORD			-5
#define POLY_ERR_FIT_SINGULAR_MATIX_1		-4
#define POLY_ERR_FIT_SINGULAR_MATIX_2		-3
#define POLY_ERR_FIT_NO_COEFFS_TO_FIT		-2
#define POLY_ERR_GENERAL_ERROR              -1
#define POLY_ERR_NO_ERROR					 0

#define MATRIX_ALLOCATION_ERROR -1
#define MATRIX_ALLOCATION_OKAY   0

int poly_determine_coeffs(  double  *a,
                            int      numofcoeffs,
                            double  *x,
                            double  *y,
                            double  *z,
                            int      numofdatapts,
                            double  *chisq);


int poly_compute_basis_fct_values(double  x,
                                         double  y,
                                         double *p,
                                         int     numofcoeffs);


double *mtrx_CreateDoubleVector(int nl,int nh, int *result);


void free_dvector(double *v,int nl,int nh);


#endif
