#include "PolyAlgos.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*=================================================================================*/
/* Function:  mtrx_CreateIntegerVector,                                            */
/*            mtrx_CreateDoubleVector,                                             */
/*            mtrx_CreateDoubleMatrix,                                             */
/*            free_ivector,                                                        */
/*            free_dvector,                                                        */
/*            free_dmatrix.                                                        */
/*---------------------------------------------------------------------------------*/
/* Purpose:   Vector and matrix creation and destruction for types 'int' and       */
/*            'double'.                                                            */
/*---------------------------------------------------------------------------------*/
/* Ref:       None.                                                                */
/*---------------------------------------------------------------------------------*/
/* Notes:     Modified from and utilised throughout Numerical Recipes.             */
/*---------------------------------------------------------------------------------*/
/* Arguments: nl - low index, nh - high index                                      */
/*            (i.e. defining vector with nh-nl+1 elements).                        */
/*            result: location to store result of vector/matrix creation.          */
/*---------------------------------------------------------------------------------*/
/* Return:    Pointer to the location of the '0' element of the vector (which, of  */
/*            course, may not have been malloced as nl does not have to be zero).  */
/*=================================================================================*/


int *mtrx_CreateIntegerVector(int nl, int nh, int *result)
{
    int *v;

    v = (int *) malloc((unsigned) (nh-nl+1)*sizeof(int));

    if (!v)
        *result = MATRIX_ALLOCATION_ERROR;
    else
        *result = MATRIX_ALLOCATION_OKAY;

    return v-nl;
}


double *mtrx_CreateDoubleVector(int nl,int nh, int *result)
{
    double *v;

    v = (double *) malloc((unsigned)(nh-nl+1)*sizeof(double));

    if (!v)
        *result = MATRIX_ALLOCATION_ERROR;
    else
        *result = MATRIX_ALLOCATION_OKAY;

    return v-nl;
}


double **mtrx_CreateDoubleMatrix(int nrl,int nrh,int ncl,int nch, int *result)
{
    int      i;
    double **m;

    m = (double **) malloc((unsigned)(nrh-nrl+1)*sizeof(double*));

    if (!m)
    {
        *result = MATRIX_ALLOCATION_ERROR;
        return NULL;
    }
    else
    {
        m -= nrl;

        for (i=nrl; i<=nrh; i++)
        {
            m[i] = (double *) malloc((unsigned)(nch-ncl+1)*sizeof(double));

            if (!m[i])
            {
                *result = MATRIX_ALLOCATION_ERROR;
                return NULL;			
            }
            else
                m[i] -= ncl;  
        }

        *result = MATRIX_ALLOCATION_OKAY;

        return m;
    }
}


void free_ivector(int *v,int nl,int nh)
{
    free((char*) (v+nl));
}


void free_dvector(double *v,int nl,int nh)
{
    free((char*) (v+nl));
}
 

void free_dmatrix(double **m,int nrl,int nrh,int ncl,int nch)
{
    int i;

    for (i=nrh;i>=nrl;i--)
        free((char*) (m[i]+ncl));

    free((char*) (m+nrl));
}




static double swap;
#define WPD_SWAP(a,b) {swap=(a);(a)=(b);(b)=swap;}

static double sqrarg;
#define SQR(a)      ((sqrarg=(a)) == 0.0 ? 0.0 : sqrarg*sqrarg) 


/*=================================================================================*/
/* Function:  covsrt                                                               */
/*---------------------------------------------------------------------------------*/
/* Purpose:   Expand in storage the covariance matrix, so as to take into account  */
/*            parameters that are being held fixed.                                */
/*            [1] - Section 15.4, General Linear Least Squares, p.675).            */
/*---------------------------------------------------------------------------------*/
/* Ref:       [1] - Numerical Recipes in C (2nd edition).                          */
/*---------------------------------------------------------------------------------*/
/* Notes:     None.                                                                */
/*---------------------------------------------------------------------------------*/
/* Arguments: x, y:        values for basis function computation.                  */
/*            p:           vector to store the computed basis function values.     */
/*            numofcoeffs: currently unused.                                       */
/*---------------------------------------------------------------------------------*/
/* Return:    POLY_ERR_NO_ERROR.                                                    */
/*=================================================================================*/

static void covsrt(double **covar, int ma, int ia[], int mfit)
{
    int    i, j, k;
    double swap;

    for (i=mfit+1; i<=ma; i++)
    {
        for (j=1; j<=i; j++)
        {
            covar[i][j] = covar[j][i] = 0.0;
        }
    }

    k = mfit;

    for (j=ma; j>=1; j--)
    {
        if (ia[j])
        {
            for (i=1; i<=ma; i++) WPD_SWAP(covar[i][k], covar[i][j])
            for (i=1; i<=ma; i++) WPD_SWAP(covar[k][i], covar[j][i])

            k--;
        }
    }
}


/*=================================================================================*/
/* Function:  poly_compute_basis_fct_values                                        */
/*---------------------------------------------------------------------------------*/
/* Purpose:   Implements computation of the 'basis functions' for a polynomial of  */
/*            the form z = a1 + a2.x + a3.x^2 + a4.y + a5.y^2 + a6.x.y,            */
/*            [1] - Eqn 15.4.2 (Section 15.4, General Linear Least Squares, p.671).*/
/*---------------------------------------------------------------------------------*/
/* Ref:       [1] - Numerical Recipes in C (2nd edition).                          */
/*---------------------------------------------------------------------------------*/
/* Notes:     None.                                                                */
/*---------------------------------------------------------------------------------*/
/* Arguments: x, y:        values for basis function computation.                  */
/*            p:           vector to store the computed basis function values.     */
/*            numofcoeffs: currently unused.                                       */
/*---------------------------------------------------------------------------------*/
/* Return:    POLY_ERR_NO_ERROR.                                                    */
/*=================================================================================*/

int poly_compute_basis_fct_values(double  x,
                                  double  y,
                                  double *p,
                                  int     numofcoeffs)
{
    p[0] = 0.0;
    p[1] = 1.0;
    p[2] = x;
    p[3] = x*x;
    p[4] = y;
    p[5] = y*y;
    p[6] = x*y;

    return POLY_ERR_NO_ERROR;
}


/*=================================================================================*/
/* Function:  gaussj                                                               */
/*---------------------------------------------------------------------------------*/
/* Purpose:   Linear equation solution by Gauss-Jordan elimination.                */
/*            [1] - Section 2.1, p.36, Gauss-Jordan Elimination.                   */
/*---------------------------------------------------------------------------------*/
/* Ref:       [1] - Numerical Recipes in C (2nd edition).                          */
/*---------------------------------------------------------------------------------*/
/* Notes:     Modified version (for error reporting) of routine 'gaussj' of [1].   */
/*---------------------------------------------------------------------------------*/
/* Arguments: a[1..n][1..n]: input matrix, [1]-Eqn. 2.1.1.                         */
/*            b[1..n][1..m]: input matrix containing m RHS vectors, [1]-Eqn. 2.1.1.*/
/*            m, n:          matrix dimensions, see above.                         */
/*---------------------------------------------------------------------------------*/
/* Return:    Appropriate error code.                                              */
/*=================================================================================*/

static int gaussj(double **a, int n, double **b, int m)
{
    int    *indxc, *indxr, *ipiv;
    int    i, icol, irow, j, k, l, ll;
    double big, dum, pivinv;
    int    err1, err2, err3;

    indxc = mtrx_CreateIntegerVector(0,n, &err1);
    indxr = mtrx_CreateIntegerVector(0,n, &err2);
    ipiv  = mtrx_CreateIntegerVector(0,n, &err3);

    if ((err1 != MATRIX_ALLOCATION_OKAY) ||
        (err2 != MATRIX_ALLOCATION_OKAY) ||
        (err3 != MATRIX_ALLOCATION_OKAY))
        return POLY_ERR_GENERAL_ERROR;

    for (j=1; j<=n; j++)
        ipiv[j] = 0;

    for (i=1; i<=n; i++)
    {
        big = 0.0;

        for (j=1; j<=n; j++)
        {
            if (ipiv[j] != 1)
            {
                for (k=1; k<=n; k++)
                {
                    if (ipiv[k] == 0)
                    {
                        if (fabs(a[j][k]) >= big)
                        {
                            big=fabs(a[j][k]);
                            irow=j;
                            icol=k;
                        }
                    }
                    else if (ipiv[k] > 1) /* ERROR: Singular Matrix (-1) */
                    {
                        free_ivector(ipiv, 0,n);
                        free_ivector(indxr,0,n);
                        free_ivector(indxc,0,n);

                        return POLY_ERR_FIT_SINGULAR_MATIX_1;
                    }
                }
            }
        }

        ++(ipiv[icol]);

        if (irow != icol)
        {
            for (l=1; l<=n; l++) WPD_SWAP(a[irow][l], a[icol][l])
            for (l=1; l<=m; l++) WPD_SWAP(b[irow][l], b[icol][l])
        }

        indxr[i] = irow;
        indxc[i] = icol;

        if (a[icol][icol] == 0.0) /* ERROR: Singular Matrix (-2) */
        {
            free_ivector(ipiv, 0,n);
            free_ivector(indxr,0,n);
            free_ivector(indxc,0,n);

            return POLY_ERR_FIT_SINGULAR_MATIX_2;
        }

        pivinv        = 1.0/a[icol][icol];
        a[icol][icol] = 1.0;

        for (l=1; l<=n; l++) a[icol][l] *= pivinv;
        for (l=1; l<=m; l++) b[icol][l] *= pivinv;

        for (ll=1; ll<=n; ll++)
            if (ll != icol)
            {
                dum         = a[ll][icol];
                a[ll][icol] = 0.0;

                for (l=1; l<=n; l++) a[ll][l] -= a[icol][l]*dum;
                for (l=1; l<=m; l++) b[ll][l] -= b[icol][l]*dum;
        }
    }

    for (l=n; l>=1; l--)
    {
        if (indxr[l] != indxc[l])
        {
            for (k=1;k<=n;k++)
                WPD_SWAP(a[k][indxr[l]], a[k][indxc[l]]);
        }
    }

    free_ivector(ipiv, 0,n);
    free_ivector(indxr,0,n);
    free_ivector(indxc,0,n);

    return POLY_ERR_NO_ERROR;
}


/*=================================================================================*/
/* Function:  fit_NrModifiedLinearFit                                              */
/*---------------------------------------------------------------------------------*/
/* Purpose:   General linear least squares fitting.                                */
/*            [1] - Section 15.4, p.671, General Linear Least Squares.             */
/*---------------------------------------------------------------------------------*/
/* Ref:       [1] - Numerical Recipes in C (2nd edition).                          */
/*---------------------------------------------------------------------------------*/
/* Notes:     Modified version (multi-dimensional fitting and error reporting) of  */
/*            routine 'lfit' of [1].                                               */
/*---------------------------------------------------------------------------------*/
/* Arguments: x, y, z: set of 'ndat' data points.                                  */
/*            sig:     individual standard deviation of data points.               */
/*            ndat:    the number of data points (i.e. (x,y,z) triplets).          */
/*            a:       vector for storage of fitted polynomial coefficient values. */
/*            ia:      indicates components (i.e. coefficients) to be fitted.      */
/*            ma:      the number of polynomial coefficients.                      */
/*            covar:   covariance matrix storage.                                  */
/*            chisq:   location to write determined chi-squared value.             */
/*            funcs:   routine for polynomial basis function computation.          */
/*---------------------------------------------------------------------------------*/
/* Return:    Appropriate error code.                                              */
/*=================================================================================*/

static int fit_NrModifiedLinearFit( double   *x,
                                    double   *y,
                                    double   *z,
                                    double    sig[],
                                    int       ndat,
                                    double    a[],
                                    int       ia[],
                                    int       ma,
                                    double  **covar,
                                    double   *chisq,
                                    int     (*funcs)(double, double, double*, int))
{
    int     i, j, k, l, m, mfit=0, errcode, err1, err2;
    double  zm, wt, sum, sig2i, **beta, *afunc;

    beta  = mtrx_CreateDoubleMatrix(0,ma,0,1,&err1);
    afunc = mtrx_CreateDoubleVector(0,ma,&err2);

    if ((err1 != MATRIX_ALLOCATION_OKAY) || (err2 != MATRIX_ALLOCATION_OKAY))
        return POLY_ERR_GENERAL_ERROR;

    for (j=1; j<=ma; j++)
    {
	    if (ia[j])
            mfit++;
    }

    if (mfit == 0) /* ERROR: No parameters to be fitted */
    {
        free_dvector(afunc,0,ma);
        free_dmatrix(beta, 0,ma,0,1);

        return POLY_ERR_FIT_NO_COEFFS_TO_FIT;
    }

    for (j=1; j<=mfit; j++)
    {
	    for (k=1; k<=mfit; k++)
            covar[j][k] = 0.0;

	    beta[j][1]=0.0;
    }

    for (i=1; i<=ndat; i++)
    {
	    (*funcs)(x[i], y[i], afunc, ma);
    	
        zm=z[i];
    	
        if (mfit < ma)
        {
            for (j=1;j<=ma;j++)
            {
                if (!ia[j])
                    zm -= a[j]*afunc[j];
            }
        }

        sig2i = 1.0/SQR(sig[i]);
 	
        for (j=0,l=1; l<=ma; l++)
        {
            if (ia[l])
            {
                wt = afunc[l]*sig2i;

                for (j++,k=0,m=1; m<=l; m++)
                {
                    if (ia[m])
                        covar[j][++k] += wt*afunc[m];
                }

                beta[j][1] += zm*wt;
            }
        }
    }

    for (j=2; j<=mfit; j++)
    {
        for (k=1; k<j; k++)
        {
            covar[k][j] = covar[j][k];
        }
    }

    errcode = gaussj(covar, mfit, beta, 1);

    if (errcode < 0)
    {
        free_dvector(afunc,0,ma);
        free_dmatrix(beta, 0,ma,0,1);

        return errcode;
    }

    for (j=0,l=1; l<=ma; l++)
    {
        if (ia[l])
            a[l] = beta[++j][1];
    }

    *chisq = 0.0;

    for (i=1; i<=ndat; i++)
    {
        (*funcs)(x[i], y[i], afunc, ma);
		
        for (sum=0.0,j=1; j<=ma; j++)
            sum += a[j]*afunc[j];

        *chisq += SQR((z[i]-sum)/sig[i]);
    }

    covsrt(covar, ma, ia, mfit);
	
    free_dvector(afunc,0,ma);
    free_dmatrix(beta, 0,ma,0,1);

    return POLY_ERR_NO_ERROR;
}


/*=================================================================================*/
/* Function:  poly_determine_coeffs                                                */
/*---------------------------------------------------------------------------------*/
/* Purpose:   Routine that utilizes the routines of [1] to fit a polynomial to     */
/*            the data points (i.e. the triplets (x,y,z)).                         */
/*---------------------------------------------------------------------------------*/
/* Ref:       [1] - Numerical Recipes in C (2nd edition).                          */
/*---------------------------------------------------------------------------------*/
/* Notes:     None.                                                                */
/*---------------------------------------------------------------------------------*/
/* Arguments: a:            vector for storage of fitted polynomial coefficients.  */
/*            numofcoeffs:  the number of polynomial coefficients.                 */
/*            x, y, z:      set of 'numofdatapts' data points.                     */
/*            numofdatapts: the number of data points (i.e. (x,y,z) triplets).     */
/*            chisq:        location to write determined chi-squared value.        */
/*---------------------------------------------------------------------------------*/
/* Return:    Appropriate error code.                                              */
/*=================================================================================*/

int poly_determine_coeffs(  double  *a,
                            int      numofcoeffs,
                            double  *x,
                            double  *y,
                            double  *z,
                            int      numofdatapts,
                            double  *chisq)
{
    double  *sig, **covar;
    int     *ia, i, errcode, err1, err2, err3;

    ia    = mtrx_CreateIntegerVector(0,numofcoeffs,&err1);
    sig   = mtrx_CreateDoubleVector(0,numofdatapts,&err2);
    covar = mtrx_CreateDoubleMatrix(0,numofcoeffs,0,numofcoeffs,&err3);

    if ((err1 != MATRIX_ALLOCATION_OKAY) ||
        (err2 != MATRIX_ALLOCATION_OKAY) ||
        (err3 != MATRIX_ALLOCATION_OKAY))
        return POLY_ERR_GENERAL_ERROR;

    for (i=0; i<=numofdatapts; i++)
        sig[i] = 1.00;

    for (i=0; i<numofcoeffs; i++)
        ia[i] = 1;

    errcode = fit_NrModifiedLinearFit(x, y, z, sig, numofdatapts, a, ia, numofcoeffs, covar, chisq, poly_compute_basis_fct_values);

    free_dvector(sig,  0,numofdatapts);
    free_ivector(ia,   0,numofcoeffs);
    free_dmatrix(covar,0,numofcoeffs,0,numofcoeffs);

    return (errcode);
}

