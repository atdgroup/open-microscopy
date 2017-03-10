#ifndef __SPECTRUM_UTILS__
#define __SPECTRUM_UTILS__

#include <userint.h>

#define FULL_RECT MakeRect(0,0,32767,32767)

void GCI_getRGBforWl(double wl, double *R, double *G, double *B);
int GCI_canvasDisplaySpectrum (double *w, double *s, int opitcalDensity, int noOfPoints, double displayStart, double displayStop, int panel, int canvas, Rect target, int horizontal, int borderColour, char *message);
int GCI_canvasDisplayColourScale (int lut[], int panel, int canvas, Rect target, int horizontal, int borderColour, char *message);
void GCI_getColTableForWavelength(int *col, double wl, int overloadPalette);
int GCI_getRGBForSpectrum(double *y, double *x, int n, double *r, double *g, double *b);
void GCI_spectrumAutoContrast1D (double *s, double *out, int n, double top);
double GCI_spectralDifference (double *s1, double *s2, int n, int filter, int rank);
int GCI_spline(double *x, double *y, int n, double *y2);
int GCI_splintResample1DData (double *xIn, double *yIn, double *yIn2, int nIn, double *xTarget, int n, double *yOut, int ExtrapolationType);
int GCI_resample1DData (double *xIn, double *yIn, int nIn, double *xTarget, int n, double *yOut, int ExtrapolationType);
int GCI_loadXYData (double *xData[], double *yData[], int *number, char *fileName);
int GCI_saveXYData (char title[], char type[], int number, double xData[], double yData[], char fileName[]);
int GCI_truncateSpectrum(double *in, double *inWls, double *inSd, int inN, double min, double max, double *out, double *outWls, double *outSd, int *outN);
int GCI_truncateAndFilterSpectrumForNaN(double *in, double *inWls, double *inSd, int inN, double min, double max, double *out, double *outWls, double *outSd, int *outN);
int GCI_filterSpectrumForNaN(double *in, double *inWls, double *inSd, int inN, double *out, double *outWls, double *outSd, int *outN);
int GCI_RGBToHLS (double R, double G, double B, double *H, double *L, double *S);
int GCI_HLSToRGB (double H, double L, double S, double *R, double *G, double *B);
double getIEEESpecialValue (char buffer[]);

#endif