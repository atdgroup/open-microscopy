#include <userint.h>
#include <math.h>
#include <analysis.h>
#include "filters.h"
#include "ipi_object_drawing.h"
#include "gci_utils.h"
#include "string_utils.h"
#include "Imaging.h"
#include "focus.h"
#include "CHARM.h"
#include "GL_CVIRegistry.h"
#include "icsviewer_signals.h"
#include <utility.h>
#include "cvixml.h"
#include "GeneralPurposeMicroscope.h"
#include "alignment_uir.h"
#include "step_sampled_alignment.h"
#include "hardware.h"

////////////////////////////////////////////////////////////////////
// Module to stage/camera angle for the microfocus system
// Glenn Pierce and Ros Locke - November 2005
////////////////////////////////////////////////////////////////////
// RJL - 6 March 2006
// Pass correct pixel size to CHARM_Run
////////////////////////////////////////////////////////////////////

struct _Alignment
{
	int alignment_panel;
	int info_panel;
	
	int abort;
	int prev_mode;	//fluorescence or bright field
	
	IPIImageRef frame_image;
	IPIImageRef frame_display_image;

	GCIWindow *processing_window;
	GciCamera *camera;
	
	double pinhole_diameter;  //initially in microns
	
	Rect 	box; 

	Pointd *point_array;
	
	double fov_width;  // field of view (microns)
	double fov_height;
	
	double slope;
	
	int window_id;
	int	timer;
	
};

int alignment_acquire_image(Alignment* alignment);

static void read_or_write_processing_window_registry_settings(int panel_id, int write)
{
	char buffer[500];

	if(panel_id == -1)
		return;

	// load or save panel positions
	
	// make sure the panel is not minimised as this will put v. big values
	// in the reg and at next startup the panel will not be visible!	
	if(write == 1)
		SetPanelAttribute (panel_id, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
	
	sprintf(buffer, "software\\GCI\\Microscope\\Alignment\\ProcessingWindow\\");
	
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "top", panel_id, ATTR_TOP);
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "left", panel_id, ATTR_LEFT);
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "width", panel_id, ATTR_WIDTH); 
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "height", panel_id, ATTR_HEIGHT); 
}


static int display_ipi_image(GciCamera* camera, GCIWindow* window, IPIImageRef image, char *title)
{
	FIBITMAP *dib;
	int ret;
	
	if (image == NULL) {
	
		return -1;
	}

	dib = GCI_FreeImage_IPIImageRefToFIB(image);
	
	ret = gci_camera_display_image(camera, dib, title);
/*
	GCI_ImagingWindow_LoadFreeImageBitmap(window, dib);

	GCI_ImagingWindow_SetWindowTitle(window, title);

	GCI_ImagingWindow_Show(window);

	FreeImage_Unload(dib);
*/	
	return ret;
}


static int CVICALLBACK on_window_timer_tick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Alignment* alignment = (Alignment*) callbackData;      

	switch (event)
	{
		case EVENT_TIMER_TICK:

			display_ipi_image(alignment->camera, alignment->processing_window, alignment->frame_display_image, "Alignment"); 

			break;
	}
		
	return 0;
}


static void ProccessingWindowCloseEventHandler( GCIWindow *win, void* callback_data )
{
	Alignment* alignment = (Alignment*) callback_data; 
	
	if (alignment->frame_image) {
		IPI_Dispose(alignment->frame_image);
		alignment->frame_image = 0;
	}
	if (alignment->frame_display_image) {
		IPI_Dispose(alignment->frame_display_image);
		alignment->frame_display_image = 0;
	}
	
	SetCtrlAttribute(alignment->window_id, alignment->timer, ATTR_ENABLED, 0);
	
	GCI_ImagingWindow_Close(alignment->processing_window); 
	
	alignment->processing_window = NULL;
	
	return;
}


static void ProcessingWindowResizedorMovedEventHandler( GCIWindow *win, void* callback_data )
{
	Alignment* alignment = (Alignment*) callback_data; 
	
	read_or_write_processing_window_registry_settings(alignment->window_id, 1);
	
	return;
}


static void CreateProcessingWindow(Alignment* alignment)
{
	// Create the window to display processing
	if(alignment->processing_window == NULL)
	{
		if ( (alignment->processing_window = GCI_ImagingWindow_CreateAdvanced("Processing", 300, 300, 500, 500, 0, 1)) == NULL ) {
			
			MessagePopup("Error", "Can not create window");
		
			return;
		}
		
		GCI_ImagingWindow_Initialise(alignment->processing_window);
				
		if ((alignment->window_id = GCI_ImagingWindow_GetPanelID(alignment->processing_window)) != -1) {
			read_or_write_processing_window_registry_settings(alignment->window_id, 0);
		}

		GCI_ImagingWindow_SetResizedorMovedHandler( alignment->processing_window, ProcessingWindowResizedorMovedEventHandler, alignment); 
		GCI_ImagingWindow_SetCloseEventHandler( alignment->processing_window, ProccessingWindowCloseEventHandler, alignment );
		
		GCI_ImagingWindow_HideToolBar(alignment->processing_window);
	
		alignment->timer = NewCtrl(alignment->window_id, CTRL_TIMER, "", 0, 0); 
		
		InstallCtrlCallback (alignment->window_id, alignment->timer, on_window_timer_tick, alignment);
			
		SetCtrlAttribute(alignment->window_id, alignment->timer, ATTR_ENABLED, 0);
		
		GCI_ImagingWindow_SetResizeFitStyle(alignment->processing_window);
		GCI_ImagingWindow_Show(alignment->processing_window);
	}
}


void alignment_display_panel(Alignment* alignment)
{
	double microns_per_pixel;
	int binning;
	char filepath[MAX_PATHNAME_LEN];

	CreateProcessingWindow(alignment);  

	//Set up CHARM
	microns_per_pixel = gci_camera_get_microns_per_pixel(alignment->camera);
	binning = gci_camera_get_binning_mode(alignment->camera);
	if (binning < 1) binning = 1;	//avoid crashing if binning not implemented for camera

	alignment->pinhole_diameter = 10.0;	//um
	GetCtrlVal(alignment->alignment_panel, ALIGNPANEL_PINHOLE_DIAM, &alignment->pinhole_diameter);
	alignment->pinhole_diameter /= microns_per_pixel;
	alignment->pinhole_diameter /= binning;							  //now in pixels

	CHARM_setup (REGKEY_HKCU, "software\\GCI\\Microscopy", "software\\GCI\\Microscope\\Cellfinding\\");
	CHARM_setUnitText ("pixels");
	CHARM_setupMultithreading (-1, -1);
	CHARM_setMinMaxRadius(alignment->pinhole_diameter*0.3/2.0, alignment->pinhole_diameter*1.5/2.0); // low minRad is good for elongated shapes

	if ( gci_camera_get_data_mode(alignment->camera) == BPP8) 
		FindPathForFile("pinhole_CHARM_8bit.xml",  filepath);
	else
		FindPathForFile("pinhole_CHARM_12bit.xml", filepath);

	if (CHARM_LoadParameters (filepath) < 0) 
		MessagePopup("CHARM", "Failed to load the pinhole finding parameters.");

	DisplayPanel(alignment->alignment_panel);

	//GCI_ImagingSetFluorMode(0);	//Ensure fluorescence mode for a bead
	alignment->prev_mode = GCI_ImagingGetOperationMode();
	GCI_ImagingSetFluorMode(1);		//Ensure bright field mode for a pinhole
	GCI_ImagingLampOff();		//lamp off
	
	gci_camera_set_live_mode(alignment->camera); 
	gci_camera_activate_live_display(alignment->camera); 
}


static int CVICALLBACK on_alignment_info_ok_clicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Alignment *alignment = (Alignment *) callbackData; 

	switch (event)
	{
		case EVENT_COMMIT:
	
			DiscardPanel(alignment->info_panel);
			alignment->info_panel = -1;

			//Issue an invitation to try again
			DisplayPanel(alignment->alignment_panel);
			gci_camera_set_live_mode(alignment->camera); 
			break;
	}
		
	return 0;
}

static char* myftoa(double val)
{
	static char str[50], *p;
	
	//ftoa only gives 2 dps, not enough for this
	p=str;
	sprintf(str, "%f", val);
	return p;
}
static int newXmlSettingDbl (CVIXMLElement root, const char *name, double val)
{
	CVIXMLElement el=-1;
	//char buffer[64];

	if (CVIXMLNewElement      (root, -1, name, &el)) goto Error;
	if (CVIXMLSetElementValue (el, myftoa(val))) goto Error;
	CVIXMLDiscardElement(el);
	return 0;

	Error:
	if (el >= 0) CVIXMLDiscardElement(el);
	return -1;
}
static void alignment_save_stage_angle(Alignment* alignment)
{
	char path[MAX_PATHNAME_LEN];
	CVIXMLElement root=-1;
	CVIXMLDocument stage_angle=-1;
	
	//Actually we save the slope in xml format
	//GetProjectDir (path);
	//strcat(path, "\\Microscope Data");
	//if (!FileExists(path, 0)) MakeDir (path);
	GetPrivateDataFolder("Microscope Data", path);
	strcat(path, "\\Stage Angle.xml");
	RemoveFileIfExists (path);
	
	if (CVIXMLNewDocument ("stage_angle", &stage_angle)) return;
	if (CVIXMLGetRootElement  (stage_angle, &root))    goto Error;

	if (newXmlSettingDbl (root, "slope", alignment->slope)) goto Error;

	if (FileExists(path, NULL))
		SetFileAttrs (path, 0, -1, -1, -1);   //clear read only atribute
	if (CVIXMLSaveDocument (stage_angle, 0, path)) goto Error;
	SetFileAttrs (path, 1, -1, -1, -1);   //set read only atribute
	
	CVIXMLDiscardElement(root);
	CVIXMLDiscardDocument (stage_angle);
	
	return;

Error:

	if (root >= 0) CVIXMLDiscardElement(root);
	if (stage_angle >= 0) CVIXMLDiscardDocument(stage_angle);
}

static int getXmlSettingDbl (CVIXMLElement root, const char *name, double *val)
{
	CVIXMLElement el=-1;
	char str[64];
	
	if (CVIXMLGetChildElementByTag (root, name, &el)) goto Error;
	if (CVIXMLGetElementValue (el, str)) goto Error;
	CVIXMLDiscardElement(el);
	*val = atof(str);
	return 0;

	Error:
	if (el >= 0) CVIXMLDiscardElement(el);
	return -1;
}
static void alignment_load_stage_angle(Alignment* alignment)
{
	char path[MAX_PATHNAME_LEN];
	CVIXMLElement root=-1;
	CVIXMLDocument stage_angle=-1;
	
	if (FindPathForFile("Stage Angle.xml", path) == -1) goto Error;;
	if (CVIXMLLoadDocument (path, &stage_angle)) goto Error;
	if (CVIXMLGetRootElement (stage_angle, &root))   goto Error;

	if (getXmlSettingDbl (root, "slope", &alignment->slope)) goto Error;	 //in um

	CVIXMLDiscardElement(root);
	CVIXMLDiscardDocument (stage_angle);

	return;

Error:

	alignment->slope = 0.0;
	
	if (root >= 0) CVIXMLDiscardElement(root);
	if (stage_angle >= 0) CVIXMLDiscardDocument(stage_angle);
}

static int CVICALLBACK on_result_ok_clicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Alignment *alignment = (Alignment *) callbackData; 

	switch (event)
	{
		case EVENT_COMMIT:
			//Finished. Tidy up.
			
			//Revert to previous settings
			GCI_ImagingSetFluorMode(alignment->prev_mode);		
			GCI_ImagingLampOn();		//lamp on

			DiscardPanel(panel);
			
			if (alignment->info_panel >= 0) {
				DiscardPanel(alignment->info_panel);
				alignment->info_panel = -1;
			}
			
			if (alignment->frame_image) {
				IPI_Dispose(alignment->frame_image);
				alignment->frame_image = 0;
			}
			if (alignment->frame_display_image) {
				IPI_Dispose(alignment->frame_display_image);
				alignment->frame_display_image = 0;
			}
	
			SetCtrlAttribute(alignment->window_id, alignment->timer, ATTR_ENABLED, 0);

			HidePanel(alignment->alignment_panel);
			
			GCI_ImagingWindow_Close(alignment->processing_window);
			alignment->processing_window = NULL;
			
			alignment_save_stage_angle(alignment);
			break;
	}
		
	return 0;
}

static void alignment_show_error(Alignment* alignment, char *message)
{
	if (alignment->info_panel < 0) {
	
		alignment->info_panel = FindAndLoadUIR(0, "alignment_uir.uir", INFO_PANEL);

		InstallCtrlCallback(alignment->info_panel, INFO_PANEL_OK, on_alignment_info_ok_clicked, alignment); 
	}
	
	if (message != NULL) SetCtrlVal(alignment->info_panel, INFO_PANEL_TEXTMSG, message);
	
    DisplayPanel(alignment->info_panel);
}

//////////////////////////////////////////////////////////////////////////////////
// Pinhole finding - do a preliminary threshold to speed things up

static int NormaliseImage(IPIImageRef imin, IPIImageRef imout)
{
	IPIQuantifyElem report;
	int n;
	
	//NB imin and imout must be of the same type e.g. 16 bit
	IPI_Quantify (imin, IPI_NOMASK, &report, NULL, &n);
	if (report.maxValue == 0.0) report.maxValue = 4080.0;   //avoid crashing
	return IPI_Divide (imin, IPI_USECONSTANT, imout, report.maxValue/255.0);
}

static IPIImageRef alignment_frame_threshold(IPIImageRef frame_image) 
{
	int hist[256], maxv, maxi, threshold_min, threshold_value;
	IPIImageInfo info;
	IPIImageRef temp=0;
	IPIImageRef frame_threshold_image = 0;
	
	//Threshold the image to find bright objects
	IPI_Create (&frame_threshold_image, IPI_PIXEL_U8, 2);      

	//8 bit histogram
	IPI_GetImageInfo (frame_image, &info);
	IPI_Create (&temp, info.pixelType, 2);

	if (info.pixelType != IPI_PIXEL_U8) {
		NormaliseImage(frame_image, temp);
		IPI_Cast (temp, IPI_PIXEL_U8);
	}
	else 
		IPI_Copy (frame_image, temp);

	IPI_Histogram (temp, IPI_NOMASK, 256, 0, 0, hist, NULL);

	maxi = Find_Int_Max (hist, 256, &maxv);  

	threshold_value = maxv * 0.1 / 100.0;

	for (threshold_min=maxi; threshold_min < 256; threshold_min++) {
	
		if (hist[threshold_min] < threshold_value)
			break;
	}

	IPI_Threshold (temp, frame_threshold_image, threshold_min, 255, 1.0, 1);
	
	IPI_Dispose(temp);
	
	return frame_threshold_image;
}

int alignment_define_box(Alignment* alignment)
{
	int n, w, h, left, top, number_of_blobs;
	int i, brightest = 0;
	double max_intensity = 0.0;
	int params[4] = {IPI_PP_RectLeft,IPI_PP_RectWidth,IPI_PP_RectTop,IPI_PP_RectHeight};
	float *Coeffs=NULL;
	IPIQuantifyElem report;
	IPIQuantifyElemPtr regionReport=NULL;
	IPIFullPReportPtr PReport = NULL;
	IPIImageRef frame_threshold_image = 0; 
	IPIImageInfo info;
	//int penwidth = 4, p;
	//Rect tempRect;
	
	IPI_GetImageInfo (alignment->frame_image, &info);       
	
	frame_threshold_image = alignment_frame_threshold(alignment->frame_image);
	//IPI_WindDraw (frame_threshold_image, 0, "", TRUE);
	//IPI_WSetPalette (0, IPI_PLT_BINARY, NULL);

	//Check out the area - a big number indicates gunk on the slide
	IPI_Quantify (frame_threshold_image, IPI_NOMASK, &report, NULL, &n);
	if (report.mean > 0.25) {  //more than a quarter of pixels are red
		alignment_show_error(alignment, "Cannot find pinhole - insufficient contrast");
		IPI_Dispose(frame_threshold_image);
		return ALIGNMENT_ERROR;
	}
	
	//Filter out small blobs
	IPI_LowHighPass (frame_threshold_image, frame_threshold_image, TRUE, FALSE, 2, IPI_MO_STD3X3);
	
	//Count the objects
	IPI_Particle (frame_threshold_image, TRUE, &PReport, &number_of_blobs);
	
	//if (number_of_blobs != 1) {
	//	alignment_show_error(alignment, NULL);
	//	IPI_Dispose(frame_threshold_image);
	//	return ALIGNMENT_ERROR;
	//}
	
	//Assume the pinhole is the brightest object. If not we're in trouble
	if (number_of_blobs > 1) {
		IPI_Label (frame_threshold_image, frame_threshold_image, TRUE, &n);
		IPI_Quantify (alignment->frame_image, frame_threshold_image, &report, &regionReport, &n);
		for (i=0; i < number_of_blobs; i++)  {
			if (regionReport[i].mean > max_intensity) {
				max_intensity = regionReport[i].mean;
				brightest = i;
			}
		}
		IPI_free (regionReport);
	}
	
	Coeffs = (float *) malloc(number_of_blobs * 4 * sizeof(float));
	IPI_ParticleCoeffs (frame_threshold_image, params, 4, PReport, number_of_blobs, Coeffs);
	
	left = Coeffs[brightest*4+0];
	w = Coeffs[brightest*4+1];
	top = Coeffs[brightest*4+2];
	h = Coeffs[brightest*4+3];
			
	// Check size of object
	if ( (w < alignment->pinhole_diameter) || (h < alignment->pinhole_diameter) ) {
		alignment_show_error(alignment, "Brightest object found is too small to be a pinhole");
		IPI_free (PReport);
		free(Coeffs);
		IPI_Dispose(frame_threshold_image);
		return ALIGNMENT_ERROR;
	}
	if ( (w > alignment->pinhole_diameter*5) || (h > alignment->pinhole_diameter*5 ) ) {
		alignment_show_error(alignment, "Brightest object found is too large to be a pinhole");
		IPI_free (PReport);
		free(Coeffs);
		IPI_Dispose(frame_threshold_image);
		return ALIGNMENT_ERROR;
	}
	// Check the object is within the image
	if ((left < 1) || ((left+w) == info.width) || (top < 1) || ((top+h) == info.height)) {
		alignment_show_error(alignment, "Brightest object found is touching the edge of the image");
		IPI_free (PReport);
		free(Coeffs);
		IPI_Dispose(frame_threshold_image);
		return ALIGNMENT_ERROR;
	}

	//Define box 40% bigger than the blob
	alignment->box.left 	= max(0, left - w * ( 40  / 200 ));
	alignment->box.width 	= w + w * ( 40  / 100 );
	alignment->box.top 		= max(0, top - h * ( 40 / 200 ));
	alignment->box.height 	= h + h * ( 40 / 100 );
			
	//Draw it. Need linewidth of 3 to be sure of seeing all the lines on a reduced image
	//ipi_draw_colour_rect (cf->frame_display_image, cf->boxes[i].rect, 255, 0, 0);   // Red
	//for (p=0; p<penwidth; p++) {
	//	tempRect = MakeRect (alignment->box.top-p, alignment->box.left-p, alignment->box.height+2*p, alignment->box.width+2*p);
	//	ipi_draw_colour_rect (alignment->frame_display_image, tempRect, 128, 0, 0);		// Red
	//}
	
	IPI_free (PReport);
	free(Coeffs);
	IPI_Dispose(frame_threshold_image);
	
	return ALIGNMENT_SUCCESS;
}

//int alignment_process_box(Alignment* alignment)
static int alignment_find_pinhole(Alignment* alignment, IPIImageRef image, GlShape **shape)
{
	IPIImageInfo info;
	IPIImageRef box_image=0;
	int nShapes, binning;
	double microns_per_pixel;
	
	microns_per_pixel = gci_camera_get_microns_per_pixel(alignment->camera);
	binning = gci_camera_get_binning_mode(alignment->camera);
	if (binning < 1) binning = 1;	//avoid crashing if binning not implemented for camera

	alignment_define_box(alignment);
	
	//Extract the box containing this cell
	IPI_GetImageInfo (alignment->frame_image, &info);
	IPI_Create (&box_image, info.pixelType, 2);
	if (IPI_Extract (alignment->frame_image, box_image, 1, 1, alignment->box) != 0) goto Error;
	//IPI_WindDraw (box_image, 0, "", TRUE);

	//Ensure that no pixels < 0
  	IPI_Compare (box_image, IPI_USECONSTANT, box_image, IPI_CP_MAX, 0.0);

	CHARM_run(box_image, 0, 0, microns_per_pixel*binning, shape, &nShapes);

	if (nShapes != 1)
		goto Error;	// Seriously wrong - this should never happen. We know the box had something in it.

	if (shape == NULL)
	{
		MessagePopup("Error", "shapes is null");
		goto Error;
	}

	IPI_Dispose(box_image); 
	
	return ALIGNMENT_SUCCESS;

Error:

	alignment_show_error(alignment, "I can't find it. Ensure there is just one sharp, bright object?");
	IPI_Dispose(box_image);
	
	if (shape) CHARM_shapesDispose();

	return ALIGNMENT_ERROR;
}

// This function finds the pinhole object and get the position.
/*
static int alignment_find_pinhole(Alignment* alignment, IPIImageRef image, GlShape **shape)
{
	int info_panel, nShapes;
	
	CHARM_run(image, 0, 0, 1.0, shape, &nShapes);
	
	if(nShapes != 1) {
	
		info_panel = FindAndLoadUIR(0, "alignment_uir.uir", INFO_PANEL);

		InstallCtrlCallback(info_panel, INFO_PANEL_OK, on_alignment_info_ok_clicked, alignment); 

	    DisplayPanel(info_panel);
	    
	    return ALIGNMENT_ERROR;
	}
	
	return ALIGNMENT_SUCCESS;
}
*/

static int alignment_move_pinhole_to_start_position(Alignment* alignment)
{
	double microns_per_pixel, rel_x, rel_y;
	int width, height, binning;
	GlShape *original_pinhole;
	
	alignment_acquire_image(alignment);
	
	// Find the orignal pinhole position	
	if(alignment_find_pinhole(alignment, alignment->frame_image, &original_pinhole) == ALIGNMENT_ERROR)
		return ALIGNMENT_ERROR;
	
	// Get the field of view, ie the image size form the camera.
	gci_camera_get_size(alignment->camera, &width, &height);

	microns_per_pixel = gci_camera_get_microns_per_pixel(alignment->camera);
	binning = gci_camera_get_binning_mode(alignment->camera);
	if (binning < 1) binning = 1;	//avoid crashing if binning not implemented for camera
	
	alignment->fov_width = width * microns_per_pixel * binning;
	alignment->fov_height = height * microns_per_pixel * binning;
	
	//rel_x = (original_pinhole->centreOfMass.x + alignment->box.left - alignment->pinhole_diameter) * microns_per_pixel * binning;
	//rel_y = (original_pinhole->centreOfMass.y + alignment->box.top - height/2) * microns_per_pixel * binning; 
	rel_x = (Round(original_pinhole->centroid.x) + alignment->box.left - 0.05*width) * microns_per_pixel * binning;
	rel_y = (Round(original_pinhole->centroid.y) + alignment->box.top - height/2) * microns_per_pixel * binning; 
	 
	GCI_ImagingStageMoveRelXY(rel_x, rel_y); 
	 
	return ALIGNMENT_SUCCESS;
}

int alignment_acquire_image(Alignment* alignment)
{
	FIBITMAP *dib, *dib_8bit, *colour_dib; 
	
	if (alignment->frame_image) {	   //avoid memory leaks
		IPI_Dispose(alignment->frame_image);
		alignment->frame_image=0;
	}

	dib = gci_camera_get_image(alignment->camera, NULL);   

	dib_8bit = FreeImage_ConvertToStandardType(dib, 1);
	colour_dib = FreeImage_ConvertTo24Bits(dib_8bit); 
	
	gci_camera_display_image(alignment->camera, dib, "Snap");

	alignment->frame_image =  GCI_FreeImage_FIBToIPIImageRef(dib);
	
	if (alignment->frame_display_image == 0) 
		alignment->frame_display_image = GCI_FreeImage_FIBToIPIImageRef(colour_dib); 

	FreeImage_Unload(dib);
	FreeImage_Unload(dib_8bit);
	FreeImage_Unload(colour_dib);
	
	return 0;
}

static int alignment_increment_through_pinhole_positions(Alignment* alignment)
{
	GlShape *pinhole_shape;
	int point = 0;
	double step_size, cog_x, cog_y;
	double x=0, um_per_pix, microns_per_pixel;
	char msg[200];
	
	// We have the first and last pinhole positions. We must find all the
	// intermediate steps depending on the #define NUMBER_OF_POINTS
	
	GCI_ImagingDisableAllPanels(1);
	//GCI_ElectronBeamSourceDisableAllPanels(1);
	GCI_GPscope_DisablePanel(1);

	if (alignment_move_pinhole_to_start_position(alignment) == ALIGNMENT_ERROR)
		return ALIGNMENT_ERROR;
	
	//Use 90% of the image width
	step_size = -(0.9 * alignment->fov_width) / (NUMBER_OF_POINTS-1);
	
	SetCtrlAttribute(alignment->window_id, alignment->timer, ATTR_ENABLED, 1); 
	
	do {
		ProcessSystemEvents();
		if (alignment->abort != 0) goto Error;
		
		alignment_acquire_image(alignment);
	
		if(alignment_find_pinhole(alignment, alignment->frame_image, &pinhole_shape) == ALIGNMENT_ERROR)
			return ALIGNMENT_ERROR;
	
		cog_x = pinhole_shape->centroid.x + alignment->box.left; 
		cog_y = pinhole_shape->centroid.y + alignment->box.top; 
		
		alignment->point_array[point].x = cog_x;
		alignment->point_array[point].y = cog_y;
	
		IPI_DrawColorRect (alignment->frame_display_image, MakeRect(cog_y - 1, cog_x - 1, 3, 3), 255, 0, 0); 
		IPI_DrawColorRect (alignment->frame_display_image, MakeRect(cog_y - 2, cog_x - 2, 5, 5), 255, 0, 0); 
		draw_cell_borders_from_shape (alignment->frame_display_image, *pinhole_shape, alignment->box.left, alignment->box.top, 255, 0, 0);
	
		if (point < NUMBER_OF_POINTS-1) GCI_ImagingStageMoveRelX(step_size); 
	
		point++;
	}
	while(point < NUMBER_OF_POINTS);

	CHARM_shapesDispose();  
	
	SetCtrlAttribute(alignment->window_id, alignment->timer, ATTR_ENABLED, 0);

	// Make sure the full frames are shown as the multithread display may not update in time.
	display_ipi_image(alignment->camera, alignment->processing_window, alignment->frame_display_image, "Alignment"); 

	IPI_Dispose(alignment->frame_image);
	alignment->frame_image = 0;
	
	GCI_ImagingDisableAllPanels(0);
	//GCI_ElectronBeamSourceDisableAllPanels(0);
	GCI_GPscope_DisablePanel(0);

	//Check um per pixel
	for (point = 1; point < NUMBER_OF_POINTS; point++) 
		x += alignment->point_array[point].x - alignment->point_array[point-1].x;
	x /= (NUMBER_OF_POINTS-1);
	um_per_pix = fabs(step_size/x);
	
	microns_per_pixel = gci_camera_get_microns_per_pixel(alignment->camera);
	if (fabs(microns_per_pixel - um_per_pix) > microns_per_pixel/100.0) {
		sprintf(msg, "Measured um per pixel = %.3f, Value from calibration table = %.3f. Objective may need recalibrating.", um_per_pix, microns_per_pixel);
		GCI_MessagePopup("Alignment Check", msg);
	}
	//printf("um per pix %.3f, measured %.3f\n", microns_per_pixel, um_per_pix);	
	
	return ALIGNMENT_SUCCESS;
	
Error:

	GCI_ImagingDisableAllPanels(0);
	//GCI_ElectronBeamSourceDisableAllPanels(0);
	GCI_GPscope_DisablePanel(0);

	CHARM_shapesDispose();  
	
	SetCtrlAttribute(alignment->window_id, alignment->timer, ATTR_ENABLED, 0);

	// Make sure the full frames are shown as the multithread display may not update in time.
	display_ipi_image(alignment->camera, alignment->processing_window, alignment->frame_display_image, "Alignment"); 

	IPI_Dispose(alignment->frame_image);
	alignment->frame_image = 0;
	
	return ALIGNMENT_ERROR;
}


static int alignment_find_alignment_slope(Alignment* alignment)
{
	int i, width, height;
	double x_points[NUMBER_OF_POINTS], y_points[NUMBER_OF_POINTS], output_array[NUMBER_OF_POINTS];
	double intercept, mean_squared_error;
	Point p1, p2;
	
	if (alignment_increment_through_pinhole_positions(alignment) == ALIGNMENT_ERROR)
		return ALIGNMENT_ERROR;
	
	for(i=0; i < NUMBER_OF_POINTS; i++) {
	
		x_points[i] = alignment->point_array[i].x;
	}
	
	for(i=0; i < NUMBER_OF_POINTS; i++) {
	
		y_points[i] = alignment->point_array[i].y;
	}
	
	if(LinFit (x_points, y_points, NUMBER_OF_POINTS, output_array,
		&(alignment->slope), &intercept, &mean_squared_error) < 0)
	{
		return ALIGNMENT_ERROR;
	}
	
	gci_camera_get_size(alignment->camera, &width, &height);
	p1.x = 0;
	p1.y = intercept;
	p2.x = width - 1;
	p2.y = alignment->slope * p2.x + intercept;
	
	//Indicate fit line - need pen width of three to see it all
	IPI_DrawColorLine (alignment->frame_display_image, p1, p2, 0, 255, 0);
	p1.y --;
	p2.y --;
	IPI_DrawColorLine (alignment->frame_display_image, p1, p2, 0, 255, 0);
	p1.y += 2;
	p2.y += 2;
	IPI_DrawColorLine (alignment->frame_display_image, p1, p2, 0, 255, 0);
	display_ipi_image(alignment->camera, alignment->processing_window, alignment->frame_display_image, "Alignment"); 

	return ALIGNMENT_SUCCESS;	
}


static int CVICALLBACK on_alignment_abort_clicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Alignment *alignment = (Alignment *) callbackData; 

	switch (event)
	{
		case EVENT_COMMIT:
	
			alignment->abort = 1;			
			break;
	}
		
	return 0;
}

int CVICALLBACK cbPinholeDiameter (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Alignment *alignment = (Alignment *) callbackData; 
	double microns_per_pixel;
	int binning;
	
	switch (event)
	{
		case EVENT_COMMIT:
			microns_per_pixel = gci_camera_get_microns_per_pixel(alignment->camera);
			binning = gci_camera_get_binning_mode(alignment->camera);
			if (binning < 1) binning = 1;	//avoid crashing if binning not implemented for camera

			GetCtrlVal(alignment->alignment_panel, ALIGNPANEL_PINHOLE_DIAM, &alignment->pinhole_diameter);
			alignment->pinhole_diameter /= microns_per_pixel;
			alignment->pinhole_diameter /= binning;							  //now in pixels
			break;
	}
		
	return 0;
}


static int CVICALLBACK on_alignment_cancel_clicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Alignment *alignment = (Alignment *) callbackData; 

	switch (event)
	{
		case EVENT_COMMIT:
			//Revert to previous settings
			GCI_ImagingSetFluorMode(alignment->prev_mode);		
			GCI_ImagingLampOn();		//lamp on

			GCI_ImagingDisableAllPanels(0);
			//GCI_ElectronBeamSourceDisableAllPanels(0);
			GCI_GPscope_DisablePanel(0);

			HidePanel(alignment->alignment_panel);
			
			if (alignment->info_panel >= 0) {
				DiscardPanel(alignment->info_panel);
				alignment->info_panel = -1;
			}
			
			if (alignment->frame_image) {
				IPI_Dispose(alignment->frame_image);
				alignment->frame_image = 0;
			}
			if (alignment->frame_display_image) {
				IPI_Dispose(alignment->frame_display_image);
				alignment->frame_display_image = 0;
			}
	
			SetCtrlAttribute(alignment->window_id, alignment->timer, ATTR_ENABLED, 0);

			if (alignment->processing_window != NULL) 
				GCI_ImagingWindow_Close(alignment->processing_window);
			alignment->processing_window = NULL;
			
			break;
	}
		
	return 0;
}

static int CVICALLBACK on_alignment_ok_clicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Alignment *alignment = (Alignment *) callbackData; 
	int result_panel, prog_panel;
	char result[200];

	switch (event)
	{
		case EVENT_COMMIT:
	
			// Turn joystick, focus indicator and microscope monitor off to speed things up a bit.  
			GCI_ImagingStageJoystickOff();
			GCI_FocusOnOff(0);
			GCI_ImagingStopAllTimers();
			GCI_ImagingStageDisableTimer();
	
			alignment->abort = 0;			
			prog_panel = FindAndLoadUIR(0, "alignment_uir.uir", PROG_PNL);
			InstallCtrlCallback(prog_panel, PROG_PNL_ABORT, on_alignment_abort_clicked, alignment); 
			DisplayPanel(prog_panel);

			HidePanel(panel);
			
			gci_camera_set_snap_mode(alignment->camera); 

			if (alignment_find_alignment_slope(alignment) != ALIGNMENT_ERROR) {
			
				result_panel = FindAndLoadUIR(0, "alignment_uir.uir", RESULT_PNL);
			
				sprintf(result, "%.2f degrees", atan(alignment->slope)/PI*180.0 );  //display in degrees
			
				SetCtrlVal(result_panel, RESULT_PNL_SLOPE, alignment->slope); 
				SetCtrlVal(result_panel, RESULT_PNL_RESULT_TEXT, result); 
			
				InstallCtrlCallback(result_panel, RESULT_PNL_SLOPE, cbSetSlope, alignment); 
				InstallCtrlCallback(result_panel, RESULT_PNL_OK, on_result_ok_clicked, alignment); 
			
				DisplayPanel(result_panel);
			}
			
			DiscardPanel(prog_panel);

			GCI_ImagingStartAllTimers();
			GCI_ImagingStageEnableTimer();
			//Turn joystick and focus indicator on
			GCI_ImagingStageJoystickOn();
			GCI_FocusOnOff(1);
			
			break;
	}
		
	return 0;
}

double alignment_step_sampled_get_slope(Alignment* alignment)
{
	return alignment->slope;
}

double alignment_step_sampled_get_angle(Alignment* alignment)
{
	return atan(alignment->slope);  //radians
}


Alignment* alignment_step_sampled_new(GciCamera *camera)
{
	Alignment* alignment = (Alignment *) calloc (1, sizeof(Alignment));
	
	alignment->info_panel = -1;
	
	alignment->camera = camera;
	
	alignment->pinhole_diameter = 10.0;	//um
	
	alignment->slope = 0.0;	
	alignment_load_stage_angle(alignment);

	alignment->alignment_panel = FindAndLoadUIR(0, "alignment_uir.uir", ALIGNPANEL);
	
	if(alignment->alignment_panel < 0)
		return ALIGNMENT_ERROR;
	
	InstallCtrlCallback(alignment->alignment_panel, ALIGNPANEL_OK, on_alignment_ok_clicked, alignment); 
	InstallCtrlCallback(alignment->alignment_panel, ALIGNPANEL_CANCEL, on_alignment_cancel_clicked, alignment); 
	InstallCtrlCallback(alignment->alignment_panel, ALIGNPANEL_PINHOLE_DIAM, cbPinholeDiameter, alignment); 
	
	alignment->point_array = (Pointd *) malloc(sizeof(Pointd) * NUMBER_OF_POINTS);

	alignment->processing_window = NULL;

	return alignment; 
}


void alignment_step_sampled_destroy(Alignment* alignment)
{
	if(alignment->alignment_panel >= 0) {
		DiscardPanel(alignment->alignment_panel);
		alignment->alignment_panel = -1;
	}
		
	if (alignment->point_array != NULL) {
		free(alignment->point_array);
		alignment->point_array = NULL;
	}
		
	if (alignment->processing_window != NULL) {
		GCI_ImagingWindow_Close(alignment->processing_window);
		alignment->processing_window = NULL;
	}
		
	if (alignment != NULL) {
		free(alignment);
		alignment = NULL;
	}
}




int CVICALLBACK cbSetSlope (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Alignment *alignment = (Alignment *) callbackData; 

	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(panel, RESULT_PNL_SLOPE, &alignment->slope); 
			break;
		}
	return 0;
}
