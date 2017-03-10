#include "easytab.h"
#include "radioGroup.h"
#include <analysis.h>
#include <userint.h>

#include "camera\gci_camera.h"
#include "stage\stage.h"
#include "StagePlate.h"

#include "RegionOfInterest.h"
#include "RegionOfInterest_ui.h"

#include "string_utils.h"
#include "GL_CVIRegistry.h"
#include "gci_utils.h"

#include "microscope.h"

#include <utility.h>
#include <limits.h>

#ifdef _CVI_
#undef DBL_MAX
#undef DBL_MIN
#define DBL_MAX 999999999999999
#define DBL_MIN -999999999999999 
#endif


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

static void region_of_interest_panel_set_region_from_stage_plate(region_of_interest* roi)
{
	int width, height;
	RECT rect;

	stage_plate_get_entire_region_of_interest(roi->ms->_stage_plate_module, &rect);
			
	width = rect.right - rect.left + 1;
	height = rect.bottom - rect.top + 1;

	region_of_interest_set_region(roi, rect.left, rect.top, width, height);
}

static void region_of_interest_stage_plate_changed (StagePlateModule* stage_plate_module, int pos, void *data) 
{
	region_of_interest* roi = (region_of_interest*) data;
	int ret;

	if(stage_plate_module == NULL)
		return;

	if(!stage_current_plate_is_valid(stage_plate_module))
		return;
	
	if(!ui_module_panel_is_visible(UIMODULE_CAST(roi), roi->region_panel))
		return;

	ret = GCI_ConfirmPopup("Info", IDI_INFORMATION, "The stage plate has been changed.\n"
		"Would you like to used the plate as a new region of interest");

	if(ret > 0) 
		region_of_interest_panel_set_region_from_stage_plate(roi);
}

static void OnMasterCameraChanged (Microscope* microscope, void *data)
{
	region_of_interest* roi = (region_of_interest*) data;

//	printf("OnMasterCameraChanged called for roi: 0x%x %d\n", roi, roi->_master_camera_signal_id);

    roi->camera = MICROSCOPE_MASTER_CAMERA(roi->ms); 

//	printf("Completed roi: 0x%x\n", roi);
}

region_of_interest* region_of_interest_selection_new(GciCamera *camera)
{
	region_of_interest *roi = (region_of_interest*) malloc (sizeof(region_of_interest));		
	
	roi->id = 1;
	
	roi->region_panel = -1;
	roi->region_panel_tab1 = -1;
	roi->region_panel_tab2 = -1;
	roi->region_panel_tab3 = -1;
	roi->region_panel_tab4 = -1;
	
  	ui_module_constructor(UIMODULE_CAST(roi), "Region Of Interest");
	
  	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(roi), "RegionSelected", VOID_DOUBLE_DOUBLE_DOUBLE_DOUBLE_MARSHALLER); 
  		
  	roi->focal_plane_valid=0;
  	roi->focal_point_a=0.0;
  	roi->focal_point_b=0.0;
  	roi->focal_point_c=1.0;
  	roi->focal_point_c_original=roi->focal_point_c;
  	
	roi->ms = microscope_get_microscope();
  	roi->camera = camera; 
  	roi->stage = microscope_get_stage(roi->ms); 
	
	if(roi->ms->_stage_plate_module != NULL) {
		roi->_stage_plate_signal_plate_changed_signal_id = stage_plate_signal_plate_changed_handler_connect(roi->ms->_stage_plate_module,
																 region_of_interest_stage_plate_changed, roi); // Signal: StagePlateChanged
	}

	roi->_master_camera_changed_signal_id = microscope_master_camera_changed_handler_connect(roi->ms, OnMasterCameraChanged, roi);
//	printf("roi: 0x%x %d conneted to master camera change signal.\n", roi, roi->_master_camera_signal_id);

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

void region_of_interest_clear_button_state(region_of_interest* roi)
{
	SetCtrlAttribute (roi->region_panel_tab1, ROI_TAB1_SET_START, ATTR_CMD_BUTTON_COLOR, VAL_GRAY);
	SetCtrlAttribute (roi->region_panel_tab1, ROI_TAB1_SET_END, ATTR_CMD_BUTTON_COLOR, VAL_GRAY);

	SetCtrlAttribute (roi->region_panel_tab2, ROI_TAB2_SET_Y_START, ATTR_CMD_BUTTON_COLOR, VAL_GRAY);
	SetCtrlAttribute (roi->region_panel_tab2, ROI_TAB2_SET_X_START, ATTR_CMD_BUTTON_COLOR, VAL_GRAY);
	SetCtrlAttribute (roi->region_panel_tab2, ROI_TAB2_SET_Y_END, ATTR_CMD_BUTTON_COLOR, VAL_GRAY);
	SetCtrlAttribute (roi->region_panel_tab2, ROI_TAB2_SET_X_END, ATTR_CMD_BUTTON_COLOR, VAL_GRAY);
	
	SetCtrlAttribute (roi->region_panel_tab2, ROI_TAB3_SET_CENTRE, ATTR_CMD_BUTTON_COLOR, VAL_GRAY);
	SetCtrlAttribute (roi->region_panel_tab2, ROI_TAB3_SET_RADIUS, ATTR_CMD_BUTTON_COLOR, VAL_GRAY);
}

int region_of_interest_panel_init(region_of_interest* roi)
{
	int easyTabCtrl;
	
	//There are three methods of setting up the ROI. We use a tab control for easy selection.
	
	if (roi->region_panel >= 0)
		return 0;	//Already loaded
	
	roi->region_panel = ui_module_add_panel(UIMODULE_CAST(roi), "RegionOfInterest_ui.uir", ROI_PANEL, 1);
	
	roi->region_panel_tab1 = ui_module_add_panel_as_child(UIMODULE_CAST(roi), "RegionOfInterest_ui.uir", ROI_TAB1, roi->region_panel); 
	roi->region_panel_tab2 = ui_module_add_panel_as_child(UIMODULE_CAST(roi), "RegionOfInterest_ui.uir", ROI_TAB2, roi->region_panel); 
	roi->region_panel_tab3 = ui_module_add_panel_as_child(UIMODULE_CAST(roi), "RegionOfInterest_ui.uir", ROI_TAB3, roi->region_panel); 
	roi->region_panel_tab4 = ui_module_add_panel_as_child(UIMODULE_CAST(roi), "RegionOfInterest_ui.uir", ROI_TAB4, roi->region_panel); 
	

	//Install callbacks such that they cal access roi
  	InstallCtrlCallback (roi->region_panel, ROI_PANEL_OK, onRoiOk, roi); 
	InstallCtrlCallback (roi->region_panel, ROI_PANEL_OK_2, onRoiCancel, roi); 
	InstallCtrlCallback (roi->region_panel, ROI_PANEL_PLATE_COORDS, onGetCoordsFromStagePlate, roi); 
	
  	InstallCtrlCallback (roi->region_panel, ROI_PANEL_SET_FOCAL_PLANE, cbRoiSetFocalPlane , roi);
  	InstallCtrlCallback (roi->region_panel, ROI_PANEL_SET_F_OFFSET, cbRoiSetFocalPlaneOffset , roi);
  	
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

	EasyTab_SetTabAttribute(roi->region_panel, easyTabCtrl,
		roi->region_panel_tab1, ATTR_EASY_TAB_TAB_COLOR, MICROSCOPE_GRAY);

	EasyTab_SetTabAttribute(roi->region_panel, easyTabCtrl,
		roi->region_panel_tab2, ATTR_EASY_TAB_TAB_COLOR, MICROSCOPE_GRAY);

	EasyTab_SetTabAttribute(roi->region_panel, easyTabCtrl,
		roi->region_panel_tab3, ATTR_EASY_TAB_TAB_COLOR, MICROSCOPE_GRAY);

	EasyTab_SetTabAttribute(roi->region_panel, easyTabCtrl,
		roi->region_panel_tab4, ATTR_EASY_TAB_TAB_COLOR, MICROSCOPE_GRAY);

	if(roi->ms->_stage_plate_module == NULL) {

		// Hide get points button
		SetCtrlAttribute(roi->region_panel, ROI_PANEL_PLATE_COORDS, ATTR_VISIBLE, 0);
	}

	return 0;
}

int region_of_interest_panel_display(region_of_interest* roi, int parent_panel_id)
{
	if (roi->region_panel >= 0) {
		
		int left, top, width;
		
		GetPanelAttribute(parent_panel_id, ATTR_LEFT, &left);
		GetPanelAttribute(parent_panel_id, ATTR_TOP, &top);
		GetPanelAttribute(parent_panel_id, ATTR_WIDTH, &width);
		
		SetPanelPos(roi->region_panel, top, left + width + 10);
		
		//read_or_write_roiwindow_registry_settings(roi->region_panel, 0);  //restore window position
		//InstallPopup(roi->region_panel);
		DisplayPanel(roi->region_panel);
	}
	
	return 0;
}

int region_of_interest_display(region_of_interest* roi)
{
	if (roi->region_panel >= 0) {
		
		//read_or_write_roiwindow_registry_settings(roi->region_panel, 0);  //restore window position
		//InstallPopup(roi->region_panel);
		DisplayPanel(roi->region_panel);
	}
	
	return 0;
}

int region_of_interest_panel_hide(region_of_interest* roi)
{
	if (roi->region_panel >= 0) {
		read_or_write_roiwindow_registry_settings(roi->region_panel, 1);  //save window position
		HidePanel(roi->region_panel);
	}
	
	return 0;
}

void region_of_interest_destroy(region_of_interest* roi)
{
	if (roi->_master_camera_changed_signal_id >= 0)
		microscope_master_camera_changed_handler_disconnect(roi->ms, roi->_master_camera_changed_signal_id);

	if (roi->_stage_plate_signal_plate_changed_signal_id >= 0)
		stage_plate_signal_plate_changed_handler_disconnect(roi->ms->_stage_plate_module, roi->_stage_plate_signal_plate_changed_signal_id);

	ui_module_destroy(UIMODULE_CAST(roi));

	free(roi);
}

		   
int region_of_interest_selected_handler(region_of_interest* roi, REGION_SELECTED_EVENT_HANDLER handler, void *callback_data)
{
	int id = 0;

	if((id = GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(roi), "RegionSelected", handler, callback_data)) == SIGNAL_ERROR) {
		send_microscope_error_text(roi->ms, "Cannot connect signal handler for roi RegionSelected signal");
	}

	return id;
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

		
int region_of_interest_goto_stage_xy(region_of_interest* roi, double x, double y)
{
	int err= 0;
	double z;

	//Move in xy and implement the focal plane if it's been set up
	if (roi->focal_plane_valid) {

		region_of_interest_z_for_xy(roi, x, y, &z);

		if(z_drive_is_part_of_stage(MICROSCOPE_MASTER_ZDRIVE(roi->ms))) {

			// Ok the stage and z drive are one of the same.
			// So we move them at the same time to give a potential speed up.
			if (stage_goto_xyz_position (roi->stage, x, y, z) == STAGE_ERROR){
				return -1;
			}

			z_drive_update_current_position(MICROSCOPE_MASTER_ZDRIVE(roi->ms), z);
		}
		else {
			z_drive_set_position(MICROSCOPE_MASTER_ZDRIVE(roi->ms), z);   
		
			if (stage_goto_xy_position (roi->stage, x, y) == STAGE_ERROR){
				return -1;
			}

			// We need to wait here for the z drive to stop moving 
			z_drive_wait_for_stop_moving (MICROSCOPE_MASTER_ZDRIVE(roi->ms), 2.0);
		}
	}
	else {
		
		if (stage_goto_xy_position (roi->stage, x, y) == STAGE_ERROR){
			return -1;
		}
	}

	return 0;
}


int region_of_interest_goto_stage_xy_advanced(region_of_interest* roi, double x, double y,
											  double wait_time, double z_speed, double z_accel, double start_z)
{
	int err= 0;
	double z, z_wait_time;
	double t, start_time, move_time;
	static double current_z = DBL_MAX;

	if(current_z == DBL_MAX)
		current_z = start_z;

	//Move in xy and implement the focal plane if it's been set up
	if (roi->focal_plane_valid) {
	
		region_of_interest_z_for_xy(roi, x, y, &z);

		// Ok the stage and z drive are one of the same.
		// So we move them at the same time to give a potential speed up.
		if(z_drive_is_part_of_stage(MICROSCOPE_MASTER_ZDRIVE(roi->ms))) {		

			// Ok get the time to move the z direction.
			// If not part of stage assume the device is a fast peizo electric one
			// and so will be must faster than x, y moves of the stage.
			stage_calc_move_time_from_vars(current_z - z, z_speed*1000.0, z_accel*1000.0, &z_wait_time);

			// assume we manage to get to the required z
			current_z = z;

			t = Timer();
			start_time = Timer();

			if (stage_async_goto_xyz_position (roi->stage, x, y, z) == STAGE_ERROR){
				return -1;
			}

			move_time = Timer() - start_time;

			//printf("Move Time %f\n", move_time);

			// Wait for the time calculated my Newtons equations minus the time taken calling
			// the stage move function.
			//printf("Calcs wait time %f\n", max(wait_time, z_wait_time) - move_time);

			Delay(max(wait_time, z_wait_time) - move_time);

			z_drive_update_current_position(MICROSCOPE_MASTER_ZDRIVE(roi->ms), z);

			//printf("Delay Time %f\n", Timer() - move_time - start_time);
		}
		else {

			z_drive_set_position(MICROSCOPE_MASTER_ZDRIVE(roi->ms), z);   
		
			if (stage_goto_xy_position (roi->stage, x, y) == STAGE_ERROR){
				return -1;
			}

			// We need to wait here for the z drive to stop moving 
			z_drive_wait_for_stop_moving (MICROSCOPE_MASTER_ZDRIVE(roi->ms), 2.0);
		}
	}
	else {
		
		start_time = Timer();

		if (stage_async_goto_xy_position (roi->stage, x, y) == STAGE_ERROR){
			return -1;
		}

		move_time = Timer() - start_time;

		Delay(wait_time - move_time);
	}

	return 0;
}

void region_of_interest_set_region(region_of_interest* roi, double left, double top, double width, double height)
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


void region_of_interest_get_region(region_of_interest* roi, double *left, double *top, double *width, double *height)
{
	if(left != NULL)
		*left = roi->left;
	
	if(top != NULL)
		*top = roi->top;
	 
	if(width != NULL)
		*width = roi->width;
	 
	if(height != NULL)
		*height = roi->height;
}



int region_of_interest_setup_focal_plane(region_of_interest* roi, int doItOutside)
{
	double ox, oy, gx, gy;
	unsigned int xPixels, yPixels;
	double frameX, frameY, umperpixel;
	double fx[3], fy[3];
	double z, xVal, yVal, midX, midY, A[9], f[3], coeffs[3]= {0,0,0};
	int i, msgPnl, pnl, ctrl, err, left, top, should_process = 0;
	char msg[100], buffer[500];
	FIBITMAP *dib = NULL;
	CameraState state;
	double zCalc, zMinAllowed, zMaxAllowed, zMin=1.0e10, zMax=-1.0e10;
	int x, y, rangeOK=1, valuesOK=1;
	
	//Ask user to focus on three points and use them to calculate focal plane

//	int was_on = stage_get_joystick_status(roi->stage);

	// Some stage IE LStep dont do Z moves when the joystick
	// is enable so here we work around the bug.
	// It can't go in the stage_move_function as turning off the
	// joystick seems to be a very slow operation.
//	if(was_on) {
//		stage_set_joystick_off(roi->stage);
//	}

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
	
	find_resource("RegionOfInterest_ui.uir", buffer); 
	msgPnl = LoadPanel(0, buffer, MSGPANEL); 
	
	sprintf(msg, "Set Focal Plane - Region %d", roi->id);
	SetPanelAttribute (msgPnl, ATTR_TITLE, msg);
	SetCtrlAttribute (msgPnl, MSGPANEL_CANCEL, ATTR_VISIBLE, 1);	 //Unhide the Cancel button

	should_process = ref_images_should_process(roi->ms->_ri);
	
	SetPanelAttribute (UIMODULE_MAIN_PANEL_ID(roi->ms->_ri), ATTR_DIMMED, 1);

	if(should_process) {	
		ref_images_disable(roi->ms->_ri);
	}

	//Remember camera settings
	gci_camera_save_state(roi->camera, &state); 
	
	//Set to 8 bit binned mode to enable easy focussing
	microscope_set_focusing_mode(roi->ms);	
	
	//Give him the opportunity to focus
	gci_camera_set_live_mode(roi->camera);
	gci_camera_activate_live_display(roi->camera);
		
	for (i=0; i<3; i++) {
	
		stage_set_joystick_off (roi->stage);

		//Move stage to required location
		if (region_of_interest_goto_stage_xy(roi, fx[i], fy[i]))
			break;
		
		stage_set_joystick_on (roi->stage);
		
		sprintf(msg, "Please focus at position %d.", i+1);
		SetCtrlVal(msgPnl, MSGPANEL_TEXTMSG, msg);
		//InstallPopup(msgPnl);
		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
		
		GetPanelAttribute (roi->region_panel, ATTR_LEFT, &left);
		GetPanelAttribute (roi->region_panel, ATTR_TOP, &top);
		SetPanelPos (msgPnl, top+10, left+10);
		DisplayPanel(msgPnl);
		
		while (1) {
			
			ProcessSystemEvents();
			
			GetUserEvent (0, &pnl, &ctrl);
			if (pnl == msgPnl) {
				if (ctrl == MSGPANEL_OK) break;
				if (ctrl == MSGPANEL_CANCEL) break;
			}

			#ifndef THREADED_CAM_AQ
			//For some unfathomable reason the presence of the message panel 
			//prevents camera timer tick events, so get and display an image.
			dib = gci_camera_get_image(roi->camera, NULL); 			
			gci_camera_display_image(roi->camera, dib, NULL);	
			FreeImage_Unload(dib);   
			#endif
		}
		
		if (ctrl == MSGPANEL_CANCEL) break;

		//RemovePopup(msgPnl);
		HidePanel(msgPnl);
	
		stage_get_xy_position (roi->stage, &xVal, &yVal);
		
		z_drive_get_position(MICROSCOPE_MASTER_ZDRIVE(roi->ms), &z);   
	
		//Set up matrices
        A[i*3] = xVal;
        A[i*3+1] = yVal;
        A[i*3+2] = 1.0;
		
        f[i] = z;
	}
	
	SetPanelAttribute (UIMODULE_MAIN_PANEL_ID(roi->ms->_ri), ATTR_DIMMED, 0);

	if(should_process) {		
		ref_images_enable(roi->ms->_ri);
	}
	
	//gci_camera_set_snap_mode(roi->camera);
	gci_camera_restore_state(roi->camera, &state);
	gci_camera_snap_image(roi->camera);
	
	//On Cancel, keep any previous focal plane values
	if (ctrl == MSGPANEL_CANCEL) goto Cancel;

	if ((f[0] == f[1]) && (f[1] == f[2])) {  //It's horizontal
		
		roi->focal_point_a = 0;
		roi->focal_point_b = 0;
		roi->focal_point_c = f[0];
		roi->focal_point_c_original=roi->focal_point_c;
		roi->focal_plane_valid = 1;
		
		goto End;
	}
	
    err = LinEqs (A, f, 3, coeffs);
    
	if (err) {
	
		GCI_MessagePopup("", "I can't calculate the focal plane. Please try again");
		goto Stop;
	}

    roi->focal_point_a = coeffs[0];
    roi->focal_point_b = coeffs[1];
    roi->focal_point_c = coeffs[2];
  	roi->focal_point_c_original=roi->focal_point_c;
	roi->focal_plane_valid = 1;
	//Now z = roi->focal_point_a * x + roi->focal_point_b * y + roi->focal_point_c;
	
	// Check here that we can reach the extremes of Z required at the four corners of the slide
	zMinAllowed = MICROSCOPE_MASTER_ZDRIVE(roi->ms)->_min_microns;
	zMaxAllowed = MICROSCOPE_MASTER_ZDRIVE(roi->ms)->_max_microns;

	// get min and max from four corners
	region_of_interest_z_for_xy(roi, roi->left, roi->top, &zCalc);
	if (zCalc > zMax) zMax = zCalc; if (zCalc < zMin) zMin = zCalc;
	region_of_interest_z_for_xy(roi, roi->left+roi->width, roi->top, &zCalc);
	if (zCalc > zMax) zMax = zCalc; if (zCalc < zMin) zMin = zCalc;
	region_of_interest_z_for_xy(roi, roi->left, roi->top+roi->height, &zCalc);
	if (zCalc > zMax) zMax = zCalc; if (zCalc < zMin) zMin = zCalc;
	region_of_interest_z_for_xy(roi, roi->left+roi->width, roi->top+roi->height, &zCalc);
	if (zCalc > zMax) zMax = zCalc; if (zCalc < zMin) zMin = zCalc;

	// check range
	if (fabs(zMax-zMin) > fabs(zMaxAllowed-zMinAllowed)) {
		GCI_MessagePopup("Error", "The sample is crooked beyond the travel of the Z drive.\nTry to seat the slide/plate better on the stage,\nor scan a smaller area.");
		roi->focal_plane_valid = 0;
		goto Stop;
	}

	// check absolute values
	if (zMin < zMinAllowed || zMax > zMaxAllowed) {
		double zMed = (zMax + zMin) / 2.0;   // we know we need to apply an offset of zMed

		roi->focal_point_c -= zMed;
	  	roi->focal_point_c_original=roi->focal_point_c;

		// the point where z should be zero has to be in the middle of the scan
		x = (int) (roi->left + roi->width/2.0);  // set x in midlle of scan
		y = (int) (roi->top + roi->height/2.0);  // set y in midlle of scan

		// move stage to where we expect z mid point to be and allow manual focus with the hw knob
		gci_camera_set_live_mode(roi->camera);
		gci_camera_activate_live_display(roi->camera);
	
		stage_set_joystick_off (roi->stage);

		//Move stage to required location
		region_of_interest_goto_stage_xy(roi, x, y);
		
		stage_set_joystick_on (roi->stage);
		
		GCI_MessagePopup("Attention!", "The hardware Z drive offset needs to be reset for this sample.\nPlease click OK and then focus at or near this stage position using the hardware knob or buttons.");

		SetCtrlVal(msgPnl, MSGPANEL_TEXTMSG, "Focus with hardware control.");
		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
		
		GetPanelAttribute (roi->region_panel, ATTR_LEFT, &left);
		GetPanelAttribute (roi->region_panel, ATTR_TOP, &top);
		SetPanelPos (msgPnl, top+10, left+10);
		DisplayPanel(msgPnl);
		
		while (1) {
			
			ProcessSystemEvents();
			
			GetUserEvent (0, &pnl, &ctrl);
			if (pnl == msgPnl) {
				if (ctrl == MSGPANEL_OK) break;
				if (ctrl == MSGPANEL_CANCEL) break;
			}
	
			// set the z drive to compensate if the user moves the stage
			stage_get_xy_position (roi->stage, &xVal, &yVal);
			region_of_interest_z_for_xy(roi, xVal, yVal, &zCalc);
			z_drive_set_position(MICROSCOPE_MASTER_ZDRIVE(roi->ms), zCalc);

			#ifndef THREADED_CAM_AQ
			//For some unfathomable reason the presence of the message panel 
			//prevents camera timer tick events, so get and display an image.
			dib = gci_camera_get_image(roi->camera, NULL); 			
			gci_camera_display_image(roi->camera, dib, NULL);	
			FreeImage_Unload(dib);   
			#endif
		}
		
		if (ctrl == MSGPANEL_CANCEL) goto Cancel;

		HidePanel(msgPnl);
	}

//	if(was_on) {
//		stage_set_joystick_on(roi->stage);
//	}

End:
	DiscardPanel(msgPnl);
	return roi->focal_plane_valid;

Cancel:
	DiscardPanel(msgPnl);
	return 0;
//	return -1;  // this fn should not return -1

Stop:
	DiscardPanel(msgPnl);
	return 0;
}

void region_of_interest_set_focus_points(region_of_interest* roi, double a, double b, double c)
{
    roi->focal_point_a = a;
    roi->focal_point_b = b;
    roi->focal_point_c = c;
  	roi->focal_point_c_original=roi->focal_point_c;

	roi->focal_plane_valid = 1;
}


void region_of_interest_get_focus_points(region_of_interest* roi, double *a, double *b, double *c)
{
    *a = roi->focal_point_a;
    *b = roi->focal_point_b;
    *c = roi->focal_point_c;
}

int region_of_interest_is_focal_plane_valid(region_of_interest* roi)
{
	return roi->focal_plane_valid;
}

/////////////////////////////////////////////////////////////////////////////////////

static int region_of_interest_focus_on_point(region_of_interest* roi, char* message)
{
	int msgPnl, pnl, ctrl, top, left, should_process = 0;
	FIBITMAP *dib = NULL;
	static int focusing = 0;
	CameraState state;
	
	if(focusing == 1)
		return -1;
	
	focusing = 1;

	//Get the user to define an xy point
	msgPnl = ui_module_add_panel(UIMODULE_CAST(roi), "RegionOfInterest_ui.uir", MSGPANEL, 0);
	
	SetPanelAttribute (msgPnl, ATTR_TITLE, "Set ROI Co-ords");
	SetCtrlVal(msgPnl, MSGPANEL_TEXTMSG, message);

	// put near the parent panel 
	GetPanelAttribute (roi->region_panel, ATTR_LEFT, &left);
	GetPanelAttribute (roi->region_panel, ATTR_TOP, &top);
	SetPanelPos (msgPnl, top+10, left+10);

	DisplayPanel(msgPnl);

	//Remember camera settings
	gci_camera_save_state(roi->camera, &state);
		
	should_process = ref_images_should_process(roi->ms->_ri);
	
	SetPanelAttribute (UIMODULE_MAIN_PANEL_ID(roi->ms->_ri), ATTR_DIMMED, 1);

	if(should_process) {		
		ref_images_disable(roi->ms->_ri);
	}

	//Set to 8 bit binned mode for fast display
	microscope_set_focusing_mode(roi->ms);
	gci_camera_set_live_mode(roi->camera);
	gci_camera_activate_live_display(roi->camera);
		
	stage_set_joystick_on (roi->stage);
			
	while (1) {
		ProcessSystemEvents();
		
		GetUserEvent (0, &pnl, &ctrl);
		if ((pnl == msgPnl) && (ctrl == MSGPANEL_OK))
			break;

		#ifndef THREADED_CAM_AQ
		//For some unfathomable reason the presence of the message panel 
		//prevents camera timer tick events, so get and display an image.
		dib = gci_camera_get_image(roi->camera, NULL); 			
		gci_camera_display_image(roi->camera, dib, NULL);	
		FreeImage_Unload(dib);   
		#endif
	}
		
	ui_module_destroy_panel(UIMODULE_CAST(roi), msgPnl);
	
	//Restore camera settings
	gci_camera_restore_state(roi->camera, &state);
	gci_camera_set_snap_mode(roi->camera);   
	
	SetPanelAttribute (UIMODULE_MAIN_PANEL_ID(roi->ms->_ri), ATTR_DIMMED, 0);

	if(should_process) {	
		ref_images_enable(roi->ms->_ri);
	}

	focusing = 0; 

	return 0;
}

static void region_of_interest_dim_focus_buttons(region_of_interest* roi, int dim)
{
	SetCtrlAttribute (roi->region_panel_tab1, ROI_TAB1_SET_START, ATTR_DIMMED, dim);
	SetCtrlAttribute (roi->region_panel_tab1, ROI_TAB1_SET_END, ATTR_DIMMED, dim);

	SetCtrlAttribute (roi->region_panel_tab2, ROI_TAB2_SET_Y_START, ATTR_DIMMED, dim);
	SetCtrlAttribute (roi->region_panel_tab2, ROI_TAB2_SET_X_START, ATTR_DIMMED, dim);
	SetCtrlAttribute (roi->region_panel_tab2, ROI_TAB2_SET_Y_END, ATTR_DIMMED, dim);
	SetCtrlAttribute (roi->region_panel_tab2, ROI_TAB2_SET_X_END, ATTR_DIMMED, dim);
	
	SetCtrlAttribute (roi->region_panel_tab2, ROI_TAB3_SET_CENTRE, ATTR_DIMMED, dim);
	SetCtrlAttribute (roi->region_panel_tab2, ROI_TAB3_SET_RADIUS, ATTR_DIMMED, dim);
}

int CVICALLBACK cbRoiTab1_SetStart (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
		
			double xStart, yStart, xEnd, yEnd;
			int was_live;

			region_of_interest* roi = (region_of_interest*) callbackData;

			region_of_interest_dim_focus_buttons(roi, 1);

			was_live = gci_camera_is_live_mode(roi->camera);

			//Define a new start point - adjust the roi dimensions
			if(region_of_interest_focus_on_point(roi, "Move stage to start position using joystick") == -1) {
				region_of_interest_dim_focus_buttons(roi, 0);
				return 0;
			}

			stage_get_xy_position (roi->stage, &xStart, &yStart);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_END, &xEnd);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_END, &yEnd);
			
			//Update all tabs
			region_of_interest_set_region(roi, xStart, yStart, xEnd-xStart, yEnd-yStart);

			//Indicate that this point has been defined at least once
			SetCtrlAttribute (roi->region_panel_tab1, ROI_TAB1_SET_START, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);

			//Leave in live mode to keep BV happy
			if (was_live){
				gci_camera_set_live_mode(roi->camera);
				gci_camera_activate_live_display(roi->camera);
			}
			
			region_of_interest_dim_focus_buttons(roi, 0);

			}break;
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
	switch (event)
		{
		case EVENT_COMMIT:{
			double xEnd, yEnd, xStart, yStart;
			int was_live;

			region_of_interest* roi = (region_of_interest*) callbackData;

			region_of_interest_dim_focus_buttons(roi, 1);

			was_live = gci_camera_is_live_mode(roi->camera);

			//Define a new end point - adjust the roi dimensions
			if(region_of_interest_focus_on_point(roi, "Move stage to end position using joystick") == -1) {
				region_of_interest_dim_focus_buttons(roi, 0);
				return 0;
			}
		
			stage_get_xy_position (roi->stage, &xEnd, &yEnd);

			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_START, &xStart);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_START, &yStart);
			
			if (xStart > xEnd) swap (&xStart, &xEnd);
			if (yStart > yEnd) swap (&yStart, &yEnd);
			
			//Update all tabs
			region_of_interest_set_region(roi, xStart, yStart, xEnd-xStart, yEnd-yStart);

			//Indicate that this point has been defined at least once
			SetCtrlAttribute (roi->region_panel_tab1, ROI_TAB1_SET_END, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);

			//Leave in live mode to keep BV happy
			if (was_live){
				gci_camera_set_live_mode(roi->camera);
				gci_camera_activate_live_display(roi->camera);
			}

			region_of_interest_dim_focus_buttons(roi, 0);

			}break;
		}
		
	return 0;
}

int CVICALLBACK cbRoiTab1_pointChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			double xStart, yStart, xEnd, yEnd;

			region_of_interest* roi = (region_of_interest*) callbackData;

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

			}break;
		}
		
	return 0;
}

int CVICALLBACK cbRoiTab1_dimChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			double xStart, yStart, xLength, yLength;

			region_of_interest* roi = (region_of_interest*) callbackData;

			//A new dimension has been typed in - adjust the end point
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_START, &xStart);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_START, &yStart);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_LENGTH, &xLength);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_LENGTH, &yLength);
			
			//Update all tabs
			region_of_interest_set_region(roi, xStart, yStart, xLength, yLength);
			}break;
		}
		
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
int CVICALLBACK cbRoiTab2_pointSetup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	region_of_interest* roi = (region_of_interest*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:{

			double x, y, xStart, yStart, xEnd, yEnd;
			unsigned int image_width, image_height;
			int colour;
			int was_live;
	
			//Define a new point - adjust the roi dimensions
			GetCtrlVal(roi->region_panel_tab2, ROI_TAB2_X_START, &xStart);
			GetCtrlVal(roi->region_panel_tab2, ROI_TAB2_X_END, &xEnd);
			GetCtrlVal(roi->region_panel_tab2, ROI_TAB2_Y_START, &yStart);
			GetCtrlVal(roi->region_panel_tab2, ROI_TAB2_Y_END, &yEnd);

			// We want the region up to the right edge of the image.
			gci_camera_get_size(roi->camera, &image_width, &image_height);

			region_of_interest_dim_focus_buttons(roi, 1);

			was_live = gci_camera_is_live_mode(roi->camera);

			switch (control) {

				case ROI_TAB2_SET_X_START:
				{	
					if(region_of_interest_focus_on_point(roi, "Move stage to X Start coordinate using joystick") == -1)
						goto ERROR_OCCURED;
						
					stage_get_xy_position (roi->stage, &xStart, &y);

					break;
				}
				
				case ROI_TAB2_SET_X_END:
				{
					if(region_of_interest_focus_on_point(roi, "Move stage to X End coordinate using joystick") == -1)
						goto ERROR_OCCURED;
						
					stage_get_xy_position (roi->stage, &xEnd, &y);

					break;
				}
				
				case ROI_TAB2_SET_Y_START:
				{
					if(region_of_interest_focus_on_point(roi, "Move stage to Y Start coordinate using joystick") == -1)
						goto ERROR_OCCURED;
						
					stage_get_xy_position (roi->stage, &x, &yStart);
		
					break;
				}
				
				case ROI_TAB2_SET_Y_END:
				{
					if(region_of_interest_focus_on_point(roi, "Move stage to Y end coordinate using joystick") == -1)
						goto ERROR_OCCURED;
						
					stage_get_xy_position (roi->stage, &x, &yEnd);

					break;
				}
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
			
			region_of_interest_dim_focus_buttons(roi, 0);

			//Update all tabs
			region_of_interest_set_region(roi, xStart, yStart, xEnd-xStart, yEnd-yStart);

			//Indicate that this point has been defined at least once
			SetCtrlAttribute (roi->region_panel_tab2, control, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);

			//Leave in live mode to keep BV happy
			if (was_live){
				gci_camera_set_live_mode(roi->camera);
				gci_camera_activate_live_display(roi->camera);
			}
			}break;
		}

	return 0;

ERROR_OCCURED:

	region_of_interest_dim_focus_buttons(roi, 0);
	return -1;
}

int CVICALLBACK cbRoiTab2_pointChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			double xStart, yStart, xEnd, yEnd;
	
			region_of_interest* roi = (region_of_interest*) callbackData;

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
		}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
int CVICALLBACK cbRoiTab3_SetCentre (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			double xCentre, yCentre, radius;
			int was_live;

			region_of_interest* roi = (region_of_interest*) callbackData;

			region_of_interest_dim_focus_buttons(roi, 1);

			was_live = gci_camera_is_live_mode(roi->camera);

			//Define a new centre point - adjust the roi position
			region_of_interest_focus_on_point(roi, "Move stage to centre position using joystick");
		
			stage_get_xy_position (roi->stage, &xCentre, &yCentre);
			GetCtrlVal(roi->region_panel_tab3, ROI_TAB3_RADIUS, &radius);
			
			radius = fabs(radius);	//cannot be negative
			SetCtrlVal(roi->region_panel_tab3, ROI_TAB3_RADIUS, radius);
			
			//Update all tabs
			region_of_interest_set_region(roi, xCentre-radius, yCentre-radius, radius*2, radius*2);

			//Indicate that this point has been defined at least once
			SetCtrlAttribute (roi->region_panel_tab3, ROI_TAB3_SET_CENTRE, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);

			region_of_interest_dim_focus_buttons(roi, 0);

			//Leave in live mode to keep BV happy
			if (was_live){
				gci_camera_set_live_mode(roi->camera);
				gci_camera_activate_live_display(roi->camera);
			}

			}break;
		}
		
	return 0;
}

int CVICALLBACK cbRoiTab3_SetRadius (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			double x, y, xCentre, yCentre, radius;
			int was_live;

			region_of_interest* roi = (region_of_interest*) callbackData;

			was_live = gci_camera_is_live_mode(roi->camera);

			//Define a new edge point - adjust the radius
			region_of_interest_focus_on_point(roi, "Move stage to edge position using joystick");
		
			stage_get_xy_position (roi->stage, &x, &y);
			GetCtrlVal(roi->region_panel_tab3, ROI_TAB3_CENTRE_X, &xCentre);
			GetCtrlVal(roi->region_panel_tab3, ROI_TAB3_CENTRE_Y, &yCentre);
			
			// What was this. I just get it from the panel now
			//radius = sqrt((x-xCentre)*(x-xCentre) + (y-yCentre)*(y-yCentre));	//Pythagoras
			GetCtrlVal(roi->region_panel_tab3, ROI_TAB3_RADIUS, &radius);
			
			radius = fabs(radius);	//cannot be negative

			if(radius == 0)
				return 0;

			//Update all tabs
			region_of_interest_set_region(roi, xCentre-radius, yCentre-radius, radius*2, radius*2);

			//Indicate that this point has been defined at least once
			SetCtrlAttribute (roi->region_panel_tab3, ROI_TAB3_SET_RADIUS, ATTR_CMD_BUTTON_COLOR, VAL_YELLOW);

			//Leave in live mode to keep BV happy
			if (was_live){
				gci_camera_set_live_mode(roi->camera);
				gci_camera_activate_live_display(roi->camera);
			}

			break;
		}
	}
		
	return 0;
}

int CVICALLBACK cbRoiTab3_valChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			double xCentre, yCentre, radius;

			region_of_interest* roi = (region_of_interest*) callbackData;

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

			}break;
		}
		
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////

int CVICALLBACK cbRoiTab4_GoPos (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			double xStart, yStart, xEnd, yEnd, x, y;

			region_of_interest* roi = (region_of_interest*) callbackData; 
	
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
			
			region_of_interest_goto_stage_xy(roi, x, y);

			//Ensure a live image
			gci_camera_set_live_mode(roi->camera);
			gci_camera_activate_live_display(roi->camera);

			stage_set_joystick_on(roi->stage);
			
			}break;
		}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////
int region_of_interest_set_offset(region_of_interest* roi)
{
	int msgPnl, pnl, ctrl, top, left;
	FIBITMAP *dib = NULL;
	double x, y, z, zCalc, zDiff;
	char msg[50], buffer[500];
	static int focusing = 0;
	CameraState state;
	
	if(focusing == 1)
		return -1;
	
	gci_camera_save_state(roi->camera, &state);   
	
	focusing = 1;
	ui_module_disable_panel(roi->region_panel, 0);
	
	find_resource("RegionOfInterest_ui.uir", buffer); 
	msgPnl = LoadPanel(0, buffer, MSGPANEL); 
	
	SetPanelAttribute (msgPnl, ATTR_TITLE, "Set Offset to Focal Plane");
	SetCtrlAttribute (msgPnl, MSGPANEL_CANCEL, ATTR_VISIBLE, 1);	 //Unhide the Cancel button
	
	//Give him the opportunity to focus
	microscope_set_focusing_mode(roi->ms);
	gci_camera_set_live_mode(roi->camera);
	gci_camera_activate_live_display(roi->camera);
	stage_set_joystick_on (roi->stage);
		
	SetCtrlVal(msgPnl, MSGPANEL_TEXTMSG, "Please focus on the object.");
	
	GetPanelAttribute (roi->region_panel, ATTR_LEFT, &left);
	GetPanelAttribute (roi->region_panel, ATTR_TOP, &top);
	SetPanelPos(msgPnl, top, left);
	
	DisplayPanel(msgPnl);
		
	while (1) {
		ProcessSystemEvents();
			
		GetUserEvent (0, &pnl, &ctrl);
		if (pnl == msgPnl) {
			if (ctrl == MSGPANEL_OK)
				break;
			
			if (ctrl == MSGPANEL_CANCEL)
				break;
		}

		#ifndef THREADED_CAM_AQ
		//For some unfathomable reason the presence of the message panel 
		//prevents camera timer tick events, so get and display an image.
		dib = gci_camera_get_image(roi->camera, NULL); 			
		gci_camera_display_image(roi->camera, dib, NULL);	
		FreeImage_Unload(dib);   
		#endif
	}
		
	//Reset camera settings
	gci_camera_restore_state(roi->camera, &state);
	gci_camera_snap_image(roi->camera);  
	
	ui_module_enable_panel(roi->region_panel, 0);  
	focusing = 0;

	DiscardPanel(msgPnl);
	
	if (ctrl == MSGPANEL_CANCEL)
		return 0;
	
	stage_get_xy_position (roi->stage, &x, &y);
	
	z_drive_get_position(MICROSCOPE_MASTER_ZDRIVE(roi->ms), &z);   
	
	//What should z be for this xy?
	zCalc = roi->focal_point_a * x + roi->focal_point_b * y + roi->focal_point_c;
	zDiff = z - zCalc;
	roi->focal_point_c += zDiff;
  	roi->focal_point_c_original=roi->focal_point_c;  // this function redefines the original focal plane
	
	sprintf(msg, "Offset of %.2f um applied.", zDiff);
	GCI_MessagePopup("Focal Plane Setup", msg);
	
	return 0;
}		

int region_of_interest_setup_focus_plane(region_of_interest* roi)
{
	int val;
	static int focusing = 0;
	
	if(focusing == 1)
		return -1;
	
	focusing = 1;
	ui_module_disable_panel(roi->region_panel, 0);

	// Check for valid ROI
	if(roi->width < 1.0 || roi->height < 1.0) {  //height and/or width < 1 um
		GCI_MessagePopup("Error", "ROI width or height is zero. Select a valid region.");
		ui_module_enable_panel(roi->region_panel, 0);  
		focusing = 0;
		return 0;
	}
			
	// Do it outside has a value of 1
	Radio_GetMarkedOption (roi->region_panel, ROI_PANEL_IN_OUT_ROI, &val);

	gci_camera_set_snap_mode(roi->camera);
		
	roi->focal_plane_valid = region_of_interest_setup_focal_plane(roi, val);
	
	ui_module_enable_panel(roi->region_panel, 0);

	SetCtrlAttribute(roi->region_panel, ROI_PANEL_SET_F_OFFSET, ATTR_DIMMED, 0);

	focusing = 0;

	return 0;
}

int CVICALLBACK cbRoiSetFocalPlane (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			region_of_interest* roi = (region_of_interest*) callbackData;
	
			region_of_interest_setup_focus_plane(roi);

			break;
		}
	}
		
	return 0;
}

int CVICALLBACK cbRoiSetFocalPlaneOffset (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			region_of_interest* roi = (region_of_interest*) callbackData;
	
			region_of_interest_set_offset(roi);

			break;
		}
	}
		
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

int CVICALLBACK onRoiOk (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			region_of_interest* roi = (region_of_interest*) callbackData; 
			double startx, x_length;
			double starty, y_length;


			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_START, &startx);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_X_LENGTH, &x_length);
			
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_START, &starty);
			GetCtrlVal(roi->region_panel_tab1, ROI_TAB1_Y_LENGTH, &y_length);

			// Less than 1 micron ?
			if(x_length < 1.0 || y_length < 1.0) {
			
				GCI_MessagePopup("Error", "End point is the same as the start point. Select a region");
				return 0;
			}
			
			roi->left = startx;
			roi->top = starty;
			roi->width = x_length;
			roi->height = y_length;

			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(roi), "RegionSelected",
				GCI_DOUBLE, roi->left, GCI_DOUBLE, roi->top, GCI_DOUBLE, roi->width, GCI_DOUBLE, roi->height);   

			read_or_write_roiwindow_registry_settings(roi->region_panel, 1);  //save window position
			HidePanel(roi->region_panel); 

			//Ensure joystick enabled
			stage_set_joystick_on (roi->stage);

			}break;
		}
		
	return 0;
}


int CVICALLBACK onRoiCancel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			region_of_interest* roi = (region_of_interest*) callbackData;   
	
			
			HidePanel(panel); 

			//Ensure joystick enabled
			stage_set_joystick_on (roi->stage);
			
			}break;
		}
	return 0;
}


int CVICALLBACK onGetCoordsFromStagePlate (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			region_of_interest* roi = (region_of_interest*) callbackData;   
			int ret;

			if(roi->ms->_stage_plate_module == NULL)
				return -1;

			if(!stage_current_plate_is_valid(roi->ms->_stage_plate_module))
				return -1;
	
			ret = GCI_ConfirmPopup("Info", IDI_INFORMATION, 
				"Are you sure you would like to use the plate as a new region of interest");

			if(ret > 0) 
				region_of_interest_panel_set_region_from_stage_plate(roi);

			break;
		}
	}

	return 0;
}





