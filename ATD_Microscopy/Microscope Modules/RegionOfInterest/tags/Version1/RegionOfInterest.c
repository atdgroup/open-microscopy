#include "easytab.h"
#include <analysis.h>
#include <userint.h>
#include "gci_camera.h"
//#include "gci_orca_camera.h"

#include "RegionOfInterest.h"
#include "RegionOfInterest_ui.h"

#include "Imaging.h"
#include "ImagingFacade.h"
//#include "AutoFocus.h"

#include "string_utils.h"
#include "GL_CVIRegistry.h"
#include "gci_utils.h"

#include <utility.h>

////////////////////////////////////////////////////////////////////
// Module to define a region of interest for the microfocus system
// Glenn Pierce - July 2005
////////////////////////////////////////////////////////////////////
// October 2005 - RJL
// Added further mechanisms to set up the ROI.
// Added flag to region_of_interest_setup_focal_plane() to determine whether
// to focus on points inside or outside the ROI.
// Return the z stage position from region_of_interest_goto_stage_xy()
// so that it can be plotted if required
////////////////////////////////////////////////////////////////////
// Ros Locke - 28 Feb 2005
// On using the Centre/radius method the number of frames went negative on Scan Start
// Added new panel to set focal plane or add a fixed offset to the current plane.
////////////////////////////////////////////////////////////////////

region_of_interest* region_of_interest_selection_new(GciCamera *camera)
{
	region_of_interest *roi = (region_of_interest*) malloc (sizeof(region_of_interest));		
	
	roi->id = 1;
	
	roi->region_panel = -1;
	roi->region_panel_tab1 = -1;
	roi->region_panel_tab2 = -1;
	roi->region_panel_tab3 = -1;
	roi->region_panel_tab4 = -1;
	roi->focal_panel = -1;
	
  	GCI_SignalSystem_Create(&(roi->signal_table), 5);
  		
  	GCI_Signal_New(&(roi->signal_table), "RegionSelected", VOID_INT_INT_INT_INT_MARSHALLER); 
  		
  	roi->focal_plane_valid=0;
  	roi->focal_point_a=0.0;
  	roi->focal_point_b=0.0;
  	roi->focal_point_c=1.0;
  		
  	roi->camera = camera; 
  	
  	return roi;
}

static void read_or_write_roiwindow_registry_settings(int panel_id, int write)
{
	int visible;
	char buffer[500];

	if(panel_id == -1) return;

	// load or save panel positions
	
	// make sure the panel is not minimised as this will put v. big values
	// in the reg and at next startup the panel will not be visible!	
	if(write == 1) {
		GetPanelAttribute (panel_id, ATTR_VISIBLE, &visible);
		if(!visible) return;

		SetPanelAttribute (panel_id, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
	}
	
	sprintf(buffer, "software\\GCI\\Microscope\\RegionOfInterest\\");
	
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "top", panel_id, ATTR_TOP);
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "left", panel_id, ATTR_LEFT);
}

int region_of_interest_panel_init(region_of_interest* roi)
{
	int easyTabCtrl;
	
	//There are three methods of setting up the ROI. We use a tab control for easy selection.
	
	if (roi->region_panel >= 0) return 0;	//Already loaded
	
	roi->region_panel = FindAndLoadUIR(0, "RegionOfInterest_ui.uir", ROI_PANEL); 
	roi->region_panel_tab1 = FindAndLoadUIR(roi->region_panel, "RegionOfInterest_ui.uir", ROI_TAB1);
	roi->region_panel_tab2 = FindAndLoadUIR(roi->region_panel, "RegionOfInterest_ui.uir", ROI_TAB2);
	roi->region_panel_tab3 = FindAndLoadUIR(roi->region_panel, "RegionOfInterest_ui.uir", ROI_TAB3);
	roi->region_panel_tab4 = FindAndLoadUIR(roi->region_panel, "RegionOfInterest_ui.uir", ROI_TAB4);


	//Install callbacks such that they cal access roi
  	InstallCtrlCallback (roi->region_panel, ROI_PANEL_OK, onRoiOk, roi); 
  	InstallCtrlCallback (roi->region_panel, ROI_PANEL_SET_PLANE, cbRoiSetFocalPlane , roi);
  	
  	InstallCtrlCallback (roi->region_panel_tab1, ROI_TAB1_X_START, cbRoiTab1_pointChange, roi); 
  	InstallCtrlCallback (roi->region_panel_tab1, ROI_TAB1_X_END, cbRoiTab1_pointChange, roi); 
  	InstallCtrlCallback (roi->region_panel_tab1, ROI_TAB1_X_LENGTH, cbRoiTab1_dimChange, roi);  
  	InstallCtrlCallback (roi->region_panel_tab1, ROI_TAB1_Y_START, cbRoiTab1_pointChange, roi); 
  	InstallCtrlCallback (roi->region_panel_tab1, ROI_TAB1_Y_END, cbRoiTab1_pointChange, roi); 
  	InstallCtrlCallback (roi->region_panel_tab1, ROI_TAB1_Y_LENGTH, cbRoiTab1_dimChange, roi);
  	InstallCtrlCallback (roi->region_panel_tab1, ROI_TAB1_SET_START, cbRoiTab1_SetStart, roi);
  	InstallCtrlCallback (roi->region_panel_tab1, ROI_TAB1_SET_END, cbRoiTab1_SetEnd , roi);
  	
  	InstallCtrlCallback (roi->region_panel_tab2, ROI_TAB2_X_START, cbRoiTab2_pointChange, roi); 
  	InstallCtrlCallback (roi->region_panel_tab2, ROI_TAB2_Y_START, cbRoiTab2_pointChange, roi); 
  	InstallCtrlCallback (roi->region_panel_tab2, ROI_TAB2_X_END, cbRoiTab2_pointChange, roi); 
  	InstallCtrlCallback (roi->region_panel_tab2, ROI_TAB2_Y_END, cbRoiTab2_pointChange, roi); 
  	InstallCtrlCallback (roi->region_panel_tab2, ROI_TAB2_SET_X_START, cbRoiTab2_pointSetup, roi); 
  	InstallCtrlCallback (roi->region_panel_tab2, ROI_TAB2_SET_Y_START, cbRoiTab2_pointSetup, roi); 
  	InstallCtrlCallback (roi->region_panel_tab2, ROI_TAB2_SET_X_END, cbRoiTab2_pointSetup, roi); 
  	InstallCtrlCallback (roi->region_panel_tab2, ROI_TAB2_SET_Y_END, cbRoiTab2_pointSetup, roi); 

  	InstallCtrlCallback (roi->region_panel_tab3, ROI_TAB3_CENTRE_X, cbRoiTab3_valChange, roi); 
  	InstallCtrlCallback (roi->region_panel_tab3, ROI_TAB3_CENTRE_Y, cbRoiTab3_valChange, roi); 
  	InstallCtrlCallback (roi->region_panel_tab3, ROI_TAB3_RADIUS, cbRoiTab3_valChange, roi); 
  	InstallCtrlCallback (roi->region_panel_tab3, ROI_TAB3_SET_CENTRE, cbRoiTab3_SetCentre, roi); 
  	InstallCtrlCallback (roi->region_panel_tab3, ROI_TAB3_SET_RADIUS, cbRoiTab3_SetRadius, roi);
  	
  	InstallCtrlCallback (roi->region_panel_tab4, ROI_TAB4_GO_1, cbRoiTab4_GoPos, roi);
  	InstallCtrlCallback (roi->region_panel_tab4, ROI_TAB4_GO_2, cbRoiTab4_GoPos, roi);
  	InstallCtrlCallback (roi->region_panel_tab4, ROI_TAB4_GO_3, cbRoiTab4_GoPos, roi);
  	InstallCtrlCallback (roi->region_panel_tab4, ROI_TAB4_GO_4, cbRoiTab4_GoPos, roi);
  	InstallCtrlCallback (roi->region_panel_tab4, ROI_TAB4_GO_5, cbRoiTab4_GoPos, roi);
  	InstallCtrlCallback (roi->region_panel_tab4, ROI_TAB4_GO_6, cbRoiTab4_GoPos, roi);
  	InstallCtrlCallback (roi->region_panel_tab4, ROI_TAB4_GO_7, cbRoiTab4_GoPos, roi);
  	InstallCtrlCallback (roi->region_panel_tab4, ROI_TAB4_GO_8, cbRoiTab4_GoPos, roi);
  	InstallCtrlCallback (roi->region_panel_tab4, ROI_TAB4_GO_MIDDLE, cbRoiTab4_GoPos, roi);
  	
	easyTabCtrl = EasyTab_ConvertFromCanvas (roi->region_panel, ROI_PANEL_CANVAS);
	EasyTab_AddPanels (roi->region_panel, easyTabCtrl, 1, roi->region_panel_tab1,roi->region_panel_tab2,roi->region_panel_tab3,roi->region_panel_tab4,0);

	return 0;
}

int region_of_interest_panel_display(region_of_interest* roi)
{
	if (roi->region_panel >= 0) {
		read_or_write_roiwindow_registry_settings(roi->region_panel, 0);  //restore window position
		//InstallPopup(roi->region_panel);
		DisplayPanel(roi->region_panel);
	}
	
	return 0;
}

int region_of_interest_panel_hide(region_of_interest* roi)
{
	if (roi->region_panel >= 0) {
		read_or_write_roiwindow_registry_settings(roi->region_panel, 1);  //save window position
		//RemovePopup(roi->region_panel);
		HidePanel(roi->region_panel);
	}
	
	return 0;
}

void region_of_interest_destroy(region_of_interest* roi)
{
	if(roi->region_panel != -1) {
		read_or_write_roiwindow_registry_settings(roi->region_panel, 1);  //save window position
		DiscardPanel(roi->region_panel); 
	}
	
	free(roi);
}

		   
void region_of_interest_selected_handler(region_of_interest* roi, REGION_SELECTED_EVENT_HANDLER handler, void *callback_data)
{
	if(GCI_Signal_Connect(&(roi->signal_table), "RegionSelected", handler, callback_data) == SIGNAL_ERROR) {
		printf("Error cannot connect signal handler\n");
	}
}

		   
int region_of_interest_goto_stage_xy(region_of_interest* roi, double x, double y, double *z)
{
	int joyon;
	
	//Move in xy and implement the focal plane if it's been set up
	
	joyon = GCI_ImagingStageJoystickState();
	
	*z=0;
	
	if (roi->focal_plane_valid) {
		*z = roi->focal_point_a * x + roi->focal_point_b * y + roi->focal_point_c;
		
		if (GCI_ImagingStageFastAbsXYZ(x, y, *z))
			return -1;
	}
	else {
		if (GCI_ImagingStageFastAbsXY(x, y))
			return -1;
		GCI_ImagingStageFastReadCoords(&x, &y, z);
	}
	
	if (joyon) GCI_ImagingStageJoystickOn();
	
	return 0;
}


int region_of_interest_z_for_xy(region_of_interest* roi, double x, double y, double *z)
{
	//Calculate z for xy using the focal plane if it's been set up
	
	*z=0;
	
	if (!roi->focal_plane_valid) 
		return -1;
		
	*z = roi->focal_point_a * x + roi->focal_point_b * y + roi->focal_point_c;
	return 0;
}


void region_of_interest_set_region(region_of_interest* roi, int left, int top, int width, int height)
{
	width = fabs(width);	//cannot be negative
	height = fabs(height);	//cannot be negative
	
	roi->left = left;
	roi->top = top;
	roi->width = width;
	roi->height = height;

	SetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_START, (double)left);
	SetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_START, (double)top);
	SetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_END, (double)left + width);
	SetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_END, (double)top + height);
	SetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_LENGTH, (double)width);
	SetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_LENGTH, (double)height);

	SetCtrlVal(roi->region_panel_tab2, ROI_TAB2_X_START, (double)left);
	SetCtrlVal(roi->region_panel_tab2, ROI_TAB2_Y_START, (double)top);
	SetCtrlVal(roi->region_panel_tab2, ROI_TAB2_X_END, (double)left + width);
	SetCtrlVal(roi->region_panel_tab2, ROI_TAB2_Y_END, (double)top + height);

	SetCtrlVal(roi->region_panel_tab3, ROI_TAB3_CENTRE_X, (double)left + width/2);
	SetCtrlVal(roi->region_panel_tab3, ROI_TAB3_CENTRE_Y, (double)top + height/2);
	SetCtrlVal(roi->region_panel_tab3, ROI_TAB3_RADIUS, (double)width/2);

	SetCtrlVal(roi->region_panel_tab4, ROI_TAB4_X_START, (double)left);
	SetCtrlVal(roi->region_panel_tab4, ROI_TAB4_Y_START, (double)top);
	SetCtrlVal(roi->region_panel_tab4, ROI_TAB4_X_END, (double)left + width);
	SetCtrlVal(roi->region_panel_tab4, ROI_TAB4_Y_END, (double)top + height);
}


void region_of_interest_get_region(region_of_interest* roi, int *left, int *top, int *width, int *height)
{
	 *left = roi->left;
	 *top = roi->top;
	 *width = roi->width;
	 *height = roi->height;
}



int region_of_interest_setup_focal_plane(region_of_interest* roi, int doItOutside)
{
	static int focal_plane_valid = 0;
	double ox, oy, gx, gy;
	unsigned int xPixels, yPixels;
	double frameX, frameY, umperpixel;
	double fx[3], fy[3];
	double z, xVal, yVal, zVal, midX, midY, A[9], f[3], coeffs[3]= {0,0,0};
	int i, msgPnl, pnl, ctrl, err;
	char msg[100];
	FIBITMAP *dib = NULL;
	
	//Ask user to focus on three points and use them to calculate focal plane

	ox = roi->left;
	oy = roi->top;
	gx = roi->width;
	gy = roi->height;
	
	midX = ox + gx/2;
	midY = oy + gy/2;

	if (doItOutside) {
		//These are the coords for cells/comets
		//The three points lie just outside the ROI to avoid bleaching cells of interest

		gci_camera_get_size(roi->camera, &xPixels, &yPixels);
		umperpixel = gci_camera_get_microns_per_pixel(roi->camera);
		frameX = umperpixel * xPixels;
		frameY = umperpixel * yPixels;
	
		fx[0] = ox - frameX;
		fy[0] = oy - frameY;
		fx[1] = ox + gx + frameX;
		fy[1] = oy - frameY;
		fx[2] = midX;
		fy[2] = oy + gy + frameY;
	}
	else {
		//These are the coords for tissue sections
		//The three points lie inside the section if possible, otherwise there's nothing to focus on
		fx[0] = ox + gx/4;
		fy[0] = oy + gy/4;
		fx[1] = midX + gx/4;
		fy[1] = oy + gy/4;
		fx[2] = midX;
		fy[2] = midY + gy/3;
	}
	
	//Remember camera settings
	gci_camera_save_state(roi->camera); 
	
	//Set to 8 bit binned mode to enable easy focussing
	GCI_Imaging_SetCameraFocusingMode(roi->camera);
	
	msgPnl = FindAndLoadUIR(0, "RegionOfInterest_ui.uir", MSGPANEL);
	
	sprintf(msg, "Set Focal Plane - Region %d", roi->id);
	SetPanelAttribute (msgPnl, ATTR_TITLE, msg);
	SetCtrlAttribute (msgPnl, MSGPANEL_CANCEL, ATTR_VISIBLE, 1);	 //Unhide the Cancel button
	
	//Give him the opportunity to focus using the joystick
	gci_camera_set_live_mode(roi->camera);
	gci_camera_activate_live_display(roi->camera);
		
	for (i=0; i<3; i++) {
	
		//Move stage to required location
		if (region_of_interest_goto_stage_xy(roi, fx[i], fy[i], &z))
			break;
		
		GCI_ImagingStageJoystickOn();
		
		sprintf(msg, "Please focus on position %d using the joystick.", i+1);
		SetCtrlVal(msgPnl, MSGPANEL_TEXTMSG, msg);
		//InstallPopup(msgPnl);
		DisplayPanel(msgPnl);
		
		while (1) {
			ProcessSystemEvents();
			
			GetUserEvent (0, &pnl, &ctrl);
			if (pnl == msgPnl) {
				if (ctrl == MSGPANEL_OK) break;
				if (ctrl == MSGPANEL_CANCEL) break;
			}

			//For some unfathomable reason the presence of the message panel 
			//prevents camera timer tick events, so get and display an image.
			dib = gci_camera_get_image(roi->camera, NULL); 
			gci_camera_display_image(roi->camera, dib, NULL);
		}
		
		if (ctrl == MSGPANEL_CANCEL) break;

		//RemovePopup(msgPnl);
		HidePanel(msgPnl);
	
		GCI_ImagingStageReadCoords(&xVal, &yVal, &zVal);
		
		//Set up matrices
        A[i*3] = xVal;
        A[i*3+1] = yVal;
        A[i*3+2] = 1.0;
        f[i] = zVal;
	}

	DiscardPanel(msgPnl);
	
	//Reset camera settings
	gci_camera_snap_image(roi->camera);
	//gci_camera_set_snap_mode(roi->camera);
	gci_camera_restore_state(roi->camera);

	//On Cancel, keep any previous focal plane values
	if (ctrl == MSGPANEL_CANCEL) return -1;

	if ((f[0] == f[1]) && (f[1] == f[2])) {  //It's horizontal
		
		roi->focal_point_a = 0;
		roi->focal_point_b = 0;
		roi->focal_point_c = f[0];
		focal_plane_valid = 1;
		
		return focal_plane_valid;
	}
	
    err = LinEqs (A, f, 3, coeffs);
    
	if (err) {
	
		MessagePopup("", "I can't calculate the focal plane. Please try again");
		return 0;
	}
	
    roi->focal_point_a = coeffs[0];
    roi->focal_point_b = coeffs[1];
    roi->focal_point_c = coeffs[2];
	focal_plane_valid = 1;
	
	//Now z = roi->focal_point_a * x + roi->focal_point_b * y + roi->focal_point_c;
	
	return focal_plane_valid;
}

/////////////////////////////////////////////////////////////////////////////////////

static int region_of_interest_focus_on_point(region_of_interest* roi, char* message)
{
	int msgPnl, pnl, ctrl;
	FIBITMAP *dib = NULL;

	//Get the user to define an xy point
	
	//Remember camera settings
	gci_camera_save_state(roi->camera);
		
	//Set to 8 bit binned mode for fast display
	GCI_Imaging_SetCameraFocusingMode(roi->camera);
	gci_camera_set_live_mode(roi->camera);
	gci_camera_activate_live_display(roi->camera);
		
	GCI_ImagingStageJoystickOn();
		
	msgPnl = FindAndLoadUIR(0, "RegionOfInterest_ui.uir", MSGPANEL);
	
	SetPanelAttribute (msgPnl, ATTR_TITLE, "Set ROI Co-ords");
	SetCtrlVal(msgPnl, MSGPANEL_TEXTMSG, message);
	//InstallPopup(msgPnl);
	DisplayPanel(msgPnl);
		
	while (1) {
		ProcessSystemEvents();
		
		GetUserEvent (0, &pnl, &ctrl);
		if ((pnl == msgPnl) && (ctrl == MSGPANEL_OK))
			break;

		//For some unfathomable reason the presence of the message panel 
		//prevents camera timer tick events, so get and display an image.
		dib = gci_camera_get_image(roi->camera, NULL); 
		gci_camera_display_image(roi->camera, dib, NULL);
	}
		
	DiscardPanel(msgPnl);
	
	//Restore camera settings
	gci_camera_set_snap_mode(roi->camera);
	gci_camera_restore_state(roi->camera);
	
	return 0;
}

int CVICALLBACK cbRoiTab1_SetStart (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double xStart, yStart, xEnd, yEnd, dummy;

	region_of_interest* roi = (region_of_interest*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
		
			//Define a new start point - adjust the roi dimensions
			region_of_interest_focus_on_point(roi, "Move stage to start position using joystick");

			GCI_ImagingStageReadCoords(&xStart, &yStart, &dummy);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_END, &xEnd);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_END, &yEnd);
			
			//Update all tabs
			region_of_interest_set_region(roi, xStart, yStart, xEnd-xStart, yEnd-yStart);

			//Indicate that this point has been defined at least once
			SetCtrlAttribute (roi->region_panel_tab1, ROI_TAB1_SET_START, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);

			//Leave in live mode to keep BV happy
			gci_camera_set_live_mode(roi->camera);
			gci_camera_activate_live_display(roi->camera);
			
			break;
		}
		
	return 0;
}

static void swap(double *val1, double *val2)
{
	double temp;
	
	temp = *val1;
	*val1 = *val2;
	*val2 = temp;
}

int CVICALLBACK cbRoiTab1_SetEnd (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double xEnd, yEnd, dummy, xStart, yStart;

	region_of_interest* roi = (region_of_interest*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			//Define a new end point - adjust the roi dimensions
			region_of_interest_focus_on_point(roi, "Move stage to end position using joystick");
		
			GCI_ImagingStageReadCoords(&xEnd, &yEnd, &dummy);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_START, &xStart);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_START, &yStart);
			
			if (xStart > xEnd) swap (&xStart, &xEnd);
			if (yStart > yEnd) swap (&yStart, &yEnd);
			
			//Update all tabs
			region_of_interest_set_region(roi, xStart, yStart, xEnd-xStart, yEnd-yStart);

			//Indicate that this point has been defined at least once
			SetCtrlAttribute (roi->region_panel_tab1, ROI_TAB1_SET_END, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);

			//Leave in live mode to keep BV happy
			gci_camera_set_live_mode(roi->camera);
			gci_camera_activate_live_display(roi->camera);
			break;
		}
		
	return 0;
}

int CVICALLBACK cbRoiTab1_pointChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double xStart, yStart, xEnd, yEnd;

	region_of_interest* roi = (region_of_interest*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			//A new start or end point has been typed in - adjust the roi dimensions
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_START, &xStart);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_START, &yStart);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_END, &xEnd);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_END, &yEnd);
			
			if (xStart > xEnd) swap (&xStart, &xEnd);
			if (yStart > yEnd) swap (&yStart, &yEnd);
			
			//Update all tabs
			region_of_interest_set_region(roi, xStart, yStart, xEnd-xStart, yEnd-yStart);

			//Indicate that this point has been defined at least once
			if ((control == ROI_TAB1_X_START) || (control == ROI_TAB1_Y_START))
				SetCtrlAttribute (roi->region_panel_tab1, ROI_TAB1_SET_START, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);
			else
				SetCtrlAttribute (roi->region_panel_tab1, ROI_TAB1_SET_END, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);

			break;
		}
		
	return 0;
}

int CVICALLBACK cbRoiTab1_dimChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double xStart, yStart, xLength, yLength;

	region_of_interest* roi = (region_of_interest*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			//A new dimension has been typed in - adjust the end point
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_START, &xStart);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_START, &yStart);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_LENGTH, &xLength);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_LENGTH, &yLength);
			
			//Update all tabs
			region_of_interest_set_region(roi, xStart, yStart, xLength, yLength);
			break;
		}
		
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
int CVICALLBACK cbRoiTab2_pointSetup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double x, y, xStart, yStart, xEnd, yEnd, dummy;
	int colour;
	
	region_of_interest* roi = (region_of_interest*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			//Define a new point - adjust the roi dimensions
			GetCtrlVal(roi->region_panel_tab2, ROI_TAB2_X_START, &xStart);
			GetCtrlVal(roi->region_panel_tab2, ROI_TAB2_X_END, &xEnd);
			GetCtrlVal(roi->region_panel_tab2, ROI_TAB2_Y_START, &yStart);
			GetCtrlVal(roi->region_panel_tab2, ROI_TAB2_Y_END, &yEnd);

			switch (control) {
				case ROI_TAB2_SET_X_START:
					region_of_interest_focus_on_point(roi, "Move stage to X Start coordinate using joystick");
					GCI_ImagingStageReadCoords(&xStart, &y, &dummy);
				break;
				
				case ROI_TAB2_SET_X_END:
					region_of_interest_focus_on_point(roi, "Move stage to X End coordinate using joystick");
					GCI_ImagingStageReadCoords(&xEnd, &y, &dummy);
				break;
				
				case ROI_TAB2_SET_Y_START:
					region_of_interest_focus_on_point(roi, "Move stage to Y Start coordinate using joystick");
					GCI_ImagingStageReadCoords(&x, &yStart, &dummy);
				break;
				
				case ROI_TAB2_SET_Y_END:
					region_of_interest_focus_on_point(roi, "Move stage to Y end coordinate using joystick");
					GCI_ImagingStageReadCoords(&x, &yEnd, &dummy);
				break;
			}
			
			if (xStart > xEnd) {
				GetCtrlAttribute (roi->region_panel_tab2, ROI_TAB2_SET_X_END, ATTR_CMD_BUTTON_COLOR, &colour);
				if (colour == VAL_YELLOW)
					swap (&xStart, &xEnd);  //Only swap if both have been set up
			}
			
			if (yStart > yEnd) {
				GetCtrlAttribute (roi->region_panel_tab2, ROI_TAB2_SET_Y_END, ATTR_CMD_BUTTON_COLOR, &colour);
				if (colour == VAL_YELLOW)
					swap (&yStart, &yEnd);
			}
			
			//Update all tabs
			region_of_interest_set_region(roi, xStart, yStart, xEnd-xStart, yEnd-yStart);

			//Indicate that this point has been defined at least once
			SetCtrlAttribute (roi->region_panel_tab2, control, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);

			//Leave in live mode to keep BV happy
			gci_camera_set_live_mode(roi->camera);
			gci_camera_activate_live_display(roi->camera);
			break;
		}
	return 0;
}

int CVICALLBACK cbRoiTab2_pointChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double xStart, yStart, xEnd, yEnd;
	
	region_of_interest* roi = (region_of_interest*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			//A new point value has been typed in - adjust the roi dimensions
			GetCtrlVal(roi->region_panel_tab2, ROI_TAB2_X_START, &xStart);
			GetCtrlVal(roi->region_panel_tab2, ROI_TAB2_X_END, &xEnd);
			GetCtrlVal(roi->region_panel_tab2, ROI_TAB2_Y_START, &yStart);
			GetCtrlVal(roi->region_panel_tab2, ROI_TAB2_Y_END, &yEnd);

			if (xStart > xEnd) swap (&xStart, &xEnd);
			if (yStart > yEnd) swap (&yStart, &yEnd);
			
			//Update all tabs
			region_of_interest_set_region(roi, xStart, yStart, xEnd-xStart, yEnd-yStart);

			//Indicate that this point has been defined at least once
			switch (control) {
				case ROI_TAB2_X_START:
					SetCtrlAttribute (roi->region_panel_tab2, ROI_TAB2_SET_X_START, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);
				break;

				case ROI_TAB2_X_END:
					SetCtrlAttribute (roi->region_panel_tab2, ROI_TAB2_SET_X_END, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);
				break;
				case ROI_TAB2_Y_START:
					SetCtrlAttribute (roi->region_panel_tab2, ROI_TAB2_SET_Y_START, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);
				break;
				case ROI_TAB2_Y_END:
					SetCtrlAttribute (roi->region_panel_tab2, ROI_TAB2_SET_Y_END, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);
				break;
			}
		}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
int CVICALLBACK cbRoiTab3_SetCentre (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double xCentre, yCentre, dummy, radius;

	region_of_interest* roi = (region_of_interest*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			//Define a new centre point - adjust the roi position
			region_of_interest_focus_on_point(roi, "Move stage to centre position using joystick");
		
			GCI_ImagingStageReadCoords(&xCentre, &yCentre, &dummy);
			GetCtrlVal(roi->region_panel_tab3, ROI_TAB3_RADIUS, &radius);
			
			radius = fabs(radius);	//cannot be negative
			SetCtrlVal(roi->region_panel_tab3, ROI_TAB3_RADIUS, radius);
			
			//Update all tabs
			region_of_interest_set_region(roi, xCentre-radius, yCentre-radius, radius*2, radius*2);

			//Indicate that this point has been defined at least once
			SetCtrlAttribute (roi->region_panel_tab3, ROI_TAB3_SET_CENTRE, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);

			//Leave in live mode to keep BV happy
			gci_camera_set_live_mode(roi->camera);
			gci_camera_activate_live_display(roi->camera);
			break;
		}
		
	return 0;
}

int CVICALLBACK cbRoiTab3_SetRadius (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double x, y, xCentre, yCentre, dummy, radius;

	region_of_interest* roi = (region_of_interest*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			//Define a new edge point - adjust the radius
			region_of_interest_focus_on_point(roi, "Move stage to edge position using joystick");
		
			GCI_ImagingStageReadCoords(&x, &y, &dummy);
			GetCtrlVal(roi->region_panel_tab3, ROI_TAB3_CENTRE_X, &xCentre);
			GetCtrlVal(roi->region_panel_tab3, ROI_TAB3_CENTRE_Y, &yCentre);
			radius = sqrt((x-xCentre)*(x-xCentre) + (y-yCentre)*(y-yCentre));	//Pythagoras
			
			//Update all tabs
			region_of_interest_set_region(roi, xCentre-radius, yCentre-radius, radius*2, radius*2);

			//Indicate that this point has been defined at least once
			SetCtrlAttribute (roi->region_panel_tab3, ROI_TAB3_SET_RADIUS, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);

			//Leave in live mode to keep BV happy
			gci_camera_set_live_mode(roi->camera);
			gci_camera_activate_live_display(roi->camera);
			break;
		}
		
	return 0;
}

int CVICALLBACK cbRoiTab3_valChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double xCentre, yCentre, radius;

	region_of_interest* roi = (region_of_interest*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			//A new centre point or radius has been typed in
			GetCtrlVal(roi->region_panel_tab3, ROI_TAB3_CENTRE_X, &xCentre);
			GetCtrlVal(roi->region_panel_tab3, ROI_TAB3_CENTRE_Y, &yCentre);
			GetCtrlVal(roi->region_panel_tab3, ROI_TAB3_RADIUS, &radius);
			
			radius = fabs(radius);	//cannot be negative
			SetCtrlVal(roi->region_panel_tab3, ROI_TAB3_RADIUS, radius);
			
			//Update all tabs
			region_of_interest_set_region(roi, xCentre-radius, yCentre-radius, radius*2, radius*2);

			//Indicate that this value has been changed at least once
			if (control == ROI_TAB3_RADIUS)
				SetCtrlAttribute (roi->region_panel_tab3, ROI_TAB3_SET_RADIUS, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);
			else
				SetCtrlAttribute (roi->region_panel_tab3, ROI_TAB3_SET_CENTRE, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);

			break;
		}
		
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////

int CVICALLBACK cbRoiTab4_GoPos (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double xStart, yStart, xEnd, yEnd, x, y, z;

	region_of_interest* roi = (region_of_interest*) callbackData; 
	
	switch (event)
		{
		case EVENT_COMMIT:
			//Allow checking of the selected ROI by moving to the centre or a corner
			
			GetCtrlVal(roi->region_panel_tab4, ROI_TAB4_X_START, &xStart);
			GetCtrlVal(roi->region_panel_tab4, ROI_TAB4_X_END, &xEnd);
			GetCtrlVal(roi->region_panel_tab4, ROI_TAB4_Y_START, &yStart);
			GetCtrlVal(roi->region_panel_tab4, ROI_TAB4_Y_END, &yEnd);
			switch (control) {
				case ROI_TAB4_GO_1:
					x = xStart;
					y = yStart;
				break;
				case ROI_TAB4_GO_2:
					x = xEnd;
					y = yStart;
				break;
				case ROI_TAB4_GO_3:
					x = xEnd;
					y = yEnd;
				break;
				case ROI_TAB4_GO_4:
					x = xStart;
					y = yEnd;
				break;
				case ROI_TAB4_GO_5:
					x = xStart + (xEnd - xStart)/2.0;
					y = yStart;
				break;
				case ROI_TAB4_GO_6:
					x = xStart + (xEnd - xStart)/2.0;
					y = yEnd;
				break;
				case ROI_TAB4_GO_7:
					x = xStart;
					y = yStart + (yEnd - yStart)/2.0;
				break;
				case ROI_TAB4_GO_8:
					x = xEnd;
					y = yStart + (yEnd - yStart)/2.0;
				break;
				case ROI_TAB4_GO_MIDDLE:
					x = xStart + (xEnd - xStart)/2.0;
					y = yStart + (yEnd - yStart)/2.0;
				break;
			}
			
			region_of_interest_goto_stage_xy(roi, x, y, &z);

			//GCI_ImagingStageMoveAbsXY(x, y);
			
			//Ensure a live image
			gci_camera_set_live_mode(roi->camera);
			gci_camera_activate_live_display(roi->camera);
			
			break;
		}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////
int region_of_interest_set_offset(region_of_interest* roi)
{
	int msgPnl, pnl, ctrl;
	FIBITMAP *dib = NULL;
	double x, y, z, zCalc, zDiff;
	char msg[50];
	
	msgPnl = FindAndLoadUIR(0, "RegionOfInterest_ui.uir", MSGPANEL);
	
	SetPanelAttribute (msgPnl, ATTR_TITLE, "Set Offset to Focal Plane");
	SetCtrlAttribute (msgPnl, MSGPANEL_CANCEL, ATTR_VISIBLE, 1);	 //Unhide the Cancel button
	
	//Give him the opportunity to focus using the joystick
	gci_camera_set_live_mode(roi->camera);
	gci_camera_activate_live_display(roi->camera);
	GCI_ImagingStageJoystickOn();
		
	SetCtrlVal(msgPnl, MSGPANEL_TEXTMSG, "Please focus on any object using the joystick.");
	DisplayPanel(msgPnl);
		
	while (1) {
		ProcessSystemEvents();
			
		GetUserEvent (0, &pnl, &ctrl);
		if (pnl == msgPnl) {
			if (ctrl == MSGPANEL_OK) break;
			if (ctrl == MSGPANEL_CANCEL) break;
		}

		//For some unfathomable reason the presence of the message panel 
		//prevents camera timer tick events, so get and display an image.
		dib = gci_camera_get_image(roi->camera, NULL); 
		gci_camera_display_image(roi->camera, dib, NULL);
	}
		
	//Reset camera settings
	gci_camera_snap_image(roi->camera);
	gci_camera_restore_state(roi->camera);

	if (ctrl == MSGPANEL_CANCEL) return 0;

	DiscardPanel(msgPnl);
	
	GCI_ImagingStageReadCoords(&x, &y, &z);
	
	//What should z be for this xy?
	zCalc = roi->focal_point_a * x + roi->focal_point_b * y + roi->focal_point_c;
	zDiff = z - zCalc;
	roi->focal_point_c += zDiff;
	
	sprintf(msg, "Offset of %.2f um applied.", zDiff);
	MessagePopup("Focal Plane Setup", msg);
	
	return 0;
}		

int region_of_interest_show_focus_options(region_of_interest* roi)
{
	int pnl, ctrl, val;
	//double ox, oy, gx, gy;

	// Check for valid ROI
	if(roi->width < 1.0 || roi->height < 1.0) {  //height and/or width < 1 um
		MessagePopup("Error", "ROI width or height is zero. Select a valid region.");
		return 0;
	}
			
	roi->focal_panel = FindAndLoadUIR(0, "RegionOfInterest_ui.uir", FOCAL_PNL); 
	if (roi->focal_panel < 0) return -1;
	
	
	DisplayPanel(roi->focal_panel);
	
	while (1) {
		//Dim the offset button if there's no valid focal plane
		SetCtrlAttribute(roi->focal_panel, FOCAL_PNL_SET_F_OFFSET, ATTR_DIMMED, !roi->focal_plane_valid);
		
		GetUserEvent (1, &pnl, &ctrl);
		if (pnl != roi->focal_panel)
			continue;

		switch (ctrl) {
			case FOCAL_PNL_OK:
				DiscardPanel(roi->focal_panel);
				roi->focal_panel = -1;
				break;
			
			case FOCAL_PNL_OUTSIDE_ROI:
				GetCtrlVal(roi->focal_panel, FOCAL_PNL_OUTSIDE_ROI, &val);
				SetCtrlVal(roi->focal_panel, FOCAL_PNL_INSIDE_ROI, !val);
				break;
		
			case FOCAL_PNL_INSIDE_ROI:
				GetCtrlVal(roi->focal_panel, FOCAL_PNL_INSIDE_ROI, &val);
				SetCtrlVal(roi->focal_panel, FOCAL_PNL_OUTSIDE_ROI, !val);
				break;
		
			case FOCAL_PNL_SET_FOCAL_PLANE:
				gci_camera_set_snap_mode(roi->camera);
			
				/*   Already done? - remove after testing
				//Get ROI co-ords from panel
				GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_START, &ox);
				GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_START, &oy);
				GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_LENGTH, &gx);
				GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_LENGTH, &gy);
				roi->left = (int)(ox + 0.5);
				roi->top = (int)(oy + 0.5);
				roi->width = (int)(gx + 0.5);
				roi->height = (int)(gy + 0.5);
				*/
				
				GetCtrlVal(roi->focal_panel, FOCAL_PNL_OUTSIDE_ROI, &val);
				roi->focal_plane_valid = region_of_interest_setup_focal_plane(roi, val);
				break;

			case FOCAL_PNL_SET_F_OFFSET:
				region_of_interest_set_offset(roi);
				break;
		}
		if (roi->focal_panel == -1)
			break;
	}
	
	return 0;
}

int CVICALLBACK cbRoiSetFocalPlane (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	region_of_interest* roi = (region_of_interest*) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
			region_of_interest_show_focus_options(roi);
			break;
		}
		
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

int CVICALLBACK onRoiOk (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	region_of_interest* roi = (region_of_interest*) callbackData; 
	double startx, x_length;
	double starty, y_length;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_START, &startx);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_LENGTH, &x_length);
			
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_START, &starty);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_LENGTH, &y_length);

			// Less than 1 micron ?
			if(x_length < 1.0 || y_length < 1.0) {
			
				MessagePopup("Error", "End point is the same as the start point. Select a region");
				return 0;
			}
			
			roi->left = startx;
			roi->top = starty;
			roi->width = x_length;
			roi->height = y_length;

			GCI_Signal_Emit(&(roi->signal_table), "RegionSelected",
				GCI_INT, roi->left, GCI_INT, roi->top, GCI_INT, roi->width, GCI_INT, roi->height);   

			read_or_write_roiwindow_registry_settings(roi->region_panel, 1);  //save window position
			//RemovePopup(roi->region_panel);
			HidePanel(roi->region_panel); 

			//Ensure joystick enabled
			GCI_ImagingStageJoystickOn();

			//DiscardPanel(roi->region_panel_tab1); 
			//roi->region_panel_tab1 = -1;

			break;
		}
		
	return 0;
}

int CVICALLBACK cbMessageOK (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:

			break;
		}
	return 0;
}

int CVICALLBACK onRoiCancel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			HidePanel(panel); 

			//Ensure joystick enabled
			GCI_ImagingStageJoystickOn();
			break;
		}
	return 0;
}

