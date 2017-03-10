#include "LensDistortionCorrection_ui.h"
#include "LensDistortionCorrection.h"
//#include "RemoveQuadraticBg16.h"

#include "FreeImageAlgorithms_IO.h"
#include "FreeImageAlgorithms_Utilities.h"
#include "FreeImageAlgorithms_DistortionCorrection.h"

#include "BasicWin32Window.h"

#include "string_utils.h"
#include "gci_utils.h"

#include <ansi_c.h>
#include "toolbox.h"   
#include <userint.h>
#include <formatio.h>
#include <analysis.h>
#include <utility.h>

LensDistortion* lens_distortion_new(Microscope *ms, const char *data_dir)
{
	LensDistortion *ld = (LensDistortion *) malloc (sizeof(LensDistortion));		
	
	memset(ld, 0, sizeof(LensDistortion));

	ui_module_constructor(UIMODULE_CAST(ld), "LensDistortion");  
	
	ui_module_set_data_dir(UIMODULE_CAST(ld), data_dir);

	ld->panel_id = ui_module_add_panel(UIMODULE_CAST(ld), "LensDistortionCorrection_ui.uir", DIST_PANEL, 1);  
	ld->poly_panel_id = ui_module_add_panel(UIMODULE_CAST(ld), "LensDistortionCorrection_ui.uir", POLY_PNL, 0);   
	ld->options_panel_id = ui_module_add_panel(UIMODULE_CAST(ld), "LensDistortionCorrection_ui.uir", DCOPTIONS, 0);  
	ld->offset_panel_id = ui_module_add_panel(UIMODULE_CAST(ld), "LensDistortionCorrection_ui.uir", OFFSET, 0);  
	ld->profile_panel_id = ui_module_add_panel(UIMODULE_CAST(ld), "LensDistortionCorrection_ui.uir", PROFILE, 0);  

//	if ( InstallCtrlCallback (tl->panel_id, TIMELAPSE_NEW_POINT, OnNewPointClicked, tl) < 0)
//		return NULL;	

	return ld;
}



void lens_distortion_set_coeffs(LensDistortion* ld, double a, double b, double c, double d)
{
	ld->a = a;
	ld->b = b;
	ld->c = c;
	ld->d = d;

	ld->coeffs_set = 1;
}

void lens_distortion_set_offset(LensDistortion* ld, FIAPOINT point)
{
	ld->offset = point;
}

FIBITMAP* lens_distortion_correct_distortion(LensDistortion* ld, FIBITMAP *input)
{
	if(ld->coeffs_set)
		return FIA_CorrectLensDistortion(input, ld->a, ld->b, ld->c, ld->d, ld->offset);
}

static int lens_distortion_get_line_profile (LensDistortion* ld)
{
    int smooth = 0;
     
	int len = FIA_GetGreyScalePixelValuesForLine (ld->input, ld->center, ld->end, FIT_BITMAP, ld->profile);

	if(len == 0)
		return;

	if(ld->profile_plot > 0)
		DeleteGraphPlot (ld->profile_panel_id, PROFILE_GRAPH, ld->profile_plot, VAL_IMMEDIATE_DRAW);

        //GetCtrlVal (panelOptions, DCOPTIONS_SMOOTH, &smooth);
        
		//if (smooth)
		//	Mean_Int_Filter (profile, profile, smooth, smooth, noOfElements);
        
	ld->profile_plot = PlotY (ld->profile_panel_id, PROFILE_GRAPH, ld->profile, len, VAL_UNSIGNED_CHAR,
		VAL_FAT_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_RED);
  
	ui_module_display_panel(UIMODULE_CAST(ld), ld->profile_panel_id); 

	return len;
}

static double find_distance_and_angle_between_points(FIAPOINT start, FIAPOINT end, double *angle)
{
	double dx = end.x - start.x;
    double dy = end.y - start.y;

	if(angle != NULL)
		*angle = atan2 (dy, dx);

	return sqrt((double)(dx*dx + dy*dy));
}

void lens_distortion_set_image(LensDistortion *ld, FIBITMAP *input)
{
	int width, height, max;

	width = FreeImage_GetWidth(input);
	height = FreeImage_GetHeight(input);

	//ld->discal_normalisation = ((double) sqrt((double) (width*width + height*height))) / 2.0;
	ld->discal_normalisation = (double) MIN(width, height) / 2.0;

	ld->input = FreeImage_ConvertTo8Bits(input);
	ld->center = MakeFIAPoint (width/2, height/2);   // imager centre in image
    ld->offset = MakeFIAPoint(0, 0);
	ld->end = MakeFIAPoint(width, height);

	max = (int) ceil(find_distance_and_angle_between_points(ld->center, ld->offset, &(ld->last_point_angle)));

	if (ld->profile != NULL)
		free(ld->profile);

	if (ld->points != NULL)
		free(ld->points);

    if (ld->regular_points != NULL)
		free(ld->regular_points);

	ld->profile = (BYTE *)malloc(max*sizeof(BYTE));
	
	if (ld->profile == NULL) {
		GCI_MessagePopup ("Error", "malloc error in lens_distortion_set_image.");
		return;
	}

    // number_of_points+2 since  number_of_points may have been previously reduced by discarding this first and last points
    ld->points = (double *) malloc(max * sizeof(double));
	
	if (ld->points == NULL) {
		GCI_MessagePopup ("Error", "malloc error in lens_distortion_set_image.");
		return;
	}

    ld->regular_points = (double *)malloc(max * sizeof(double));
	
	if (ld->regular_points == NULL) {
		GCI_MessagePopup ("Error", "malloc error in lens_distortion_set_image.");
		return;
	}
}

static void lens_distortion_get_coordinates(LensDistortion* ld)
{
	int len, ignore = 0, ignore_last = 0, start = 0;
	double direction_factor;

	int i, j=0, k=0;

	len = lens_distortion_get_line_profile (ld);

    direction_factor = find_distance_and_angle_between_points(ld->center, ld->end, NULL) / len;
                     
    //ld->last_point_angle = atan2 (dy, dx); // Store this for pixels/mm calc

	GetCtrlVal (ld->panel_id, DIST_PANEL_IGNOREFIRST, &ignore); // Ignore 1st pt?
    
	if(ignore)
		start = 1;		 // This pt may have 2 elements so skip next element also

	for (i=0, k=start; k < len; k++)
    {
        if (ld->profile[k-start] < 220)
        {    
			ld->points[i] = (double)k * direction_factor;		// Store the real pixel distance coord
            
			if (ld->profile[k+1] < 220)							// Two elements for one point
            {
                ld->points[i] += (0.5 * direction_factor);		// take coord between the 2 elements
                k++;											// skip 2nd element
            }

            ld->regular_points[i] = (double)i+1.0;				// store a pt on a regular grid, start at 1.0 
			i++;
        }
    }

	GetCtrlVal (ld->panel_id, DIST_PANEL_IGNORELAST, &ignore_last); // Ignore Last pt?

    if (ignore_last)
		i--;

    ld->number_of_points = i;  // Record the correct number of points found
}


void lens_distortion_fit_polynomial(LensDistortion* ld, FIBITMAP *input)
{
	int i, zerozero;
	double coeffs[5], mse, *temp, magnification = 1.0f, maxP;
	FIBITMAP *fib = NULL;

	lens_distortion_set_image(ld, input);

	lens_distortion_get_coordinates(ld);

	ld->window = BasicWin32Window("Input Image", 10, 10, 500, 500, ld->input);

    GetCtrlVal (ld->panel_id, DIST_PANEL_ZEROZERO, &zerozero); // force fit through (0,0)?
    	
	if ((ld->number_of_points < 4 && zerozero) || ld->number_of_points < 5) {
		MessagePopup ("Error", "Not enough points for a fit.\nMust have at least 5.");
		return;
	} 

   // GetCtrlVal (ld->panel_id, DIST_PANEL_MAG, &magnification);
  
	FIA_FindDoubleMax(ld->points, ld->number_of_points,  &maxP);	 // Real pixel distance to last pt

    for (i=0; i < ld->number_of_points; i++)
    {
        ld->points[i] = ld->points[i] / ld->discal_normalisation;      // Normalise distances to definitely 0-1
        ld->regular_points[i] = ld->regular_points[i]/ ld->number_of_points*maxP/ld->discal_normalisation*magnification; // scale regular pts to maxP, normalise 0-1 and magnify to desired size (desired = size for destination/corrected image)
    }

   // firstLastPointDistance = (ld->regular_points[noOfPoints-1] - ld->number_of_points[0]) * normalisation; // real pixel distance between first and last points in the corrected image, store this value for pixel/mm calc
    
    temp = (double *)malloc(ld->number_of_points*sizeof(double));
	
	if (temp==NULL && ld->number_of_points > 0)
	{
		MessagePopup ("Error", "malloc error in fitPolnomial.");
		return;
	}

//    if (zerozero)
 //   {
        for (i=0; i<ld->number_of_points; i++)
            ld->points[i] = ld->points[i]/ld->regular_points[i];

        PolyFit (ld->regular_points, ld->points, ld->number_of_points, 3, temp, coeffs, &mse);

        ld->a = coeffs[3]; 
        ld->b = coeffs[2]; 
        ld->c = coeffs[1]; 
        ld->d = coeffs[0]-1.0; 
        
//      printf("(a)%f (b)%f (c)%f (d)%f 0.0    error %f\n", coeffA, coeffB, coeffC, coeffD, mse);   
        for (i=0; i<ld->number_of_points; i++)
        {
            ld->points[i] = ld->points[i]*ld->regular_points[i];
            temp[i] = temp[i]*ld->regular_points[i];
        }
 //   }
 //   else
//    {
 //       PolyFit (ld->regular_points, ld->points, ld->number_of_points, 4, temp, coeffs, &mse);

 //       ld->a = coeffs[4]; 
 //       ld->b = coeffs[3]; 
 //       ld->c = coeffs[2]; 
 //       ld->d = coeffs[1]-1.0; 

//      printf("(a)%f (b)%f (c)%f (d)%f %f    error %f\n", coeffA, coeffB, coeffC, coeffD, coeffs[0], mse); 
//    }
    
    SetCtrlVal (ld->poly_panel_id, POLY_PNL_A, ld->a);
    SetCtrlVal (ld->poly_panel_id, POLY_PNL_B, ld->b);
    SetCtrlVal (ld->poly_panel_id, POLY_PNL_C, ld->c);
    SetCtrlVal (ld->poly_panel_id, POLY_PNL_D, ld->d);
    
 //   SetCtrlVal (ld->panel_id, MAINPANEL_A_VAL, ld->a);
 //   SetCtrlVal (ld->panel_id, MAINPANEL_B_VAL, ld->b);
 //   SetCtrlVal (ld->panel_id, MAINPANEL_C_VAL, ld->c);
 //   SetCtrlVal (ld->panel_id, MAINPANEL_D_VAL, ld->d);
 //   SetCtrlVal (ld->panel_id, MAINPANEL_ERROR_VAL, mse);
    
    SetCtrlVal (ld->poly_panel_id, POLY_PNL_ERROR, mse);
    SetCtrlVal (ld->poly_panel_id, POLY_PNL_OFFSETX, ld->offset.x);
    SetCtrlVal (ld->poly_panel_id, POLY_PNL_OFFSETY, ld->offset.y);
    
    DeleteGraphPlot (ld->poly_panel_id, POLY_PNL_GRAPH, -1, VAL_IMMEDIATE_DRAW);
    
    PlotXY (ld->poly_panel_id, POLY_PNL_GRAPH, ld->regular_points, ld->points,
            ld->number_of_points, VAL_DOUBLE, VAL_DOUBLE, VAL_SCATTER, VAL_CROSS,
            VAL_SOLID, 1, VAL_RED);

    PlotXY (ld->poly_panel_id, POLY_PNL_GRAPH, ld->regular_points, temp,
            ld->number_of_points, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE,
            VAL_NO_POINT, VAL_SOLID, 1, VAL_BLUE);
    
   // calcPixelsPerMm();
    
    DisplayPanel (ld->poly_panel_id);

    free(temp);


	// If you do not want to scale the image, you should set d so that a + b + c + d = 1. 

	// Leave image as is
	//fib = FIA_CorrectLensDistortion (ld->input, 0.0, 0.0, 0.0, 1.0, ld->offset);
	
	// Zoom
	//fib = FIA_CorrectLensDistortion (ld->input, 0.0, 0.0, 0.0, 0.8, ld->offset);

	// An eaxmple that seems to correct for the pix cushion demo image

	//fib = FIA_CorrectLensDistortion (ld->input, 0.06, 0.04, 0.0, 0.9, ld->offset);
	fib = FIA_CorrectLensDistortion (ld->input, 0.01, 0.05, 0.0, 0.94, ld->offset);

//	fib = FIA_CorrectLensDistortion (ld->input, ld->a, ld->b, ld->c, ld->d, ld->offset);

	BasicWin32Window("Output Image", 50, 50, 500, 500, fib);

}


/*********************************************************************************

/***********************************************************************
 
  Paul R Barber
  
  This file was part of the Colony Counter project.
  
  copyright Gray Cancer Institute 1999, 2000, 2001, 2002.
  
  Stand alone program to measure the image pin and barrel distortion and fit a forth order
  polynomial to it.
  The parameters a, b, c and d can then be used in with the distortion removal code
  to correct the image.

 It needs an image of a regularly spaced structure, from the centre of the image to the edge, taken using the imager.
 The parameters, a, b, c and d, produced can be used in the function imagePinBarrelDistortion below to correct for the distortion in any
 cvi app using the imager.

  ***********************************************************************/

/*


#define DISCAL_VERSION "0.0.0.0"
#define MAX_PROFILE_LENGTH 1000
#define BRIGHTFIELD_THRESHOLD 10

//int gFlaskSetupPanel, jigSensorPanel;
//Point  cameraOffset;  // cameraOffset is shift of image centre wrt drawer target.
//static Rect lastLine;

//static int profile[MAX_PROFILE_LENGTH], binaryProfile[MAX_PROFILE_LENGTH];
//static int noOfElements, threshold=127, profilePlot=-99, binaryPlot=-99, brightfield;
//static IPIProfReport profileReport;
//static double *points=NULL, *regularPoints=NULL, coeffA, coeffB, coeffC, coeffD, firstLastPointDistance, lastPointAngle;

//static int live=FALSE;

//static int drawerStop=FALSE;

//********************************* calcPixelsPerMm ********************************************************************

int calcPixelsPerMm (void)
{
    int correctForAngle=0;
    double spacing=0.0, angleCorrection, ppmm, ca, sa;
    
    GetCtrlVal (panel2Handle, POLY_PNL_SPACING, &spacing);
    GetCtrlVal (panel2Handle, POLY_PNL_ANGLE, &correctForAngle);

    if (spacing==0.0 || firstLastPointDistance==0.0 || noOfPoints<=1) return(-1);
    
    if (correctForAngle)
    {
        ca = fabs(cos (lastPointAngle));
        sa = fabs(sin (lastPointAngle));
        if (ca>sa) angleCorrection=ca;
        else angleCorrection=sa;
        spacing /= angleCorrection;
    }
    
    ppmm = firstLastPointDistance / (spacing * (noOfPoints-1));   // (noOfPoints-1) because distance is between first and last points
    
    SetCtrlVal (panel2Handle, POLY_PNL_PIXELSPERMM, ppmm);

    return(0);
}


void thinBinaryArray( int *a, int noOfPoints )
{
    int i, stop=FALSE, *b;
    
    for (i=0; i<2; i++) {a[i]=0; a[noOfPoints-i]=0;} // too near centre of image or too near end of array

    b = (int *)malloc(noOfPoints*sizeof(int)); if (b==NULL && noOfPoints>0) {MessagePopup ("Error", "Memory allocation error in thinBinaryArray"); return;}
    for (i=0; i<noOfPoints; i++) b[i]=a[i];      // integer array copy
    
    while (!stop)
    {
        stop=TRUE;
        for (i=2; i<noOfPoints-2; i++) // never need to thin first or last pixel
        {
            if (a[i]>0)
                if ((a[i-1]==0 && a[i+1]+a[i+2]==2) || (a[i+1]==0 && a[i-1]+a[i-2]==2)) // 1 on either side of a[i], not both + not 2 isolated pixels
                {
                    b[i]=0;
                    stop=FALSE;
                }
        }
        for (i=0; i<noOfPoints; i++) a[i]=b[i];      // integer array copy
    }
    free(b);
}

void findDots ()
{
    thinBinaryArray( binaryProfile, noOfElements );
}


void getCoordinates ()
{
    int i, j=0, dx, dy, first=TRUE, ignore;
    double directionFactor;
    
    if (points!=NULL) free(points);
    if (regularPoints!=NULL) free(regularPoints);
    //  noOfPoints+2 since  noOfPoints may have been previously reduced by discarding this first and last points
    points = (double *)malloc((noOfPoints+2)*sizeof(double)); if (points==NULL) {MessagePopup ("Error", "malloc error in getCoordinates."); return;}
    regularPoints = (double *)malloc((noOfPoints+2)*sizeof(double)); if (regularPoints==NULL) {MessagePopup ("Error", "malloc error in getCoordinates."); return;}

    dx = lineEnd.x-imagerCentre.x;
    dy = lineEnd.y-imagerCentre.y;
    directionFactor = sqrt((double)(dx*dx + dy*dy))/noOfElements;
                     
    lastPointAngle = atan2 (dy, dx); // store this for pixels/mm calc

    GetCtrlVal (panelHandle, DIST_PANEL_IGNOREFIRST, &ignore); // ignore 1st pt?
    
    for (i=0; i<noOfElements; i++)
    {
        if (binaryProfile[i]>0)
        {
            if (ignore && first)     // ignore the first pt
            {
                first=FALSE;
                i++;                 // this pt may have 2 elements so skip next element also
            }
            else                     // process the pt
            {
                points[j] = (double)i * directionFactor;     // store the real pixel distance coord
                if (binaryProfile[i+1]>0)                    // two elements for one point
                {
                    points[j] += (0.5 * directionFactor);    // take coord between the 2 elements
                    i++;                                     // skip 2nd element
                }
                regularPoints[j] = (double)j+1.0;            // store a pt on a regular grid, start at 1.0
                j++;                                         // next pt
                first=FALSE;
            }
        }
    }
    
    GetCtrlVal (panelHandle, DIST_PANEL_IGNORELAST, &ignore);  // ignore last pt?
    if (ignore) j--;
    noOfPoints = j;  // record the correct number of points found
}


void fitPolynomial ()
{
    int i, zerozero;
    double coeffs[5], mse, *temp, magnification, normalisation, maxP;
    //char message[256];
    
    GetCtrlVal (panelHandle, DIST_PANEL_ZEROZERO, &zerozero); // force fit through (0,0)?
    if ((noOfPoints<4 && zerozero) || noOfPoints<5) {MessagePopup ("Error", "Not enough points for a fit.\nMust have at least 5."); return;} 

    normalisation = DISCAL_NORMALISATION;
    GetCtrlVal (panelHandle, DIST_PANEL_MAG, &magnification);
    Find_Double_Max (points, noOfPoints, &maxP);     // real pixel distance to last pt

    for (i=0; i<noOfPoints; i++)
    {
        points[i] = points[i]/normalisation;      // normalise distances to definitely 0-1
        regularPoints[i] = regularPoints[i]/noOfPoints*maxP/normalisation*magnification; // scale regular pts to maxP, normalise 0-1 and magnify to desired size (desired = size for destination/corrected image)
    }

    firstLastPointDistance = (regularPoints[noOfPoints-1]-regularPoints[0]) * normalisation; // real pixel distance between first and last points in the corrected image, store this value for pixel/mm calc
    
    temp = (double *)malloc(noOfPoints*sizeof(double)); if (temp==NULL && noOfPoints>0) {MessagePopup ("Error", "malloc error in fitPolnomial."); return;}

    if (zerozero)
    {
        for (i=0; i<noOfPoints; i++)
            points[i] = points[i]/regularPoints[i];
        PolyFit (regularPoints, points, noOfPoints, 3, temp, coeffs, &mse);
        coeffA = coeffs[3]; 
        coeffB = coeffs[2]; 
        coeffC = coeffs[1]; 
        coeffD = coeffs[0]-1.0; 
        
//      printf("(a)%f (b)%f (c)%f (d)%f 0.0    error %f\n", coeffA, coeffB, coeffC, coeffD, mse);   
        for (i=0; i<noOfPoints; i++)
        {
            points[i] = points[i]*regularPoints[i];
            temp[i] = temp[i]*regularPoints[i];
        }
    }
    else
    {
        PolyFit (regularPoints, points, noOfPoints, 4, temp, coeffs, &mse);
        coeffA = coeffs[4]; 
        coeffB = coeffs[3]; 
        coeffC = coeffs[2]; 
        coeffD = coeffs[1]-1.0; 
//      printf("(a)%f (b)%f (c)%f (d)%f %f    error %f\n", coeffA, coeffB, coeffC, coeffD, coeffs[0], mse); 
    }
    
    SetCtrlVal (panel2Handle, POLY_PNL_A, coeffA);
    SetCtrlVal (panel2Handle, POLY_PNL_B, coeffB);
    SetCtrlVal (panel2Handle, POLY_PNL_C, coeffC);
    SetCtrlVal (panel2Handle, POLY_PNL_D, coeffD);
    
    SetCtrlVal (distmeasurepanel, MAINPANEL_A_VAL, coeffA);
    SetCtrlVal (distmeasurepanel, MAINPANEL_B_VAL, coeffB);
    SetCtrlVal (distmeasurepanel, MAINPANEL_C_VAL, coeffC);
    SetCtrlVal (distmeasurepanel, MAINPANEL_D_VAL, coeffD);
    SetCtrlVal (distmeasurepanel, MAINPANEL_ERROR_VAL, mse);
    
    SetCtrlVal (panel2Handle, POLY_PNL_ERROR, mse);
    SetCtrlVal (panel2Handle, POLY_PNL_OFFSETX, imageOffset.x);
    SetCtrlVal (panel2Handle, POLY_PNL_OFFSETY, imageOffset.y);
    
    DeleteGraphPlot (panel2Handle, POLY_PNL_GRAPH, -1, VAL_IMMEDIATE_DRAW);
    
    PlotXY (panel2Handle, POLY_PNL_GRAPH, regularPoints, points,
            noOfPoints, VAL_DOUBLE, VAL_DOUBLE, VAL_SCATTER, VAL_CROSS,
            VAL_SOLID, 1, VAL_RED);
    PlotXY (panel2Handle, POLY_PNL_GRAPH, regularPoints, temp,
            noOfPoints, VAL_DOUBLE, VAL_DOUBLE, VAL_THIN_LINE,
            VAL_NO_POINT, VAL_SOLID, 1, VAL_BLUE);
    
    calcPixelsPerMm();
    
    DisplayPanel (panel2Handle);

    free(temp);
}


int CVICALLBACK OnMeasureDistortion (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_COMMIT:
            getCoordinates();
            fitPolynomial();
            break;
	}

    return 0;
}

void doThreshold ()
{
    int i;
    
    Threshold_Int_Filter (profile, binaryProfile, noOfElements, threshold, threshold+1);
    for (i=0; i<noOfElements; i++) binaryProfile[i]-=threshold;
    if (brightfield) for (i=0; i<noOfElements; i++) binaryProfile[i]=abs(binaryProfile[i]-1); // invert binary profile
    if (binaryPlot!=-99) DeleteGraphPlot (profilePanel, PROFILE_GRAPH, binaryPlot, VAL_IMMEDIATE_DRAW);
    SetCtrlAttribute (profilePanel, PROFILE_GRAPH, ATTR_ACTIVE_YAXIS, VAL_RIGHT_YAXIS);
    binaryPlot = PlotY (profilePanel, PROFILE_GRAPH, binaryProfile, noOfElements, VAL_INTEGER, VAL_FAT_LINE, VAL_NO_POINT, VAL_SOLID, 1, VAL_BLUE);
    SetCtrlAttribute (profilePanel, PROFILE_GRAPH, ATTR_ACTIVE_YAXIS, VAL_LEFT_YAXIS);
}




void preProcess()
{
    SetWaitCursor (1);
    
    if (brightfield) ReverseImage(image, result);
    RemoveQuadraticBg(result, result);  
    if (brightfield) ReverseImage(result, result);
    putImageOnCanvas (result, imagePanel, imageCanvas);
    Normalise(result, result);
    putImageOnCanvas (result, imagePanel, imageCanvas);

    SetWaitCursor (0);
}



int CVICALLBACK cbGraph (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    double temp, temp2;
    
    switch (event)
        {
        case EVENT_VAL_CHANGED:
            GetGraphCursor (profilePanel, PROFILE_GRAPH, 1, &temp2, &temp);
            threshold=(int)temp;
            doThreshold();
            findDots();
            updateOverlay();

            break;
        }
    return 0;
}


int CVICALLBACK cbPreProcess (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    int preprocess;
    
    switch (event)
        {
        case EVENT_COMMIT:
            if (imageLoaded)
            {
                GetCtrlVal (panelOptions, DCOPTIONS_PREPROCESS, &preprocess);
                if (preprocess) preProcess();
                else IPI_Copy(image, result);
                putImageOnCanvas (result, imagePanel, imageCanvas);
                lineProfile();
                doThreshold();
                findDots();
                updateOverlay();
            }
            break;
        }
    return 0;
}

int CVICALLBACK cbDarkDots (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    int preprocess;

    switch (event)
        {
        case EVENT_COMMIT:
            GetCtrlVal (panelOptions, DCOPTIONS_DARKDOTS, &brightfield);
            if (imageLoaded)
            {
                GetCtrlVal (panelOptions, DCOPTIONS_PREPROCESS, &preprocess);
                if (preprocess) preProcess();
                else IPI_Copy(image, result);
                putImageOnCanvas (result, imagePanel, imageCanvas);
                lineProfile();
                doThreshold();
                findDots();
                updateOverlay();
            }

            break;
        }
    return 0;
}


int CVICALLBACK cbSmoothProfile (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
        {
        case EVENT_COMMIT:
                    lineProfile();
                    doThreshold();
                    findDots();
                    updateOverlay();
            break;
        }
    return 0;
}


int CVICALLBACK cbImageCentering (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
        {
        case EVENT_COMMIT:
    

                DisplayPanel (panelOffset);

            
            break;
        }
    return 0;
}
 

int CVICALLBACK cbCalcPixelsPerMm (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
        {
        case EVENT_COMMIT:
            calcPixelsPerMm();
            break;
        }
    return 0;
}


int CVICALLBACK cbShowOptions (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
        {
        case EVENT_COMMIT:

            DisplayPanel (panelOptions);
            
            break;
        }
    return 0;
}


int CVICALLBACK cbShowOffset (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
        {
        case EVENT_COMMIT:

            DisplayPanel (panelOffset);
            
            break;
        }
    return 0;
}


int CVICALLBACK cbCloseDistortionPanel (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
        {
        case EVENT_COMMIT:
            HidePanel (panel);

            break;
        }
    return 0;
}


int CVICALLBACK cbDCClose (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
        {
        case EVENT_COMMIT:
            HidePanel (panel);

            // replace any camera offset set by image centering
            SetCtrlVal (panelOffset, OFFSET_XOFFSET, (cameraOffset.x));
            SetCtrlVal (panelOffset, OFFSET_YOFFSET, (cameraOffset.y));

            break;
        }
    return 0;
}


int CVICALLBACK cbImagerCentreOffset (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    int xOffset, yOffset;
    
    switch (event)
        {
        case EVENT_COMMIT:

            GetCtrlVal (panelOffset, OFFSET_XOFFSET, &xOffset);
            GetCtrlVal (panelOffset, OFFSET_YOFFSET, &yOffset);
            // NB below is copied from createImagePanel()
            imagerCentre = MakePoint(origImagerCentre.x - xOffset, origImagerCentre.y - yOffset);   // imager centre in image - offset
            imageOffset  = MakePoint(origImageOffset.x + xOffset,  origImageOffset.y + yOffset);

            lineProfile();
            doThreshold();
            findDots();
            updateOverlay();

            break;
        }
    return 0;
}


int CVICALLBACK cbDistortion (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
        {
        case EVENT_COMMIT:
                DisplayPanel (panelHandle);
                DisplayPanel (profilePanel);
                
                // hold any camera offset set by image centering
                GetCtrlVal (panelOffset, OFFSET_XOFFSET, &(cameraOffset.x));
                GetCtrlVal (panelOffset, OFFSET_YOFFSET, &(cameraOffset.y));

                // need to do distortion calibration wrt imager/lens centre
                SetCtrlVal (panelOffset, OFFSET_XOFFSET, 0);
                SetCtrlVal (panelOffset, OFFSET_YOFFSET, 0);

            break;
        }
    return 0;
}

int CVICALLBACK cbImagePanelCloseControl (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
        {
        case EVENT_COMMIT:
                SetCtrlAttribute (panelHandle, DIST_PANEL_TIMER, ATTR_ENABLED, 0);
                HidePanel (imagePanel);
                

            break;
        }
    return 0;
}


int CVICALLBACK cbOpenImageTweakerPanel (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
        {
        case EVENT_COMMIT:
            DisplayPanel (panelOffset);
            break;
        }
    return 0;
}



*/
