#include "easytab.h"
#include "ExcelAutomation.h"
#include "toolbox.h"
#include <userint.h>
//#include "hardware.h"
#include "cell_finding_ui.h"
#include "cell_finding.h"
#include "cellmap.h"
#include "CellRevisit_ui.h"
#include "CellRevisit.h"
#include "refimages.h"
#include "ElectronBeamSource.h"
#include "gci_imaging_window_signals.h"
#include "Imaging.h"
#include "GeneralPurposeMicroscope.h"
#include "string_utils.h"
#include "GL_CVIRegistry.h"

#include <utility.h>
#include <formatio.h>

#include "CHARM.h"

////////////////////////////////////////////////////////////////////
// Module to manage user interface of cell/object finding for the microfocus system
// Glenn Pierce - July 2005
////////////////////////////////////////////////////////////////////
// Ros Locke - October 2005
// Add more controls and panels to cell_finder_dim_ui_controls()
////////////////////////////////////////////////////////////////////
// Ros Locke - 28 Feb 2006
// Added cf.safe_radius so that we can prevent trying to drive the objective
// into the side of the dish. The radius is relative to the dish centre
// which is itself relative to the stage zero at initialisation.
////////////////////////////////////////////////////////////////////
// RJL - 6 March 2006
// Display radius values rather than cell diameters
////////////////////////////////////////////////////////////////////
// Ros Locke - 10 may 2006
// Added control to switch off pre_processing
// Pre-processing can only be used where the cells are sparse
////////////////////////////////////////////////////////////////////

void cell_finder_dim_ui_controls(cell_finder *cf, int dim)
{
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_MIN_CELL_DIAMETER, ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_MAX_CELL_DIAMETER, ATTR_DIMMED, dim);
	
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_1_OF_1,   ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_1_OF_4,   ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_2_OF_4,   ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_3_OF_4,   ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_4_OF_4,   ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_1_OF_16,  ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_2_OF_16,  ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_3_OF_16,  ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_4_OF_16,  ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_5_OF_16,  ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_6_OF_16,  ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_7_OF_16,  ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_8_OF_16,  ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_9_OF_16,  ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_10_OF_16, ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_11_OF_16, ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_12_OF_16, ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_13_OF_16, ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_14_OF_16, ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_15_OF_16, ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_16_OF_16, ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_XL_FNAME, ATTR_DIMMED, dim);

	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_CUSTOM_ROI, ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_SCAN_OPTIONS, ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_CRITERIA, ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->experiment_panel, EXPT_PANEL_CLOSE, ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_CUSTOM_ROI, ATTR_DIMMED, dim);
	SetCtrlAttribute (cf->automatic_panel, AUTOPNL_GO_CENTRE, ATTR_DIMMED, dim);
	
	SetPanelAttribute (cf->criteria_panel, ATTR_DIMMED, dim);
	SetPanelAttribute (cf->scan_panel, ATTR_DIMMED, dim);
	ProcessDrawEvents();
}


static void ProccessingWindowCloseEventHandler( GCIWindow *win, void* callback_data )
{
	cell_finder *cf = (cell_finder *) callback_data; 
	
	SetCtrlAttribute(cf->automatic_panel, AUTOPNL_TIMER, ATTR_ENABLED, 0);
	
	GCI_ImagingWindow_Destroy(cf->processing_window); 
	
	cf->processing_window = NULL;
	
	return;
}

static void read_or_write_processingwindow_registry_settings(int panel_id, int write)
{
	char buffer[500];

	if(panel_id == -1)
		return;

	// load or save panel positions
	
	// make sure the panel is not minimised as this will put v. big values
	// in the reg and at next startup the panel will not be visible!	
	if(write == 1)
		SetPanelAttribute (panel_id, ATTR_WINDOW_ZOOM, VAL_NO_ZOOM);
	
	sprintf(buffer, "software\\GCI\\MicroFocus\\Cellfinding\\ProcessingWindow\\");
	
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "top", panel_id, ATTR_TOP);
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "left", panel_id, ATTR_LEFT);
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "width", panel_id, ATTR_WIDTH); 
	checkRegistryValueForPanelAttribInt(write, REGKEY_HKCU, buffer, "height", panel_id, ATTR_HEIGHT); 
}


static void ProcessingWindowResizedorMovedEventHandler( GCIWindow *win, void* callback_data )
{
	int id;
	cell_finder *cf = (cell_finder *) callback_data; 
	
	if ((id = GCI_ImagingWindow_GetPanelID(win)) != -1) {
		read_or_write_processingwindow_registry_settings(id, 1);
	}
	
	return;
}


static void CreateProcessingWindow(cell_finder *cf)
{
	int id;

	// Create the window to display processing
	if(cf->processing_window == NULL)
	{
		if ( (cf->processing_window = GCI_ImagingWindow_Create("Processing", 300, 300, 500, 500, 0, 1)) == NULL ) {
			
			MessagePopup("Error", "Can not create window");
		
			return;
		}
				
		if ((id = GCI_ImagingWindow_GetPanelID(cf->processing_window)) != -1) {
			read_or_write_processingwindow_registry_settings(id, 0);
		}

		GCI_ImagingWindow_SetResizedorMovedHandler( cf->processing_window, ProcessingWindowResizedorMovedEventHandler, cf); 
		GCI_ImagingWindow_SetCloseEventHandler( cf->processing_window, ProccessingWindowCloseEventHandler, cf );
		
		GCI_ImagingWindow_HideToolBar(cf->processing_window);
		GCI_ImagingWindow_HidePaletteBar(cf->processing_window);    
		
		GCI_ImagingWindow_SetResizeFitStyle(cf->processing_window, 1);
		GCI_ImagingWindow_Show(cf->processing_window);
	}
}


static void OnMosaicClicked (GCIWindow *window, const Point image_point, const Point viewer_point, void* data )
{
	IPIImageInfo info;
	int pixelx, pixely;
	double microns_per_pixel, x_in_microns, y_in_microns, stage_x, stage_y, z;

	cell_finder *cf = (cell_finder *) data;    

	microns_per_pixel = cf->mosaic_window->um_per_pixel;
	
	//Start represents middle of top left tile and y is "upside down"
	pixelx = image_point.x - cf->mosaic_window->tile_width/2;
	pixely = image_point.y + cf->mosaic_window->tile_height/2;
	
	x_in_microns = pixelx * microns_per_pixel;
	IPI_GetImageInfo (cf->mosaic_window->mosaic_image, &info);
	y_in_microns = (info.height-pixely) * microns_per_pixel;
	
	stage_x = x_in_microns + cf->frame_roi_left;
	stage_y = y_in_microns + cf->frame_roi_top;
	
	region_of_interest_goto_stage_xy(cf->roi, stage_x, stage_y, &z);
	
	gci_camera_snap_image(cf->camera);
}


static void CreateMosaic(cell_finder *cf)
{
	int image_width, image_height, tileW, tileH, x, y;
	double maxW=500.0, maxH=500.0, h, w;
	double overlapPctX, overlapPctY, imAspectRatio;
	int monitors, screenW, screenH, top, left;
	
	gci_camera_get_size(cf->camera, &image_width, &image_height);  
	overlapPctX = (double)cf->overlap_horz_pixels/image_width * 100;
	overlapPctY = (double)cf->overlap_vert_pixels/image_height * 100;
	
	//Set mosaic image size with correct aspect ratio
	x = cf->number_of_frames_in_row;	//save typing
	y = cf->number_of_frames_in_col;
	
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
	GetSystemAttribute (ATTR_NUM_MONITORS, &monitors);
	GetScreenSize (&screenH, &screenW);
	
	top = screenH - h - 5;
	left = screenW - w - 5;
	
	if (monitors > 1)
		top += screenH;

	if (cf->mosaic_window != NULL) {
		mosaic_window_destroy(cf->mosaic_window);
		cf->mosaic_window = NULL;
	}
	
	if (cf->mosaic_window == NULL) {
		cf->mosaic_window = mosaic_window_new(left, top, w, h); 
		GCI_ImagingWindow_SetMouseDownHandler (cf->mosaic_window->window, OnMosaicClicked , cf);    
	}
		
	mosaic_window_set_row_and_col_size(cf->mosaic_window, cf->number_of_frames_in_row, cf->number_of_frames_in_col);
	mosaic_window_clear(cf->mosaic_window);
	mosaic_window_show(cf->mosaic_window);
	mosaic_window_set_overlap(cf->mosaic_window, overlapPctX, overlapPctY);
}

///////////////////////////////////////////////////////////////////////////////
// Automatic cell-finding control callbacks

int CVICALLBACK cbCellsInitialCellDiameter (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double min, max;
	cell_finder *cf = (cell_finder *) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlVal(panel, AUTOPNL_MIN_CELL_DIAMETER, &min);
			GetCtrlVal(panel, AUTOPNL_MAX_CELL_DIAMETER, &max);
	
			cell_finder_set_cell_diameter(cf, min, max); 
			
			CHARM_setMinMaxDiameter(min, max); // low minRad is good for elongated shapes

			break;
	}
	
	return 0;
}

static void DeleteImageSequence (char *fname)
{
    int dot, n=1, size;
    char ext[5]=".ics", fname2[MAX_PATHNAME_LEN], fname3[MAX_PATHNAME_LEN], temp[6];
    
	//Delete a sequence of images such as myImage_00001.ics to myImage_00123.ics

    SetWaitCursor(1);
    dot = FindPattern (fname, 0, -1, ".", 0, 1);
    if (dot == -1) dot = strlen(fname);
    FillBytes (fname2, 0, MAX_PATHNAME_LEN, 0);
    strncpy (fname2, fname, dot);
    while (1) {
        FillBytes (fname3, 0, MAX_PATHNAME_LEN, 0);
        strcpy (fname3, fname2);
        Fmt(temp, "%s<%i[w5p0]", n); //five chars with leading zeros
        strcat(fname3, temp);
        strcat(fname3, ext);
        if (!FileExists (fname3, &size)) break;
        if (size < 0) break;
        DeleteFile (fname3);
        n++;
    }
    SetWaitCursor(0);
}

int CVICALLBACK cbGetExcelFilename (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData; 
	char fname[500], defaultpath[500];
	char *default_extensions = "*.xls";
	int retval;
	
	switch (event)
		{
		case EVENT_COMMIT:
	
			str_get_path_for_my_documents(defaultpath); 
	
			retval = FileSelectPopup (defaultpath, "*.xls", default_extensions, "Open Data File", VAL_SAVE_BUTTON, 0, 1, 1, 1, fname);
			if (retval <= 0)			
				return 0;
			
			if (retval == 1) {	//overwrite existing file
				retval = GCI_ShutdownExcelApp ();
				RemoveFileIfExists (fname);
				DeleteImageSequence (fname);
			}
			
			SetCtrlVal(panel, AUTOPNL_FNAME, fname);
			
			break;
		}
		
	return 0;
}

int CVICALLBACK cbExcelVisible (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData;      
	
	switch (event)
		{
		case EVENT_COMMIT:
			cell_finder_excel_visible(cf, panel, control);

			break;
		}
	return 0;
}

////////////////////////////////////////////////////////////////////////
//Dish and region functions

int CVICALLBACK cbCellsRegion (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData; 

	switch (event)
	{
		case EVENT_COMMIT:
		
			SetCtrlAttribute (cf->automatic_panel, AUTOPNL_CUSTOM_ROI, ATTR_CMD_BUTTON_COLOR, VAL_GRAY);

			cell_finder_set_dish_region(cf, control);
			
			break;
	}
	
	return 0;
}

int CVICALLBACK cbCustomRegionOfInterest (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData; 
	  
	switch (event)
	{
		case EVENT_COMMIT:
		
			cell_finder_grey_dish_region_buttons(cf);
		
			region_of_interest_panel_display(cf->roi);
		
			SetCtrlAttribute (panel, control, ATTR_CMD_BUTTON_COLOR, VAL_BLUE);
			break;
	}
	
	return 0;
}

int CVICALLBACK cbSetDishCentre (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData; 
	int pnl, tab1, tab2, easyTabCtrl;
	int turretPos;
	char obj_data[30]="";
	
	switch (event)
		{
		case EVENT_COMMIT:
			pnl = FindAndLoadUIR(0, "cell_finding_ui.uir", DISH_PANEL);
			tab1 = FindAndLoadUIR(pnl, "cell_finding_ui.uir", DISH_CENTR);
			tab2 = FindAndLoadUIR(pnl, "cell_finding_ui.uir", DISH_RAD);
		
			if ( InstallCtrlCallback (pnl, DISH_PANEL_OK, cbDishCentre_close, cf) < 0)
				return 0;
	
			if ( InstallCtrlCallback (tab1, DISH_CENTR_SET_X_LEFT, cbDishCentre_pointSetup, cf) < 0)
				return 0;
	
			if ( InstallCtrlCallback (tab1, DISH_CENTR_SET_X_RIGHT, cbDishCentre_pointSetup, cf) < 0)
				return 0;
	
			if ( InstallCtrlCallback (tab1, DISH_CENTR_SET_Y_TOP, cbDishCentre_pointSetup, cf) < 0)
				return 0;
	
			if ( InstallCtrlCallback (tab1, DISH_CENTR_SET_Y_BOTTOM, cbDishCentre_pointSetup, cf) < 0)
				return 0;
	
			if ( InstallCtrlCallback (tab1, DISH_CENTR_OK, cbDishCentre_ok, cf) < 0)
				return 0;
	
			if ( InstallCtrlCallback (tab2, DISH_RAD_TURRETPOS, cbDish_setTurret, cf) < 0)
				return 0;
	
			if ( InstallCtrlCallback (tab2, DISH_RAD_GO_CENTRE, cbGotoDishCentre, cf) < 0)
				return 0;
	
			if ( InstallCtrlCallback (tab2, DISH_RAD_SET_RADIUS, cbDish_setRadius, cf) < 0)
				return 0;
	
			if ( InstallCtrlCallback (tab2, DISH_RAD_OK, cbDishRadius_ok, cf) < 0)
				return 0;
	
			easyTabCtrl = EasyTab_ConvertFromCanvas (pnl, DISH_PANEL_CANVAS);
			EasyTab_AddPanels (pnl, easyTabCtrl, 1, tab1, tab2,0);

			GCI_ImagingGetTurretPos(&turretPos);
			SetCtrlVal (tab2, DISH_RAD_TURRETPOS, turretPos);
			GCI_ImagingGetObjectiveString(obj_data);
			SetCtrlVal (tab2, DISH_RAD_OBJ_STRING, obj_data);
			SetCtrlVal(tab2, DISH_RAD_RADIUS, cf->safe_radius[turretPos]);

			DisplayPanel(pnl);
			GCI_ImagingSetFluorMode(1);	//Ensure bright field mode
			break;
		}
	return 0;
}

static int focus_on_point(cell_finder *cf, char* message)
{
	int msgPnl, pnl, ctrl;
	FIBITMAP *dib = NULL;

	//Get the user to define an xy point
	
	gci_camera_set_live_mode(cf->camera);
	gci_camera_activate_live_display(cf->camera);
		
	GCI_ImagingStageJoystickOn();
		
	msgPnl = FindAndLoadUIR(0, "cell_finding_ui.uir", MSGPANEL);
	
	SetCtrlVal(msgPnl, MSGPANEL_TEXTMSG, message);
	DisplayPanel(msgPnl);
		
	while (1) {
		ProcessSystemEvents();
		
		GetUserEvent (0, &pnl, &ctrl);
		if ((pnl == msgPnl) && (ctrl == MSGPANEL_OK))
			break;

		//For some unfathomable reason the presence of the message panel 
		//prevents camera timer tick events, so get and display an image.
		dib = gci_camera_get_image(cf->camera, NULL); 
		gci_camera_display_image(cf->camera, dib, NULL);
	}
		
	DiscardPanel(msgPnl);
	
	//Restore camera settings
	gci_camera_set_snap_mode(cf->camera);
	
	return 0;
}

static void swap(double *val1, double *val2)
{
	double temp;
	
	temp = *val1;
	*val1 = *val2;
	*val2 = temp;
}

int CVICALLBACK cbDishCentre_pointSetup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData; 
	double x, y, xStart, yStart, xEnd, yEnd, dummy;

	switch (event)
		{
		case EVENT_COMMIT:
			switch (control) {
				case DISH_CENTR_SET_X_LEFT:
					focus_on_point(cf, "Focus on leftmost edge of dish using joystick");
					GCI_ImagingStageReadCoords(&xStart, &y, &dummy);
					SetCtrlVal(panel, DISH_CENTR_X_LEFT, xStart);
				break;
				
				case DISH_CENTR_SET_X_RIGHT:
					focus_on_point(cf, "Focus on rightmost edge of dish using joystick");
					GCI_ImagingStageReadCoords(&xEnd, &y, &dummy);
					SetCtrlVal(panel, DISH_CENTR_X_RIGHT, xEnd);
				break;
				
				case DISH_CENTR_SET_Y_TOP:
					focus_on_point(cf, "Focus on far edge of dish using joystick");
					GCI_ImagingStageReadCoords(&x, &yStart, &dummy);
					SetCtrlVal(panel, DISH_CENTR_Y_TOP, yStart);
				break;
				
				case DISH_CENTR_SET_Y_BOTTOM:
					focus_on_point(cf, "Focus on near edge of dish using joystick");
					GCI_ImagingStageReadCoords(&x, &yEnd, &dummy);
					SetCtrlVal(panel, DISH_CENTR_Y_BOTTOM, yEnd);
				break;
			}
			
			//Indicate that this point has been defined at least once
			SetCtrlAttribute (panel, control, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);

			break;
		}
	return 0;
}

int CVICALLBACK cbDishCentre_pointChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:

			//Indicate that this point has been defined at least once
			switch (control) {
				case DISH_CENTR_X_LEFT:
					SetCtrlAttribute (panel, DISH_CENTR_SET_X_LEFT, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);
				break;
				
				case DISH_CENTR_X_RIGHT:
					SetCtrlAttribute (panel, DISH_CENTR_SET_X_RIGHT, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);
				break;
				
				case DISH_CENTR_Y_TOP:
					SetCtrlAttribute (panel, DISH_CENTR_SET_Y_TOP, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);
				break;
				
				case DISH_CENTR_Y_BOTTOM:
					SetCtrlAttribute (panel, DISH_CENTR_SET_Y_BOTTOM, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);
				break;
			}
			break;
		}
	return 0;
}

int CVICALLBACK cbDishCentre_ok (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData; 
	double x1, x2, y1, y2;
	double xMin, xMax, yMin, yMax, zMin, zMax;
	
	switch (event)
		{
		case EVENT_COMMIT:
			//Apply new dish centre
			GetCtrlVal(panel, DISH_CENTR_X_LEFT, &x1);
			GetCtrlVal(panel, DISH_CENTR_X_RIGHT, &x2);
			GetCtrlVal(panel, DISH_CENTR_Y_TOP, &y1);
			GetCtrlVal(panel, DISH_CENTR_Y_BOTTOM, &y2);

			if (x1 > x2) {
				swap (&x1, &x2);
				SetCtrlVal(panel, DISH_CENTR_X_LEFT, x1);
				SetCtrlVal(panel, DISH_CENTR_X_RIGHT, x2);
			}
			if (y1 > y2) {
				swap (&y1, &y2);
				SetCtrlVal(panel, DISH_CENTR_Y_TOP, y1);
				SetCtrlVal(panel, DISH_CENTR_Y_BOTTOM, y2);
			}

			//printf("x = %.0f, y = %.0f\n", cf->dish_centre_x, cf->dish_centre_y);
			cf->dish_centre_x = x1 + (x2-x1)/2;
			cf->dish_centre_y = y1 + (y2-y1)/2;
			//printf("x = %.0f, y = %.0f\n", cf->dish_centre_x, cf->dish_centre_y);

			//Correct for XY datum position - we want it relative to the centre of travel.
			GCI_ImagingStageGetLimits(&xMin, &xMax, &yMin, &yMax, &zMin, &zMax);
			cf->dish_centre_x -= (xMin + xMax)/2;
			cf->dish_centre_y -= (yMin + yMax)/2;
			cell_finder_save_dish_centre(cf);
			cell_finder_set_safe_region(cf);
			break;
		}
	return 0;
}

int CVICALLBACK cbDish_setTurret (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData; 
	int turretPos, raiseTurret=1;
	char obj_data[30]="";
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal (panel, control, &turretPos);
			GCI_ImagingSetObjective(turretPos, raiseTurret);

			GCI_ImagingGetObjectiveString(obj_data);
			SetCtrlVal (panel, DISH_RAD_OBJ_STRING, obj_data);
			SetCtrlVal(panel, DISH_RAD_RADIUS, cf->safe_radius[turretPos]);
			break;
		}
	return 0;
}

int CVICALLBACK cbDish_setRadius (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData; 
	double x, y, dummy, xDiff, yDiff, radius;
	double xMin, xMax, yMin, yMax, zMin, zMax;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GCI_ImagingStageReadCoords(&x, &y, &dummy);

			GCI_ImagingStageGetLimits(&xMin, &xMax, &yMin, &yMax, &zMin, &zMax);
			xDiff = x - cf->dish_centre_x+(xMin+xMax)/2;
			yDiff = y - cf->dish_centre_y+(yMin+yMax)/2;
			radius = sqrt((xDiff*xDiff) + (yDiff*yDiff));
			SetCtrlVal(panel, DISH_RAD_RADIUS, radius);
			break;
		}
	return 0;
}

int CVICALLBACK cbDishRadius_ok (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData; 
	int obj;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GCI_ImagingGetObjective(&obj);
			if (obj < 0 || obj > 9) break;
			
			GetCtrlVal(panel, DISH_RAD_RADIUS, &cf->safe_radius[obj]);
			cell_finder_save_dish_centre(cf);
			cell_finder_set_safe_region(cf);
			break;
		}
	return 0;
}

int CVICALLBACK cbDishCentre_close (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData; 
	
	switch (event)
		{
		case EVENT_COMMIT:
			DiscardPanel(panel);

			GCI_ImagingSetFluorMode(0);	//Ensure fluorescence mode
			break;
		}
	return 0;
}

int CVICALLBACK cbGotoDishCentre (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double x, y, z;
	double xMin, xMax, yMin, yMax, zMin, zMax;
	cell_finder *cf = (cell_finder *) callbackData; 
	  
	switch (event)
		{
		case EVENT_COMMIT:
			//Correct for XY datum position - dish_centre_x,y are relative to the centre of travel.
			GCI_ImagingStageGetLimits(&xMin, &xMax, &yMin, &yMax, &zMin, &zMax);
			region_of_interest_goto_stage_xy(cf->roi, cf->dish_centre_x+(xMin+xMax)/2, cf->dish_centre_y+(yMin+yMax)/2, &z);
			break;
		}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
static void cellfinder_set_cell_diameter(cell_finder *cf)
{
	double min, max;
	
	GetCtrlVal(cf->automatic_panel, AUTOPNL_MIN_CELL_DIAMETER, &min);
	GetCtrlVal(cf->automatic_panel, AUTOPNL_MAX_CELL_DIAMETER, &max);
	
	cell_finder_set_cell_diameter(cf, min, max); 
}

int CVICALLBACK cellfinder_on_start_autoscan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char text[100], filepath[500];
	int fsize, ret, multithread;
	cell_finder *cf = (cell_finder *) callbackData;       

	switch (event)
	{
		case EVENT_COMMIT:

			GetCtrlAttribute (cf->automatic_panel, AUTOPNL_RASTER_START, ATTR_LABEL_TEXT, text); 
		
			if(strcmp(text, "Pause") == 0) {
				
				cf->pause_scan = 1;
				
				SetCtrlAttribute (cf->automatic_panel, AUTOPNL_RASTER_START, ATTR_LABEL_TEXT, "Resume");
			
			}
			else if(strcmp(text, "Resume") == 0) {
			
				cf->pause_scan = 0;
				
				SetCtrlAttribute (cf->automatic_panel, AUTOPNL_RASTER_START, ATTR_LABEL_TEXT, "Pause");
			}
			else {
				GetCtrlVal(panel, AUTOPNL_FNAME, filepath);
		
				if (strcmp(filepath,"") == 0) {
				
					MessagePopup ("Warning", "You must select a filename to save data to.");
					return 0;
				}
		
				if (FileExists (filepath, &fsize)) {
	
					ret = ConfirmPopup ("Warning", "File selected to save data already exists. Do you wish to overwrite it?");
			
					if(!ret)
						return 0;
						
					GCI_ShutdownExcelApp ();
					RemoveFileIfExists (filepath);
					
					DeleteImageSequence (filepath);
				}
		
				cf->pause_scan = 0;
				cf->abort_scan = 0;
				cf->current_frame = 0;
				cf->number_of_cells = 0;
				cf->cell_diameter_sumation = 0;

				cellfinder_set_cell_diameter(cf);	//in case objective or binning has changed
				cell_finder_set_stepsize(cf);   	//in case objective or binning has changed
				
				gci_cellmap_clear(cf->map);
				gci_cellmap_new_file(cf->map, filepath);

				#ifdef __MICROFOCUS_APP__		
				if (cf->show_mosaic) {
					CreateMosaic(cf);
					mosaic_window_update(cf->mosaic_window);
				}
				#endif
				
				GCI_ImagingDisableAllPanels(1);
				GCI_ElectronBeamSourceDisableAllPanels(1);
				GCI_GPscope_DisablePanel(1);
				cell_revisit_disable_ui(cf, 1);
				cell_finder_dim_ui_controls(cf, 1); // Discourage button pushes 
		
				SetCtrlAttribute (cf->automatic_panel, AUTOPNL_RASTER_START, ATTR_LABEL_TEXT, "Pause");
				SetCtrlAttribute (cf->automatic_panel, AUTOPNL_RASTER_STOP, ATTR_DIMMED, 0);
				SetCtrlAttribute (cf->experiment_panel, EXPT_PANEL_CLOSE, ATTR_DIMMED, 1);

				if(cf->show_processing) 
					CreateProcessingWindow(cf);
				
				#ifdef __MICROFOCUS_APP__

				if (ConfirmPopup("", "Do you want to set up or modify the focal plane?")) {
					ret = region_of_interest_show_focus_options(cf->roi);
					if (ret < 0) return ret;
				}

				GetCtrlVal (cf->scan_panel, SCAN_PANEL_MULTITHREAD, &multithread);
				if (multithread) {
					cell_finder_autoscan_multithread(cf);
				}
				else {
					SetCtrlAttribute(cf->automatic_panel, AUTOPNL_TIMER, ATTR_ENABLED, 1); 
					cell_finder_autoscan(cf);
				}

				cell_revisit_display(cf);	//Update the revisiting panel if it's visible

				#else
				cell_finder_process_frame(cf, 0, 0, 0.0, 0.0); 
				#endif

				cell_finder_dim_ui_controls(cf, 0);
				GCI_ImagingDisableAllPanels(0);
				GCI_ElectronBeamSourceDisableAllPanels(0);
				GCI_GPscope_DisablePanel(0);
				cell_revisit_disable_ui(cf, 0);

				SetCtrlAttribute (cf->experiment_panel, EXPT_PANEL_CLOSE, ATTR_DIMMED, 0);
				SetCtrlAttribute (cf->automatic_panel, AUTOPNL_RASTER_START, ATTR_LABEL_TEXT, "Start");
				SetCtrlAttribute (cf->automatic_panel, AUTOPNL_RASTER_STOP, ATTR_DIMMED, 1);
			}
			
			break;
		}
		
	return 0;
}


int CVICALLBACK cbAbortScan (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:

			cf->pause_scan = 0;
			cf->abort_scan = 1;

			SetCtrlAttribute (cf->automatic_panel, AUTOPNL_RASTER_START, ATTR_LABEL_TEXT, "Start");
			SetCtrlAttribute (cf->automatic_panel, AUTOPNL_RASTER_STOP, ATTR_DIMMED, 1);
			SetCtrlAttribute (cf->automatic_panel, AUTOPNL_RASTER_START, ATTR_DIMMED, 0);
			
			SetCtrlAttribute(cf->automatic_panel, AUTOPNL_TIMER, ATTR_ENABLED, 0); 

			break;
		}
	return 0;
}

int CVICALLBACK onWindowUpdateTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData;      

	switch (event)
		{
		case EVENT_TIMER_TICK:

			gci_cellmap_plot_cells(cf->map);
			
			if (cf->show_mosaic)
				mosaic_window_update(cf->mosaic_window);

			break;
		}
	return 0;
}

int CVICALLBACK cbAutoClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData; 

	switch (event)
	{
		case EVENT_COMMIT:
		
			cell_finder_hide(cf);
			
			break;
	}
		
	return 0;
}

/////////////////////////////////////////////////////////////////////////
//Cell criteria

int CVICALLBACK cbCellsSetCriteria (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:

			read_or_write_cfwindow_registry_settings(cf, cf->criteria_panel, 0);
			DisplayPanel(cf->criteria_panel);

			break;
		}
	return 0;
}

int CVICALLBACK cbCellsPreProcess (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData; 
	int pre_process;
	
	switch (event)
	{
		case EVENT_COMMIT:
		
			GetCtrlVal (cf->criteria_panel, CRITERIA_PRE_PROCESS, &pre_process);
			
			if (cf) {
				cf->pre_process = pre_process;
				
				SetCtrlAttribute (cf->criteria_panel, CRITERIA_THRESH, ATTR_DIMMED, !pre_process);
				SetCtrlAttribute (cf->criteria_panel, CRITERIA_BOX_WIDTH, ATTR_DIMMED, !pre_process);
				SetCtrlAttribute (cf->criteria_panel, CRITERIA_MAX_WIDTH, ATTR_DIMMED, !pre_process);
				SetCtrlAttribute (cf->criteria_panel, CRITERIA_TEXTMSG_9, ATTR_DIMMED, !pre_process);
				SetCtrlAttribute (cf->criteria_panel, CRITERIA_TEXTMSG_4, ATTR_DIMMED, !pre_process);
				SetCtrlAttribute (cf->criteria_panel, CRITERIA_TEXTMSG_6, ATTR_DIMMED, !pre_process);
			}
			
			break;
	}
	
	return 0;
}

int CVICALLBACK cbImOverlap (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData; 

	double percentage;
	
	switch (event)
	{
		case EVENT_COMMIT:
		
			GetCtrlVal (cf->criteria_panel, CRITERIA_IMAGE_OVERLAP, &percentage);
			
			if (cf) {
				cell_finder_set_frame_overlap(cf, percentage); 
				cell_finder_set_stepsize(cf);
			}
			
			break;
	}
	
	return 0;
}

int CVICALLBACK cbOnThresholdPercentage (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:

			if (cf) {
				#ifdef __MICROFOCUS_APP__
				gci_camera_set_snap_mode(cf->camera);   
				cell_finder_camera_acquire_image(cf);
				#endif
				
				if (cf->frame_image <=0) break;	//no valid image
				GetCtrlVal(panel, control, &(cf->threshold_percentage) );
				cell_finder_set_threshold_percentage(cf, cf->threshold_percentage);
				cell_finder_test_threshold(cf);
			}
			
			break;
		}
	return 0;
}

int CVICALLBACK cbApproxBoxWidth (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData; 

	double approx_width_percentage;

	switch (event)
	{
		case EVENT_COMMIT:
		
			GetCtrlVal (cf->criteria_panel, CRITERIA_BOX_WIDTH, &approx_width_percentage); 
		
			if (cf)
				cell_finder_set_box_width_percentage_larger(cf, approx_width_percentage); 
			
			break;
	}
	
	return 0;
}
int CVICALLBACK cbMaxSize (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double width, height;
	cell_finder *cf = (cell_finder *) callbackData; 

	switch (event)
	{
		case EVENT_COMMIT:
		
			if (cf) {
				GetCtrlVal (cf->criteria_panel, CRITERIA_MAX_WIDTH, &width);
			
				cell_finder_set_cell_max_width_and_height_percentage(cf, width, width); 
			}
			
			break;
	}
	
	return 0;
}

int CVICALLBACK cbCriteriaTest (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData;  

	switch (event)
		{
		case EVENT_COMMIT:
			//Test criteria by processing the current image
			gci_camera_set_snap_mode(cf->camera);   
			cell_finder_camera_acquire_image(cf);
			
			SetWaitCursor(1);
			cf->number_of_cells = 0;
			cf->show_processing = 1;
			CreateProcessingWindow(cf);
			cellfinder_set_cell_diameter(cf);	//in case objective or binning has changed
			cell_finder_process_current_image(cf);
			SetWaitCursor(0);

			//cell_finder_process_frame(cf, 0, 0, 0.0, 0.0);
			//cell_finder_display_ipi_image(cf->processing_window, cf->frame_display_image, "Processing"); 
			
			//Restore the "show processing option"
			//cbCellsSetShowProcessing (cf->scan_panel, SCAN_PANEL_SHOW_PROCESSING, EVENT_COMMIT, cf, 0, 0);
			break;
		}
	return 0;
}

int CVICALLBACK cbCriteriaHelp (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    char buffer[MAX_PATHNAME_LEN], path[MAX_PATHNAME_LEN];
    
    switch (event)
        {
        case EVENT_COMMIT:
          
            FindPathForFile("Cell Analysis Parameters.doc", path);
            sprintf(buffer, "explorer.exe ""%s""", path); 
            LaunchExecutable (buffer); // fakes a double click on our file from within nt explorer.
			break;
		}
	return 0;
}

int CVICALLBACK cbCHARMEParams (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData;      

	switch (event)
		{
		case EVENT_COMMIT:
			cellfinder_set_cell_diameter(cf);	//in case objective or binning has changed
			CHARM_displayParameterPanel();
			break;
		}
	return 0;
}

int CVICALLBACK cbCHARMEOptimizer (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData;      
	double microns_per_pixel; 
	int binning;
	
	switch (event)
		{
		case EVENT_COMMIT:
			
			cellfinder_set_cell_diameter(cf);	//in case objective or binning has changed
			if (cf->frame_image<=0) cell_finder_camera_acquire_image(cf);
			microns_per_pixel = gci_camera_get_microns_per_pixel(cf->camera);
			binning = gci_camera_get_binning_mode(cf->camera);
			if (binning < 1) binning = cf->camera->_camera_window->binning_size;	
			if (binning < 1) binning = 1;	//avoid crashing if binning not implemented for camera
			CHARM_displayParameterOptimizerPanel (cf->frame_image, 0, microns_per_pixel*binning);
			
			break;
		}
	return 0;
}

int CVICALLBACK cbCriteriaClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData; 

	switch (event)
	{
		case EVENT_COMMIT:
		
			read_or_write_cfwindow_registry_settings(cf, cf->criteria_panel, 1);
			HidePanel(cf->criteria_panel);
			
			break;
	}
		
	return 0;
}

///////////////////////////////////////////////////////////////////////////
//Scan Options panel

int CVICALLBACK cbScanOptions (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:

			read_or_write_cfwindow_registry_settings(cf, cf->scan_panel, 0);
			DisplayPanel(cf->scan_panel); 

			break;
		}
	return 0;
}

int CVICALLBACK cbScanOptionsClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:

			read_or_write_cfwindow_registry_settings(cf, cf->scan_panel, 1);
			HidePanel(cf->scan_panel);

			break;
		}
	return 0;
}

int CVICALLBACK cbOnCameraBitModeChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int bpp, live;
	cell_finder *cf = (cell_finder *) callbackData;  
	char filepath[MAX_PATHNAME_LEN];

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &bpp);   
/* ///
			gci_camera_set_data_mode(cf->camera, bpp);

			if (bpp == 16)
				FindPathForFile(CHARM_FILE_12BIT, filepath);
			else 
				FindPathForFile(CHARM_FILE_8BIT,  filepath);
				
			// Setup CHARM module
			if (ConfirmPopup ("Cell Finding", "Do you wish to load the saved CHARM parameters for this bit mode?"))
				if (CHARM_LoadParameters (filepath)<0) 
					MessagePopup("Cell Finding", "Failed to load the cell finding parameters for this bit mode");
*/
			live = gci_camera_is_live_mode(cf->camera);
    		if(live) gci_camera_set_snap_mode(cf->camera);
    		
			cell_finder_set_bitmode(cf, bpp, 1);

			if (live){
    			gci_camera_set_live_mode(cf->camera);
    			gci_camera_activate_live_display(cf->camera);
  			}
  			else 
  				gci_camera_snap_image(cf->camera);

			break;
		}
	return 0;
}

int CVICALLBACK cbCellsSetCorrectBackground (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &(cf->correct_background) );
			
			//If images are to be corrected during the scan check if we have  //201005
			//black and white reference images
			if (cf->correct_background) {
				if (!GCI_RefImagesOK()) {
					cf->correct_background = 0;
					SetCtrlVal(panel, control, 0 );
					MessagePopup("Object Finding", "Please acquire reference images first.");
				}
			}
			break;
		}
	return 0;
}

int CVICALLBACK cbCellsShowImages (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData;  

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &(cf->show_images) );  

			break;
		}
	return 0;
}

int CVICALLBACK cbCellsSetShowProcessing (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData;  

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &(cf->show_processing) );  

			if (!cf->show_processing) {
				if (cf->processing_window != NULL) {
					GCI_ImagingWindow_Destroy(cf->processing_window);
					cf->processing_window = NULL;
				}
			}
			else 
				CreateProcessingWindow(cf);
			
			break;
		}
	return 0;
}

int CVICALLBACK cbCellsSetShowMosaic (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData;  

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &(cf->show_mosaic) );  

			if (!cf->show_mosaic) {
				if (cf->mosaic_window != NULL) {
					mosaic_window_destroy(cf->mosaic_window);
					cf->mosaic_window = NULL;
				}
			}
			else 
				CreateMosaic(cf);
			
			break;
		}
	return 0;
}

int CVICALLBACK cbCellsFindObjects (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData;  

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &(cf->find_objects) );  

			//Make sure control combinations are sensible
			if (!cf->find_objects) {
				SetCtrlVal(cf->scan_panel, SCAN_PANEL_SHOW_PROCESSING, 0);
				cbCellsSetShowProcessing (cf->scan_panel, SCAN_PANEL_SHOW_PROCESSING, EVENT_COMMIT, cf, 0, 0);

				SetCtrlVal(cf->options_panel, OPTION_PNL_NUCEII, 0);
				SetCtrlVal(cf->options_panel, OPTION_PNL_CYTOPLASM, 0);
				SetCtrlVal(cf->options_panel, OPTION_PNL_IRRADIATE_ALL, 0);
				SetCtrlVal(cf->options_panel, OPTION_PNL_IRRADIATE_SINGLE, 0);
				SetCtrlVal(cf->options_panel, OPTION_PNL_IRRADIATE_MULTI_CELLS, 0);
			}
			else {  //Set some sensible defaults
				SetCtrlVal(cf->scan_panel, SCAN_PANEL_SHOW_PROCESSING, 1);
				cbCellsSetShowProcessing (cf->scan_panel, SCAN_PANEL_SHOW_PROCESSING, EVENT_COMMIT, cf, 0, 0);

				SetCtrlVal(cf->options_panel, OPTION_PNL_NUCEII, 1);
				SetCtrlVal(cf->options_panel, OPTION_PNL_IRRADIATE_ALL, 1);
			}
			
			break;
		}
	return 0;
}

int CVICALLBACK cbCellsSaveImages (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	cell_finder *cf = (cell_finder *) callbackData;  

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &(cf->save_images) );  

			break;
		}
	return 0;
}

int CVICALLBACK cbCellsNotAvailable (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			SetCtrlVal(panel, control, 0);
			MessagePopup("Object Finding Options", "Sorry, this option is not available yet");
			break;
		}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

