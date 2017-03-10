#include "AutoFocus.h" 
#include "AutoFocus_ui.h"
#include "focus.h"
#include "gci_utils.h"
#include "camera\gci_camera.h" 
#include "optical_calibration.h"

#include "FreeImageAlgorithms_Utilities.h" 

#include "microscope.h"

#include <utility.h>
#include <analysis.h>
#include <ansi_c.h>
#include <formatio.h>
#include <userint.h>
#include <toolbox.h>


////////////////////////////////////////////////////////////////////////////
//Kings College MPTR System - Autofocus
//
//Rosalind Locke - April 2003
//
//Fibonacci code by Tim Idzenga 2001.
////////////////////////////////////////////////////////////////////////////
//RJ Locke, June 2004
//Added fast shutter 
////////////////////////////////////////////////////////////////////////
//RJL August 2004
//Limit search range for high magnification
//Make it work for colour images
////////////////////////////////////////////////////////////////////////////

#define MAX_AF_IMAGES 50
static int FAST_ACQ=0;	  // uses camera live mode, for the imperx this is slower than doing repeated snaps

SWAutoFocus* sw_autofocus_new(void)   
{
	SWAutoFocus *af = (SWAutoFocus *) malloc (sizeof(SWAutoFocus));		
	
	af->ms = microscope_get_microscope();
	af->freeimage_dib_array = (FIBITMAP **) calloc(MAX_AF_IMAGES, sizeof(FIBITMAP *));         
	af->_abort = 0;

	memset(af->freeimage_dib_array, 0, sizeof(FIBITMAP *) * MAX_AF_IMAGES);
	
	ui_module_constructor(UIMODULE_CAST(af), "SoftwareAutoFocus");  
	
	af->_panel_id = ui_module_add_panel(UIMODULE_CAST(af), "AutoFocus_ui.uir", AFPNL, 1);   
	af->_msg_panel_id = ui_module_add_panel(UIMODULE_CAST(af), "AutoFocus_ui.uir", MSGPANEL, 0);   
	
	if ( InstallCtrlCallback (af->_panel_id, AFPNL_CLOSE, OnCloseButton, af) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (af->_panel_id, AFPNL_AUTOFOCUS, OnAutoFocusGoButton, af) < 0)
		return NULL;
	
	return af;
}

static void sw_autofocus_free_images(SWAutoFocus *af)
{
	int i;
	
	for (i=0; i < MAX_AF_IMAGES; i++) {
	
		if(af->freeimage_dib_array[i] == NULL)
			break;
		
		FreeImage_Unload(af->freeimage_dib_array[i]);
		af->freeimage_dib_array[i] = NULL;
	}
}


void sw_autofocus_destroy(SWAutoFocus *af)
{
	sw_autofocus_free_images(af);   
		
	free(af->freeimage_dib_array);
	af->freeimage_dib_array = NULL;
	
	ui_module_destroy(UIMODULE_CAST(af));
	
	free(af);
}

static FIBITMAP* AcquireFocusedImage(SWAutoFocus *af)
{
	double t1, exposure;
	FIBITMAP *dib;
	
	// It's faster to put the camera into live mode than to do individual snaps.
	// However, the stage may still be moving during the acquisition, 
	// so we have to acquire two frames at each stage position.
	
	if (FAST_ACQ) {
		
		exposure = gci_camera_get_exposure_time(MICROSCOPE_MASTER_CAMERA(af->ms));
	
		z_drive_wait_for_stop_moving(MICROSCOPE_MASTER_ZDRIVE(af->ms), 1.0);
		dib = gci_camera_get_image(MICROSCOPE_MASTER_CAMERA(af->ms), NULL);       // Dummy image because stage is moving during acquisition
		
		t1=clock();
		
		while ((clock() - t1) < exposure){        // Ensure its the correct frame
			FreeImage_Unload(dib);
			dib = gci_camera_get_image(MICROSCOPE_MASTER_CAMERA(af->ms), NULL);
		}

		
		dib = gci_camera_get_image(MICROSCOPE_MASTER_CAMERA(af->ms), NULL);
	
	}
	else { 
	
		z_drive_wait_for_stop_moving(MICROSCOPE_MASTER_ZDRIVE(af->ms), 1.0);
		dib = gci_camera_get_image(MICROSCOPE_MASTER_CAMERA(af->ms), NULL);
	}
	
	return dib;
}



////////////////////////////////////////////////////////////////////////
// Tim's Fibonacci search

static double TailFib (int n, int next, int result)		
{
	//Function for creating Fibonacci-numbers
	if (n==0)
		return result;
		
	return (TailFib(n-1, next+result, next));
}


static void SetupFibonacci(SWAutoFocus *af, double *a, double *b,  double e, double u, double *Fn, int *j)
{
	int range, i=0, m=10;
	double L, curZ;
	Objective obj;
	
	GetCtrlVal(af->_panel_id, AFPNL_END_POINTS, &range);
	
	//If magnification is > x20 we don't want too big a range
	objective_manager_get_current_objective(af->ms->_objective, &obj);
	sscanf(obj._magnification_str, "%d", &m);        
	
	// Not sure why this was here. I'm going to assume the user knows what they want.
	//if ((m>20) && (range>50)){
	//	range = 50;
	//	SetCtrlVal(af->_panel_id, AFPNL_END_POINTS, range);
	//}

	z_drive_get_position(MICROSCOPE_MASTER_ZDRIVE(af->ms), &curZ);    
		
	// Define initial end points
	a[0] = curZ - range;
	b[0] = curZ + range;
	
	// Calculate reduction factor of uncertainty range
	L = ((1 + 2*e)*(b[0] - a[0]))/u;
	
	// Create Fibonacci-numbers-array
	do {
		i++;
		Fn[i] = TailFib(i,1,1);
	} while (Fn[i] < L);
	
	*j=i;
}


static double Phik (int i, int k, double Fn[])				
{
	//Function for creating Phi-factors
	return  1 - Fn[i-k] / Fn[i-k+1];
}


static double ak (double x[], double y[], double Phi[], int k)				//Function for new a-boundary
{
	return x[k-2] + Phi[k] * (y[k-1] - x[k-1]);	
}


static double bk (double x[], double y[], double Phi[], int k)				//Function for new b-boundary
{
	return x[k-1] + (1 - Phi[k]) * (y[k-1] - x[k-1]);
}


static void AutoFocusFib(SWAutoFocus *af)
{
	FIBITMAP *dib;
	int maxi_a, maxi_b, mini;
	double start_time, maxa, maxb, min;
	double a[50], b[50], x[50], y[50];
	double uncertainty = 0.1, e = 0.1;
	double Fn[50], Phi[50], fa[50], fb[50];
	double min_value, max_value;
	int i, k, im=0;
	char time_taken[50] = "";
	
	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);

	GetCtrlVal(af->_panel_id, AFPNL_UNCERTAINTY, &uncertainty);
	start_time = Timer();
	
	SetupFibonacci(af, a, b, e, uncertainty, Fn, &i);
	
	if (i > 50) {
	
		GCI_MessagePopup("Autofocus Error", "Too many steps");
		return;
	}
	
	memset(fa, 0, sizeof(double) * 50);
	memset(fb, 0, sizeof(double) * 50);  

	gci_camera_set_snap_mode(MICROSCOPE_MASTER_CAMERA(af->ms));
	
	af->freeimage_dib_array[0] = gci_camera_get_image(MICROSCOPE_MASTER_CAMERA(af->ms), NULL);
	
	if( FreeImage_GetBPP(af->freeimage_dib_array[0]) > 16 && FreeImage_GetImageType(af->freeimage_dib_array[0]) == FIT_BITMAP) {
	
		// Colour Image
		dib = FreeImage_ConvertTo8Bits(af->freeimage_dib_array[0]);
	}
	else {
	
		dib = FreeImage_Clone(af->freeimage_dib_array[0]);
	}
	
	if(dib == NULL)
		return;

	FIA_FindMinMax(dib, &min_value, &max_value);

	if( (max_value - min_value) < 10 )
		return;
	
	FreeImage_Unload(dib);
	FreeImage_Unload(af->freeimage_dib_array[0]);  // free this also, aquired with im=0 below
	
	// Put camera into continuous acquisition mode
	if (FAST_ACQ) {
	
		gci_camera_set_live_mode(MICROSCOPE_MASTER_CAMERA(af->ms));
	}	
	else {
	
    	gci_camera_set_snap_sequence_mode(MICROSCOPE_MASTER_CAMERA(af->ms)); 
    }

	//First iteration
	x[0] = a[0];
	y[0] = b[0];
	Phi[1] = Phik(i, 1, Fn);
	a[1] = a[0] + Phi[1] * (b[0] - a[0]);		//specification of new calculation-point a
	b[1] = a[0] + (1 - Phi[1]) * (b[0] - a[0]);	//specification of new calculation-point b
	
	z_drive_set_position(MICROSCOPE_MASTER_ZDRIVE(af->ms), a[1]);    
	
	af->freeimage_dib_array[im] = AcquireFocusedImage(af); 
	
	focus_get_image_focus(af->ms->_focus, af->freeimage_dib_array[im], &fa[1]);      
	
	im++;

	z_drive_set_position(MICROSCOPE_MASTER_ZDRIVE(af->ms), b[1]);    
	
	af->freeimage_dib_array[im] = AcquireFocusedImage(af);
	
	focus_get_image_focus(af->ms->_focus, af->freeimage_dib_array[im], &fb[1]);    
	
	im++;
	
	if (fa[1] > fb[1]) {
	
		x[1] = a[0];
		y[1] = b[1];
	}
	else {
	
		x[1] = a[1];
		y[1] = b[0];
	}
	
	// Subsequent interations
	for (k=2; k<i; k++)	{	//number of iterations to reach the specified uncertainty range
	
		ProcessDrawEvents();

		if(af->_abort)
			break;

		Phi[k] = Phik(i, k,Fn);
		
		if (Phi[k] == 0.5)
			Phi[k] = 0.5 - e;

		if (fa[k-1] > fb[k-1]) {
		
			a[k] = ak(x, y, Phi, k);
			b[k] = a[k-1];
			
			z_drive_set_position(MICROSCOPE_MASTER_ZDRIVE(af->ms), a[k]);    
			
			af->freeimage_dib_array[im] = AcquireFocusedImage(af);
			
			focus_get_image_focus(af->ms->_focus, af->freeimage_dib_array[im], &fa[k]);     
			
			im++;
			
			fb[k] = fa[k-1];
		
			if (fa[k] > fb[k]) {
			
				x[k] = x[k-1];
				y[k] = b[k];
			}
			else {
			
				x[k] = a[k];
				y[k] = y[k-1];
			}
		}
		else  {
		
			a[k] = b[k-1];
			b[k] = bk(x, y, Phi, k);
			fa[k] = fb[k-1];
			z_drive_set_position(MICROSCOPE_MASTER_ZDRIVE(af->ms), b[k]);    
			
			af->freeimage_dib_array[im] = AcquireFocusedImage(af);
			
			focus_get_image_focus(af->ms->_focus, af->freeimage_dib_array[im], &fb[k]);     
			
			im++;

			if (fa[k] > fb[k]) {
				x[k] = x[k-1];
				y[k] = b[k];
			}
			else {
				x[k] = a[k];
				y[k] = y[k-1];
			}
		}
	}
	
	MaxMin1D (fa, k, &maxa, &maxi_a, &min, &mini);
	MaxMin1D (fb, k, &maxb, &maxi_b, &min, &mini);
	
	if (maxa > maxb)
		z_drive_set_position(MICROSCOPE_MASTER_ZDRIVE(af->ms), a[maxi_a]);
	else
		z_drive_set_position(MICROSCOPE_MASTER_ZDRIVE(af->ms), b[maxi_b]);   

	seconds_to_friendly_time(Timer() - start_time, time_taken);   
	SetCtrlVal(af->_panel_id, AFPNL_TIME_TAKEN, time_taken);   
	
	gci_camera_set_snap_mode(MICROSCOPE_MASTER_CAMERA(af->ms));
		
	// Free old image array
	sw_autofocus_free_images(af);
}


void sw_autofocus_autofocus_abort(SWAutoFocus *af)
{
	af->_abort = 1;
}

////////////////////////////////////////////////////////////////////////
void sw_autofocus_autofocus(SWAutoFocus *af)
{
	//int was_live;
	CameraState state;
	
	af->_abort = 0;

//	InstallPopup(af->_msg_panel_id);
	logger_log(UIMODULE_LOGGER(af), LOGGER_INFORMATIONAL, "Performing autofocus");

	//In fluorescence mode the timers which update the panels interfere with the shutter
	microscope_stop_all_timers(af->ms);
	
	//Remember camera settings
	gci_camera_save_state(MICROSCOPE_MASTER_CAMERA(af->ms), &state);
	//was_live = gci_camera_is_live_mode(MICROSCOPE_MASTER_CAMERA(af->ms));

	microscope_set_focusing_mode(af->ms);
	
	AutoFocusFib(af);

	//Restore camera settings
	gci_camera_restore_state(MICROSCOPE_MASTER_CAMERA(af->ms), &state);
	
//	gci_camera_snap_image(MICROSCOPE_MASTER_CAMERA(af->ms));

	microscope_start_all_timers(af->ms);    

	//if (was_live)
	//{
	//	gci_camera_set_live_mode(MICROSCOPE_MASTER_CAMERA(af->ms));
	//}
	
//	RemovePopup(0);
}

////////////////////////////////////////////////////////////////////////
int CVICALLBACK OnCloseButton (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			SWAutoFocus *af = (SWAutoFocus *) callbackData;	
			
			ui_module_hide_all_panels(UIMODULE_CAST(af));      
			
			break;
		}
	}
	return 0;
}


int CVICALLBACK OnAutoFocusGoButton (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{	
	switch (event)
	{
		case EVENT_COMMIT:
		{
			SWAutoFocus *af = (SWAutoFocus *) callbackData;	   
			
			sw_autofocus_autofocus(af); 
			
			if (!gci_camera_is_live_mode(MICROSCOPE_MASTER_CAMERA(af->ms)))	gci_camera_snap_image(MICROSCOPE_MASTER_CAMERA(af->ms));
 			
			break;
		}
	}
	return 0;
}

