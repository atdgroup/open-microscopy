#include <cvirte.h>		
#include <userint.h>
#include "asynctmr.h"
#include <toolbox.h>
#include "gci_camera.h"
#include <formatio.h>
#include <utility.h>

#include "GL_CVIRegistry.h"
#include "hardware.h"
#include "icsviewer_signals.h"
#include "FilenameUtils.h"
#include "string_utils.h"
//#include "ElectronBeamSource.h"
#include "focus.h"			   
#include "refImages.h"
#include "modDistortionMeasurement.h"
#include "ImagingFacade.h"
#include "imaging.h"			
//#include "GeneralPurposeMicroscope.h"
#include "RegionOfInterest.h"
#include "RegionScan.h"
#include "RegionScan_ui.h"

#include "mosaic.h"

#include "signals.h"

////////////////////////////////////////////////////////////////////////////
//RJL/GP Jan 2005
//GCI Microfocus development. 
//Region scan application.
////////////////////////////////////////////////////////////////////////////
//RJ Locke June 2005
//Lock the image during multi-threaded operations
//Fixes to prevent failure to exit the multi-threaded scan
//Removed option to autofocus from SetFocalPlane.
//Prev and Next buttons did not work properly.
////////////////////////////////////////////////////////////////////////////
//RJ Locke October 2005
//Use new RegionOfInterest module.
//Correct the stage limits of the ROI if the objective changes.
////////////////////////////////////////////////////////////////////////////
//RJ Locke, 18 Nov 2004
//Change macro MICROFOCUS_DATA_DIR to GetProjectDir() etc. 
//////////////////////////////////////////////////////////////////////////////////
//RJ Locke, 28 June 2006
//Adapted for 90i Microscope system with new ics_viewer 
//////////////////////////////////////////////////////////////////////////////////

//#define REGIONSCAN_STANDALONE_ENABLED

#define MULTITHREAD
//#define TESTMODE
//#define TESTMODE2		//display times
//#define TESTMODE3		//repeated scans

static int regionScanPanel=-1;
static int gAsynchTimer=0;  

static IPIImageRef image=0;

static int gRow=0, gCol=0;
static int gThisFrame=1;
static int gFramesX=0, gFramesY=0;
static int gSaveImages=0;
static char gFilename[MAX_PATHNAME_LEN]="";
static int gOverlapSet = 0;

static int gPauseScan=0, gStopScan=0;

static signal_table RegionScan_signal_table;

static int mouse_down_handler;

//Variables to control synchronisation of multiple threads
static int gMT_scanComplete, gMT_exposureComplete1, gMT_exposureComplete2, gMT_xyzComplete;
static int gMT_makeImageComplete1, gMT_makeImageComplete2, gMT_makeImageComplete3, gMT_displayComplete, gMT_saveComplete, gMT_mosaicComplete;
//RJL 29_06_05 - line below added
static int lock = -1;
static double gOx, gOy, gGx, gGy;
#ifdef TESTMODE2
static double gTxyz, gTacq, gTbck, gTdisp, gTsave, gTmos, gTmosDisp;  //global timers for MT mode
#endif

static GciCamera *camera;
static GCIWindow *image_window = NULL;
static region_of_interest *roi;
static MosaicWindow* mosaic_window = NULL;

int CVICALLBACK cbRegionScanTimer (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
static int RegionScan_GotoStageXY(double x, double y);
static void RegionScan_PlotPosition(double x, double y, double z);
static void RegionScan_SetROI(void);

static void ImageWindowCloseEventHandler( GCIWindow *win, void* callback_data )
{
	/* Destroy imaging window */
	GCI_ImagingWindow_Close(image_window);

	image_window = NULL;
	
	QuitUserInterface(0); 
	
	return;
}


static void OnMosaicClicked (GCIWindow *window, const Point image_point, const Point viewer_point, void* data )
{
	//IPIImageInfo info;
	int pixelx, pixely;
	double startx, starty, microns_per_pixel, x_in_microns, y_in_microns, stage_x, stage_y;

	if ((image_point.x < 0) || (image_point.y < 0)) return;  //click outside window

	GetCtrlVal(regionScanPanel, ROI_SCAN_X_START, &startx);
	GetCtrlVal(regionScanPanel, ROI_SCAN_Y_START, &starty);
	
	microns_per_pixel = mosaic_window->um_per_pixel;
	
	//Start represents middle of top left tile and y is "upside down"
	pixelx = image_point.x - mosaic_window->tile_width/2;
	pixely = image_point.y - mosaic_window->tile_height/2;
	//pixely = image_point.y + mosaic_window->tile_height/2;
	
	x_in_microns = pixelx * microns_per_pixel;
	y_in_microns = pixely * microns_per_pixel;
	//IPI_GetImageInfo (mosaic_window->mosaic_image, &info);
	//y_in_microns = (info.height-pixely) * microns_per_pixel;
	
	stage_x = x_in_microns + startx;
	stage_y = y_in_microns + starty;
	
	RegionScan_GotoStageXY(stage_x, stage_y);
		
	gci_camera_snap_image(camera);
}


////////////////////////////////////////////////////////////////////////////
#ifdef REGIONSCAN_STANDALONE_ENABLED
int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	if (InitCVIRTE (hInstance, 0, 0) == 0) return -1;	/* out of memory */

	if (GCI_ImagingInit(hInstance)) return -1;
	GCI_ImagingDisplayPanel();

	if (GCI_RegionScan_Init()) return -1;
	GCI_RegionScan_DisplayPanel();

	RunUserInterface ();
	
	GCI_RegionScan_Close();
	GCI_ImagingClose(hInstance);
	
	return 0;
}
#endif
////////////////////////////////////////////////////////////////////////////
int RegionScan_signal_hide_handler_connect (void (*handler) (void*), void *callback_data)
{
	if (GCI_Signal_IsConnected(&RegionScan_signal_table, "Hide")) return 0;	//already connected
	
	if( GCI_Signal_Connect(&RegionScan_signal_table, "Hide", handler, callback_data) == SIGNAL_ERROR) {
		MessagePopup("Region Scan", "Can not connect signal handler for Hide signal");
		return -1;
	}

	return 0;
}

static int readOrWriteRegistry (int write)
{
	int visible;
	
	// load or save panel position
	
	// Make sure the panel is not minimised as this will put v. big values in the reg and at next startup the panel will not be visible!
	if(write == 1) {
		GetPanelAttribute (regionScanPanel, ATTR_VISIBLE, &visible);
		if(!visible) return 0;

		SetPanelAttribute (regionScanPanel, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);		
	}
	
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, "software\\GCI\\Microscope\\regionScan",  "top",   regionScanPanel, ATTR_TOP);
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, "software\\GCI\\Microscope\\regionScan",  "left",  regionScanPanel, ATTR_LEFT);

	return 0;																						   
}


static void onRegionOfInterestSelected (int left, int top, int width, int height, void *data)
{
	SetCtrlVal(regionScanPanel, ROI_SCAN_X_START, (double)left);
	SetCtrlVal(regionScanPanel, ROI_SCAN_Y_START, (double)top);
	SetCtrlVal(regionScanPanel, ROI_SCAN_X_LENGTH, (double)width);
	SetCtrlVal(regionScanPanel, ROI_SCAN_Y_LENGTH, (double)height);
	
	RegionScan_SetROI();
}

static void setInitialROI()
{
	double ox, oy, gx, gy;

	//Get ROI co-ords from panel
	GetCtrlVal(regionScanPanel, ROI_SCAN_X_START, &ox);
	GetCtrlVal(regionScanPanel, ROI_SCAN_Y_START, &oy);
	GetCtrlVal(regionScanPanel, ROI_SCAN_X_LENGTH, &gx);
	GetCtrlVal(regionScanPanel, ROI_SCAN_Y_LENGTH, &gy);
	region_of_interest_set_region(roi, ox, oy, gx, gy);
}

int GCI_RegionScan_Init()
{
	if (regionScanPanel >= 0)
		return 0;	//already done 
	
	if ((regionScanPanel = FindAndLoadUIR (0, "RegionScan_ui.uir", ROI_SCAN)) < 0)
		return -1;
		
	GCI_SignalSystem_Create(&RegionScan_signal_table, 2);
	GCI_Signal_New(&RegionScan_signal_table, "Hide", VOID_VOID_MARSHALLER);

	if (image == 0)
		IPI_Create (&image, IPI_PIXEL_U8, 0);

	gAsynchTimer = NewAsyncTimer (1.0, -1, 1, cbRegionScanTimer, 0);
	SetAsyncTimerAttribute (gAsynchTimer, ASYNC_ATTR_ENABLED,  0);
	
	camera = GCI_Imaging_GetCamera(); 


	#ifdef REGIONSCAN_STANDALONE_ENABLED
	
		if ( (image_window = GCI_ImagingWindow_Create("Image", 300, 300, 500, 500, 0, 1)) == NULL ) {
			
			MessagePopup("Error", "Can not create window");
		
			return -1;
		} 
		
		GCI_ImagingWindow_SetCloseEventHandler(image_window, ImageWindowCloseEventHandler, NULL );
		
	#else
	
		image_window = camera->_camera_window;
		
	#endif

	//if (mosaic_window == NULL) {
	
		//mosaic_window = mosaic_window_new(100, 100, 500, 500);
		
		//mouse_down_handler = GCI_ImagingWindow_SetMouseDownHandler (window, OnMosaicClicked , NULL); 
	//}

	//Create and initialise roi
	roi = region_of_interest_selection_new(camera);
	region_of_interest_selected_handler(roi, onRegionOfInterestSelected, NULL);
	region_of_interest_panel_init(roi);
	setInitialROI();
	
	return 0;
}


int GCI_RegionScan_Close()
{
	if (mosaic_window != NULL) {
		mosaic_window_destroy(mosaic_window);
		mosaic_window = NULL;
	}

	if (roi != NULL) region_of_interest_destroy(roi);
	
	if (image != 0)
		IPI_Dispose(image);
		
	if (gAsynchTimer != 0)
		DiscardAsyncTimer(gAsynchTimer);
		
	if (regionScanPanel >= 0)
		DiscardPanel (regionScanPanel);
	
	return 0;
}


int GCI_RegionScan_DisplayPanel()
{
	readOrWriteRegistry(0);
	DisplayPanel (regionScanPanel);
	
	//SetAsyncTimerAttribute (gAsynchTimer, ASYNC_ATTR_ENABLED,  1);
	
	return 0;
}


int GCI_RegionScan_HidePanel()
{
	readOrWriteRegistry(1);
	HidePanel (regionScanPanel);
	SetAsyncTimerAttribute (gAsynchTimer, ASYNC_ATTR_ENABLED,  0);

	//Pass on the event to higher modules		
	GCI_Signal_Emit(&RegionScan_signal_table, "Hide", GCI_VOID_POINTER, 1);  

	return 0;
}

int GCI_RegionScanPanelVisible()
{
	int visible;
	
	if (GetPanelAttribute (regionScanPanel, ATTR_VISIBLE, &visible))
		return -1;
		
	return visible;
}

int GCI_RegionScan_DisablePanel(int disable)
{
	SetPanelAttribute (regionScanPanel, ATTR_DIMMED, disable);
	return 0;
}

static void RegionScan_DisableControls(int disable)
{
	SetCtrlAttribute (regionScanPanel, ROI_SCAN_PREV, ATTR_DIMMED, disable);
	SetCtrlAttribute (regionScanPanel, ROI_SCAN_NEXT, ATTR_DIMMED, disable);
	SetCtrlAttribute (regionScanPanel, ROI_SCAN_SET_ROI, ATTR_DIMMED, disable);
	SetCtrlAttribute (regionScanPanel, ROI_SCAN_SET_PLANE, ATTR_DIMMED, disable);
	SetCtrlAttribute (regionScanPanel, ROI_SCAN_OVERLAP, ATTR_DIMMED, disable);
	SetCtrlAttribute (regionScanPanel, ROI_SCAN_DO_CORRECTIONS, ATTR_DIMMED, disable);
	SetCtrlAttribute (regionScanPanel, ROI_SCAN_QUIT, ATTR_DIMMED, disable);
}

////////////////////////////////////////////////////////////////////////////
static void RegionScan_CheckOverlap()
{
	unsigned int cols, rows;
	double umPerPix=1.0, x=1344.0, overlap;
	int binning, overlap_pixels;
//	GciOrcaCamera *orca_camera = (GciOrcaCamera *) camera;

	//Check size of overlap
	gci_camera_get_size(camera, &cols, &rows);
	umPerPix = gci_camera_get_microns_per_pixel(camera);
	binning = gci_camera_get_binning_mode(camera);
	if (binning < 1) binning = 1;	//avoid crashing if binning not implemented for camera
	
	if (gOverlapSet) {
		GetCtrlVal(regionScanPanel, ROI_SCAN_OVERLAP, &overlap);  
		overlap_pixels = Round(overlap/umPerPix);

		if (overlap_pixels < cols/50) {		 // < 2% image width
			if (ConfirmPopup("Warning", "The image overlap is rather small. Shall I set it to 10%?"))
				gOverlapSet = 0;
		}		
		else if (overlap_pixels/binning > cols/5) {   // > 20% image width
			if (ConfirmPopup("Warning", "The image overlap is rather large. Shall I set it to 10%?"))
				gOverlapSet = 0;
		}
	}
	
	//Set initial default overlap to 10% x frame width
	if (!gOverlapSet) {
		x = cols * umPerPix * binning;
		SetCtrlVal(regionScanPanel, ROI_SCAN_OVERLAP, ceil(x/10));
	}

	//Ensure it's a whole number of pixels
	GetCtrlVal(regionScanPanel, ROI_SCAN_OVERLAP, &overlap);  
	overlap_pixels = Round(overlap/umPerPix);
	SetCtrlVal(regionScanPanel, ROI_SCAN_OVERLAP, overlap_pixels*umPerPix);  
	gOverlapSet = 1;
}

static int RegionScan_UpdateUI()
{
	unsigned int cols, rows;
	double umPerPix=1.0, x=1344.0, y=1024.0;
	static double prev_x=0.0, prev_y=0.0;
	int binning;
	double start, len;
//	GciOrcaCamera *orca_camera = (GciOrcaCamera *) camera;
	
	gci_camera_get_size(camera, &cols, &rows);
	umPerPix = gci_camera_get_microns_per_pixel(camera);
	binning = gci_camera_get_binning_mode(camera);
	if (binning < 1) binning = 1;	//avoid crashing if binning not implemented for camera
	
	x = cols * umPerPix * binning;
	y = rows * umPerPix * binning;
	
	//Has the objective changed?
	if ((FP_Compare(x, prev_x) != 0) || (FP_Compare(y, prev_y) != 0)) {
		if (prev_x != 0) {
			//Yes, so adjust the extents of the stage movement to keep the visib;e ROI the same
			GetCtrlVal(regionScanPanel, ROI_SCAN_X_START, &start);
			SetCtrlVal(regionScanPanel, ROI_SCAN_X_START, start - prev_x/2 + x/2);
			GetCtrlVal(regionScanPanel, ROI_SCAN_Y_START, &start);
			SetCtrlVal(regionScanPanel, ROI_SCAN_Y_START, start - prev_y/2 + y/2);
			GetCtrlVal(regionScanPanel, ROI_SCAN_X_LENGTH, &len);
			SetCtrlVal(regionScanPanel, ROI_SCAN_X_LENGTH, len + prev_x - x);
			GetCtrlVal(regionScanPanel, ROI_SCAN_Y_LENGTH, &len);
			SetCtrlVal(regionScanPanel, ROI_SCAN_Y_LENGTH, len + prev_y - y);
		}

		prev_x = x;
		prev_y = y;
		SetCtrlVal(regionScanPanel, ROI_SCAN_X_STEP, x);
		SetCtrlVal(regionScanPanel, ROI_SCAN_Y_STEP, y);
	}

	RegionScan_CheckOverlap();
	
	return 0;
}

int CVICALLBACK cbRegionScanSetOverlap (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			//Check size of overlap. Set to a whole number of pixels
			gOverlapSet = 1;
			RegionScan_CheckOverlap();
			break;
		}
	return 0;
}

static void RegionScan_PlotPosition(double x, double y, double z)
{
	static int ZplotHdl = -1;
	double xMin, xMax, yMin, yMax, zMin, zMax;
	int xDir, yDir, zDir;
	
	GCI_ImagingStageDirectionReversed(&xDir, &yDir, &zDir);
	GCI_ImagingStageGetLimits(&xMin, &xMax, &yMin, &yMax, &zMin, &zMax);
	SetAxisScalingMode (regionScanPanel, ROI_SCAN_SCAN_DISP, VAL_BOTTOM_XAXIS, VAL_MANUAL, xMin, xMax);
	
	if (xDir == 1)
		SetCtrlAttribute (regionScanPanel, ROI_SCAN_SCAN_DISP, ATTR_XREVERSE, 1);  //xMin is to right of xMax
	else
		SetCtrlAttribute (regionScanPanel, ROI_SCAN_SCAN_DISP, ATTR_XREVERSE, 0);  //xMin is to right of xMax
	SetAxisScalingMode (regionScanPanel, ROI_SCAN_SCAN_DISP, VAL_LEFT_YAXIS, VAL_MANUAL, yMin, yMax);
	if (yDir == -1)
		SetCtrlAttribute (regionScanPanel, ROI_SCAN_SCAN_DISP, ATTR_YREVERSE, 1);  //xMin is to right of xMax
	else
		SetCtrlAttribute (regionScanPanel, ROI_SCAN_SCAN_DISP, ATTR_YREVERSE, 0);  //xMin is to right of xMax
	SetGraphCursor (regionScanPanel, ROI_SCAN_SCAN_DISP, 1, x, y);
	
	if (ZplotHdl > 0)
		DeleteGraphPlot (regionScanPanel, ROI_SCAN_Z_DISP, ZplotHdl, VAL_IMMEDIATE_DRAW);
	
	ZplotHdl=-1;
	
	SetAxisScalingMode (regionScanPanel, ROI_SCAN_Z_DISP, VAL_LEFT_YAXIS, VAL_MANUAL, zMin, zMax);
	ZplotHdl = PlotPoint (regionScanPanel, ROI_SCAN_Z_DISP, 0.5, z, VAL_SMALL_SOLID_SQUARE, VAL_RED);
}


static int RegionScan_GotoStageXY(double x, double y)
{
	double z;
	
	region_of_interest_goto_stage_xy(roi, x, y, &z);
	RegionScan_PlotPosition(x, y, z);
	
	return 0;
}


static void RegionScan_PlotROI(double ox, double oy, double gx, double gy)
{
	static int XYplotHdl = -1;
	double xMin, xMax, yMin, yMax, zMin, zMax;
	int xDir, yDir, zDir;
	
	//Plot a rectangle to show the ROI
	GCI_ImagingStageDirectionReversed(&xDir, &yDir, &zDir);
	GCI_ImagingStageGetLimits(&xMin, &xMax, &yMin, &yMax, &zMin, &zMax);

	if (XYplotHdl > 0) DeleteGraphPlot (regionScanPanel, ROI_SCAN_SCAN_DISP, XYplotHdl, VAL_IMMEDIATE_DRAW);
	XYplotHdl=-1;
	SetAxisScalingMode (regionScanPanel, ROI_SCAN_SCAN_DISP, VAL_BOTTOM_XAXIS, VAL_MANUAL, xMin, xMax);
	if (xDir == 1)
		SetCtrlAttribute (regionScanPanel, ROI_SCAN_SCAN_DISP, ATTR_XREVERSE, 1);  //xMin is to right of xMax
	else
		SetCtrlAttribute (regionScanPanel, ROI_SCAN_SCAN_DISP, ATTR_XREVERSE, 0);  //xMin is to right of xMax
	SetAxisScalingMode (regionScanPanel, ROI_SCAN_SCAN_DISP, VAL_LEFT_YAXIS, VAL_MANUAL, yMin, yMax);
	if (yDir == -1)
		SetCtrlAttribute (regionScanPanel, ROI_SCAN_SCAN_DISP, ATTR_YREVERSE, 1);  //xMin is to right of xMax
	else
		SetCtrlAttribute (regionScanPanel, ROI_SCAN_SCAN_DISP, ATTR_YREVERSE, 0);  //xMin is to right of xMax
	XYplotHdl = PlotRectangle (regionScanPanel, ROI_SCAN_SCAN_DISP, ox, oy, ox + gx, oy + gy, VAL_BLUE, VAL_TRANSPARENT);
}

//RJL 220605 - function below added to avoid memory leaks
static void RegionScan_GetImage()
{
	if (image) {
		IPI_Dispose(image);
		image=0;
	}
	image = gci_camera_get_ipi_image(camera, NULL);
}

static void RegionScan_SetROI()
{
	double xStep, yStep, overlap;
	double ox, oy, oz, gx, gy, gz;
	
	//Get/check step details
	RegionScan_UpdateUI();
	
	GetCtrlVal (regionScanPanel, ROI_SCAN_X_STEP, &xStep);
	GetCtrlVal (regionScanPanel, ROI_SCAN_Y_STEP, &yStep);
	GetCtrlVal (regionScanPanel, ROI_SCAN_OVERLAP, &overlap);
	
	if ((overlap >= fabs(xStep)/2) || (overlap >= fabs(yStep)/2)) {
	
		overlap = ceil(xStep/10);
		SetCtrlVal (regionScanPanel, ROI_SCAN_OVERLAP, overlap);
	}
	
	//Get ROI co-ords from panel
	GetCtrlVal(regionScanPanel, ROI_SCAN_X_START, &ox);
	GetCtrlVal(regionScanPanel, ROI_SCAN_Y_START, &oy);
	GetCtrlVal(regionScanPanel, ROI_SCAN_X_LENGTH, &gx);
	GetCtrlVal(regionScanPanel, ROI_SCAN_Y_LENGTH, &gy);

	GetAxisScalingMode (regionScanPanel, ROI_SCAN_Z_DISP, VAL_LEFT_YAXIS, VAL_MANUAL, &oz, &gz);
	gz -= oz;	//range required
	
	//Update display and go to start position
	RegionScan_PlotROI(ox, oy, gx, gy);
	
	//Update stage module
	GCI_ImagingStageSetROI(ox, oy, oz, gx, gy, gz);
	
	//Go to first frame position
	RegionScan_GotoStageXY(ox, oy);
	
	gci_camera_set_snap_mode(camera);
	
	//RJL 220605 - avoid memory leaks
	RegionScan_GetImage();
	//image = gci_camera_get_ipi_image(camera, NULL);
	
	gci_camera_snap_image(camera);
	
	SetCtrlAttribute (regionScanPanel, ROI_SCAN_PREV, ATTR_DIMMED, 0);
	SetCtrlAttribute (regionScanPanel, ROI_SCAN_NEXT, ATTR_DIMMED, 0);
	gRow=0;
	gCol=0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Go to next or previous frame

static int RegionScan_GotoNextFrame(double x, double y)
{
	double t1, t2;
	
	t1=clock();
	if (RegionScan_GotoStageXY(x, y)) return -1;
	gThisFrame ++;
	SetCtrlVal (regionScanPanel, ROI_SCAN_FRAME, gThisFrame);
	t2 = clock() - t1;
	//printf("XY move %d ms\n", (int)t2);
	return 0;
}

//RJL 29 June 2005 - Corrections made to the next 3 functions

static void FrameSetup(double *xStep, double *yStep, int *stepsX, int *stepsY)
{
	double overlap, xLength, yLength;
	int nFrames;
	
	//Determine frame size and number of frames for ROI
	GetCtrlVal (regionScanPanel, ROI_SCAN_X_LENGTH, &xLength);
	GetCtrlVal (regionScanPanel, ROI_SCAN_Y_LENGTH, &yLength);
	GetCtrlVal (regionScanPanel, ROI_SCAN_X_STEP, xStep);
	GetCtrlVal (regionScanPanel, ROI_SCAN_Y_STEP, yStep);
	GetCtrlVal (regionScanPanel, ROI_SCAN_OVERLAP, &overlap);
	*xStep -= overlap;
	*yStep -= overlap;
	*stepsX = Round(xLength/(*xStep)) + 1;
	*stepsY = Round(yLength/(*yStep)) + 1;
	nFrames = (*stepsX) * (*stepsY);
	SetCtrlVal (regionScanPanel, ROI_SCAN_FRAMES, nFrames);
}


static int NextFrame()
{
	double ox, oy, xCoord, yCoord, xStep, yStep;
	int action, stepsX, stepsY, doBackgroundCorrection=0;
	
	GetCtrlVal(regionScanPanel, ROI_SCAN_X_START, &ox);
	GetCtrlVal(regionScanPanel, ROI_SCAN_Y_START, &oy);

	FrameSetup(&xStep, &yStep, &stepsX, &stepsY);
	GetCtrlVal (regionScanPanel, ROI_SCAN_ACTION, &action);

	if (gRow%2) {	//odd row
		gCol --;
		if (gCol < 0) {
			gCol = 0;
			gRow ++;
		}		
	}
	else {
		gCol ++;
		if (gCol >= stepsX) {
			gCol = stepsX-1;
			gRow ++;
		}		
	}
	
	if (gRow >= stepsY) {   //back to the beginning
		gRow = 0;
		gCol = 0;
		gThisFrame = 0;
	}

	yCoord = oy + gRow*yStep;
	xCoord = ox + gCol*xStep;
	
	if (RegionScan_GotoNextFrame(xCoord, yCoord))
		return -1;	//error
	
	if (action > 0) {
		GetCtrlVal (regionScanPanel, ROI_SCAN_DO_CORRECTIONS, &doBackgroundCorrection);
		doBackgroundCorrection &= GCI_RefImagesOK();
		SetCtrlVal (regionScanPanel, ROI_SCAN_DO_CORRECTIONS, doBackgroundCorrection);
		
		RegionScan_GetImage();
		if (doBackgroundCorrection) 
			GCI_RefImagesProcess(image, 1);
		else					
			gci_camera_display_ipi_image(camera, image, NULL);
	}	
	return 0;		
}


static int PreviousFrame()
{
	double ox, oy, xCoord, yCoord, xStep, yStep;
	int action, stepsX, stepsY, doBackgroundCorrection;
	
	GetCtrlVal(regionScanPanel, ROI_SCAN_X_START, &ox);
	GetCtrlVal(regionScanPanel, ROI_SCAN_Y_START, &oy);

	FrameSetup(&xStep, &yStep, &stepsX, &stepsY);
	GetCtrlVal (regionScanPanel, ROI_SCAN_ACTION, &action);
	//if (action > 1) GetFname();	
	
	if (gRow%2) {	//odd row
		
		gCol ++;
		
		if (gCol >= stepsX) {
			gCol = stepsX-1;
			gRow --;
		}		
	}
	else {
		
		gCol --;
		
		if (gCol < 0) {
			gCol = 0;
			gRow --;
		}		
	}
	
	if (gRow < 0) {   //back at the beginning, stay there
	
		gRow = 0;
		gCol = 0;
		gThisFrame = 0;
	
		return 0;
	}

	yCoord = oy + gRow*yStep;
	xCoord = ox + gCol*xStep;
	gThisFrame -= 2;	//because GotoNextFrame adds one to it
	
	if (RegionScan_GotoNextFrame(xCoord, yCoord))
		return -1;	//error
	
	if (action > 0) {
		GetCtrlVal (regionScanPanel, ROI_SCAN_DO_CORRECTIONS, &doBackgroundCorrection);
		doBackgroundCorrection &= GCI_RefImagesOK();
		SetCtrlVal (regionScanPanel, ROI_SCAN_DO_CORRECTIONS, doBackgroundCorrection);
		
		RegionScan_GetImage();
		if (doBackgroundCorrection) 
			GCI_RefImagesProcess(image, 1);
		else					
			gci_camera_display_ipi_image(camera, image, NULL);
	}	
	
	return 0;		
}

/////////////////////////////////////////////////////////////////////////////
//Mosaic routines

static void RegionScanCreateMosaicHeader()
{
	double ox, oy, gx, gy, overlap, umPerPixel=1.0;
	char hname[MAX_PATHNAME_LEN], ext[6]="";
	FILE *fh;
	int dot, binning;
	//GciOrcaCamera *orca_camera = (GciOrcaCamera *) camera;

	//Create header file for the image stitching application
	gci_camera_save_state(camera);

	umPerPixel = gci_camera_get_microns_per_pixel(camera);
	binning = gci_camera_get_binning_mode(camera);
	if (binning < 1) binning = 1;	//avoid crashing if binning not implemented for camera

	dot = FindPattern (gFilename, 0, -1, ".", 0, 1);
	
	if (dot == -1)
		dot = strlen(gFilename);
	else
		dot++;
		
	CopyString (ext, 0, gFilename, dot, strlen(gFilename)-dot); //keep extension

	strncpy(hname, gFilename, dot);
	hname[dot] = '\0';	   //Doesn't work without this
	strcat(hname, "seq");
	fh = fopen (hname, "w");
	
	//Store stage coords of the region.
	GetCtrlVal(regionScanPanel, ROI_SCAN_X_START, &ox);
	GetCtrlVal(regionScanPanel, ROI_SCAN_Y_START, &oy);
	GetCtrlVal(regionScanPanel, ROI_SCAN_X_LENGTH, &gx);
	GetCtrlVal(regionScanPanel, ROI_SCAN_Y_LENGTH, &gy);
	fprintf(fh, "%.2f\t%.2f\t%.2f\t%.2f\n", ox, ox, ox+gx, oy+gy);
	
	GetCtrlVal (regionScanPanel, ROI_SCAN_OVERLAP, &overlap);
	fprintf(fh, "%.2f\n", overlap);									  //overlap in um
	fprintf(fh, "%d\t%d\n", gFramesX, gFramesY);					  //no. frames
	fprintf(fh, "%.2f\n", 1.0 / umPerPixel / binning);		  //pixels per micron
	fprintf(fh, ".%s\n", ext);  									  //image file extension
	fclose(fh);
}


/////////////////////////////////////////////////////////////////////////////
//The nitty-gritty region scan routines

static void GetMyDocumentsUserFolder(char *path)
{
	str_get_path_for_my_documents(path);

	strcat(path, "\\GCI_MicroscopeData");
	if (!FileExists(path, 0)) MakeDir (path);
}
static void GetFname(char *filename)
{
	IPIImageInfo info;
	UInt64Type total_bytes, free_bytes;
	double disk_space, free_space;
	char path[500];
	
	//Calculate approximate required disk space and check there is enough free space
	IPI_GetImageInfo (image, &info);
	
	if (info.pixelType == IPI_PIXEL_RGB32)
		info.pixelSpace = Round(0.75 * info.pixelSpace);	//32 -> 24 bit
	
	disk_space = gFramesX * gFramesY * (info.pixelSpace + 2000);	//bytes
	GetDiskSpace ("C:", &total_bytes, &free_bytes);  //64 bit variables
	free_space = (double)free_bytes.hiBytes * 4294967296.0 + (double)free_bytes.loBytes;

	if (free_space < (disk_space + 50)) {
	
		MessagePopup("Problem", "There's not enough disk space for this acquisition.");
		gSaveImages = 0;
		return;
	}

	gSaveImages = 1;
	
	GetMyDocumentsUserFolder(path);
	
	//This function brings up the file selection dialog. 
	//If the files already exist they are deleted if the user opts to overwrite them..
	if (GCI_SetFilename(path, filename)) {
		
		gSaveImages = 0;
		return;
	}
	
	strcpy (gFilename, filename);
}


static void RegionScanStop()
{
   	if(gPauseScan == 1) {	//scan is paused
   	
   		gPauseScan = 0;
		SetCtrlAttribute (regionScanPanel, ROI_SCAN_PAUSE, ATTR_LABEL_TEXT, "Pause Scan");
	}    		
	
   	gStopScan = 1;
	SetCtrlAttribute (regionScanPanel, ROI_SCAN_PAUSE, ATTR_DIMMED, 1);
	SetCtrlAttribute (regionScanPanel, ROI_SCAN_STOP, ATTR_DIMMED, 1);
}

static int RegionScanSaveImage(char *filename)
{
	char sfname[MAX_PATHNAME_LEN]="";
	
	//RJL 230605 - the image was being re-acquired here. Fixed it.
	
	//Save image in a file with the correct sequence number e.g. MyImage_00045.ics
	GCI_NextFname(filename, gThisFrame, sfname);
	
	if (GCI_Imaging_SaveIPIImage(image, sfname) < 0) return -1;
	
	return 0;
}

/*
static int RegionScanSaveImage(char *filename)
{
	char sfname[MAX_PATHNAME_LEN]="";
	FIBITMAP *dib;
	
	//Save image in a file with the correct sequence number e.g. MyImage_00045.ics
	GCI_NextFname(filename, gThisFrame, sfname);
	
	dib = gci_camera_get_image(camera, NULL);
	
	if( GCI_Imaging_SaveImage(dib, sfname) < 0)
		return -1;
	
	FreeImage_Unload(dib);
	
	return 0;
}
*/

static void CreateMosaic()
{
	int image_width, image_height, tileW, tileH, x, y;
	double maxW=500.0, maxH=500.0, h, w;
	double xStep, yStep, overlap, overlapPctX, overlapPctY, imAspectRatio;
	int monitors, screenW, screenH, top, left;
	
	//Set mosaic image size with correct aspect ratio
	gci_camera_get_size(camera, &image_width, &image_height);  

	GetCtrlVal (regionScanPanel, ROI_SCAN_X_STEP, &xStep);
	GetCtrlVal (regionScanPanel, ROI_SCAN_Y_STEP, &yStep);
	GetCtrlVal (regionScanPanel, ROI_SCAN_OVERLAP, &overlap);

	overlapPctX = overlap/xStep*100;
	overlapPctY = overlap/yStep*100;
	
	x = gFramesX;	//save typing
	y = gFramesY;
	
	imAspectRatio = (double)image_width/image_height;
	tileW = Round(maxW/x);
	tileH = Round(tileW/imAspectRatio);
	
	if (tileH*y > maxH) {
		tileH = Round(maxH / y);
		tileW = Round(tileH * imAspectRatio);
	}
	
	w = tileW*x;
	h = tileH*y;
	w -= Round((x-1) * (double)tileW*overlapPctX/100.0);
	h -= Round((y-1) * (double)tileH*overlapPctY/100.0);

	//Display the mosaic window beside the main image window
	
	GetScreenSize (&screenH, &screenW);
	top = screenH - h - 5;
	left = screenW - w - 5;
	
	GetSystemAttribute (ATTR_NUM_MONITORS, &monitors);
	if (monitors > 1) 
		left += screenW;
		//top += screenH;
	
	if (mosaic_window != NULL) {
		mosaic_window_hide(mosaic_window);
		//mosaic_window_destroy(mosaic_window);
		//mosaic_window = NULL;
	}
	
	if (mosaic_window == NULL) {
		mosaic_window = mosaic_window_new(left, top, w, h); 
		mouse_down_handler = GCI_ImagingWindow_SetMouseDownHandler (mosaic_window->window, OnMosaicClicked , NULL);    
	}
	
	mosaic_window_set_row_and_col_size(mosaic_window, x, y);
	mosaic_window_clear(mosaic_window);
	mosaic_window_show(mosaic_window);
	mosaic_window_set_overlap(mosaic_window, overlapPctX, overlapPctY);
}

static int RegionScanDoScan()
{
	double x, y, xStep, yStep, overlap, overlapPctX, overlapPctY;
	double ox, oy, gx, gy;
	double t1;
	int action, nFrames, doBackgroundCorrection=0;
	char filename[MAX_PATHNAME_LEN];
	IPIImageInfo info;  
	
	GetCtrlVal (regionScanPanel, ROI_SCAN_X_STEP, &xStep);
	GetCtrlVal (regionScanPanel, ROI_SCAN_Y_STEP, &yStep);
	GetCtrlVal (regionScanPanel, ROI_SCAN_OVERLAP, &overlap);
	
	GetCtrlVal(regionScanPanel, ROI_SCAN_X_START, &ox);
	GetCtrlVal(regionScanPanel, ROI_SCAN_Y_START, &oy);
	GetCtrlVal(regionScanPanel, ROI_SCAN_X_LENGTH, &gx);
	GetCtrlVal(regionScanPanel, ROI_SCAN_Y_LENGTH, &gy);

	//Overlap as percentage of step size is needed for image mosaic creation
	overlapPctX = overlap/xStep*100;
	overlapPctY = overlap/yStep*100;

	xStep -= overlap;
	yStep -= overlap;
	gFramesX = Round(gx/xStep) + 1;
	gFramesY = Round(gy/yStep) + 1;
	nFrames = gFramesX * gFramesY;
	gThisFrame = 0;
	SetCtrlVal (regionScanPanel, ROI_SCAN_FRAMES, nFrames);
	SetCtrlVal (regionScanPanel, ROI_SCAN_FRAME, gThisFrame);
	
	IPI_GetImageInfo (image, &info);
	
	CreateMosaic();
	
	GetCtrlVal (regionScanPanel, ROI_SCAN_DO_CORRECTIONS, &doBackgroundCorrection);
	doBackgroundCorrection &= GCI_RefImagesOK();
	SetCtrlVal (regionScanPanel, ROI_SCAN_DO_CORRECTIONS, doBackgroundCorrection);
	
	//Go to initial coords
	RegionScan_GotoStageXY(ox, oy);
	x = ox;
	y = oy;
	
	GetCtrlVal (regionScanPanel, ROI_SCAN_ACTION, &action);
	
	if (action > 1)
		GetFname(filename);  //Want to save images so ask for a filename
	else
		gSaveImages = 0;
		
	if (action > 0) {  //want to acquire images
	
		gci_camera_set_snap_sequence_mode(camera);
	
		IPI_Dispose(image);
	
		gci_camera_snap_image(camera);
	}
	
	t1 = Timer();

	//Turn joystick and focus indicator off to speed things up a bit
	GCI_ImagingStageJoystickOff();
	
	for (gRow=0; gRow<gFramesY; gRow++) {
	
		ProcessSystemEvents();
		SetCtrlVal (regionScanPanel, ROI_SCAN_TIME_TAKEN, Timer() - t1);
		
		if (gStopScan)
			goto End;
			
		if (gPauseScan) {
			
			gRow--;
			continue;
		}
		
		y = oy + gRow*yStep;
		
		if (gRow%2) {	//Odd row
		
			for (gCol=gFramesX-1; gCol>=0; gCol--) {
			
				ProcessSystemEvents();
				
				if (gStopScan)
					goto End;
				
				if (gPauseScan) 
				{
					gCol++;
					continue;
				}
				
				x = ox + gCol*xStep;
				
				if (RegionScan_GotoNextFrame(x, y))
					goto End;	//error
				
				if (action > 0) {
				
					RegionScan_GetImage();

					if (doBackgroundCorrection) 
						GCI_RefImagesProcess(image, 0);
					
					GCI_CorrectDistortion(image, image);

					gci_camera_display_ipi_image(camera, image, NULL);
					
					mosaic_window_add_image(mosaic_window, image, gCol, gRow);
					
					if (gSaveImages)
						RegionScanSaveImage(filename);
				}	
			}
		}	
		else {		//Even row
			
			for (gCol=0; gCol<gFramesX; gCol++) {
			
				ProcessSystemEvents();
				
				if (gStopScan)
					goto End;
				
				if (gPauseScan)
				{
					gCol--;
					continue;
				}
				
				x = ox + gCol*xStep;
				
				if (RegionScan_GotoNextFrame(x, y))
					goto End;	//error
				
				if (action > 0) {
					
					RegionScan_GetImage();

					if (doBackgroundCorrection) 
						GCI_RefImagesProcess(image, 0);
					
					GCI_CorrectDistortion(image, image);
	
					gci_camera_display_ipi_image(camera, image, NULL);
					
					mosaic_window_add_image(mosaic_window, image, gCol, gRow); 
		
					if (gSaveImages)
						RegionScanSaveImage(filename);
				}	
			}
		}
		
		IPI_Dispose(image);
	}
	
	gci_camera_set_snap_mode(camera);

End:	
	
	if (gSaveImages)
		RegionScanCreateMosaicHeader();

	SetCtrlVal (regionScanPanel, ROI_SCAN_TIME_TAKEN, Timer() - t1);
	
	//Turn joystick and focus indicator on
	GCI_ImagingStageJoystickOn();
	
	return 0;
}


static int RegionScanDisplayImage(IPIImageRef image)
{
	if (image == NULL) 
		return -1;

	gci_camera_display_ipi_image(camera, image, "");
	
	GCI_ImagingWindow_Show(image_window);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//Multi-threaded region scan functions
//Notes:
//1. The default thread pool can only have 2 + (nProcessors + 2) * 2 = 6 threads
//2. Without the calls to ProcessSystemEvents() it still works but much more slowly
//
//Rules:
//1. Each function waits for one or more flags
//2. When all conditions to proceed are satisfied it clears these flags as soon as it can
//3. It performs its function and then sets its own flag
//4. No two functions can wait for the same flag
//5. All loops must have a get-out
//6. If the function is unsuccessful, careful thought is required as to whether to set the flag
//7. In general allow three attempts for each process to succeed before giving up on it and ending it all

int CVICALLBACK MT_ScanXYZ(void *dummy)
{
	double xCoord, yCoord, xStep, yStep, overlap, t1;
	int action, attempts;
	
	//Thread controlling the xyz stage. 
	//Does a stage move as soon as the previous acquisition is complete.
	
	xCoord = gOx;
	yCoord = gOy;
	GetCtrlVal (regionScanPanel, ROI_SCAN_X_STEP, &xStep);
	GetCtrlVal (regionScanPanel, ROI_SCAN_Y_STEP, &yStep);
	GetCtrlVal (regionScanPanel, ROI_SCAN_OVERLAP, &overlap);
	xStep -= overlap;
	yStep -= overlap;

	GetCtrlVal (regionScanPanel, ROI_SCAN_ACTION, &action);

	gMT_exposureComplete1=1;  //otherwise we never start
	gRow = 0;
	gCol = -1;
	
	while (1) {
	
		ProcessSystemEvents();
		
		if (gStopScan)
			return 0;	//User has pressed Abort
			
		if (gPauseScan)
			continue;	//User has pressed Pause
		
		//Calculate next stage position
		
		if (gRow%2) {	//Odd row
		
			gCol--;
			
			if (gCol < 0) {
				gCol = 0;
				gRow ++;
			}
		}
		else {
		
			gCol++;
			
			if (gCol >= gFramesX) {
			
				gCol = gFramesX-1;
				gRow ++;
			}
		}
		
		if (gRow >= gFramesY) {
		
			gMT_scanComplete = 1;
			return (0);   //Finished
		}
		
		yCoord = gOy + gRow*yStep;
		xCoord = gOx + gCol*xStep;

		//Wait for previous acquisition to complete
		if (action > 0) { //i.e. we are acquiring images
		
			while (!gMT_exposureComplete1) {
			
				ProcessSystemEvents();
				
				if (gStopScan)
					return 0;	//User has pressed Abort
					
				if (gPauseScan)
					continue;	//User has pressed Pause
			}
		}

		gMT_exposureComplete1 = 0;

		#ifdef TESTMODE
			printf("starting_xyz move = 1\n");
		#endif

		t1 = Timer();

		//Allow three attempts at sucessful stage move
		attempts = 0;
		
		while (1) {
		
			attempts ++;
			
			if (attempts > 3) {
			
				gStopScan = 1;	  //Tell all the other threads to stop
				return 0;
			}
			
			if (RegionScan_GotoStageXY(xCoord, yCoord))
				continue;
				
			break;	//success
		}
		
		gMT_xyzComplete = 1;
		
		#ifdef TESTMODE
			printf("gMT_xyzComplete = 1\n");
		#endif

		#ifdef TESTMODE2
		//printf("xyz move took %f\n", Timer()-t1);
		gTxyz += Timer()-t1;
		#endif

	}
}


int CVICALLBACK MT_ScanExposure(void *dummy)
{
	int attempts, count=0;
	double t1;
	
	//Thread controlling the shutter and camera exposure. 
	//It's both difficult and pointless to separate these
	
	gMT_displayComplete=1;   //otherwise we never start
	gMT_saveComplete=1;
	gMT_mosaicComplete=1;

	while (1) {
	
		ProcessSystemEvents();
		
		if (gStopScan)
			return 0;		//User has pressed Abort

		//Wait for completion of stage move and processing of previous image
		while (!gMT_xyzComplete || !gMT_displayComplete || !gMT_saveComplete || !gMT_mosaicComplete) {
			
			ProcessSystemEvents();
			if (gStopScan)
				return 0;		//User has pressed Abort
		}
	
		gMT_xyzComplete=0;
		gMT_displayComplete=0;
		gMT_saveComplete=0;
		gMT_mosaicComplete=0;

		t1 = Timer();
		
		//Allow three attempts at sucessful acquisition
		attempts = 0;
		
		//RJL 29 June 05 - line below added
		CmtGetLock (lock);

		while (1) {
		
			attempts ++;
			
			if (attempts > 3) {
				gStopScan = 1;	  //Tell all the other threads to stop
				//RJL 29 June 05 - line below added
				CmtReleaseLock (lock);
				return 0;
			}
			
			//RJL 220605 - avoid memory leaks
			RegionScan_GetImage();
			//image = gci_camera_get_ipi_image(camera, NULL);
			
			if (!image)
				continue; 
		
			break;	//success
		}
		//RJL 29 June 05 - line below added
		CmtReleaseLock (lock);
		
		gMT_exposureComplete1 = 1;	  //Tell xy thread
		gMT_exposureComplete2 = 1;	  //Tell make image thread
		
		#ifdef TESTMODE
		printf("gMT_exposureComplete = 1\n");
		#endif

		#ifdef TESTMODE2
		//printf("acquisition took %f\n", Timer()-t1);
		gTacq += Timer()-t1;
		#endif

		count ++;
		
		if (count >= gFramesX*gFramesY) {
			return 0;	//Finished
		}
	}
}

int CVICALLBACK MT_ScanMakeIm(void *dummy)
{
	int doBackgroundCorrection=0, count = 0;
	double t1;
	
	//Thread controlling the image creation and processing. 
	
	GetCtrlVal (regionScanPanel, ROI_SCAN_DO_CORRECTIONS, &doBackgroundCorrection);
	doBackgroundCorrection &= GCI_RefImagesOK();
	SetCtrlVal (regionScanPanel, ROI_SCAN_DO_CORRECTIONS, doBackgroundCorrection);
	
	while (1) {
	
		ProcessSystemEvents();
		
		if (gStopScan)
			return 0;		//User has pressed Abort
		
		//Wait for completion of data transfer
		while (!gMT_exposureComplete2) {
		
			ProcessSystemEvents();
			
			if (gStopScan)
				return 0;		//User has pressed Abort
		}
		
		gMT_exposureComplete2 = 0;
		
		t1 = Timer();

		//RJL 29 June 05 - line below added
		CmtGetLock (lock);

		if (doBackgroundCorrection)
			GCI_RefImagesProcess(image, 0);
		
		GCI_CorrectDistortion(image, image);
		
		//RJL June 05 - line below added
		CmtReleaseLock (lock);

		gMT_makeImageComplete1=1;  //Tell display thread
		gMT_makeImageComplete2=1;  //Tell save thread
		gMT_makeImageComplete3=1;  //Tell mosaic thread
		
		#ifdef TESTMODE
		printf("gMT_makeImageComplete = 1\n");
		#endif

		#ifdef TESTMODE2
		//printf("processing took %f\n", Timer()-t1);
		gTbck += Timer()-t1;
		#endif

		count ++;
		if (count >= gFramesX*gFramesY) {
			return 0;	//Finished
		}
	}
}

int CVICALLBACK MT_ScanDisplayIm(void *dummy)
{
	int count = 0;
	
	while (1) {
	
		ProcessSystemEvents();
		
		if (gStopScan)
			return 0;		//User has pressed Abort
		
		//Wait for image creation
		while (!gMT_makeImageComplete1) {
		
			ProcessSystemEvents();
			
			if (gStopScan)
				return 0;		//User has pressed Abort
		}
		
		gMT_makeImageComplete1 = 0;
		
		//RJL 29 June 05 - line below added
		CmtGetLock (lock);
		RegionScanDisplayImage(image);
		//RJL June 05 - line below added
		CmtReleaseLock (lock);
		
		gMT_displayComplete=1;
		#ifdef TESTMODE
		printf("gMT_displayComplete = 1\n");
		#endif

		count ++;
		if (count >= gFramesX*gFramesY) {
			return 0;	//Finished
		}
	}
}


int CVICALLBACK MT_ScanSaveIm(void *dummy)
{
	int attempts, count=0;
	double t1;
	
	while (1) {
	
		ProcessSystemEvents();
		
		if (gStopScan)
			return 0;		//User has pressed Abort
		
		//Wait for image creation
		while (!gMT_makeImageComplete2) {
		
			ProcessSystemEvents();
			
			if (gStopScan)
				return 0;		//User has pressed Abort
		}

		gMT_makeImageComplete2 = 0;
		
		gThisFrame ++;
		SetCtrlVal (regionScanPanel, ROI_SCAN_FRAME, gThisFrame);

		t1 = Timer();
		
		if (gSaveImages) {
		
			//Allow three attempts at sucessful image creation
			attempts = 0;
			
			//RJL 29 June 05 - line below added
			CmtGetLock (lock);
			while (1) {
			
				if (RegionScanSaveImage(gFilename) == 0)
					break;	//success

				attempts ++;
				
				if (attempts > 3) {
				
					gStopScan = 1;	  //Tell all the other threads to stop
					//RJL June 05 - line below added
					CmtReleaseLock (lock);
					return 0;
				}
			}
			//RJL June 05 - line below added
			CmtReleaseLock (lock);
		}
			
		gMT_saveComplete=1;
		
		#ifdef TESTMODE
		printf("gMT_saveComplete = 1\n");
		#endif

		#ifdef TESTMODE2
		//printf("save took %f\n", Timer()-t1);
		gTsave += Timer()-t1;
		#endif

		count ++;
		
		if (count >= gFramesX*gFramesY) {
			return 0;	//Finished
		}
	}
}


int CVICALLBACK MT_ScanMosaic(void *dummy)
{
	double xStep, yStep, overlap, overlapPctX, overlapPctY, t1;
	int row=0, col=-1, count=0;
	
	GetCtrlVal (regionScanPanel, ROI_SCAN_X_STEP, &xStep);
	GetCtrlVal (regionScanPanel, ROI_SCAN_Y_STEP, &yStep);
	GetCtrlVal (regionScanPanel, ROI_SCAN_OVERLAP, &overlap);
	
	//Overlap as percentage of x step size is needed for image mosaic creation
	overlapPctX = overlap/xStep*100;
	overlapPctY = overlap/yStep*100;

	while (1) {
	
		ProcessSystemEvents();
		
		if (gStopScan)
			return 0;		//User has pressed Abort
		
		//Wait for image creation
		while (!gMT_makeImageComplete3) {
		
			ProcessSystemEvents();
			
			if (gStopScan)
				return 0;		//User has pressed Abort
		}

		gMT_makeImageComplete3 = 0;
		
		t1 = Timer();
		
		if (row%2) {	//Odd row
		
			col--;
			
			if (col < 0) {
				col = 0;
				row ++;
			}
		}
		else {
		
			col++;
			
			if (col >= gFramesX) {
			
				col = gFramesX-1;
				row ++;
			}
		}
		
		//RJL 29 June 05 - line below added
		CmtGetLock (lock);
	
		mosaic_window_add_image(mosaic_window, image, col, row); 
		//RJL June 05 - line below added
		CmtReleaseLock (lock);

		gMT_mosaicComplete=1;
		
		#ifdef TESTMODE
		printf("gMT_mosaicComplete = 1\n");
		#endif

		#ifdef TESTMODE2
		//printf("mosaic took %f\n", Timer()-t1);
		gTmos += Timer()-t1;
		#endif

		count ++;
		
		if (count >= gFramesX*gFramesY) {
			return 0;	//Finished
		}
	}
}

//RJL 29 June 05 - function below added
static void EndThread(int threadPool, int funcID)
{
	int threadStatus;
	double t1;
	
	//All threads should be finished within a second of the first one.
	//If not kill it.
	t1 = Timer();
	while (1) {
		CmtGetThreadPoolFunctionAttribute (threadPool, funcID, ATTR_TP_FUNCTION_EXECUTION_STATUS, &threadStatus);
		if (threadStatus > 3) break; //thread finished
		//printf("thread status %d\n", threadStatus);
		if (Timer() - t1 > 1.0) {
			CmtTerminateThreadPoolThread (threadPool, funcID, 0);
			//printf("Terminating thread %d\n", funcID);
			break;
		}
	}
}

static void	RegionScanDoMultithreadScan()
{
	//IPIImageInfo info;
	int myThreadPool;
	double xStep, yStep, overlap, overlapPctX, overlapPctY, t1, t2;
	int nFrames, action;
	int scanXYfuncID, scanExpfuncID, scanMakeImfuncID;
	int	scanSaveImfuncID, scanMosaicfuncID;
	int count = 0;
	
	GetCtrlVal (regionScanPanel, ROI_SCAN_X_STEP, &xStep);
	GetCtrlVal (regionScanPanel, ROI_SCAN_Y_STEP, &yStep);
	GetCtrlVal (regionScanPanel, ROI_SCAN_OVERLAP, &overlap);
	
	GetCtrlVal(regionScanPanel, ROI_SCAN_X_START, &gOx);
	GetCtrlVal(regionScanPanel, ROI_SCAN_Y_START, &gOy);
	GetCtrlVal(regionScanPanel, ROI_SCAN_X_LENGTH, &gGx);
	GetCtrlVal(regionScanPanel, ROI_SCAN_Y_LENGTH, &gGy);

	//Overlap as percentage of step size is needed for image mosaic creation
	overlapPctX = overlap/xStep*100;
	overlapPctY = overlap/yStep*100;

	xStep -= overlap;
	yStep -= overlap;
	gFramesX = Round(gGx/xStep) + 1;
	gFramesY = Round(gGy/yStep) + 1;
	nFrames = gFramesX * gFramesY;
	gThisFrame = 0;
	SetCtrlVal (regionScanPanel, ROI_SCAN_FRAMES, nFrames);
	SetCtrlVal (regionScanPanel, ROI_SCAN_FRAME, gThisFrame);
	
	CreateMosaic();
	
	//Go to initial coords
	RegionScan_GotoStageXY(gOx, gOy);
	
	GetCtrlVal (regionScanPanel, ROI_SCAN_ACTION, &action);
	
	if (action > 1)
		GetFname(gFilename);  //Want to save images so ask for a filename
	else
		gSaveImages = 0;
	
	t1 = Timer();
	
	if (action > 0)   //want to acquire images
		RegionScan_GetImage();
	
	//discourage der fingen-pokken

	//Turn joystick and focus indicator off to speed things up a bit
	GCI_ImagingStageJoystickOff();
	GCI_FocusOnOff(0);
	
	gStopScan = 0;
	gMT_scanComplete = 0;
	gMT_xyzComplete = 0;
	gMT_exposureComplete1 = 0;
	gMT_exposureComplete2 = 0;
	gMT_makeImageComplete1=0;
	gMT_makeImageComplete2=0;
	gMT_makeImageComplete3=0;

	#ifdef TESTMODE2
	//Initialise timers
	gTxyz = 0.0;
	gTacq = 0.0;
	gTbck = 0.0;
	gTdisp = 0.0;
	gTsave = 0.0;
	gTmos = 0.0;
	gTmosDisp = 0.0;
	#endif
	
	GCI_ImagingStopAllTimers();
	GCI_ImagingStageDisableTimer();
	//GCI_ElectronBeamSource_StopTimer();
	
	#ifdef TESTMODE
	printf ("***************************************\n");
	#endif

	if (lock < 0) CmtNewLock (NULL, 0, &lock);   //for multi-threading

	CmtNewThreadPool (12, &myThreadPool);
	CmtScheduleThreadPoolFunction (myThreadPool, MT_ScanXYZ, 0, &scanXYfuncID);
	
	if (action > 0) {
	
		CmtScheduleThreadPoolFunction (myThreadPool, MT_ScanExposure, 0, &scanExpfuncID);
		CmtScheduleThreadPoolFunction (myThreadPool, MT_ScanMakeIm, 0, &scanMakeImfuncID);
		//CmtScheduleThreadPoolFunction (myThreadPool, MT_ScanDisplayIm, 0, &scanDispImfuncID);
		CmtScheduleThreadPoolFunction (myThreadPool, MT_ScanSaveIm, 0, &scanSaveImfuncID);
		CmtScheduleThreadPoolFunction (myThreadPool, MT_ScanMosaic, 0, &scanMosaicfuncID);
	
		//Carry out image display in the main thread
		while (1) {
			ProcessSystemEvents();
			if (gStopScan)
				break;		//User has pressed Abort
		
			//Wait for image creation
			while (!gMT_makeImageComplete1) {
				ProcessSystemEvents();
				if (gStopScan)
					break;		//User has pressed Abort
			}
		
			gMT_makeImageComplete1 = 0;
		
			t2 = Timer();
			
			gci_camera_display_ipi_image(camera, image, NULL);
		
			gMT_displayComplete=1;
			#ifdef TESTMODE
			printf("gMT_displayComplete = 1\n");
			#endif

			#ifdef TESTMODE2
			//printf("display took %f\n", Timer()-t2);
			gTdisp += Timer()-t2;
			#endif
			
			//Wait for mosaic creation
			while (!gMT_mosaicComplete) {
				ProcessSystemEvents();
				if (gStopScan)
					break;		//User has pressed Abort
			}
		
			t2 = Timer();

			mosaic_window_update(mosaic_window);

			#ifdef TESTMODE2
			//printf("Mos display took %f\n", Timer()-t2);
			gTmosDisp += Timer()-t2;
			#endif
			
			count ++;
			if (count >= gFramesX*gFramesY) {
				break;	//Finished
			}
		
			SetCtrlVal (regionScanPanel, ROI_SCAN_TIME_TAKEN, Timer() - t1);
		}
	}
	
	CmtWaitForThreadPoolFunctionCompletion (myThreadPool, scanXYfuncID, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	
	if (action > 0) {
		//CmtWaitForThreadPoolFunctionCompletion() can wait forever, so we don't use it.
		EndThread(myThreadPool, scanExpfuncID);
		EndThread(myThreadPool, scanMakeImfuncID);
		//EndThread(myThreadPool, scanDispImfuncID);
		EndThread(myThreadPool, scanSaveImfuncID);
		EndThread(myThreadPool, scanMosaicfuncID);
		
		//CmtWaitForThreadPoolFunctionCompletion (myThreadPool, scanExpfuncID, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
		//CmtWaitForThreadPoolFunctionCompletion (myThreadPool, scanMakeImfuncID, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
		//CmtWaitForThreadPoolFunctionCompletion (myThreadPool, scanDispImfuncID, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
		//CmtWaitForThreadPoolFunctionCompletion (myThreadPool, scanSaveImfuncID, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
		//CmtWaitForThreadPoolFunctionCompletion (myThreadPool, scanMosaicfuncID, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	}
	
	CmtReleaseThreadPoolFunctionID (myThreadPool, scanXYfuncID);
	
	if (action > 0) {
	
		CmtReleaseThreadPoolFunctionID (myThreadPool, scanExpfuncID);
		CmtReleaseThreadPoolFunctionID (myThreadPool, scanMakeImfuncID);
		//CmtReleaseThreadPoolFunctionID (myThreadPool, scanDispImfuncID);
		CmtReleaseThreadPoolFunctionID (myThreadPool, scanSaveImfuncID);
		CmtReleaseThreadPoolFunctionID (myThreadPool, scanMosaicfuncID);
	}
	
	CmtDiscardThreadPool (myThreadPool);
    CmtDiscardLock (lock);
	lock = -1;
	
	#ifdef TESTMODE2
	printf("xyz %f s\n", gTxyz);
	printf("acq %f s\n", gTacq);
	printf("bck %f s\n", gTbck);
	printf("disp %f s\n", gTdisp);
	printf("save %f s\n", gTsave);
	printf("mos %f s\n", gTmos);
	printf("mosDisp %f s\n", gTmosDisp);
	printf("Total %f s\n\n", Timer()-t1);
	#endif

	GCI_ImagingStartAllTimers();
	GCI_ImagingStageEnableTimer();
	//GCI_ElectronBeamSource_StartTimer();

	if (gSaveImages)
		RegionScanCreateMosaicHeader();

	SetCtrlVal (regionScanPanel, ROI_SCAN_TIME_TAKEN, Timer() - t1);

	//Turn joystick and focus indicator on
	GCI_ImagingStageJoystickOn();
	GCI_FocusOnOff(1);
}


/////////////////////////////////////////////////////////////////////////////
int CVICALLBACK cbRegionScanStart (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
		
			gci_camera_set_snap_mode(camera);
			
			GCI_ImagingStageJoystickOff();
			//RJL 050505 - During Region scan in Fluor mode when the shutter is busy 
			//avoid occasional spurious error messages
			//GCI_ReportShutterMessages(0);

			RegionScan_SetROI();
			
		   	gStopScan = 0;
			SetCtrlAttribute (regionScanPanel, ROI_SCAN_PAUSE, ATTR_DIMMED, 0);
			SetCtrlAttribute (regionScanPanel, ROI_SCAN_STOP, ATTR_DIMMED, 0);
			SetCtrlAttribute (regionScanPanel, ROI_SCAN_START, ATTR_DIMMED, 1);

			RegionScan_DisableControls(1);
			GCI_ImagingDisableAllPanels(1);
			//GCI_ElectronBeamSourceDisableAllPanels(1);
			//GCI_GPscope_DisablePanel(1);

	   		#ifdef MULTITHREAD
			RegionScanDoMultithreadScan();
			#else
			RegionScanDoScan();
			#endif
			
			GCI_ImagingStageJoystickOn();
			//GCI_ReportShutterMessages(1);
			SetCtrlAttribute (regionScanPanel, ROI_SCAN_PAUSE, ATTR_DIMMED, 1);
			SetCtrlAttribute (regionScanPanel, ROI_SCAN_STOP, ATTR_DIMMED, 1);
			SetCtrlAttribute (regionScanPanel, ROI_SCAN_START, ATTR_DIMMED, 0);
			
			GCI_ImagingDisableAllPanels(0);
			//GCI_ElectronBeamSourceDisableAllPanels(0);
			//GCI_GPscope_DisablePanel(0);
			RegionScan_DisableControls(0);

			break;
		}
	return 0;
}

int CVICALLBACK cbRegionScanPause (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
		
	    	if (gPauseScan == 0)	{	//Pause scan 
	    	
    			SetCtrlAttribute (regionScanPanel, ROI_SCAN_PAUSE, ATTR_LABEL_TEXT, "Resume");
				SetCtrlAttribute (regionScanPanel, ROI_SCAN_PAUSE, ATTR_DIMMED, 0);
				SetCtrlAttribute (regionScanPanel, ROI_SCAN_STOP, ATTR_DIMMED, 0);
			}
			else {					//Restart scan	
			
    			SetCtrlAttribute (regionScanPanel, ROI_SCAN_PAUSE, ATTR_LABEL_TEXT, "Pause Scan");
				SetCtrlAttribute (regionScanPanel, ROI_SCAN_PAUSE, ATTR_DIMMED, 0);
				SetCtrlAttribute (regionScanPanel, ROI_SCAN_STOP, ATTR_DIMMED, 0);
			}	
			
			gPauseScan = !gPauseScan;
			
			break;
		}
	return 0;
}

int CVICALLBACK cbRegionScanStop (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
		
    		RegionScanStop();
			break;
		}
	return 0;
}

int CVICALLBACK cbRegionScanPrevFrame (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
		
			PreviousFrame();
			break;
		}
	return 0;
}

int CVICALLBACK cbRegionScanNextFrame (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
		
			NextFrame();
			break;
		}
	return 0;
}

int CVICALLBACK cbRegionScanSetROI (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			region_of_interest_panel_display(roi);
			break;
		}
	return 0;
}


int CVICALLBACK cbRegionScanGotoFrame (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:

			break;
		}
	return 0;
}


int CVICALLBACK cbRegionScanSetFocalPlane (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	//int ret; 
	
	switch (event)
		{
		case EVENT_COMMIT:
			region_of_interest_show_focus_options(roi);

			//ret = region_of_interest_setup_focal_plane(roi, 1);
			//if (ret >= 0)
			//	roi->focal_plane_valid = ret;
			break;
		}
		
	return 0;
}

int CVICALLBACK cbRegionScanClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
		
			GCI_RegionScan_HidePanel();

			#ifdef REGIONSCAN_STANDALONE_ENABLED
			
			//GCI_ImagingWindow_Destroy(image_window);
			//image_window = NULL;
			
			if( gci_camera_is_live_mode(camera) )
				gci_camera_set_snap_mode(camera);
			
			SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
/*			
			if (ConfirmPopup("", "Do you want to turn everything off?")) {
			
				GCI_ImagingHgLampOff();
				GCI_ImagingStagePowerDown();
			}
*/			
			QuitUserInterface(0);
			
			#endif
			
			break;
		}
	return 0;
}

int CVICALLBACK cbRegionScanTimer (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2)
{
    switch (event)
        {
        case EVENT_TIMER_TICK:
        
            RegionScan_UpdateUI();
            break;
        }
    return 0;
}

