#include "DTYPE.H"
#include "FILTERS.h"
#include "gci_nrutil.h"
#include "GCI_SpectrumUtils.h"
#include <formatio.h>
#include <analysis.h>
#include <ansi_c.h>
#include <utility.h>
#include <userint.h>

//********************************* GCI_getRGBforWl **********************************************************************

void GCI_getRGBforWl(double wl, double *R, double *G, double *B)
{
	// Get rgb proportions for false colour display of a wavelength 380-780 nm.
	     if ( wl < 380.0 ) { *R = 1.0;               *G = 0.0;               *B = 1.0;                }
	else if ( wl < 440.0 ) { *R = (440.0-wl)/(60.0); *G = 0.0;               *B = 1.0;                } // 60.0 is (440.0-380.0)
	else if ( wl < 490.0 ) { *R = 0.0;               *G = (wl-440.0)/(50.0); *B = 1.0;                } // 50.0 is (490.0-440.0)
	else if ( wl < 510.0 ) { *R = 0.0;               *G = 1.0;               *B = (510.0-wl )/(20.0); } // 20.0 is (510.0-490.0)
	else if ( wl < 580.0 ) { *R = (wl-510.0)/(70.0); *G = 1.0;               *B = 0.0;                } // 70.0 is (580.0-510.0)
	else if ( wl < 645.0 ) { *R = 1.0;               *G = (645.0-wl)/(65.0); *B = 0.0;                } // 65.0 is (645.0-580.0)
	else                   { *R = 1.0;               *G = 0.0;               *B = 0.0;                }
}

//********************************* GCI_canvasDisplaySpectrum **********************************************************************

int GCI_canvasDisplaySpectrum (double *w, double *s, int opitcalDensity, int noOfPoints, double displayStart, double displayStop, int panel, int canvas, Rect target, int horizontal, int borderColour, char *message)
{
	int i, j, maxW, colour, minJ, maxJ, ps, d;
	double pw, r, g, b, minS, maxS, fps;
	
	
	// check if target rect is bigger than canvas (allows the use of FULL_RECT = MakeRect(0,0,32767,32767) )
	GetCtrlAttribute (panel, canvas, ATTR_WIDTH, &d);
	if (target.width>d) target.width=d;
	GetCtrlAttribute (panel, canvas, ATTR_HEIGHT, &d);
	if (target.height>d) target.height=d;

	if (horizontal) maxW = target.width;
	else maxW = target.height;
	
	MaxMin1D (s, noOfPoints, &maxS, &maxJ, &minS, &minJ);
	SetCtrlAttribute (panel, canvas, ATTR_PEN_WIDTH, 1);
	
	for (i=0, j=0; i<maxW; i++)
	{
		pw = i * (displayStop-displayStart) / maxW + displayStart;               // wavelength required for this line of pixels
		if (noOfPoints>1)
		{
			while (j<(noOfPoints-2) && pw > w[j+1]) j++;		                         // find roughly this wavelength in x array
			fps = ((pw - w[j]) / (w[j+1]-w[j]) * (s[j+1]-s[j]) + s[j]);  // linear interpolate to find exact value at this wavelength and normalise
		}
		else 
		{
			fps = s[j];
		}
		if (opitcalDensity) ps = (int)(255.0 * exp(-fps*log(10)));   // representing OD spectrum
		else ps = (int)(fps / maxS * 255.0);
		if (ps>255) ps=255;  // make sure ps is ok, can occur when extrapolating, results in incorrect colour.
		else if (ps<0) ps=0;

		GCI_getRGBforWl(pw, &r, &g, &b);
		colour = (int)(r*ps)<<16 | (int)(g*ps)<<8 | (int)(b*ps);
		SetCtrlAttribute (panel, canvas, ATTR_PEN_COLOR, colour);
		if (horizontal) CanvasDrawLine (panel, canvas, MakePoint(target.left+i, target.top), MakePoint(target.left+i, target.top+target.height-1));
		else CanvasDrawLine (panel, canvas, MakePoint(target.left, target.top+i), MakePoint(target.left+target.width-1, target.top+i));
	}
	SetCtrlAttribute (panel, canvas, ATTR_PEN_COLOR, borderColour);
	SetCtrlAttribute (panel, canvas, ATTR_PEN_WIDTH, 3);
	CanvasDrawRect (panel, canvas, target, VAL_DRAW_FRAME);

	CanvasDrawTextAtPoint (panel, canvas, message, VAL_APP_META_FONT, MakePoint(target.left, target.top), VAL_UPPER_LEFT);
	
	return(0);
}

//********************************* GCI_canvasDisplayColourScale **********************************************************************

int GCI_canvasDisplayColourScale (int lut[], int panel, int canvas, Rect target, int horizontal, int borderColour, char *message)
{
	int i, j, maxW, colour, d;
	
	// check if target rect is bigger than canvas (allows the use of FULL_RECT = MakeRect(0,0,32767,32767) )
	GetCtrlAttribute (panel, canvas, ATTR_WIDTH, &d);
	if (target.width>d) target.width=d;
	GetCtrlAttribute (panel, canvas, ATTR_HEIGHT, &d);
	if (target.height>d) target.height=d;
	
	if (horizontal) maxW = target.width;
	else maxW = target.height;
	
	CanvasStartBatchDraw (panel, canvas);
	SetCtrlAttribute (panel, canvas, ATTR_PEN_WIDTH, 1);
	
	for (i=0, j=0; i<maxW; i++)
	{
		colour = lut[(int)(255.0 * (float)i / (float)maxW)];
		SetCtrlAttribute (panel, canvas, ATTR_PEN_COLOR, colour);
		if (horizontal) CanvasDrawLine (panel, canvas, MakePoint(target.left+i, target.top), MakePoint(target.left+i, target.top+target.height-1));
		else CanvasDrawLine (panel, canvas, MakePoint(target.left, target.top+i), MakePoint(target.left+target.width-1, target.top+i));
	}
	SetCtrlAttribute (panel, canvas, ATTR_PEN_COLOR, borderColour);
	SetCtrlAttribute (panel, canvas, ATTR_PEN_WIDTH, 3);
	CanvasDrawRect (panel, canvas, target, VAL_DRAW_FRAME);

	CanvasDrawTextAtPoint (panel, canvas, message, VAL_APP_META_FONT, MakePoint(target.left, target.top), VAL_UPPER_LEFT);

	CanvasEndBatchDraw (panel, canvas);
	
	return(0);
}

//********** GCI_getColTableForWavelength *************************************************************************

void GCI_getColTableForWavelength(int *col, double wl, int overloadPalette)
{
	int i;
	double R, G, B;
	
	GCI_getRGBforWl(wl, &R, &G, &B);
	
	for (i=0; i<254; i++) 
	{
		col[i] = (RoundRealToNearestInteger(i*R)) << 16;
		col[i] |= (RoundRealToNearestInteger(i*G)) << 8;
		col[i] |= RoundRealToNearestInteger(i*B);
	}
	if (overloadPalette)
	{
		R=1-R;
		G=1-G;
		B=1-B;  // invert colours to show overload
	}
	for (i=254; i<256; i++) 
	{
		col[i] = (RoundRealToNearestInteger(i*R)) << 16;
		col[i] |= (RoundRealToNearestInteger(i*G)) << 8;
		col[i] |= RoundRealToNearestInteger(i*B);
	}
}

//********** GCI_getRGBForSpectrum *************************************************************************

int GCI_getRGBForSpectrum(double *y, double *x, int n, double *r, double *g, double *b)
{   // in same fashion as  colourImageFromStack()
	int i;
	double threshold=0.99, rSum, gSum, bSum, rCount, gCount, bCount, max;
	double R, G, B;
	
	for (i=0, rSum=0.0, gSum=0.0, bSum=0.0, rCount=0.0, gCount=0.0, bCount=0.0; i<n; i++)
	{
		GCI_getRGBforWl(x[i], &R, &G, &B);

		if (R>threshold)
		{
			rSum   += y[i];
			rCount += R;
		}
		if (G>threshold)
		{
			gSum   += y[i];
			gCount += G;
		}
		if (B>threshold)
		{
			bSum   += y[i];
			bCount += B;
		}
	}
	
	if (rCount>0.0) rSum /= rCount;
	if (gCount>0.0) gSum /= gCount;
	if (bCount>0.0) bSum /= bCount;
		
	//RJL 21-02-07
	//What's going on here? This makes the reference recalled from file different to what was saved.
	//This function seems not to be used anywhere else, so commented out lines below
	//max = rSum;
	//if (gSum>max) max = gSum;
	//if (bSum>max) max = bSum;
			
	//rSum = rSum / max;
	//gSum = gSum / max;
	//bSum = bSum / max;
		
	*r = rSum;
	*g = gSum;
	*b = bSum;

	return(0);
}

//********** GCI_spectrumAutoContrast1D *************************************************************************

void GCI_spectrumAutoContrast1D (double *s, double *out, int n, double top)
{   // normalise a spectrum to a max of top by changing contrast
	double max=0.0;
	int i;
	
	for (i=0; i<n; i++)
		if (s[i]>max) max=s[i];
		
	for (i=0; i<n; i++)
		out[i] = s[i]*top/max;	
}

//********** GCI_spectralDifference *************************************************************************

double GCI_spectralDifference (double *s1, double *s2, int n, int filter, int rank)
{   // Rothmann et al, Histol. Histopathol. 1998. Assume s1 is already normalised (reference spectrum)
	double *diff, sum=0.0, result;

	diff = (double *) malloc (n*sizeof(double));
	
	// subtract spectra
	Sub1D (s1, s2, n, diff);
	// filter residuals
	if (filter)
//		General_Median_Filter (diff, diff, rank, rank, n, FILTERS_DOUBLE);
		Median_Double_Filter (diff, diff, rank, rank, n);
	// square residuals
	Mul1D (diff, diff, n, diff);
	// sum squared residuals
	Sum1D (diff, n, &sum);
	result = sqrt(sum);
	
	free(diff);

	return (result);
}

//********************************* GCI_spline **********************************************************************

// Find the second derivatives of the function y=f(x) given by cubic
// interpolation, and stick the answers in y2.  This function and the
// next come from Numerical Recipes, section 3.3 with appropriate
// modifications.

int GCI_spline(double *x, double *y, int n, double *y2)
{
	int i,k;
	double p,sig,*u;

	if (n<4) return 0;

	u=GCI_vector(0,n-2);
	if (u==NULL) return -1;
	y2[0]=y2[n-1]=u[0]=0;
	for (i=1;i<n-1;i++) {
		sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);
		p=sig*y2[i-1]+2.0;
		y2[i]=(sig-1.0)/p;
		u[i]=(y[i+1]-y[i])/(x[i+1]-x[i]) - (y[i]-y[i-1])/(x[i]-x[i-1]);
		u[i]=(6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
	}
	for (k=n-2;k>=0;k--)
		y2[k]=y2[k]*y2[k+1]+u[k];
	GCI_free_vector(u,0,n-2);
	return 0;
}


//********** GCI_splintResample1DData *************************************************************************

// resample a data set y=f(x), currently has values at xIn, need values at xTarget, calculate yOut by spline interpolation
// inputs: xIn, yIn, y2In, nIn: x data, y data, 2nd derivative of y data (from GCI_spline), number of input points
// If y2In is NULL, then the routine calls GCI_spline internally
// xTarget, n: target values of x and their number
// yOut: calculated values of y
// yOut must be different from yIn!!
// All of the data Must be in order with lowest x value at xTarget[0] and xIn[0]
// yOut must be big enough to accept the resampled data
// For xTarget values outside xIn range use Extrapolation type:
// 0 - set vals to zero
// 1 - keeps vals same as the end of yIn
// 2 - linearly extrapolate
// 3 - use cubic spline extrapolation
//
// This code is based on the Numerical Recipes code, but has been modified
// to handle extrapolation and multiple x targets.

int GCI_splintResample1DData (double *xIn, double *yIn, double *yIn2, int nIn, double *xTarget, int n, double *yOut, int ExtrapolationType)
{
	int t, klo, khi, k;  /* indices into our arrays */
	double h,b,a,*y2local,*y2;

	if (yOut == yIn)
//		nrerror("In GCI_splintResample1DData, cannot have yOut==yIn");
		return(-2);
	
	if (n<4)  // silly, silly, silly  8-)
		return GCI_resample1DData(xIn, yIn, nIn, xTarget, n, yOut, ExtrapolationType);

	if (yIn2 == NULL) { // Need to calculate it ourselves
		y2local=GCI_vector(0,nIn-1);
		if (y2local==NULL) return -1;
		GCI_spline(xIn, yIn, nIn, y2local);
		y2=y2local;
	} else {
		y2local=NULL;
		y2=yIn2;
	}

	klo=-1;
	khi=nIn;
	while (khi-klo > 1) {
		k=(khi+klo)/2;
		if (xIn[k] > xTarget[0]) khi=k;
		else klo=k;
	}
	k=khi;  // so we'll have our xtarget lying in the range (xIn[k-1], xIn[k])

	for (t=0; t<n; t++) {
		if (k==1) k=0;  // In case we increased k for extrapolation
		while (k<nIn && xTarget[t] > xIn[k]) k++;  // find correct k

		if (k==0)  // need to extrapolate beyond low end
			switch (ExtrapolationType) {
			case 0:
				yOut[t] = 0;
				continue;  // jump to next for(k=...) loop iteration

			case 1:
				yOut[t] = yIn[0];
				continue;  // jump to next for(k=...) loop iteration

			case 2:  // standard linear interpolation formula
				yOut[t] = yIn[0] + (yIn[1]-yIn[0])*(xTarget[t]-xIn[0])/(xIn[1]-xIn[0]);
				continue;  // jump to next for(k=...) loop iteration

			case 3:  // cubic spline extrapolation
				// We extrapolate from the last two points
				k=1;
				break;  // now let the interpolation code handle this case

			default:
//				nrerror("Bad ExtrapolationType in routine GCI_splintResample1DData");
				return(-3);
			}
		else if (k==nIn)  // need to extrapolate beyond high end
			switch (ExtrapolationType) {
			case 0:
				yOut[t] = 0;
				continue;  // jump to next for(k=...) loop iteration

			case 1:
				yOut[t] = yIn[nIn-1];
				continue;  // jump to next for(k=...) loop iteration

			case 2:  // standard linear interpolation formula
				yOut[t] = yIn[nIn-1] + (yIn[nIn-2]-yIn[nIn-1])*(xTarget[t]-xIn[nIn-1])/(xIn[nIn-2]-xIn[nIn-1]);
				continue;  // jump to next for(k=...) loop iteration

			case 3:  // cubic spline extrapolation
				// We extrapolate from the last two points
				k=nIn-1;
				break;  // now let the interpolation code handle this case

			default:
//				nrerror("Bad ExtrapolationType in routine GCI_splintResample1DData");
				return(-4);
			}

		// normal cubic spline interpolation
		h=xIn[k]-xIn[k-1];
		if (h == 0)
//			nrerror("Bad xIn input to routine GCI_splintResample1DData");
			return(-5);
			
		a=(xIn[k]-xTarget[t])/h;
		b=(xTarget[t]-xIn[k-1])/h;
		yOut[t]=a*yIn[k-1] + b*yIn[k] +
			((a*a*a-a)*y2[k-1]+(b*b*b-b)*y2[k])*(h*h)/6.0;
	}  // for(t=0; t<nIn; t++)

	if (y2local != NULL)
		GCI_free_vector(y2local,0,nIn-1);

	return 0;
}


//********** GCI_resample1DData *************************************************************************

// NB This is called by the above routine in the case n<4, so don't remove it!

int GCI_resample1DData (double *xIn, double *yIn, int nIn, double *xTarget, int n, double *yOut, int ExtrapolationType)
{  // resample a data set y=f(x), currently has values at xIn, need values at targetX, calculate yOut by linear interpolation
   // n=number of points
   // NB the wavelength ranges may not overap well, the sampling frequency may be different, there may be different numbers of points
   // the data Must be in order i.e. lowest x value at xTarget[0] and xIn[0].
   // yOut must be big enough to accept the resampled data
   // For xTarget values outside xIn range use Extrapolation type:
   // 0 - set vals to zero
   // 1 - keeps vals same as the end of yIn
   // 2 - linearly extrapolate
   
	int t, x, xFound;     // indexes for the target and input x values
	double f, *yTemp;
	
	if (n<=0)   return(-1);
	if (nIn<=0) return(-1);

	if (nIn==1 && ExtrapolationType==2) ExtrapolationType=1;  // cannot linearly extrapolate from 1 pt.
	
	yTemp = (double *)malloc(n*sizeof(double));
	if (yTemp==NULL) return(-2);
	
	for (t=0; t<n; t++)                      // for each target x value
	{
		xFound=0;
		for (x=0; x<nIn; x++)				 // for each input x value
			if (xIn[x] > xTarget[t])		 // find lowest input x greater than current target
			{
				xFound=1;
				break;
			}
			
		if (xFound==1 && x==0) 					         // current target is less than the lowest input x
		{
			if      (ExtrapolationType==0) yTemp[t] = 0.0;
			else if (ExtrapolationType==1) yTemp[t] = yIn[0];
			else 						   yTemp[t] = yIn[0] + (yIn[1]-yIn[0])*(xTarget[t]-xIn[0])/(xIn[1]-xIn[0]);
		}
		else if (xFound==0) 				           // current target is greater than the highest input x
		{
			if      (ExtrapolationType==0) yTemp[t] = 0.0;
			else if (ExtrapolationType==1) yTemp[t] = yIn[nIn-1];
			else 						   yTemp[t] = yIn[nIn-1] + (yIn[nIn-2]-yIn[nIn-1])*(xTarget[t]-xIn[nIn-1])/(xIn[nIn-2]-xIn[nIn-1]);
		}
		else
		{
			f =  (xTarget[t] - xIn[x-1]) / (xIn[x] - xIn[x-1]);
	
			yTemp[t] = (1-f)*yIn[x-1] + f*yIn[x];
		}
	}

	for (t=0; t<n; t++) yOut[t]=yTemp[t];
	free(yTemp);

	return(0);
}

//********** getIEEESpecialValue *************************************************************************

double getIEEESpecialValue (char buffer[])
{
//#define NaN   ((1.0/0.0)/(1.0/0.0))
#define NaN   sqrt(-1)
//#define NaN   generateNaN()
//#define Inf  (1.0/0.0)
#define Inf  NaN
//#define Inf  generateInf()

	double d;

	if      (CompareStrings (buffer, 0, "NaN", 0, 1)==0)  d=NaN;
	else if (CompareStrings (buffer, 0, "Inf", 0, 1)==0)  d=Inf;
	else if (CompareStrings (buffer, 0, "+Inf", 0, 1)==0) d=Inf;
	else if (CompareStrings (buffer, 0, "-Inf", 0, 1)==0) d=-Inf;
	else d=0.0;

	return (d);

#undef NaN
#undef Inf
}

//********** GCI_loadXYData *************************************************************************

int GCI_loadXYData (double **xData, double **yData, int *number, char *fileName)
{
	int i, fileHandle, n, ok;
	char buffer[256];

	fileHandle = OpenFile (fileName, VAL_READ_ONLY, VAL_TRUNCATE, VAL_ASCII);
	if (fileHandle<0) {MessagePopup ("Error", "File access error. Sorry."); return (-1);} 

	// Title
	ScanFile (fileHandle, "%l>%s[t-]",  buffer);  // read to end of line ignoring spaces etc.

	// Filename
	ScanFile (fileHandle, "%l>%s[t-]",  buffer);  // read to end of line ignoring spaces etc.

	ok = ScanFile (fileHandle, "%d", &n);    // try to read number of data points
	if (ok<1) 								 // if this fails
	{
		// some spec files have an extra line for data type, read this first
		SetFilePtr (fileHandle, -1, 1);		          // back up to get beggining of string
		ScanFile (fileHandle, "%l>%s[t-]",  buffer);  // read to end of line ignoring spaces etc.
		ScanFile (fileHandle, "%d", &n);              // then read number of points
	}
			
//	if (CompareStrings (buffer, 0, "Fluorescence", 0, 0)==0 || CompareStrings (buffer, 0, "Emission", 0, 0)==0) 
//	     *spectrumType=1; 
//	else *spectrumType=0;
	
	// sanity check on n
	if (n<0) return (-1);
	if (n>10000)
	{
		sprintf(buffer, "Large number of data points, %d\nThis may be an error. Continue?", n);
		if (ConfirmPopup ("Warning", buffer)==0)
			return(-1);
	}

	*number = n;

	// make space
	if (*xData!=NULL) free(*xData);
	*xData = (double *)malloc(n*sizeof(double));
	if (*yData!=NULL) free(*yData);
	*yData = (double *)malloc(n*sizeof(double));

	// Data
	for (i=0; i<n; i++)
	{
		ok = ScanFile (fileHandle, "%f, ", &((*xData)[i]));
		if (ok<1) 
		{
			SetFilePtr (fileHandle, -1, 1);		  // back up to get beggining of string
			ScanFile (fileHandle, "%s", buffer);  // try to pick up "NaN" and "-Inf", "+Inf"
			(*xData)[i] = getIEEESpecialValue (buffer);
		}
		ok = ScanFile (fileHandle, "%f\n", &((*yData)[i]));
		if (ok<1) 
		{
			SetFilePtr (fileHandle, -1, 1);		  // back up to get beggining of string
			ScanFile (fileHandle, "%s", buffer);  // try to pick up "NaN" and "-Inf", "+Inf"
			(*yData)[i] = getIEEESpecialValue (buffer);
		}
	}	

	CloseFile (fileHandle);

	return(0);
}

//********** GCI_saveXYData *************************************************************************

int GCI_saveXYData (char title[], char type[], int number, double xData[], double yData[], char fileName[])
{
		int i, fileHandle;
	char name[MAX_FILENAME_LEN];
			
	fileHandle = OpenFile (fileName, VAL_WRITE_ONLY, VAL_TRUNCATE, VAL_ASCII);
	if (fileHandle<0) {MessagePopup ("Error", "File access error. Sorry."); return (-1);} 
	SplitPath (fileName, NULL, NULL, name);

	// Title
	FmtFile (fileHandle, "%s\n", title);
	FmtFile (fileHandle, "%s\n", name);
	FmtFile (fileHandle, "%s\n", type);
	FmtFile (fileHandle, "%d\n", number);

	// Data
	for (i=0; i<number; i++)
	{
		FmtFile (fileHandle, "%f, ", xData [i]);
		FmtFile (fileHandle, "%f\n", yData [i]);
	}	

	CloseFile (fileHandle);
	
	return(0);
}

//********************************* GCI_truncateSpectrum **********************************************************************

int GCI_truncateSpectrum(double *in, double *inWls, double *inSd, int inN, double min, double max, double *out, double *outWls, double *outSd, int *outN)
{
	int i, j;
	
	for (i=0, j=0; i<inN; i++)
	{
		if (inWls[i]>min && inWls[i]<max)
		{
			out   [j] = in   [i];
			outWls[j] = inWls[i];
			outSd [j] = inSd [i];
			j++;
		}
	}

	*outN = j;

	return(0);
}

//********************************* GCI_truncateAndFilterSpectrumForNaN **********************************************************************

int GCI_truncateAndFilterSpectrumForNaN(double *in, double *inWls, double *inSd, int inN, double min, double max, double *out, double *outWls, double *outSd, int *outN)
/*****************************************************************************/
/* The function dtype will determine the type of double. It assumes that the */
/* double is in IEEE 754 format.                                             */
/* returns	0  reg number                                                    */
/*	1  +infinity                                                             */
/*	-1 -infinity                                                             */
/*	2  NaN                                                                   */
/*	3  underflow                                                             */
/*	4  0				                                                     */
/*****************************************************************************/
// should work in place as i will always increment faster than j
{
	int i, j, type, sd=1;
	
	if (inSd==NULL) sd=0;
/*	
	for (i=0, j=0; i<inN; i++)
	{
		type = dtype(in[i]);
		if (inWls[i]>min && inWls[i]<max && (type==0 || type==4))
		{
			out   [j] = in   [i];
			outWls[j] = inWls[i];
			if (sd) outSd [j] = inSd [i];
			j++;
		}
	}
*/
	// assuming data are sequential
	for (i=0; i<inN, inWls[i]<min; i++)
		;

	for (j=0; i<inN; i++)
	{
		if (inWls[i]<max)
		{
			type = dtype(in[i]);
			if (type==0 || type==4)
			{
				out   [j] = in   [i];
				outWls[j] = inWls[i];
				if (sd) outSd [j] = inSd [i];
				j++;
			}
		}
		else break;
	}

	*outN = j;

	return(0);
}

//********************************* GCI_filterSpectrumForNaN **********************************************************************

int GCI_filterSpectrumForNaN(double *in, double *inWls, double *inSd, int inN, double *out, double *outWls, double *outSd, int *outN)
/*****************************************************************************/
/* The function dtype will determine the type of double. It assumes that the */
/* double is in IEEE 754 format.                                             */
/* returns	0  reg number                                                    */
/*	1  +infinity                                                             */
/*	-1 -infinity                                                             */
/*	2  NaN                                                                   */
/*	3  underflow                                                             */
/*	4  0				                                                     */
/*****************************************************************************/
// should work in place as i will always increment faster than j
{
	int i, j, type, sd=1;

	if (inSd==NULL) sd=0;
	
	for (i=0, j=0; i<inN; i++)
	{
		type = dtype(in[i]);
		if (type==0 || type==4)
		{
			out   [j] = in   [i];
			outWls[j] = inWls[i];
			if (sd) outSd [j] = inSd [i];
			j++;
		}
	}

	*outN = j;

	return(0);
}

double max_of(double red, double green, double blue)
{
	double max;
     
    if (red>green) max=red;
    else max=green;
    if (blue>max)  max=blue;
	return(max);
}

double min_of(double red, double green, double blue)
{
	double min;
     
    if (red<green) min=red;
    else min=green;
    if (blue<min)  min=blue;
	return(min);
}

//********************************* GCI_RGBToHLS **********************************************************************

int GCI_RGBToHLS (double R, double G, double B, double *H, double *L, double *S)
{
// adapted from
// http://astronomy.swin.edu.au/~pbourke/colour/conversion.html, Compiled by Paul Bourke, February 1994 
// and seems to correspond to how CVI and PSP seem to work according to lab book 2, p25.
	const double small_value = 0.0000001, undefined = 32767;   // maxint?
	double max_value, min_value, diff, r_dist, g_dist, b_dist;
                                                        
	max_value = max_of(R, G, B);
	min_value = min_of(R, G, B);
	diff = max_value - min_value;

	*L = (max_value + min_value) / 2.0;
	
	if(fabs(diff)<=small_value)
	{
		*H = undefined;
		*S = 0.0;
	}
	else
	{
		if (*L < 0.5) *S = diff/(max_value+min_value);
		else          *S = diff/(2.0-max_value-min_value);      

		r_dist = (max_value-R)/diff;
		g_dist = (max_value-G)/diff;
		b_dist = (max_value-B)/diff;

		if      (R==max_value) *H =     b_dist - g_dist;
		else if (G==max_value) *H = 2 + r_dist - b_dist;
		else if (B==max_value) *H = 4 + g_dist - r_dist;
		
		*H *= 60.0;
		
		if (*H < 0.0) *H += 360.0;
	}
	return(0);
}

double RGB (double q1, double q2, double hue)
{
	if      (hue > 360.0) hue -= 360.0;
	else if (hue < 0.0)   hue += 360.0;
	if (hue < 60.0)
		return(q1+(q2-q1)*hue/60.0);
	else if (hue < 180.0) 
		return(q2);
	else if (hue < 240.0) 
		return(q1+(q2-q1)*(240.0-hue)/60.0);
	else
		return(q1);
}

//********************************* GCI_HLSToRGB **********************************************************************

int GCI_HLSToRGB (double H, double L, double S, double *R, double *G, double *B)
{
// adapted from
// http://astronomy.swin.edu.au/~pbourke/colour/conversion.html, Compiled by Paul Bourke, February 1994 
// and seems to correspond to how CVI and PSP seem to work according to lab book 2, p25.
	double p1, p2;

	if (L <= 0.5) p2 = L*(1+S);
	else          p2 = L+S-(L*S);
	p1 = 2.0*L-p2;

	if (S == 0.0)
	{
		*R = L; 
		*G = L;
		*B = L;
	}
	else
	{
		*R = RGB(p1, p2, H+120.0);
		*G = RGB(p1, p2, H);
		*B = RGB(p1, p2, H-120.0);
	}
	return(0);
}
