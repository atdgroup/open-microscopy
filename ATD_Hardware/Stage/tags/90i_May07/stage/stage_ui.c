#include "stage.h"
#include "stage_ui.h"
#include "Corvus_ui.h"	 //Corvus XYZ
//#include "Corvus_90i_ui.h"   //Corvus XY with 90i Z

#include "cvixml.h"
#include <userint.h>
#include <utility.h>

#include "string_utils.h"
#include "gci_utils.h"
#include "xml_utils.h"

/////////////////////////////////////////////////////////////////////////////
// XYZ stage module for Marzhauser stage - GP & RJL Jan 2006
//
/////////////////////////////////////////////////////////////////////////////

static void	dim_xyz_controls(Stage *stage)
{
	int dimx, dimy, dimz;	//To save typing
	
	dimx = !stage->_enabled_axis[XAXIS];
	dimy = !stage->_enabled_axis[YAXIS];
	dimz = !stage->_enabled_axis[ZAXIS];
	
	if (dimx && dimy && dimz) {
		SetPanelAttribute(stage->_main_ui_panel, ATTR_DIMMED, 1);
		return;
	}
	else 
		SetPanelAttribute(stage->_main_ui_panel, ATTR_DIMMED, 0);
		
		
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_X_ABS, ATTR_DIMMED, dimx);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_ABS_X, ATTR_DIMMED, dimx);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_X_REL, ATTR_DIMMED, dimx);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_REL_X, ATTR_DIMMED, dimx);

	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_Y_ABS, ATTR_DIMMED, dimy);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_ABS_Y, ATTR_DIMMED, dimy);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_Y_REL, ATTR_DIMMED, dimy);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_REL_Y, ATTR_DIMMED, dimy);

	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_Z_ABS, ATTR_DIMMED, dimz);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_ABS_Z, ATTR_DIMMED, dimz);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_Z_REL, ATTR_DIMMED, dimz);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_REL_Z, ATTR_DIMMED, dimz);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_SET_DATUM_XYZ, ATTR_DIMMED, dimz);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_SET_DATUM_Z, ATTR_DIMMED, dimz);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_GOTO_XYZ_DATUM, ATTR_DIMMED, dimz);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_GOTO_Z_DATUM, ATTR_DIMMED, dimz);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_Z_TOP, ATTR_DIMMED, dimz);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_Z_BOTTOM, ATTR_DIMMED, dimz);

	SetCtrlAttribute (stage->_params_ui_panel, XYZ_PARAMS_Z_REVERSED, ATTR_DIMMED, dimz);
#ifndef XY_ONLY
	SetCtrlAttribute (stage->_params_ui_panel, XYZ_PARAMS_BACKLASH_Z, ATTR_DIMMED, dimz);
	SetCtrlAttribute (stage->_params_ui_panel, XYZ_PARAMS_Z_PITCH, ATTR_DIMMED, dimz);
	SetCtrlAttribute (stage->_params_ui_panel, XYZ_PARAMS_Z_ACC, ATTR_DIMMED, dimz);
#endif
	SetCtrlAttribute (stage->_params_ui_panel, XYZ_PARAMS_Z_SPEED, ATTR_DIMMED, dimz);
	SetCtrlAttribute (stage->_params_ui_panel, XYZ_PARAMS_Z_UPPER_LIMIT, ATTR_DIMMED, dimz);
	SetCtrlAttribute (stage->_params_ui_panel, XYZ_PARAMS_Z_LOWER_LIMIT, ATTR_DIMMED, dimz);
	SetCtrlAttribute (stage->_params_ui_panel, XYZ_PARAMS_TEST, ATTR_DIMMED, dimz);

	if (dimz){
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_ABS_XYZ, ATTR_LABEL_TEXT, "Go to XY");
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_REL_XYZ, ATTR_LABEL_TEXT, "Move XY by");
	}
	else {
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_ABS_XYZ, ATTR_LABEL_TEXT, "Go to XYZ");
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_REL_XYZ, ATTR_LABEL_TEXT, "Move XYZ by");
	}

	if (dimx && dimy) {
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_ABS_XYZ, ATTR_LABEL_TEXT, "Go to Z");
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_REL_XYZ, ATTR_LABEL_TEXT, "Move Z by");
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_SET_DATUM_XY, ATTR_DIMMED, 1);
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_GOTO_XY_DATUM, ATTR_DIMMED, 1);
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_TOP_LEFT, ATTR_DIMMED, 1);
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_TOP_RIGHT, ATTR_DIMMED, 1);
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_BOTTOM_LEFT, ATTR_DIMMED, 1);
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_BOTTOM_RIGHT, ATTR_DIMMED, 1);
	}
	else {
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_ABS_XYZ, ATTR_LABEL_TEXT, "Go to XYZ");
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_REL_XYZ, ATTR_LABEL_TEXT, "Move XYZ by");
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_SET_DATUM_XY, ATTR_DIMMED, 0);
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_GOTO_XY_DATUM, ATTR_DIMMED, 0);
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_TOP_LEFT, ATTR_DIMMED, 0);
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_TOP_RIGHT, ATTR_DIMMED, 0);
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_BOTTOM_LEFT, ATTR_DIMMED, 0);
		SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_BOTTOM_RIGHT, ATTR_DIMMED, 0);
	}
	ProcessDrawEvents();
}

void stage_dim_controls(Stage *stage, int dim)
{
	//Disable or enable controls
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_JOYSTICK_ENABLE, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_ADVANCED, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_QUIT, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_ABS_XYZ, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_ABS_X, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_ABS_Y, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_ABS_Z, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_REL_XYZ, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_REL_X, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_REL_Y, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_MOVE_REL_Z, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_SET_DATUM_XYZ, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_SET_DATUM_XY, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_SET_DATUM_Z, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_GOTO_XYZ_DATUM, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_GOTO_XY_DATUM, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_GOTO_Z_DATUM, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_TOP_LEFT, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_TOP_RIGHT, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_BOTTOM_RIGHT, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_BOTTOM_LEFT, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_Z_TOP, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_Z_BOTTOM, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_INITIALISE, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_ABOUT, ATTR_DIMMED, dim);
	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_SHOW_SETTINGS, ATTR_DIMMED, dim);

	SetCtrlAttribute (stage->_main_ui_panel, STAGE_PNL_ABORT_MOVE, ATTR_DIMMED, (!dim));

	if (stage->_params_ui_panel >= 0) SetPanelAttribute(stage->_params_ui_panel, ATTR_DIMMED, dim);

	//Check for disabled axes
	if (dim == 0) dim_xyz_controls(stage);
}


int CVICALLBACK OnStageJoystickToggled(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	int status;
	Stage *stage = (Stage*) callbackData; 
	
	//Joystick button
    if (event == EVENT_COMMIT) {
    
    	GetCtrlVal(panel, control, &status);
    	
    	if(status)
    		stage_set_joystick_on (stage);
    	else
    		stage_set_joystick_off (stage);
    
	} 
	
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////

int CVICALLBACK OnReinit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	int full_init;
	
	switch (event)
	{
		case EVENT_COMMIT:
			stage_dim_controls(stage, 1);
			
			if( (*stage->_cops->init)(stage) == STAGE_ERROR ) {
				send_stage_error_text(stage, "Reset operation failed for stage");
				break;
  			}
  			
			full_init = ConfirmPopup("", "OK to initialise XY stage?");
			stage_find_initialise_extents (stage, full_init);
			
			stage_dim_controls(stage, 0);
			break;
	}
		
	return 0;
}


int CVICALLBACK OnSettings (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;

	switch (event)
	{
		case EVENT_COMMIT:
			stage_display_params_ui(stage);
			break;
	}
	
	return 0;
}


static int StageDisplayInfoPanel(Stage *stage)
{
	int about_panel, pnl, ctrl;
	char info[500] = "";
	
	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);

	if (stage_get_info(stage, info) == STAGE_ERROR ) {
	
		send_stage_error_text(stage, "Failed to get stage info");
		return STAGE_ERROR;
	}
	
	about_panel = FindAndLoadUIR(0, "stage_ui.uir", ABOUT_PNL); 
	
	SetCtrlVal(about_panel, ABOUT_PNL_INFO, info);
	
	InstallPopup (about_panel);
	
	while (1) {
		GetUserEvent (1, &pnl, &ctrl);
		
		if (pnl == about_panel)
			break;
	}
	
	DiscardPanel(about_panel);
	
	return 0;
}


int CVICALLBACK OnAbout (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
		
			StageDisplayInfoPanel(stage);
			break;
		}
	return 0;
}


int CVICALLBACK OnStageJoystickSpeed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	double speed;
	Stage *stage = (Stage*) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:
		
			GetCtrlVal(stage->_main_ui_panel, STAGE_PNL_JOYSTICK_XY_SPEED, &speed);
			stage_set_joystick_speed (stage, speed);

			break;
	}

	return 0;
}


int CVICALLBACK OnStageAdvancedButton (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:

			if(!stage->_advanced_view) {
			
				SetPanelAttribute(panel, ATTR_HEIGHT, 485);
				SetCtrlAttribute(panel, control, ATTR_LABEL_TEXT, "Advanced <<");
				stage->_advanced_view = 1;
			}
			else {
			
				SetPanelAttribute(panel, ATTR_HEIGHT, 108);
				SetCtrlAttribute(panel, control, ATTR_LABEL_TEXT, "Advanced >>");
				stage->_advanced_view = 0;
			}

			break;
	}
	
	return 0;
}


int CVICALLBACK OnStageQuit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:

			stage_hide_ui(stage);
			GCI_Signal_Emit(&(stage->signal_table), "Close", GCI_VOID_POINTER, stage);

			break;
	}
	
	return 0;
}


int CVICALLBACK OnStageAbortInit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:

			stage_abort_move (stage);
			stage->_init_aborted = 1;
			
			break;
	}
		
	return 0;
}


// Move handlers

int CVICALLBACK OnStageAbortMove (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	
	switch (event) {
	
		case EVENT_COMMIT:
		
			stage_abort_move (stage);
			stage->_abort_move = 1;
			
			break;
	}
	
	return 0;
}


int CVICALLBACK MoveByX(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	double xval, t;
    Stage *stage = (Stage*) callbackData; 
    
    if (event == EVENT_COMMIT) {
    
    	//Get the distance to move
    	GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_X_REL, &xval);

		//Move stage to required location
		stage_dim_controls(stage, 1);
		
    	t = Timer();
		stage_rel_move_by (stage, xval, 0.0, 0.0); 
		SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		
		stage_dim_controls(stage, 0);
		
		stage_plot_current_position(stage);		
    }
    	
	return 0;
}


int CVICALLBACK MoveByY(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	double yval, t;
	Stage *stage = (Stage*) callbackData; 
	
    if (event == EVENT_COMMIT) {
    	
    	//Get relative distances to move
    	GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_Y_REL, &yval);

		//Move stage to required location
		stage_dim_controls(stage, 1);
		
    	t = Timer();
		stage_rel_move_by (stage, 0.0, yval, 0.0);
		SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		
		stage_dim_controls(stage, 0);
		
		stage_plot_current_position(stage);		
    }
    
	return 0;
}


int CVICALLBACK MoveByZ(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	double zval, t;
	Stage *stage = (Stage*) callbackData; 
	
    if (event == EVENT_COMMIT) {
    	
    	//Get relative distances to move
    	GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_Z_REL, &zval);

		//Move stage to required location
		stage_dim_controls(stage, 1);
		
    	t = Timer();
		stage_rel_move_by_z (stage, zval);
		SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		
		stage_dim_controls(stage, 0);
		
		stage_plot_current_position(stage);		
    }
    
	return 0;
}


int CVICALLBACK MoveByXYZ(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	double xval, yval, zval, t;
	Stage *stage = (Stage*) callbackData; 
	
    if (event == EVENT_COMMIT) {
    	
    	//Get relative distances to move
    	GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_X_REL, &xval);
    	GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_Y_REL, &yval); 
    	GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_Z_REL, &zval); 

		//Move stage to required location
		stage_dim_controls(stage, 1);
		
    	t = Timer();
		stage_rel_move_by (stage, xval, yval, zval);
		SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		
		stage_dim_controls(stage, 0);
		
		stage_plot_current_position(stage);		
    }
    
	return 0;
}


int CVICALLBACK MoveToX(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	double xval, t;
    Stage *stage = (Stage*) callbackData; 
    
    if (event == EVENT_COMMIT)
	{
		//Get wanted xcoord
		GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_X_ABS, &xval);

		stage_dim_controls(stage, 1);
		
    	t = Timer();
		stage_goto_x_position (stage, xval) ;
		SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		
		stage_dim_controls(stage, 0);
		
		stage_plot_current_position(stage);		
	}
	
	return 0;
}


int CVICALLBACK MoveToY(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	double yval, t;
    Stage *stage = (Stage*) callbackData; 
    
    if (event == EVENT_COMMIT)
	{
		//Get wanted xcoord
		GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_Y_ABS, &yval);

		stage_dim_controls(stage, 1);
		
    	t = Timer();
		stage_goto_y_position (stage, yval) ;
		SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		
		stage_dim_controls(stage, 0);
		
		stage_plot_current_position(stage);		
	}
	
	return 0;
}


int CVICALLBACK MoveToZ(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	double zval, t;
    Stage *stage = (Stage*) callbackData; 
    
    if (event == EVENT_COMMIT)
	{
		//Get wanted xcoord
		GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_Z_ABS, &zval);

		stage_dim_controls(stage, 1);
		
    	t = Timer();
		stage_goto_z_position (stage, zval) ;
		SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		
		stage_dim_controls(stage, 0);
		
		stage_plot_current_position(stage);		
	}
	
	return 0;
}


int CVICALLBACK MoveToXYZ(int panel, int control, int event, void *callbackData, int eventData1, int eventData2)
{
	double xval, yval, zval, xCur, yCur, zCur, t;
	int tolerance;
    Stage *stage = (Stage*) callbackData; 
    
    if (event == EVENT_COMMIT)
	{
		t = Timer();

		GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_X_ABS, &xval);
		GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_Y_ABS, &yval);
		GetCtrlVal (stage->_main_ui_panel, STAGE_PNL_Z_ABS, &zval);

		if (stage->_move_type == FAST) { //assume no problems
#ifdef XY_ONLY
			stage_get_z_tolerance(stage, &tolerance);
			if (tolerance != 9) stage_set_z_tolerance(stage, 9);
#endif
			stage_async_goto_xyz_position(stage, xval, yval, zval);
#ifdef XY_ONLY
			if (tolerance != 9) stage_set_z_tolerance(stage, tolerance);
#endif
			return 0;
		}

		stage_dim_controls(stage, 1);
		
		if(stage_get_xyz_position(stage, &xCur, &yCur, &zCur) == STAGE_ERROR)
			return 0;	
		stage_goto_xyz_position (stage, xval, yval, zval, xCur, yCur, zCur); 
		SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		
		stage_dim_controls(stage, 0);
	
		stage_plot_current_position(stage);		
	}
	
	return 0;
}


int CVICALLBACK MoveTopLeft(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	double t;
	
    if (event == EVENT_COMMIT) {
		stage_dim_controls(stage, 1);
		
		if (ConfirmPopup("XYZ Stage Warning", "Are you sure?")) {
	    	t = Timer();
			stage_goto_xy_position (stage, stage->_limits.min_x, stage->_limits.min_y);
			SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		}
		
		stage_dim_controls(stage, 0);
	}

	return 0;
}


int CVICALLBACK MoveTopRight(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	double t;
	
    if (event == EVENT_COMMIT) {
		stage_dim_controls(stage, 1);
		
		if (ConfirmPopup("XYZ Stage Warning", "Are you sure?")) {
	    	t = Timer();
			stage_goto_xy_position (stage, stage->_limits.max_x, stage->_limits.min_y);
			SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		}
		
		stage_dim_controls(stage, 0);
	}

	return 0;
}


int CVICALLBACK MoveBottomLeft(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	double t;
	
    if (event == EVENT_COMMIT) {
		stage_dim_controls(stage, 1);
		
		if (ConfirmPopup("XYZ Stage Warning", "Are you sure?")) {
	    	t = Timer();
			stage_goto_xy_position (stage, stage->_limits.min_x, stage->_limits.max_y);
			SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		}
		
		stage_dim_controls(stage, 0);
	}

	return 0;
}


int CVICALLBACK MoveBottomRight(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	double t;
	
    if (event == EVENT_COMMIT) {
		stage_dim_controls(stage, 1);
		
		if (ConfirmPopup("XYZ Stage Warning", "Are you sure?")) {
	    	t = Timer();
			stage_goto_xy_position (stage, stage->_limits.max_x, stage->_limits.max_y);
			SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		}
		
		stage_dim_controls(stage, 0);
	}

	return 0;
}


int CVICALLBACK MoveZBottom(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	double t;
	
    if (event == EVENT_COMMIT) {
		stage_dim_controls(stage, 1);
		
		if (ConfirmPopup("XYZ Stage Warning", "Are you sure?")) {
	    	t = Timer();
			stage_goto_z_position (stage, stage->_limits.min_z);
			SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		}
		
		stage_dim_controls(stage, 0);
	}

	return 0;
}


int CVICALLBACK MoveZTop(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	double t;
	
    if (event == EVENT_COMMIT) {
		stage_dim_controls(stage, 1);
		
		if (ConfirmPopup("XYZ Stage Warning", "Are you sure?")) {
	    	t = Timer();
			stage_goto_z_position (stage, stage->_limits.max_z);
			SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		}
		
		stage_dim_controls(stage, 0);
	}

	return 0;
}



int CVICALLBACK OnSetXYZDatum(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	
    if (event == EVENT_COMMIT) {
    
		stage_dim_controls(stage, 1);
		
		stage_set_xyz_datum (stage);
		
		stage_dim_controls(stage, 0);
	}

	return 0;
}


int CVICALLBACK OnSetXYDatum(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	
    if (event == EVENT_COMMIT) {
    
		stage_dim_controls(stage, 1);
		
		stage_set_xy_datum (stage);
		
		stage_dim_controls(stage, 0);
	}

	return 0;
}


int CVICALLBACK OnSetZDatum(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	
    if (event == EVENT_COMMIT) {
    
		stage_dim_controls(stage, 1);
		
		stage_set_z_datum (stage);
		
		stage_dim_controls(stage, 0);
	}

	return 0;
}



int CVICALLBACK OnGotoXYZDatum(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	double xCur, yCur, zCur, t;
	Stage *stage = (Stage*) callbackData;
	
    if (event == EVENT_COMMIT) {
    
		stage_dim_controls(stage, 1);
		
    	t = Timer();
		if(stage_get_xyz_position(stage, &xCur, &yCur, &zCur) == STAGE_ERROR)
			return 0;
			
		stage_goto_xyz_position (stage, 0.0, 0.0, 0.0, xCur, yCur, zCur);
		SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		
		stage_dim_controls(stage, 0);
	}

	return 0;
}


int CVICALLBACK OnGotoXYDatum(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	double t;
	Stage *stage = (Stage*) callbackData;
	
    if (event == EVENT_COMMIT) {
    
		stage_dim_controls(stage, 1);
		
    	t = Timer();
		stage_goto_xy_position (stage, 0.0, 0.0);
		SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		
		stage_dim_controls(stage, 0);
	}

	return 0;
}


int CVICALLBACK OnGotoZDatum(int panel, int control, int event,
							void *callbackData, int eventData1, int eventData2)
{
	double t;
	Stage *stage = (Stage*) callbackData;
	
    if (event == EVENT_COMMIT) {
    
		stage_dim_controls(stage, 1);
		
    	t = Timer();
		stage_goto_z_datum (stage);
		SetCtrlVal (stage->_main_ui_panel, STAGE_PNL_MOVE_TIME, Timer() - t);
		
		stage_dim_controls(stage, 0);
	}

	return 0;
}


////////////////////////////////////////////////////////////////////////////////////
//Parameters

// ******** save panel vals to XML **************************************************

int stage_save_parameters(const char *filename, Stage *stage)
{
	CVIXMLElement root=-1;
	CVIXMLDocument stageSettings=-1;

	//Save all stage settings in xml format
	if (CVIXMLNewDocument ("stageSettings", &stageSettings)) return -1;
	if (CVIXMLGetRootElement  (stageSettings, &root))          goto Error;

	if (newXmlSettingInt (root, "Baud", stage->_baud_rate))   			    		goto Error;
	if (newXmlSettingDbl (root, "XY_cal_speed_1", stage->_cal_speed_1))          	goto Error;
	if (newXmlSettingDbl (root, "XY_cal_speed_2", stage->_cal_speed_2))          	goto Error;

	if (newXmlSettingInt (root, "X_enabled", stage->_enabled_axis[XAXIS]))       	goto Error;
	if (newXmlSettingInt (root, "X_reversed", stage->_dir[XAXIS]))          		goto Error;
	if (newXmlSettingDbl (root, "X_backlash", stage->_backlash[XAXIS]))          	goto Error;
	if (newXmlSettingDbl (root, "X_pitch", stage->_pitch[XAXIS]))          			goto Error;
	if (newXmlSettingDbl (root, "X_speed", stage->_default_speed[XAXIS]))          	goto Error;
	if (newXmlSettingDbl (root, "X_acceleration", stage->_acceleration[XAXIS]))     goto Error;
	
	if (newXmlSettingInt (root, "Y_enabled", stage->_enabled_axis[YAXIS]))       	goto Error;
	if (newXmlSettingInt (root, "Y_reversed", stage->_dir[YAXIS]))          		goto Error;
	if (newXmlSettingDbl (root, "Y_backlash", stage->_backlash[YAXIS]))          	goto Error;
	if (newXmlSettingDbl (root, "Y_pitch", stage->_pitch[YAXIS]))          			goto Error;
	if (newXmlSettingDbl (root, "Y_speed", stage->_default_speed[YAXIS]))          	goto Error;
	if (newXmlSettingDbl (root, "Y_acceleration", stage->_acceleration[YAXIS]))     goto Error;
	
	if (newXmlSettingInt (root, "Z_enabled", stage->_enabled_axis[ZAXIS]))       	goto Error;
	if (newXmlSettingInt (root, "Z_reversed", stage->_dir[ZAXIS]))          		goto Error;
#ifdef XY_ONLY
	if (newXmlSettingDbl (root, "Z_speed", stage->focus_drive->_speed))          	goto Error;
	if (newXmlSettingInt (root, "Z_tolerance", stage->focus_drive->_tolerance))     goto Error;
	if (newXmlSettingInt (root, "Z_dial_enabled", stage->focus_drive->_remote))  	goto Error;
	if (newXmlSettingInt (root, "Z_dial_sensitivity", stage->focus_drive->_dial_sensitivity))  goto Error;
#else
	if (newXmlSettingDbl (root, "Z_backlash", stage->_backlash[ZAXIS]))          	goto Error;
	if (newXmlSettingDbl (root, "Z_pitch", stage->_pitch[ZAXIS]))          			goto Error;
	if (newXmlSettingDbl (root, "Z_speed", stage->_default_speed[ZAXIS]))          	goto Error;
	if (newXmlSettingDbl (root, "Z_acceleration", stage->_acceleration[ZAXIS]))     goto Error;
#endif
	if (newXmlSettingDbl (root, "Z_low_limit", stage->_limits.min_z))         		goto Error;
	if (newXmlSettingDbl (root, "Z_high_limit", stage->_limits.max_z))         		goto Error;

	if (FileExists(filename, 0)) SetFileAttrs (filename, 0, -1, -1, -1);   //clear read only atribute
	if (CVIXMLSaveDocument (stageSettings, 0, filename)) goto Error;
	SetFileAttrs (filename, 1, -1, -1, -1);   //set read only atribute
	
	CVIXMLDiscardElement(root);
	CVIXMLDiscardDocument (stageSettings);
    
    return 0;
    
Error:

	if (root >= 0) CVIXMLDiscardElement(root);
	if (stageSettings >= 0) CVIXMLDiscardDocument(stageSettings);

	return -1;
}

int stage_load_parameters(const char *filename, Stage *stage)
{
	CVIXMLElement root=-1;
	CVIXMLDocument stageSettings=-1;
    
    if (!FileExists (filename, 0)) return -1;
    
	if (CVIXMLLoadDocument (filename, &stageSettings)) goto Error;
	if (CVIXMLGetRootElement (stageSettings, &root))   goto Error;

	if (getXmlSettingInt (root, "Baud", &stage->_baud_rate))   			    		goto Error;
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_BAUD, stage->_baud_rate);
	if (getXmlSettingDbl (root, "XY_cal_speed_1", &stage->_cal_speed_1)==0)          	
		SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_XY_CAL_SPEED, stage->_cal_speed_1);
	if (getXmlSettingDbl (root, "XY_cal_speed_2", &stage->_cal_speed_2)==0)          	
		SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_XY_CAL_SPEED_2, stage->_cal_speed_2);

	if (getXmlSettingInt (root, "X_enabled", &stage->_enabled_axis[XAXIS]))       	goto Error;
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_ENABLED, stage->_enabled_axis[XAXIS]);
	if (getXmlSettingInt (root, "X_reversed", &stage->_dir[XAXIS]))          		goto Error;
	if (stage->_dir[XAXIS] == 1) 
		SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_REVERSED, 0);
	else
		SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_REVERSED, 1);
	if (getXmlSettingDbl (root, "X_backlash", &stage->_backlash[XAXIS]))          	goto Error;
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_BACKLASH_X, stage->_backlash[XAXIS]);
	if (getXmlSettingDbl (root, "X_pitch", &stage->_pitch[XAXIS]))          		goto Error;
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_PITCH, stage->_pitch[XAXIS]);
	SetCtrlAttribute (stage->_params_ui_panel, XYZ_PARAMS_X_SPEED, ATTR_MAX_VALUE, 45.0*stage->_pitch[XAXIS]);
	if (getXmlSettingDbl (root, "X_speed", &stage->_speed[XAXIS]))          		goto Error;
	stage->_default_speed[XAXIS] = stage->_speed[XAXIS];
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_SPEED, stage->_speed[XAXIS]);
	if (getXmlSettingDbl (root, "X_acceleration", &stage->_acceleration[XAXIS]))    goto Error;
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_ACC, stage->_acceleration[XAXIS]);
	
	if (getXmlSettingInt (root, "Y_enabled", &stage->_enabled_axis[YAXIS]))       	goto Error;
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_ENABLED, stage->_enabled_axis[YAXIS]);
	if (getXmlSettingInt (root, "Y_reversed", &stage->_dir[YAXIS]))          		goto Error;
	if (stage->_dir[YAXIS] == 1) 
		SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_REVERSED, 0);
	else
		SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_REVERSED, 1);
	if (getXmlSettingDbl (root, "Y_backlash", &stage->_backlash[YAXIS]))          	goto Error;
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_BACKLASH_Y, stage->_backlash[YAXIS]);
	if (getXmlSettingDbl (root, "Y_pitch", &stage->_pitch[YAXIS]))          		goto Error;
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_PITCH, stage->_pitch[YAXIS]);
	SetCtrlAttribute (stage->_params_ui_panel, XYZ_PARAMS_Y_SPEED, ATTR_MAX_VALUE, 45.0*stage->_pitch[YAXIS]);
	SetCtrlAttribute (stage->_params_ui_panel, XYZ_PARAMS_XY_CAL_SPEED, ATTR_MAX_VALUE, 45.0*min(stage->_pitch[XAXIS], stage->_pitch[YAXIS]));
	if (getXmlSettingDbl (root, "Y_speed", &stage->_speed[YAXIS]))          		goto Error;
	stage->_default_speed[YAXIS] = stage->_speed[YAXIS];
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_SPEED, stage->_speed[YAXIS]);
	if (getXmlSettingDbl (root, "Y_acceleration", &stage->_acceleration[YAXIS]))    goto Error;
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_ACC, stage->_acceleration[YAXIS]);
	
	if (getXmlSettingInt (root, "Z_enabled", &stage->_enabled_axis[ZAXIS]))       	goto Error;
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_ENABLED, stage->_enabled_axis[ZAXIS]);
	if (getXmlSettingInt (root, "Z_reversed", &stage->_dir[ZAXIS]))          		goto Error;
	if (stage->_dir[ZAXIS] == 1) 
		SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_REVERSED, 0);
	else
		SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_REVERSED, 1);

#ifdef XY_ONLY
	if (getXmlSettingDbl (root, "Z_speed", &stage->focus_drive->_speed))          	goto Error;
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_SPEED, stage->focus_drive->_speed);
	if (getXmlSettingInt (root, "Z_tolerance", &stage->focus_drive->_tolerance))    goto Error;
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_TOLERANCE, stage->focus_drive->_tolerance);
	if (getXmlSettingInt (root, "Z_dial_enabled", &stage->focus_drive->_remote))  	goto Error;
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_DIAL_ENABLED, stage->focus_drive->_remote-3);
	if (getXmlSettingInt (root, "Z_dial_sensitivity", &stage->focus_drive->_dial_sensitivity))  goto Error;
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_SENSITIVITY, stage->focus_drive->_dial_sensitivity);
#else
	if (getXmlSettingDbl (root, "Z_backlash", &stage->_backlash[ZAXIS])==0)          	
		SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_BACKLASH_Z, stage->_backlash[ZAXIS]);
	if (getXmlSettingDbl (root, "Z_pitch", &stage->_pitch[ZAXIS])==0)
		SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_PITCH, stage->_pitch[ZAXIS]);
	if (getXmlSettingDbl (root, "Z_speed", &stage->_speed[ZAXIS])==0) {
		SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_SPEED, stage->_speed[ZAXIS]);
		stage->_default_speed[ZAXIS] = stage->_speed[ZAXIS];
	}
	if (getXmlSettingDbl (root, "Z_acceleration", &stage->_acceleration[ZAXIS])==0)
		SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_ACC, stage->_acceleration[ZAXIS]);
#endif
	if (getXmlSettingDbl (root, "Z_low_limit", &stage->_limits.min_z))         		goto Error;
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_LOWER_LIMIT, stage->_limits.min_z);
	stage->focus_drive->_min = stage->_limits.min_z;
	if (getXmlSettingDbl (root, "Z_high_limit", &stage->_limits.max_z))         	goto Error;
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_UPPER_LIMIT, stage->_limits.max_z);
	stage->focus_drive->_max = stage->_limits.max_z;
				 
	CVIXMLDiscardElement(root);
	CVIXMLDiscardDocument (stageSettings);

    return 0;

Error:
	if (root >= 0)          CVIXMLDiscardElement(root);
	if (stageSettings >= 0) CVIXMLDiscardDocument(stageSettings);

	return -2;
}

void stage_load_default_parameters(Stage *stage)
{
	char path[MAX_PATHNAME_LEN];
	
	GetPrivateDataFolder("Microscope Data", path);
	strcat(path, "StageDefaults.xml");
	if (!FileExists(path, NULL)) return;
	stage_load_parameters(path, stage);
	//dim_xyz_controls(stage);
}

static void shortname(char *path, char *sname)
{
    char drive[MAX_DRIVENAME_LEN], dir[MAX_DIRNAME_LEN];

    SplitPath (path, drive, dir, sname);
}



int CVICALLBACK OnParamsLoad (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	char path[MAX_PATHNAME_LEN], sname[MAX_FILENAME_LEN];

	switch (event)
		{
		case EVENT_COMMIT:
			GetPrivateDataFolder("Microscope Data", path);
			if (!FileExists(path, NULL)) MakeDir (path);
	
			if (FileSelectPopup (path, "StageDefaults.xml", "*.xml", "Save",
							 	 VAL_LOAD_BUTTON, 1, 1, 1, 0, path) != 1) return 0;
	
			if (!FileExists(path, NULL)) return 0;

			shortname(path, sname);
			SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_FNAME, sname);
			stage_load_parameters(path, stage);
			
			stage_send_all_params(stage);
			break;
		}
	return 0;
}

int CVICALLBACK OnParamsSave (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	char path[MAX_PATHNAME_LEN], sname[MAX_FILENAME_LEN];

	switch (event)
		{
		case EVENT_COMMIT:
			GetPrivateDataFolder("Microscope Data", path);
			if (!FileExists(path, NULL)) MakeDir (path);
	
			if (FileSelectPopup (path, "StageDefaults.xml", "*.xml", "Save",
							 	 VAL_SAVE_BUTTON, 1, 1, 1, 0, path) < 1) return 0;
	
			shortname(path, sname);
			SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_FNAME, sname);
	
			stage_save_parameters(path, stage);
			break;
		}
	return 0;
}

int CVICALLBACK OnParamsRead (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			stage_read_all_params(stage);
			break;
		}
	return 0;
}

int CVICALLBACK OnParamsSend (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			stage_send_all_params(stage);
			break;
		}
	return 0;
}

int CVICALLBACK OnParamsClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			stage_hide_params_ui (stage);
			break;
		}
	return 0;
}

int CVICALLBACK OnSetMoveType (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_MOVETYPE, &stage->_move_type);
			break;
		}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// XY params

int CVICALLBACK OnSetBaud (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	int baud_rate;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_BAUD, &baud_rate);
			if (baud_rate > 19200) {	//becomes unreliable
				baud_rate = 19200;
				SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_BAUD, baud_rate);
			}
			stage_set_baud_rate (stage, baud_rate);
			break;
		}
	return 0;
}


int CVICALLBACK OnPitch (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	double pitch;
	
	switch (event)
		{
		case EVENT_COMMIT:
			//We make the X and Y pitch values the same
			GetCtrlVal(panel, control, &pitch);
			stage->_pitch[XAXIS] = pitch;
			stage->_pitch[YAXIS] = pitch;
			SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_PITCH, stage->_pitch[XAXIS]);
			SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_PITCH, stage->_pitch[YAXIS]);
			stage_set_pitch(stage, XAXIS, pitch);
			stage_set_pitch(stage, YAXIS, pitch);
			SetCtrlAttribute (stage->_params_ui_panel, XYZ_PARAMS_X_SPEED, ATTR_MAX_VALUE, 45.0*stage->_pitch[XAXIS]);
			SetCtrlAttribute (stage->_params_ui_panel, XYZ_PARAMS_Y_SPEED, ATTR_MAX_VALUE, 45.0*stage->_pitch[YAXIS]);
			SetCtrlAttribute (stage->_params_ui_panel, XYZ_PARAMS_XY_CAL_SPEED, ATTR_MAX_VALUE, 45.0*min(stage->_pitch[XAXIS], stage->_pitch[YAXIS]));
			break;
		}
	return 0;
}

int CVICALLBACK OnZPitch (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	double pitch;
	
	switch (event)
		{
		case EVENT_COMMIT:
			//We make the X and Y pitch values the same
			GetCtrlVal(panel, control, &pitch);
			stage->_pitch[ZAXIS] = pitch;
			stage_set_pitch(stage, ZAXIS, pitch);
			break;
		}
	return 0;
}

int CVICALLBACK OnSpeed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	double speed;
	
	switch (event)
		{
		case EVENT_COMMIT:
			//We make the X and Y speed values the same
			GetCtrlVal(panel, control, &speed);
			stage->_speed[XAXIS] = speed;
			stage->_speed[YAXIS] = speed;
			stage->_speed[ZAXIS] = speed;
			stage->_default_speed[XAXIS] = speed;
			stage->_default_speed[YAXIS] = speed;
			stage->_default_speed[ZAXIS] = speed;
			SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_SPEED, stage->_default_speed[XAXIS]);
			SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_SPEED, stage->_default_speed[YAXIS]);
			SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_SPEED, stage->_default_speed[ZAXIS]);
			stage_set_speed(stage, ALL_AXIS, speed);
			break;
		}
	return 0;
}

int CVICALLBACK OnAcceleration (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	double acceleration;
	
	switch (event)
		{
		case EVENT_COMMIT:
			//We make the X and Y acceleration values the same
			GetCtrlVal(panel, control, &acceleration);
			stage->_acceleration[XAXIS] = acceleration;
			stage->_acceleration[YAXIS] = acceleration;
			stage->_acceleration[ZAXIS] = acceleration;
			SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_ACC, stage->_acceleration[XAXIS]);
			SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_ACC, stage->_acceleration[YAXIS]);
#ifndef XY_ONLY
			SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_ACC, stage->_acceleration[ZAXIS]);
#endif
			stage_set_acceleration(stage, ALL_AXIS, acceleration);
			break;
		}
	return 0;
}

int CVICALLBACK OnXdirReversed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_REVERSED, &stage->_dir[XAXIS]);
			if (stage->_dir[XAXIS] == 1) stage->_dir[XAXIS] = -1;
			else stage->_dir[XAXIS] = 1;
			break;
		}
	return 0;
}

int CVICALLBACK OnYdirReversed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_REVERSED, &stage->_dir[YAXIS]);
			if (stage->_dir[YAXIS] == 1) stage->_dir[YAXIS] = -1;
			else stage->_dir[YAXIS] = 1;
			break;
		}
	return 0;
}

int CVICALLBACK OnXenabled (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_ENABLED, &stage->_enabled_axis[XAXIS]);
			dim_xyz_controls(stage);
			break;
		}
	return 0;
}

int CVICALLBACK OnYenabled (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_ENABLED, &stage->_enabled_axis[YAXIS]);
			dim_xyz_controls(stage);
			break;
		}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// Z params

int CVICALLBACK OnZenabled (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_ENABLED, &stage->_enabled_axis[ZAXIS]);
			dim_xyz_controls(stage);
			break;
		}
	return 0;
}

int CVICALLBACK OnZLimitChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	double z_min, z_max;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_UPPER_LIMIT, &z_max);
			GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_LOWER_LIMIT, &z_min);
			if (z_max < z_min) {
				MessagePopup("Error", "Upper limit must be greater than lower limit. Please try again.");
				SetCtrlVal (stage->_params_ui_panel, XYZ_PARAMS_Z_UPPER_LIMIT, stage->_limits.max_z);
				SetCtrlVal (stage->_params_ui_panel, XYZ_PARAMS_Z_LOWER_LIMIT, stage->_limits.min_z);
				break;
			}

			if ((stage->focus_drive->_position < z_min) || (stage->focus_drive->_position > z_max)) {
				if (!ConfirmPopup("Warning", "Focus position is outside the new limit. I'll have to move it. Are you sure?")) break;
				
				if (stage->focus_drive->_position < z_min)
					stage_goto_z_position(stage, z_min - stage->_datum.z);
				else
					stage_goto_z_position(stage, z_max - stage->_datum.z);
			}

			focus_drive_set_range(stage->focus_drive, z_min, z_max, 1.0);
			stage->_limits.min_z = z_min;
			stage->_limits.max_z = z_max;
			stage_save_limits(stage);
			break;
		}
	return 0;
}

#ifdef XY_ONLY
int CVICALLBACK OnZSpeedChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	double speed;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_SPEED, &speed);
			focus_drive_set_speed(stage->focus_drive, speed);
			break;
		}
	return 0;
}

int CVICALLBACK OnZToleranceChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	int tolerance;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_TOLERANCE, &tolerance);
			focus_drive_set_tolerance(stage->focus_drive, tolerance);
			break;
		}
	return 0;
}

int CVICALLBACK OnZDialEnabled (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	int enabled;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_DIAL_ENABLED, &enabled);
			if (enabled)
				focus_drive_set_remote(stage->focus_drive, 4);
			else
				focus_drive_set_remote(stage->focus_drive, 3);
			break;
		}
	return 0;
}

int CVICALLBACK OnZDialSensitivityChange (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;
	int sensitivity;
	
	switch (event)
		{
		case EVENT_COMMIT:
			//Set the sensitivity of fine focus wheel
			//1 - coarse, 2 - medium, 3 - fine
			
			GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_SENSITIVITY, &sensitivity);
			focus_drive_set_dial_sensitivity(stage->focus_drive, sensitivity);
			break;
		}
	return 0;
}
#endif

int CVICALLBACK OnZdirReversed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_REVERSED, &stage->_dir[ZAXIS]);
			if (stage->_dir[ZAXIS] == 1) stage->_dir[ZAXIS] = -1;
			else stage->_dir[ZAXIS] = 1;
			break;
		}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////

int stage_send_all_params(Stage *stage)
{
	int tolerance, sensitivity, enabled;
	
	//Read all the params from the panel. However, only send the speed and acceleration.
	//The others are sent during the initialisation sequence only.

	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_ENABLED, &stage->_enabled_axis[XAXIS]);
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_REVERSED, &stage->_dir[XAXIS]);
	if (stage->_dir[XAXIS] == 1) stage->_dir[XAXIS] = -1;
	else stage->_dir[XAXIS] = 1;
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_BACKLASH_X, &stage->_backlash[XAXIS]);
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_PITCH, &stage->_pitch[XAXIS]);
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_SPEED, &stage->_speed[XAXIS]);
	stage_set_speed(stage, XAXIS, stage->_speed[XAXIS]);
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_ACC, &stage->_acceleration[XAXIS]);
	stage_set_acceleration(stage, XAXIS, stage->_acceleration[XAXIS]);
	
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_ENABLED, &stage->_enabled_axis[YAXIS]);
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_REVERSED, &stage->_dir[YAXIS]);
	if (stage->_dir[YAXIS] == 1) stage->_dir[YAXIS] = -1;
	else stage->_dir[YAXIS] = 1;
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_BACKLASH_Y, &stage->_backlash[YAXIS]);
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_PITCH, &stage->_pitch[YAXIS]);
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_SPEED, &stage->_speed[YAXIS]);
	stage_set_speed(stage, YAXIS, stage->_speed[YAXIS]);
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_ACC, &stage->_acceleration[YAXIS]);
	stage_set_acceleration(stage, YAXIS, stage->_acceleration[YAXIS]);
	
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_ENABLED, &stage->_enabled_axis[ZAXIS]);
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_REVERSED, &stage->_dir[ZAXIS]);
	if (stage->_dir[ZAXIS] == 1) stage->_dir[ZAXIS] = -1;
	else stage->_dir[ZAXIS] = 1;
#ifdef XY_ONLY
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_SPEED, &stage->_speed[ZAXIS]);
	focus_drive_set_speed(stage->focus_drive, stage->_speed[ZAXIS]);
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_TOLERANCE, &tolerance);
	focus_drive_set_tolerance(stage->focus_drive, tolerance);
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_DIAL_ENABLED, &enabled);
	if (enabled)
		focus_drive_set_remote(stage->focus_drive, 4);
	else
		focus_drive_set_remote(stage->focus_drive, 3);
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_SENSITIVITY, &sensitivity);
	focus_drive_set_dial_sensitivity(stage->focus_drive, sensitivity);
#else
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_BACKLASH_Z, &stage->_backlash[ZAXIS]);
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_PITCH, &stage->_pitch[ZAXIS]);
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_SPEED, &stage->_speed[ZAXIS]);
	stage_set_speed(stage, ZAXIS, stage->_speed[ZAXIS]);
	GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_ACC, &stage->_acceleration[ZAXIS]);
	stage_set_acceleration(stage, ZAXIS, stage->_acceleration[ZAXIS]);
#endif

	return STAGE_SUCCESS;
}

int stage_read_all_params(Stage *stage)
{
	int tolerance, sensitivity, remote;
	
	//Read all the params from the stage contoller

	stage_get_speed(stage, XAXIS, &stage->_speed[XAXIS]);
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_SPEED, stage->_speed[XAXIS]);
	stage_get_acceleration(stage, XAXIS, &stage->_acceleration[XAXIS]);
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_X_ACC, stage->_acceleration[XAXIS]);
	
	stage_get_speed(stage, YAXIS, &stage->_speed[YAXIS]);
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_SPEED, stage->_speed[YAXIS]);
	stage_get_acceleration(stage, YAXIS, &stage->_acceleration[YAXIS]);
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Y_ACC, stage->_acceleration[YAXIS]);
	
#ifdef XY_ONLY
	focus_drive_get_speed(stage->focus_drive, &stage->_speed[ZAXIS]);
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_SPEED, stage->_speed[ZAXIS]);
	focus_drive_get_tolerance(stage->focus_drive, &tolerance);
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_TOLERANCE, tolerance);
	focus_drive_get_remote(stage->focus_drive, &remote);
	if (remote == 3)
		SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_DIAL_ENABLED, 0);
	else
		SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_DIAL_ENABLED, 1);
	focus_drive_get_dial_sensitivity(stage->focus_drive, &sensitivity);
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_SENSITIVITY, sensitivity);
#else
	stage_get_speed(stage, ZAXIS, &stage->_speed[ZAXIS]);
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_SPEED, stage->_speed[ZAXIS]);
	stage_get_acceleration(stage, ZAXIS, &stage->_acceleration[ZAXIS]);
	SetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_Z_ACC, stage->_acceleration[ZAXIS]);
#endif

	return STAGE_SUCCESS;
}

int CVICALLBACK OnZtest (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int i;
	double xCur, yCur, zCur, t1;
	Stage *stage = (Stage*) callbackData;

	switch (event)
	{
		case EVENT_COMMIT:
			if(stage_get_xyz_position(stage, &xCur, &yCur, &zCur) == STAGE_ERROR)
				return STAGE_ERROR;	
	 		
	 		t1 = Timer();
	 		
	 		//for (i = 0; i<1000; i++)							//n moves
			//	stage_rel_move_by_z (stage, -1.0);				//rel
	 		//for (i = 0; i<1000; i++)							//n moves
			//	stage_rel_move_by_z (stage, 1.0);				//rel

	 		for (i = 0; i<10; i++)								//10 moves
	 			stage_goto_z_position (stage, zCur + i*40) ; 	//abs
			
			printf("T = %f secs\n", Timer() - t1);
			
			break;
	}
	return 0;
}

int CVICALLBACK OnCalSpeed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Stage *stage = (Stage*) callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_XY_CAL_SPEED, &stage->_cal_speed_1);
			GetCtrlVal(stage->_params_ui_panel, XYZ_PARAMS_XY_CAL_SPEED_2, &stage->_cal_speed_2);
			break;
		}
	return 0;
}
