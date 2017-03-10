#include "IMAQ_CVI.h"

#include <ansi_c.h>
#include <utility.h>
#include <analysis.h>

//********************************* imageRotate **********************************************************************

int imageRotate90 (IPIImageRef imageIn, IPIImageRef imageOut, int rotateLeft, int upDownFlip)
{   // U8, I16 or SGL images
	IPIImageRef temp=0;
	IPIImageInfo info, tInfo;
	int x, y, flip;
	
	IPI_GetImageInfo(imageIn, &info);
	
	IPI_Create(&temp, info.pixelType, 0);
	IPI_SetImageSize(temp, info.height, info.width);  // invert the width and height to accept rotated image
	IPI_GetImageInfo(temp, &tInfo);
	
	if ((rotateLeft>0 && upDownFlip>0) || (rotateLeft==0 && upDownFlip==0)) flip = 1;
	else flip = -1;
	
	if (info.pixelType == IPI_PIXEL_U8)
	{
		unsigned char *pIn, *pOut;
		
		for (y=0; y<info.height; y++)   // for each row in the input image
		{
			pIn  =  info.firstPixelAddress.Pix8_Ptr + y * info.rawPixels;  // the start of a row
			if (rotateLeft) pOut = tInfo.firstPixelAddress.Pix8_Ptr + y;				   // the start of a col
			else            pOut = tInfo.firstPixelAddress.Pix8_Ptr + tInfo.width-y-1;	   // the start of a col
			
			if (flip==-1)   pOut += ((tInfo.height-1)*tInfo.rawPixels);  // the end of a col
			
			for (x=0; x<info.width; x++)
			{
				*pOut = *pIn;
				pIn++;						  // next pixel in row
				pOut += (flip * tInfo.rawPixels);	  // next col
			}
		}
	}
	else if (info.pixelType == IPI_PIXEL_I16)
	{
		short *pIn, *pOut;
		
		for (y=0; y<info.height; y++)   // for each row in the input image
		{
			pIn  =  info.firstPixelAddress.Pix16_Ptr + y * info.rawPixels;  // the start of a row
			if (rotateLeft) pOut = tInfo.firstPixelAddress.Pix16_Ptr + y;				   // the start of a col
			else            pOut = tInfo.firstPixelAddress.Pix16_Ptr + tInfo.width-y-1;	   // the start of a col
			
			if (flip==-1)   pOut += ((tInfo.height-1)*tInfo.rawPixels);  // the end of a col
			
			for (x=0; x<info.width; x++)
			{
				*pOut = *pIn;
				pIn++;						  // next pixel in row
				pOut += (flip * tInfo.rawPixels);	  // next col
			}
		}
	}
	else if (info.pixelType == IPI_PIXEL_SGL)
	{
		float *pIn, *pOut;
		
		for (y=0; y<info.height; y++)   // for each row in the input image
		{
			pIn  =  info.firstPixelAddress.PixFloat_Ptr + y * info.rawPixels;  // the start of a row
			if (rotateLeft) pOut = tInfo.firstPixelAddress.PixFloat_Ptr + y;				   // the start of a col
			else            pOut = tInfo.firstPixelAddress.PixFloat_Ptr + tInfo.width-y-1;	   // the start of a col
			
			if (flip==-1)   pOut += ((tInfo.height-1)*tInfo.rawPixels);  // the end of a col
			
			for (x=0; x<info.width; x++)
			{
				*pOut = *pIn;
				pIn++;						  // next pixel in row
				pOut += (flip * tInfo.rawPixels);	  // next col
			}
		}
	}
	else return -1;

//	IPI_WindDraw(temp, 0, "temp", 1);
//	MessagePopup("", "");
  
	IPI_Copy(temp, imageOut);
	IPI_Dispose(temp);
	
	return 0;
}

//********************************* imageHistogram **********************************************************************
// PB/RL replacement for IPI_Histgram which has funny behaviour with some float images.
IPIError imageHistogram (IPIImageRef image, IPIImageRef mask_image, int n, float imin, float imax, int *histogram_array, IPIHistoReport *report)
{
	double f;
	float min, max;
	IPIError error;
	IPIImageRef temp=0;
	IPIImageInfo info;

	// get the image type
	error = IPI_GetImageInfo (image, &info); if (error!=0) return (error);
	
	if (imin==0 && imax==0)
	{
		// do initial histogram to get min and max values
		error = IPI_Histogram (image, mask_image, 256, 0, 0, NULL, report); if (error!=0) return (error);
		min = report->minValue;
		max = report->maxValue;
	}
	else 
	{
		min = imin;
		max = imax;
	}
	
	// calculate scale factor so that the image spans the same intensity values as there are bins.
	if (max==min)
		f=1.0;           // can't do much else with this, IPI_Histogram will make a histogram with all the pixels at the centre.
	else
		f = n / (max - min);

	// create a scaled image
	error = IPI_Create (&temp, IPI_PIXEL_SGL, 0);            if (error!=0) return (error);
	error = IPI_Cast (image, IPI_PIXEL_SGL);				if (error!=0) return (error);
	error = IPI_Multiply (image, IPI_USECONSTANT, temp, f); if (error!=0) return (error);

	// reset the image type, since it was recast for the multiply
	error = IPI_Cast (image, info.pixelType);				if (error!=0) return (error);

	// calculate THE histogram, with correct number of bins over the known range
	error = IPI_Histogram (temp, mask_image, n, (min*f), (max*f), histogram_array, report); if (error!=0) return (error);

	// scale the results back to the original units
	report->minValue     /= f;
	report->maxValue     /= f;
	report->startValue   /= f;
	report->interval     /= f;
	report->mean         /= f;
	report->stdDeviation /= f;
	
	IPI_Dispose(temp);

	return (error);
}

//********************************* doHistogram **********************************************************************
// From Julian Gilbey since IPI_Histogram seems to have a bug
// not currently used
/*
int makeHistogram(float data[], int ndata, int histo[], int nhisto, float low, float high)
{
	int i, level;
	
	if (ndata < 0)
		return -1;
	if (nhisto < 1)
		return -2;
	if (low == high)
		return -3;
	
	for (i=0; i<nhisto; i++)
		histo[i] = 0;

	for (i=0; i<ndata; i++) {
		level = (int) ((nhisto * (data[i] - low) / (high - low)));
		if (level >= 0 && level < nhisto)
			histo[level]++;
		else if (level == nhisto && (data[i] - low) / (high - low) < 1.0001)
			histo[nhisto-1]++;
	}
	
	return 0;
}
*/
//********************************* formatFloatToString **********************************************************************

int formatFloatToString (float n, char *s)
{
	if      (n==0.0)  sprintf(s, "0");
	else if (fabs(n)<0.010) sprintf(s, "%.3e",   n);
	else if (fabs(n)<1.000) sprintf(s, "%.4f", n);
	else if (fabs(n)<10.00) sprintf(s, "%.3f", n);
	else if (fabs(n)<100.0) sprintf(s, "%.2f", n);
	else if (fabs(n)<1000.0)sprintf(s, "%.1f", n);
	else			        sprintf(s, "%.3e", n);
	
	return (0);
}

/********************************* Normalise **********************************************************************/

void Normalise ( IPIImageRef image, IPIImageRef image2 )
{   // 8bit or float images
	IPIHistoReport hist_rep;
	IPI_Histogram (image, IPI_NOMASK, 256, 0, 0, NULL, &hist_rep);
	IPI_Subtract (image, IPI_USECONSTANT, image2, hist_rep.minValue);
	IPI_Multiply (image2, IPI_USECONSTANT, image2, (255.0/(hist_rep.maxValue-hist_rep.minValue)));
}			
 

//********** makeCountColTable *************************************************************************

int makeCountColTable (int *MyPal, int entry1, int colour1, int entry2, int colour2, int entry3, int colour3)
{
	int c;
	
	for (c=0; c<256; c++) 
		MyPal[c] = c << 16 | c << 8 | c ;
	MyPal[entry1] = colour1;
	MyPal[entry2] = colour2;
	MyPal[entry3] = colour3;
	return(0);
}

//**********  createLogColTable *************************************************************************

int createLogColTable (int *ct, int con, int br)
{
	int c, level;
	double logs[256];
	
	logs[0] = 0.0;
	for (c=1; c<256; c++) 
		logs[c] = log10(c);
	for (c=0; c<256; c++) {
		logs[c] = (con*255.0) * logs[c]/logs[255] + br;
		level = RoundRealToNearestInteger (logs[c]);
		if (level < 0) level = 0;
		if (level > 255) level = 255;
		ct[c] = level << 16 | level << 8 | level;
	}
	return(0);
}

//********************************* GCI_RGBToHLS **********************************************************************

double RGB_max_of(double red, double green, double blue)
{
	double max;
     
    if (red>green) max=red;
    else max=green;
    if (blue>max)  max=blue;
	return(max);
}

double RGB_min_of(double red, double green, double blue)
{
	double min;
     
    if (red<green) min=red;
    else min=green;
    if (blue<min)  min=blue;
	return(min);
}

int RGBToHLS (double R, double G, double B, double *H, double *L, double *S)
{
// adapted from
// http://astronomy.swin.edu.au/~pbourke/colour/conversion.html, Compiled by Paul Bourke, February 1994 
// and seems to correspond to how CVI and PSP seem to work according to lab book 2, p25.
	const double small_value = 0.0000001, undefined = 32767;   // maxint?
	double max_value, min_value, diff, r_dist, g_dist, b_dist;
                                                        
	max_value = RGB_max_of(R, G, B);
	min_value = RGB_min_of(R, G, B);
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

//********************************* GCI_HLSToRGB **********************************************************************

static double RGB (double q1, double q2, double hue)
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

int HLSToRGB (double H, double L, double S, double *R, double *G, double *B)
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

//********** makeODColTable *************************************************************************

int makeODColTable (int *col, int n, int colour, int contrast)
{
	int i;
	double R, G, B, h, l, s, slog, c;

	R = (colour >> 16 & 255) / 255.0;
	G = (colour >> 8  & 255) / 255.0;
	B = (colour       & 255) / 255.0;

	slog = log(1.0/8.0/contrast)/255.0;		 // to scale the od nicely 0-255
	RGBToHLS (R, G, B, &h, &l, &s);
	
	for (i=0; i<n; i++) 
	{
		c = exp((double)i*slog);
		if (c>1.0) c=1.0;
		else if (c<0.0) c=0.0;
		HLSToRGB(h, c, s, &R, &G, &B);
		col[i] = (int)(R*255.0) << 16 | (int)(G*255.0) << 8 | (int)(B*255.0);
	}
	return(0);
}

//********** makeSeismicColTable *************************************************************************

void makeSeismicColTable (int *Palette, int n)
{ // black through blue to white through red to black, replace last entry with white
	int i, p, *temp, c=6.0;
	
	p = n/2;
	if (n%2!=0) p++;
	
	// 1st half
	temp = (int *)malloc(p*sizeof(int));
	makeODColTable (temp, p, VAL_BLUE, c);
	for (i=0; i<p; i++) Palette[i] = temp[p-i-1];       // copy the OD coltab in reverse

	// 2nd half
	makeODColTable (&(Palette[p]), p, VAL_RED, c);
	
	Palette [0]   = VAL_BLACK;
	Palette [n-1] = VAL_WHITE;

	free(temp);
}

//********** GetPaletteFromFile *************************************************************************

// Reads palettes from the same formatted files that ImageJ uses (as exported by LUT panel 2.2 plugin)
int GetPaletteFromFile(const char *filepath, int *palette)
{
    FILE *fp;
    int i, index, red, green, blue;
    char dummy[100];
   
    if (palette == NULL)
    {
        return -1;
    }

    fp = fopen(filepath, "r");
   
    if(fp == NULL)
        return -2;
   
    // Read Header Line
    fscanf(fp, "%s\t%s\t%s\t%s\n", dummy, dummy, dummy, dummy);
   
    for(i = 0; i < 256; i++)
    {
        fscanf(fp, "%d\t%d\t%d\t%d\n", &index, &red, &green, &blue);
       
        palette[i]  = red << 16;
        palette[i] |= green << 8;
        palette[i] |= blue;
    }
   
    fclose(fp);
    fp = NULL;
   
    return 0;
}

// Saves palette to the same formatted files that ImageJ uses (as exported by LUT panel 2.2 plugin)
int SavePaletteToFile(const char *filepath, int *palette)
{
    FILE *fp;
    int i, index, red, green, blue;
    char dummy[100];
   
    if (palette == NULL)
    {
        return -1;
    }

    fp = fopen(filepath, "w");
   
    if(fp == NULL)
        return -2;
   
    // Header Line
    fprintf(fp, "Index\tRed\tGreen\tBlue\n");
   
    for(i = 0; i < 256; i++)
    {
        red   = (palette[i] & 0xff0000) >> 16;
        green = (palette[i] & 0xff00) >> 8;
        blue  = palette[i] & 0xff;
		
		fprintf(fp, "%d\t%d\t%d\t%d\n", i, red, green, blue);
    }
   
    fclose(fp);
    fp = NULL;
   
    return 0;
}
 
//********** makeColTable *************************************************************************

int makeColTable (int *col, int n, int colour)
{
	int i;
	float R, G, B;

	R = (colour >> 16 & 255) / 255.0;
	G = (colour >> 8  & 255) / 255.0;
	B = (colour       & 255) / 255.0;

	for (i=0; i<n; i++) 
	{
		col[i] = (RoundRealToNearestInteger(i*R)) << 16;
		col[i] |= (RoundRealToNearestInteger(i*G)) << 8;
		col[i] |= RoundRealToNearestInteger(i*B);
	}
	return(0);
}

//********** makeRainbowColTable *************************************************************************

void makeRainbowColTable (int *rainbowPalette, int n)
{
	int i, p1, p2;
	double ramp[86];
	
	p1 = n/3;
	p2 = 2*n/3;

	//First do the blue bit
	for (i=0; i<p1; i++)
		rainbowPalette[i] = 255;
	Ramp (p1+1, n-1, 0.0, ramp);
	for (i=p1; i<p2; i++)
		rainbowPalette[i] = RoundRealToNearestInteger(ramp[i-p1]);
	for (i=p2; i<n; i++)
		rainbowPalette[i] = 0;

	//Now add the green bit
	Ramp (p1+1, 0.0, n-1, ramp);
	for (i=0; i<p1; i++)
		rainbowPalette[i] |= RoundRealToNearestInteger(ramp[i]) << 8;
	for (i=p1; i<p2; i++)
		rainbowPalette[i] |= 255 << 8;
	for (i=p2; i<n; i++)
		rainbowPalette[i] |= RoundRealToNearestInteger(ramp[p1-(i-p2)]) << 8;

	//Now add the red bit
	for (i=0; i<p1; i++)
		rainbowPalette[i] |= 0 << 16;
	Ramp (p1+1, 0.0, n-1, ramp);
	for (i=p1; i<p2; i++)
		rainbowPalette[i] |= RoundRealToNearestInteger(ramp[i-p1]) << 16;
	for (i=p2; i<n; i++)
		rainbowPalette[i] |= 255 << 16;

	// Now black and white at the ends
	rainbowPalette[0] = 0;
//	rainbowPalette[n-1] = 0xFFFFFF;
}

//********** makeReverseRainbowColTable *************************************************************************

void makeReverseRainbowColTable (int *rainbowPalette, int n)
{
	int i, temp[256];

	makeRainbowColTable (temp, n);
	
	for (i=0; i<n; i++) rainbowPalette[i] = temp [n-1-i];

	// Reset black and white at the ends
	rainbowPalette[0] = 0;
//	rainbowPalette[n-1] = 0xFFFFFF;

	// reset blue at top end
	rainbowPalette[n-1] = 0x0000FF;
}


//********** adjustColTable *************************************************************************

int adjustColTable (int *col, int n, double contrast, double brightness)
{
	int i;
	double R, G, B;

	for (i=0; i<n; i++) 
	{
		R = (col[i] >> 16 & 255);
		G = (col[i] >> 8  & 255);
		B = (col[i]       & 255);

		R = RoundRealToNearestInteger(contrast*(double)R + brightness);
		G = RoundRealToNearestInteger(contrast*(double)G + brightness);
		B = RoundRealToNearestInteger(contrast*(double)B + brightness);
		
		if (R<0) R=0; else if (R>255) R=255;
		if (G<0) G=0; else if (G>255) G=255;
		if (B<0) B=0; else if (B>255) B=255;
		
		col[i] = (int)R << 16 | (int)G << 8 | (int)B;
	}
	return(0);
}

//********** thresholdColTable *************************************************************************

int thresholdColTable(int *col, int n)
{
	int i;

	for (i=0; i<n; i++) 
	{
		col[i] = 255 << 16 | 255 << 8 | 255;  // white
	}
	
	col[0]   = 0; // black
	col[n-1] = 0; // black
	
	return(0);
}

//********** overLoadColTable *************************************************************************

int overloadColTable(int *col, int n)
{
	int i;

	makeColTable (col, n, 255 << 16 | 255 << 8 | 255);   // white

	col[0]   = 0 << 16 | 255 << 8 | 0;  // green
	col[n-1] = 255 << 16 | 0 << 8 | 0;  // red
	
	return(0);
}


//********************************* rescaleImageTo8bits **********************************************************************

int rescaleImageTo8bits (IPIImageRef image, IPIImageRef image2, float min, float max)
{
	IPIImageInfo info1, info2;
	IPIPixelType oldType1=IPI_PIXEL_SGL, oldType2=IPI_PIXEL_SGL;

	IPI_GetImageInfo (image, &info1);
	if (info1.pixelType != IPI_PIXEL_SGL)
	{
		oldType1 = info1.pixelType;
		IPI_Cast (image, IPI_PIXEL_SGL);
	}

	IPI_GetImageInfo (image2, &info2);
	if (info2.pixelType != IPI_PIXEL_SGL)
	{
		oldType2 = info2.pixelType;
		IPI_Cast (image2, IPI_PIXEL_SGL);
	}

	IPI_Subtract (image, IPI_USECONSTANT, image2, min);
	IPI_Multiply (image2, IPI_USECONSTANT, image2, (255.0/(max-min)));

	if (oldType1 != IPI_PIXEL_SGL) IPI_Cast (image, oldType1);
	if (oldType2 != IPI_PIXEL_SGL) IPI_Cast (image2, oldType2);

	return(0);
}

IPIError IPI_ImageBorderSize (IPIImageRef source_image, long Operation, long *border_size)
{
// By P Barber, 05.06.03
// IPI_ImageBorderSize is in the IMAQ vision .fp library file but never existed as a function
// but here is my version of it.

	IPIError error;
	IPIImageInfo info;
	IPIImageRef temp=0;
	
	if (source_image<=0) return (IPI_ERR_BADIMAGEREF);
	if (border_size==NULL) return (IPI_ERR_NULLPTR);

	error = IPI_GetImageInfo (source_image, &info);  if (error!=IPI_ERR_NOERROR) return (error);

	if (Operation==0)  // get border size
	{
		*border_size = info.border;
	}
	else			   // set border size
	{
		if (*border_size<0) return (IPI_ERR_BADBORDER);
		if (*border_size == info.border) return (IPI_ERR_NOERROR);  // border is already the correct size

		error = IPI_Create (&temp, info.pixelType, *border_size);                     if (error!=IPI_ERR_NOERROR) return (error);
		error = IPI_SetImageSize (temp, info.width, info.height);                     if (error!=IPI_ERR_NOERROR) return (error);
		error = IPI_ImageToImage (source_image, temp, 0, 0);			      if (error!=IPI_ERR_NOERROR) return (error);
		error = IPI_SetImageOffset (temp, info.xOffset, info.yOffset);	              if (error!=IPI_ERR_NOERROR) return (error);
		error = IPI_SetImageCalibration (temp, info.unit, info.xCalib, info.yCalib);  if (error!=IPI_ERR_NOERROR) return (error);
		error = IPI_Copy (temp, source_image);								          if (error!=IPI_ERR_NOERROR) return (error);
		error = IPI_Dispose (temp);								                      if (error!=IPI_ERR_NOERROR) return (error);
	}
	return(IPI_ERR_NOERROR);
}

//********************************* ReverseImage **********************************************************************

void ReverseImage(IPIImageRef input, IPIImageRef output)
{ 	// form the negative of a gray scale image
	int lut[256], i, j;
	float val;
	IPIImageInfo info;
	
	
	IPI_GetImageInfo (input, &info);
	// images with small dimensions seem to cause problems for IPI_UserLookup!
	// so we have to do the revesal ourselves
	// it will only be used with small images it does not need to be that fast
	if (info.width<25 || info.height<25)         
	{
		IPI_Copy(input, output);
		for (i=0; i<info.height; i++)
			for (j=0; j<info.width; j++)
			{
				IPI_GetPixelValue (input,  j, i, &val);
				IPI_SetPixelValue (output, j, i, 255-val);
			}
	}
	else 
	{
		for (i=0; i<256; i++) lut[i]=255-i;
		IPI_UserLookup (input, IPI_NOMASK, output, VAL_INTEGER, lut, 256);
	}
}

//********************************* ReverseMask **********************************************************************

void ReverseMask(IPIImageRef input, IPIImageRef output)
{	// reverse a binary mask, 0->1 and 1->0
	int lut[2], i, j;
	float val;
	IPIImageInfo info;
	
	IPI_GetImageInfo (input, &info);
	// images with small dimensions seem to cause problems for IPI_UserLookup!
	// so we have to do the revesal ourselves
	// it will only be used with small images it does not need to be that fast
	if (info.width<25 || info.height<25)         
	{
		IPI_Copy(input, output);
		for (i=0; i<info.height; i++)
			for (j=0; j<info.width; j++)
			{
				IPI_GetPixelValue (input, j, i, &val);
				if (val==0) IPI_SetPixelValue (output, j, i, 1.0);
				else IPI_SetPixelValue (output, j, i, 0.0);
			}
	}
	else 
	{
		lut[0]=1; lut[1]=0;
		
		IPI_UserLookup (input, IPI_NOMASK, output, VAL_INTEGER, lut, 2);
	}
}

//********************************* makeHatchedImage **********************************************************************

int makeHatchedImage (IPIImageRef in, IPIImageRef out, int hatchType, int spacing)
{
	IPIImageInfo info;
	int i, j;
	
	IPI_GetImageInfo (in, &info);

	IPI_DrawRect (in, out, IPI_FULL_RECT, IPI_DRAW_PAINT, 0.0);

	switch (hatchType)
		{
		case 1: // horiz lines
			for (j=0; j<info.height; j+=spacing)
				IPI_DrawLine (out, out, MakePoint(0, j), MakePoint(info.width, j), IPI_DRAW_PAINT, 1.0);
							
			break;
		case 2: // vert lines
			for (i=0; i<info.width; i+=spacing)
				IPI_DrawLine (out, out, MakePoint(i, 0), MakePoint(i, info.height), IPI_DRAW_PAINT, 1.0);
			
			break;
		case 3: // horiz and vert lines
			for (j=0; j<info.height; j+=spacing)
				IPI_DrawLine (out, out, MakePoint(0, j), MakePoint(info.width, j), IPI_DRAW_PAINT, 1.0);
			for (i=0; i<info.width; i+=spacing)
				IPI_DrawLine (out, out, MakePoint(i, 0), MakePoint(i, info.height), IPI_DRAW_PAINT, 1.0);
			
			break;
		case 4:  // diag lines
			for (j=0; (j<(info.height+info.width)); j+=spacing)
				IPI_DrawLine (out, out, MakePoint(j, 0), MakePoint(0, j), IPI_DRAW_PAINT, 1.0);
			
			break;
		case 5:  // other diag lines
			for (j=0; j<(info.height+info.width); j+=spacing)
				IPI_DrawLine (out, out, MakePoint(0, j-info.width), MakePoint(info.width, j), IPI_DRAW_PAINT, 1.0);
			
			break;
		case 6:  // cross hatched
			for (j=0; (j<(info.height+info.width)); j+=spacing)
				IPI_DrawLine (out, out, MakePoint(j, 0), MakePoint(0, j), IPI_DRAW_PAINT, 1.0);
			for (j=0; j<(info.height+info.width); j+=spacing)
				IPI_DrawLine (out, out, MakePoint(0, j-info.width), MakePoint(info.width, j), IPI_DRAW_PAINT, 1.0);
			
			break;
		case 7:   // dots
			for (j=0; j<info.height; j+=spacing)
				for (i=0; i<info.width; i+=spacing)
					IPI_SetPixelValue (out, i, j, 1.0);

			break;
		default:
			
			break;
		}

	return(0);
}

//********************************* separateColour **********************************************************************

void separateColour (int c, int *R, int *G, int *B)
{   // separates 8 bit values R G and B from a colour in the form 0xRRGGBB
	*R = (c&0xff0000) >> 16;
	*G = (c&0x00ff00) >> 8;
	*B = (c&0x0000ff);
}

//********************************* blendMaskWithImage **********************************************************************

int blendMaskWithImage(IPIImageRef mask, IPIImageRef image, IPIImageRef output, int colour, int outline, int hatch, int opacity)
{
	// mask is expected to be a U8 image, vals 0 and 1 and is not changed by this function (except the border is set to 3)
	// image an RGB32, so is output, image is not changed so image and output can be the same image
	// colour is the required blend/overlay colour: 0xRRGGBB
	// outline = 1 plots an outline around the mask, outline = 0 does not
	IPIImageInfo maskInfo,  imageInfo;
	IPIImageRef maskedInner=0, maskedOuter=0, reversedMask=0, r=0, g=0, b=0, ri=0, gi=0, bi=0;
	int R, G, B, border=3;

	// Checks
	IPI_GetImageInfo (mask, &maskInfo);
	if (maskInfo.pixelType != IPI_PIXEL_U8)    return(-1);
	IPI_GetImageInfo (image, &imageInfo);
	if (imageInfo.pixelType != IPI_PIXEL_RGB32) return(-1);
	
	IPI_Create (&maskedInner, IPI_PIXEL_RGB32, 0);
	IPI_Create (&maskedOuter, IPI_PIXEL_RGB32, 0);
	IPI_Create (&reversedMask, IPI_PIXEL_U8, 0);
	IPI_Create (&r, IPI_PIXEL_U8, 0);
	IPI_Create (&g, IPI_PIXEL_U8, 0);
	IPI_Create (&b, IPI_PIXEL_U8, 0);
	IPI_Create (&ri, IPI_PIXEL_U8, 0);
	IPI_Create (&gi, IPI_PIXEL_U8, 0);
	IPI_Create (&bi, IPI_PIXEL_U8, 0);

	//IPI_SetWindow2DAttributes (0, IPI_ATTR_VH_ZOOM, 4, 4);

	//IPI_WSetPalette (1, IPI_PLT_BINARY, NULL);
	//IPI_SetWindow2DAttributes (1, IPI_ATTR_TOP_AND_LEFT, 400, 20);
	//IPI_SetWindow2DAttributes (1, IPI_ATTR_VH_ZOOM, 4, 4);
	
	// colour planes of the RGB bits within the mask
	IPI_Mask (image, mask, maskedInner);
	IPI_ExtractColorPlanes (maskedInner, ri, gi, bi, IPI_RGB);

	// calc the three colour planes of mask with the correct colour
	separateColour (colour, &R, &G, &B);
	IPI_Multiply (mask, IPI_USECONSTANT, r, R);
	IPI_Multiply (mask, IPI_USECONSTANT, g, G);
	IPI_Multiply (mask, IPI_USECONSTANT, b, B);

	// average these colour planes to perform a blend, result goes into r g and b
	if (opacity==0) // 
	{
		IPI_Copy (ri, r);
		IPI_Copy (gi, g);
		IPI_Copy (bi, b);
	}
	else			   // average once for high opacity
	{
		IPI_Compare (ri, r, r, IPI_CP_AVG, 0.0);
		IPI_Compare (gi, g, g, IPI_CP_AVG, 0.0);
		IPI_Compare (bi, b, b, IPI_CP_AVG, 0.0);
		
		if (opacity < 3)   // average again for med opacity
		{
			IPI_Compare (ri, r, r, IPI_CP_AVG, 0.0);
			IPI_Compare (gi, g, g, IPI_CP_AVG, 0.0);
			IPI_Compare (bi, b, b, IPI_CP_AVG, 0.0);
		}
		if (opacity < 2)   // average again for low opacity
		{
			IPI_Compare (ri, r, r, IPI_CP_AVG, 0.0);
			IPI_Compare (gi, g, g, IPI_CP_AVG, 0.0);
			IPI_Compare (bi, b, b, IPI_CP_AVG, 0.0);
		}
	}

	// colour planes of the RGB bits _outside_ the mask
	ReverseMask(mask, reversedMask);
	IPI_Mask (image, reversedMask, maskedOuter);
	IPI_ExtractColorPlanes (maskedOuter, ri, gi, bi, IPI_RGB);

	// add these (containing data outside the mask) to the blend images (containing data inside the mask)
	IPI_Add (r, ri, r, 0.0);    
	IPI_Add (g, gi, g, 0.0);   
	IPI_Add (b, bi, b, 0.0);   
	
	// make an RGB from these
	IPI_ReplaceColorPlanes (maskedInner, maskedInner, r, g, b, IPI_RGB); // blended result now in masked inner
	
	if (outline)  // do overlay of the outline in the required colour
	{
		
		// make sure the mask has a big enough border to perform the morphological operation (!)
		IPI_ImageBorderSize (mask, 1, &border);
		
		// find the inside border to the mask
		IPI_Morphology (mask, reversedMask, IPI_MO_GRADIN, IPI_MO_STD3X3);   // find outline, store in reversed mask temporarily
		IPI_Threshold (reversedMask, reversedMask, 1.0, 1.0, 1.0, 1);  // threshold since IPI_Morphology can put bright dots in place of single pixel holes in the mask

		// calc the three colour planes of a new outline mask with the correct colour
		IPI_Multiply (reversedMask, IPI_USECONSTANT, r, R);
		IPI_Multiply (reversedMask, IPI_USECONSTANT, g, G);
		IPI_Multiply (reversedMask, IPI_USECONSTANT, b, B);

		// reverse this outline mask and use it to get the image data from the blended image, extract these colour planes
		ReverseMask(reversedMask, reversedMask);
		IPI_Mask (maskedInner, reversedMask, maskedInner);
		IPI_ExtractColorPlanes (maskedInner, ri, gi, bi, IPI_RGB);

		// add the new outline to the surrounding image data
		IPI_Add (r, ri, r, 0.0);    
		IPI_Add (g, gi, g, 0.0);   
		IPI_Add (b, bi, b, 0.0);   

		// make an RGB from these
		IPI_ReplaceColorPlanes (maskedInner, maskedInner, r, g, b, IPI_RGB);  // overlayed result now in masked inner 
	}
	
	if (hatch)  // do overlay of the outline in the required colour
	{
		if (hatch==7) makeHatchedImage (mask, reversedMask, hatch, 3);   // dots
		else makeHatchedImage (mask, reversedMask, hatch, 10);   // store in reversed mask temporarily 
		IPI_Mask (reversedMask, mask, reversedMask);		// get just the bits of hatch inside the mask

		// calc the three colour planes of a new outline mask with the correct colour
		IPI_Multiply (reversedMask, IPI_USECONSTANT, r, R);
		IPI_Multiply (reversedMask, IPI_USECONSTANT, g, G);
		IPI_Multiply (reversedMask, IPI_USECONSTANT, b, B);

		// reverse this outline mask and use it to get the image data from the blended image, extract these colour planes
		ReverseMask(reversedMask, reversedMask);
		IPI_Mask (maskedInner, reversedMask, maskedInner);
		IPI_ExtractColorPlanes (maskedInner, ri, gi, bi, IPI_RGB);

		// add the new outline to the surrounding image data
		IPI_Add (r, ri, r, 0.0);    
		IPI_Add (g, gi, g, 0.0);   
		IPI_Add (b, bi, b, 0.0);   

		// make an RGB from these
		IPI_ReplaceColorPlanes (maskedInner, maskedInner, r, g, b, IPI_RGB);  // overlayed result now in masked inner 
	}
	
	// copy the result to the output image
	IPI_Copy(maskedInner, output);
	
	IPI_Dispose (maskedInner);
	IPI_Dispose (maskedOuter);
	IPI_Dispose (reversedMask);
	IPI_Dispose (r);
	IPI_Dispose (g);
	IPI_Dispose (b);
	IPI_Dispose (ri);
	IPI_Dispose (gi);
	IPI_Dispose (bi);

	return(0);
}

//********************************* GCI_convertIndexedColourToRGB **********************************************************************

int GCI_convertIndexedColourToRGB (IPIImageRef indexed, int *colTable, int cols, IPIImageRef rgb_in)
{
	int i, *rC, *gC, *bC;    // components of colour table
	IPIImageRef rgb=0;
	
	if (cols<=0) return(-1);
	
	IPI_Create (&rgb, IPI_PIXEL_RGB32, 0);
	
	// make individual lut's from the colour table
	rC = (int *)malloc(cols*sizeof(int)); if (rC==NULL) return(-2);
	gC = (int *)malloc(cols*sizeof(int)); if (gC==NULL) return(-2);
	bC = (int *)malloc(cols*sizeof(int)); if (bC==NULL) return(-2);

	for (i=0; i<cols; i++)
	{
		rC[i] = (colTable[i] & 0xff0000) >> 16;
		gC[i] = (colTable[i] & 0x00ff00) >> 8; 
		bC[i] = (colTable[i] & 0x0000ff);      
	}
	
	// make a "greyscale" rgb image
	IPI_ReplaceColorPlanes (rgb, rgb, indexed, indexed, indexed, IPI_RGB);
	
	// apply the lut's to the rgb image
	IPI_ColorUserLookup (rgb, IPI_NOMASK, rgb, IPI_RGB, VAL_INTEGER, rC, gC, bC);
									   
//	IPI_Cast (rgb_in, IPI_PIXEL_RGB32);		  // make sure this is rgb
	IPI_Copy (rgb, rgb_in);
	
	free(rC);
	free(gC);
	free(bC);
	IPI_Dispose (rgb);
	return(0);
}


IPIHistoReport Normalise0To1 ( IPIImageRef image, IPIImageRef image2 )
// normalise a float image over range 0-1 maps to 0-255 (works on 8-bit also)
// return 0 and 1 as min and max values
{
	IPIHistoReport hist_rep;
	IPI_Histogram (image, IPI_NOMASK, 256, 0, 0, NULL, &hist_rep);
	hist_rep.minValue=0.0;
	hist_rep.maxValue=1.0;
	IPI_Multiply (image, IPI_USECONSTANT, image2, 255.0);
	return (hist_rep);
}			

int U16ToU8Image (IPIImageRef in, IPIImageRef out)
{   // simply takes the most significant 8 bits (!)
	void *tempBuffer=NULL;
	int width, height, i, nBytes8Bit;
	IPIImageInfo info;
	
	if (in <= 0) return -1;
	if (out <= 0) return -1;
	
	IPI_GetImageInfo(in, &info);
		
	tempBuffer = malloc(info.pixelSpace);
	if (!tempBuffer) return -2;

	// the I16 image is signed data, need to recover the unsigned short data into a temp array
	if (IPI_ImageToArray (in, IPI_FULL_RECT, VAL_UNSIGNED_SHORT_INTEGER, (unsigned short *)tempBuffer, &width, &height)!= IPI_ERR_NOERROR) return -3;;

	// can then convert this 16 bit array into 8 bit, e.g. for display
	nBytes8Bit = width * height;
	for (i=0; i<nBytes8Bit; i++) *((unsigned char *)tempBuffer+i) = *((unsigned char *)tempBuffer+2*i+1);  
		// PC's are "Little Endian", little end of multibyte numbers comes first, and we want the big end, hence the +1

	IPI_Cast(out, IPI_PIXEL_U8);
	if (IPI_ArrayToImage (out, VAL_CHAR, (unsigned char *)tempBuffer, width, height) != IPI_ERR_NOERROR) return -4;
	
	free (tempBuffer);
	
	return 0;
}

int U16ToSGLImage (IPIImageRef in, IPIImageRef out)
{   // converts the 16 bit image into floating point but accounts for the unsigned U16 image being stored in a I16 signed image.
	unsigned short *tempU16Buffer=NULL;
	float *tempFloatBuffer=NULL;
	int width, height, i, n;
	IPIImageInfo info;
	
	if (in <= 0) return -1;
	if (out <= 0) return -1;
	
	IPI_GetImageInfo(in, &info);
		
	tempU16Buffer = malloc(info.pixelSpace);
	if (!tempU16Buffer) return -2;

	// the I16 image is signed data, need to recover the unsigned short data into a temp array
	if (IPI_ImageToArray (in, IPI_FULL_RECT, VAL_UNSIGNED_SHORT_INTEGER, (unsigned short *)tempU16Buffer, &width, &height)!= IPI_ERR_NOERROR) return -3;;

	n = width * height;
	tempFloatBuffer = malloc(n*sizeof(float));
	if (!tempFloatBuffer) return -2;

	// can then convert this 16 bit array into floating point
	for (i=0; i<n; i++) tempFloatBuffer[i] = (float)tempU16Buffer[i];

	IPI_Cast(out, IPI_PIXEL_SGL);
	if (IPI_ArrayToImage (out, VAL_FLOAT, tempFloatBuffer, width, height) != IPI_ERR_NOERROR) return -4;
	
	free (tempU16Buffer);
	free (tempFloatBuffer);
	
	return 0;
}
