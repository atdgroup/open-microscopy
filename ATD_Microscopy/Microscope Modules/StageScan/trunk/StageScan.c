#include "RS232CorvusXY\RS232Corvus_Communication.h" 

#include "StageScan.h"
#include "StageScan_ui.h"
#include "string_utils.h"
#include "GL_CVIRegistry.h"
#include "RegionOfInterest.h"

#include <rs232.h>
#include <userint.h>
#include <utility.h> 

#include "stage\stage.h"

#include "gci_utils.h"

////////////////////////////////////////////////////////////////////
// Module to perform XY stage scan
// Ros Locke - November 2006
////////////////////////////////////////////////////////////////////

// Added comments.
// Parts of this module are highly dependant on the Corvus stage controller
// These parts should be moved out into the stage module - once they are properly 'identified' 
// regarding what they need to do. They can they be implemented for other stages as required.
// There may be other parts later that are highly dependant on other things - e.g. z-drive, data acq.
// Paul Barber - Feb 2008

// This Stage Scan module was initially written by Ros Locke to be a stage scanning FLIM acquisition.
// That was never finished. It was then modified to become the Stage Scan Overview beta code, that
// used a continuous stage motion with a camera in a fast low resolution imaging mode to form a low
// resolution image of the whole sample (no FLIM).
// This stayed in a 'beta' form for some time and proved to be very useful, although it had a clunky UI.
// Until May 2012 when I decided/had a little time to make the code clearer and improve the UI (I hope).
// So, the code that remains uncommented in this file is needed by the Stage Scan Overview, the commented
// code may still be useful if we ever do the Stage Scan FLIM originally intended.
// Paul Barber - May 2012

//#define SS_STANDALONE

static int stage_scan_single_frame_xy_simple(stage_scan *ss);

static int GCI_ImagingStageFastAbsXYZ(XYStage* stage, double x, double y)
{
	int retval;
	
	if (stage == NULL)
		return MICROSCOPE_SUCCESS;
	
	retval = stage_async_goto_xy_position(stage, x, y);

	return retval;
}

static int the_stage_is_moving(XYStage* stage)
{
	int status = 0;
	
	stage_is_moving(stage, &status);
	
	return status;
}

void stage_scan_dim_ui_controls(stage_scan *ss, int dim)
{
	if (dim) 
	{
		ui_module_disable_panel(ss->stage_scan_panel, 4, STAGE_SCAN_PAUSE, STAGE_SCAN_STOP, STAGE_SCAN_TIME_TAKEN, STAGE_SCAN_FRAME_TIME); 
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_PAUSE, ATTR_DIMMED, 0);
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_STOP, ATTR_DIMMED, 0);

		ui_module_disable_panel(ss->overview_scan_panel, 4, OVERVIEW_PAUSE, OVERVIEW_STOP, OVERVIEW_TIME_TAKEN, OVERVIEW_FRAME_TIME); 
		SetCtrlAttribute (ss->overview_scan_panel, OVERVIEW_PAUSE, ATTR_DIMMED, 0);
		SetCtrlAttribute (ss->overview_scan_panel, OVERVIEW_STOP, ATTR_DIMMED, 0);
	}	
	else
	{
		ui_module_enable_panel (ss->stage_scan_panel, 2, STAGE_SCAN_PAUSE, STAGE_SCAN_STOP);  
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_PAUSE, ATTR_DIMMED, 1);
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_STOP, ATTR_DIMMED, 1);

		ui_module_enable_panel (ss->overview_scan_panel, 2, OVERVIEW_PAUSE, OVERVIEW_STOP);  
		SetCtrlAttribute (ss->overview_scan_panel, OVERVIEW_PAUSE, ATTR_DIMMED, 1);
		SetCtrlAttribute (ss->overview_scan_panel, OVERVIEW_STOP, ATTR_DIMMED, 1);
	}
}

static void OnStageRead(XYStage* stage, double x, double y, void *callback_data)
{   // callback for the stage changed signal
	int mode;
	double left, top, right, bottom;
	stage_scan *ss = (stage_scan *) callback_data; 

	if(ss == NULL || ss->stage == NULL)
		return;

	//The XYZ stage module has read the position, so display it
	if (ss->stage_scan_panel >= 0) {
		
		GetAxisScalingMode (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, VAL_BOTTOM_XAXIS, &mode, &left, &right);
		GetAxisScalingMode (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, VAL_LEFT_YAXIS, &mode, &top, &bottom);

		if (x<left)
			x=left; 

		if (x>right)
			x=right;

		if (y<top)
			y=top;

		if (y>bottom)
			y=bottom;

		SetGraphCursor (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, 1, x, y);
	}
}


static void ShowElapsedTime(stage_scan *ss)
{
	char time_taken[50] = "";
	double time = Timer() - ss->start_time, line_time;
	
	seconds_to_friendly_time(time, time_taken);   
	SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_TIME_TAKEN, time_taken);   
	SetCtrlVal(ss->overview_scan_panel, OVERVIEW_TIME_TAKEN, time_taken);   
	
	// Calculate Average Line Time
	if (ss->current_line < 1)
		return;   
	
	time_taken[0] = '0';

	if(ss->total_lines  > 0) {
		line_time = time / ss->total_lines;   
		seconds_to_friendly_time(line_time, time_taken);   
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_LINE_TIME, time_taken); 
	}
}

static void CalcMoveTime(double dist, double speed, double accel, double *t)
{
	double s, d;
	
	if (dist == 0) {
		*t = 0.0;
		return;
	}

	if ((accel == 0) || (speed == 0)) {
		*t = 10.0;	//Can't calculate it. Assume max move time of 10 seconds
		return;
	}

	d = fabs(dist);
	s = speed*speed/(2*accel);		//distance to reach full speed from rest
	if (s > (d/2)) 
		*t = 2*sqrt(2*(d/2)/accel);	//accelerate then decellerate at same rate
	else {
		*t = 2*(speed/accel);		//Time to reach full speed and decellerate
		*t += (d - (2*s))/speed;	//Time to move remaining distance
	}
}

static void stage_scan_calc_xy_frame_time(stage_scan *ss)
{
	double acceleration, speed, s, t, ty;
	int rows, p1, p2, p3, p4, height;
	double left, right, top, bottom, aspect_ratio;
	StageDirection horzDir, vertDir;  
	
	//Calculate distance required to accelerate from rest
	acceleration = ss->stage_acceleration * 1000.0;	//microns per sec per sec
	speed = ss->stage_speed * 1000.0;				//microns per sec
	
	t = 2*(speed/acceleration);			 //Time to reach full speed and decellerate
	t += (ss->frame_roi_width)/speed; //Add time moving at constant speed
	//t += (ss->frame_roi_width+10)/speed; //Add time moving at constant speed
	
	if (t>3600) {
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_LINE_TIME_2, t/3600);
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_UNITS, "hours");
	}
	else if (t>60) {
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_LINE_TIME_2, t/60);
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_UNITS, "mins");
	}
	else {
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_LINE_TIME_2, t);
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_UNITS, "secs");
	}

	rows = (int) (ss->frame_roi_height/ss->fov_y/ss->detectors);
	t *= rows;
	
	//Add on the time for the y moves
	if (ss->two_speed)
		speed = ss->stage_speed_2 * 1000.0;
	CalcMoveTime(ss->fov_y, speed, acceleration, &ty);
	t += rows*ty;
	
	//Add communication time - 50ms per "startmakro" command
	//t += rows/2 * 0.050;
	//Add inter-macro time - 180ms (includes "startmakro" command), found impirically
	t += rows/2 * 0.180;
	//Add 1 second for macro download time
	t += 1;
	
	if (t>3600) {
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FRAME_TIME, t/3600);
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_UNITS, "hours");
		SetCtrlVal(ss->overview_scan_panel, OVERVIEW_FRAME_TIME, t/3600);
		SetCtrlVal(ss->overview_scan_panel, OVERVIEW_UNITS, "hours");
	}
	else if (t>60) {
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FRAME_TIME, t/60);
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_UNITS, "mins");
		SetCtrlVal(ss->overview_scan_panel, OVERVIEW_FRAME_TIME, t/60);
		SetCtrlVal(ss->overview_scan_panel, OVERVIEW_UNITS, "mins");
	}
	else {
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FRAME_TIME, t);
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_UNITS, "secs");
		SetCtrlVal(ss->overview_scan_panel, OVERVIEW_FRAME_TIME, t);
		SetCtrlVal(ss->overview_scan_panel, OVERVIEW_UNITS, "secs");
	}
	ss->frame_time = t;
	
	// Set up the position display
	// Set up graph axes, correcting for direction of travel
	DeleteGraphPlot (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, -1, VAL_IMMEDIATE_DRAW);
	
	speed = ss->stage_speed * 1000.0;		//microns per sec
	s = speed*speed/(2*acceleration);		//distance to reach full speed from rest
	left = ss->frame_roi_left - s - 20;
	right = ss->frame_roi_left + ss->frame_roi_width + s + 20;
	top = ss->frame_roi_top - s -20;
	bottom = ss->frame_roi_top + ss->frame_roi_height + s + 20;

	aspect_ratio = fabs((double)(right-left)/(bottom-top));
	GetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, ATTR_HEIGHT, &height);
	if ((int)(height*aspect_ratio) <= 160) 
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, ATTR_WIDTH, (int)(height*aspect_ratio));
	else {
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, ATTR_WIDTH, 160);
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, ATTR_HEIGHT, (int)(160.0/aspect_ratio));
	}
	
	SetAxisScalingMode (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, VAL_BOTTOM_XAXIS, VAL_MANUAL, left, right);
	SetAxisScalingMode (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, VAL_LEFT_YAXIS, VAL_MANUAL, top, bottom);
	
	horzDir = stage_get_axis_dir(ss->stage, XAXIS);    
	vertDir = stage_get_axis_dir(ss->stage, YAXIS);  
	
	
	if (horzDir == STAGE_POSITIVE_TO_NEGATIVE)
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, ATTR_XREVERSE, 1);
	else
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, ATTR_XREVERSE, 0);
	
	if (vertDir == STAGE_POSITIVE_TO_NEGATIVE)
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, ATTR_YREVERSE, 1);
	else
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, ATTR_YREVERSE, 0);
	
	//Indicate start and end of the scan lines
	p1 = PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, left+s+20, bottom, left+s+20, top, VAL_BLUE);
	SetPlotAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, p1, ATTR_LINE_STYLE, VAL_DASH);
	p2 = PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, right-s-20, bottom, right-s-20, top, VAL_BLUE);
	SetPlotAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, p2, ATTR_LINE_STYLE, VAL_DASH);
	
	p3 = PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, left, top+s+20, right, top+s+20, VAL_BLUE);
	SetPlotAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, p3, ATTR_LINE_STYLE, VAL_DASH);
	p4 = PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, left, bottom-s-20, right, bottom-s-20, VAL_BLUE);
	SetPlotAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, p4, ATTR_LINE_STYLE, VAL_DASH);

	ss->um_to_accelerate = s;  
	//ss->um_to_accelerate = s + 5;  //Allow a bit extra
}
static void stage_scan_calc_yx_frame_time(stage_scan *ss)
{
	double acceleration, speed, s, t, tx;
	int rows, p1, p2, p3, p4, height;
	double left, right, top, bottom, aspect_ratio;
	StageDirection horzDir, vertDir;    
	
	//Calculate distance required to accelerate from rest
	acceleration = ss->stage_acceleration * 1000.0;	//microns per sec per sec
	speed = ss->stage_speed * 1000.0;				//microns per sec
	
	t = 2*(speed/acceleration);			 //Time to reach full speed and decellerate
	t += (ss->frame_roi_height)/speed; //Add time moving at constant speed
	//t += (ss->frame_roi_height+10)/speed; //Add time moving at constant speed
	
	if (t>3600) {
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_LINE_TIME_2, t/3600);
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_UNITS, "hours");
	}
	else if (t>60) {
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_LINE_TIME_2, t/60);
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_UNITS, "mins");
	}
	else {
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_LINE_TIME_2, t);
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_UNITS, "secs");
	}

	rows = (int) (ss->frame_roi_width/ss->fov_x/ss->detectors);
	t *= rows;
	
	//Add on the time for the x moves
	if (ss->two_speed)
		speed = ss->stage_speed_2 * 1000.0;
	CalcMoveTime(ss->fov_x, speed, acceleration, &tx);
	t += rows*tx;
	
	//Add communication time - 50ms per "startmakro" command
	//t += rows/2 * 0.050;
	//Add inter-macro time - 180ms (includes "startmakro" command), found impirically
	t += rows/2 * 0.180;
	//Add 1 second for macro download time
	t += 1;
	
	if (t>3600) {
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FRAME_TIME, t/3600);
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_UNITS, "hours");
		SetCtrlVal(ss->overview_scan_panel, OVERVIEW_FRAME_TIME, t/3600);
		SetCtrlVal(ss->overview_scan_panel, OVERVIEW_UNITS, "hours");
	}
	else if (t>60) {
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FRAME_TIME, t/60);
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_UNITS, "mins");
		SetCtrlVal(ss->overview_scan_panel, OVERVIEW_FRAME_TIME, t/60);
		SetCtrlVal(ss->overview_scan_panel, OVERVIEW_UNITS, "mins");
	}
	else {
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FRAME_TIME, t);
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_UNITS, "secs");
		SetCtrlVal(ss->overview_scan_panel, OVERVIEW_FRAME_TIME, t);
		SetCtrlVal(ss->overview_scan_panel, OVERVIEW_UNITS, "secs");
	}
	ss->frame_time = t;
	
	// Set up the position display
	// Set up graph axes, correcting for direction of travel
	DeleteGraphPlot (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, -1, VAL_IMMEDIATE_DRAW);
	
	speed = ss->stage_speed * 1000.0;		//microns per sec
	s = speed*speed/(2*acceleration);		//distance to reach full speed from rest
	top = ss->frame_roi_top - s - 20;
	bottom = ss->frame_roi_top + ss->frame_roi_height + s + 20;
	left = ss->frame_roi_left - s -20;
	right = ss->frame_roi_left + ss->frame_roi_width + s + 20;

	aspect_ratio = fabs((double)(right-left)/(bottom-top));
	GetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, ATTR_HEIGHT, &height);
	if ((int)(height*aspect_ratio) <= 160) 
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, ATTR_WIDTH, (int)(height*aspect_ratio));
	else {
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, ATTR_WIDTH, 160);
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, ATTR_HEIGHT, (int)(160.0/aspect_ratio));
	}
	
	SetAxisScalingMode (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, VAL_LEFT_YAXIS, VAL_MANUAL, top, bottom);
	SetAxisScalingMode (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, VAL_BOTTOM_XAXIS, VAL_MANUAL, left, right);
	
	horzDir = stage_get_axis_dir(ss->stage, XAXIS);    
	vertDir = stage_get_axis_dir(ss->stage, YAXIS);  
	
	if (horzDir == STAGE_NEGATIVE_TO_POSITIVE)
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, ATTR_XREVERSE, 1);
	else
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, ATTR_XREVERSE, 0);
	
	if (vertDir == STAGE_POSITIVE_TO_NEGATIVE)
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, ATTR_YREVERSE, 1);
	else
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, ATTR_YREVERSE, 0);
	
	//Indicate start and end of the scan lines
	p1 = PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, left+s+20, bottom, left+s+20, top, VAL_BLUE);
	SetPlotAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, p1, ATTR_LINE_STYLE, VAL_DASH);
	p2 = PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, right-s-20, bottom, right-s-20, top, VAL_BLUE);
	SetPlotAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, p2, ATTR_LINE_STYLE, VAL_DASH);
	
	p3 = PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, left, top+s+20, right, top+s+20, VAL_BLUE);
	SetPlotAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, p3, ATTR_LINE_STYLE, VAL_DASH);
	p4 = PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, left, bottom-s-20, right, bottom-s-20, VAL_BLUE);
	SetPlotAttribute (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, p4, ATTR_LINE_STYLE, VAL_DASH);

	ss->um_to_accelerate = s;
	//ss->um_to_accelerate = s + 5;  //Allow a bit extra
}

int CVICALLBACK cbStageScanSetROI (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			stage_scan *ss = (stage_scan *) callbackData; 

			ui_module_clear_attached_panels(UIMODULE_CAST(ss));

			ui_module_attach_panel_to_panel(UIMODULE_CAST(ss), ss->roi->region_panel,
														  UI_MODULE_REL_TOP_RIGHT,
														  5, 0);

			//region_of_interest_panel_display(ss->roi, UIMODULE_MAIN_PANEL_ID(ss));
			region_of_interest_display(ss->roi);

			}break;
		}
	return 0;
}

int CVICALLBACK cbSetScanParams (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			stage_scan *ss = (stage_scan *) callbackData; 
			double fov, speed_max;

			if (control == STAGE_SCAN_EXPOSURE) {
				//Exposure has changed - modify the speed
				if (ss->scan_type == XY_SCAN) 
					GetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FOV_X, &fov);	  		//um
				else
					GetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FOV_Y, &fov);
				GetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_EXPOSURE, &ss->exposure);  	//ms
				ss->exposure /= 1000.0;												   	//sec
				ss->stage_speed = fov/1000.0/ss->exposure;					   			//mm/sec
			
				GetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_STAGE_SPEED, ATTR_MAX_VALUE, &speed_max);
				if (ss->stage_speed > speed_max) {
					ss->stage_speed = speed_max;
					ss->exposure = fov/1000.0/ss->stage_speed;					  		//secs
					SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_EXPOSURE, ss->exposure*1000.0); //ms
				}
			
				SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_STAGE_SPEED, ss->stage_speed);
			}
			
			if (ss->scan_type == XY_SCAN) 
				stage_scan_calc_xy_frame_time(ss);
			else
				stage_scan_calc_yx_frame_time(ss);
			}break;
		}
	return 0;
}

int CVICALLBACK cbSetFOV (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			stage_scan *ss = (stage_scan *) callbackData; 
			double fov;
	
			//Field of View has changed - modify the exposure. Is this still right?
			//GetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_STAGE_SPEED, &ss->stage_speed);
			GetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FOV_X, &ss->fov_x); //um
			GetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FOV_Y, &ss->fov_y);
			
			if (ss->scan_type == XY_SCAN)
				fov = ss->fov_x;
			else
				fov = ss->fov_y;
			ss->exposure = fov/1000.0/ss->stage_speed;					  //secs
			SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_EXPOSURE, ss->exposure*1000.0); //ms
			if (ss->scan_type == XY_SCAN) 
				stage_scan_calc_xy_frame_time(ss);
			else
				stage_scan_calc_yx_frame_time(ss);
			}break;
		}
	return 0;
}

int CVICALLBACK cbSetStageSpeed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			stage_scan *ss = (stage_scan *) callbackData; 
			double fov;
	
			//Speed has changed - modify the exposure
			GetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_STAGE_SPEED, &ss->stage_speed);
			if (ss->scan_type == XY_SCAN) 
				GetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FOV_X, &fov); //um
			else
				GetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FOV_Y, &fov);
			ss->exposure = fov/1000.0/ss->stage_speed;					  //secs
			SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_EXPOSURE, ss->exposure*1000.0); //ms
			if (ss->scan_type == XY_SCAN) 
				stage_scan_calc_xy_frame_time(ss);
			else
				stage_scan_calc_yx_frame_time(ss);
			}break;
		}
	return 0;
}

int CVICALLBACK cbSetStageAcceleration (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			stage_scan *ss = (stage_scan *) callbackData; 

			GetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_STAGE_ACCELERATION, &ss->stage_acceleration);
			stage_set_acceleration(ss->stage, ALL_AXIS, ss->stage_acceleration);
			
			if (ss->scan_type == XY_SCAN) 
				stage_scan_calc_xy_frame_time(ss);
			else
				stage_scan_calc_yx_frame_time(ss);
			}break;
		}
	return 0;
}

int CVICALLBACK cbStageScanJoystickEnable (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			int joyOn;
			stage_scan *ss = (stage_scan *) callbackData; 
	
			GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_JOY_ON, &joyOn);
			if (joyOn) stage_set_joystick_off (ss->stage);
			else stage_set_joystick_on (ss->stage);
			SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_JOY_ON, !joyOn);
			}break;
		}
	return 0;
}

int CVICALLBACK cbStageScanStageReinitialise (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			stage_scan *ss = (stage_scan *) callbackData;     
	
			
			stage_find_initialise_extents(ss->stage, 1); 
			}break;
		}
	
	return 0;
}

int CVICALLBACK cbSetNoDetectors (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			stage_scan *ss = (stage_scan *) callbackData; 

			GetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_NO_DETECTORS, &ss->detectors);
			if (ss->scan_type == XY_SCAN) 
				stage_scan_calc_xy_frame_time(ss);
			else
				stage_scan_calc_yx_frame_time(ss);
			}break;
		}
	return 0;
}

int CVICALLBACK cbStageScanStart (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

/*			This was the last state of the stage scan for FLIM

			stage_scan *ss = (stage_scan *) callbackData; 
			int joyOn, i;
			double speed_cache=10.0;
	
			Microscope *ms = microscope_get_microscope();   
			UI_MODULE_ERROR_HANDLER err_handler = NULL;
	
			err_handler = ui_module_get_error_handler(UIMODULE_CAST(ms->_stage));
			
			// Turn off stage errors.
			// This module does not seem to take account of thrown errors.
			//TODO
			//stage_set_error_handler(ms->_stage, NULL, NULL);
			
			stage_get_speed(ss->stage, ALL_AXIS, &speed_cache);
			stage_set_speed(ss->stage, ALL_AXIS, ss->stage_speed); 
			stage_set_acceleration(ss->stage, ALL_AXIS, ss->stage_acceleration);
			
			GetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FRAMES, &ss->frames);
			
			stage_scan_dim_ui_controls(ss, 1);
			
			ss->pause_scan = 0;
			ss->abort_scan = 0; 

			stage_stop_timer(ss->stage); 
			microscope_disable_all_panels(ms, 1);
			
			GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_JOY_ON, &joyOn);
			
			if (joyOn)
				stage_set_joystick_off (ss->stage);
			
			SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_JOY_ON, 0);

			GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_PULSE_WIDTH, &ss->pulse_width);
			GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_POLARITY, &ss->polarity);

			ss->start_time = Timer();
			ss->total_lines = 0;
			
			for (i=0; i<ss->frames; i++) {
				
				SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FRAME, i+1);
				
				stage_scan_calc_xy_frame_time(ss);  //Clears the graph
				
				if (ss->scan_type == XY_SCAN) {
					if(stage_scan_single_frame_xy_using_macro(ss) < 0)
						break;
				}
				else {
					stage_scan_calc_yx_frame_time(ss);  //Clears the graph
			
					if(stage_scan_single_frame_yx_using_macro(ss) < 0)
						break;
				}


				if (ss->abort_scan) 
					break;
			}
			
			if (joyOn)
				stage_set_joystick_on (ss->stage);
			
			SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_JOY_ON, joyOn);

			stage_scan_dim_ui_controls(ss, 0);
			SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_PAUSE, ATTR_LABEL_TEXT, "Pause");

			if (ss->msg_panel >= 0) {
				DiscardPanel(ss->msg_panel);
				ss->msg_panel = -1;
			}
			
			ss->pause_scan = 0;
			ss->abort_scan = 0; 
			
			stage_start_timer(ss->stage); 
			microscope_disable_all_panels(ms, 0); 

			// reset the stage speed to what it was
			stage_set_speed(ss->stage, ALL_AXIS, speed_cache);
			
			// Turn stage erros on again.
			//TODO
			//stage_set_error_handler(ms->_stage, err_handler, ms); 
*/			
			}break;
		}
	return 0;
}

static int setupCamera(stage_scan *ss)
{
	double exposure;
	BinningMode binning;

	if (gci_camera_save_state(ss->camera, &(ss->camera_state_cache))<0){
		GCI_MessagePopup("Stage Scan Error", "gci_camera_save_state failed.");
		return -1; // no need to restore state, not saved
	}
	if (gci_camera_set_highest_binning_mode(ss->camera)<0){
		GCI_MessagePopup("Stage Scan Error", "gci_camera_set_highest_binning_mode failed.");
		goto Error;
	}
	if (gci_camera_set_lowest_data_mode(ss->camera)<0){  // i.e. 8 bit if possible
		GCI_MessagePopup("Stage Scan Error", "gci_camera_set_lowest_data_mode failed.");
		goto Error;
	}
	
	binning = gci_camera_get_binning_mode(ss->camera);  // reads var, will not fail
	exposure = gci_camera_get_exposure_time(ss->camera);  // reads var, will not fail
	exposure *= pow((double)ss->camera_state_cache.binning, 2.0) / pow((double)binning, 2.0);
	if (exposure < 5.0) exposure = 5.0;
	
	if (gci_camera_set_exposure_time(ss->camera, exposure)<0){
		GCI_MessagePopup("Stage Scan Error", "gci_camera_set_exposure_time failed.");
		goto Error;
	}

	gci_camera_snap_image(ss->camera);  // just snap an initial image, othwise the rto uses the full res image we already have

	if (gci_camera_set_live_mode(ss->camera)<0){
		GCI_MessagePopup("Stage Scan Error", "gci_camera_set_live_mode failed.");
		goto Error;
	}

	return 0;

Error:
	// put camera back how it was
	gci_camera_restore_state(ss->camera, &(ss->camera_state_cache));
	return -1;
}

static void setupFOV(stage_scan *ss)
{
	int left, top, width, height;
	unsigned char ac;
	double umperpixel;
	gci_camera_get_size_position(ss->camera, &left, &top, &width, &height, &ac);
	umperpixel = gci_camera_get_true_microns_per_pixel(ss->camera);
	ss->fov_x = width * umperpixel;
	ss->fov_y = height * umperpixel;
	SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FOV_X, ss->fov_x);     
	SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FOV_Y, ss->fov_y); 
	return;
}

static int stagescan_get_closest_point_within_safe_region(stage_scan *ss, double x, double y, double *changed_x, double *changed_y)
{
	// safety in case nothing changes
	*changed_x = x;
	*changed_y = y;
	
	if(!ss->stage->_safe_region_enabled) {
		return 0;
	}

	if(ss->stage->_safe_region.shape == STAGE_SHAPE_CIRCLE){
		// could do better here!
		return -1;
	}
	else {
		if(x <= ss->stage->_safe_region.roi.min_x)
			*changed_x = ss->stage->_safe_region.roi.min_x + ss->um_to_accelerate;	 
		else if(x >= ss->stage->_safe_region.roi.max_x)
			*changed_x = ss->stage->_safe_region.roi.max_x - ss->um_to_accelerate;
			
		if(y <= ss->stage->_safe_region.roi.min_y)
			*changed_y = ss->stage->_safe_region.roi.min_y + ss->um_to_accelerate;		
		else if(y >= ss->stage->_safe_region.roi.max_y)
			*changed_y = ss->stage->_safe_region.roi.max_y - ss->um_to_accelerate;
	}
	return 0;
}

int CVICALLBACK cbOverviewStart (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	stage_scan *ss = (stage_scan *) callbackData; 
	double speed_cache=10.0; 
	int wherearewe = 0;

	switch (event)
		{
		case EVENT_COMMIT:{
			int joyOn;
			double obj_mag=20.0;
			double tmp_x, tmp_y;
			double bottom, right;
			double max_val;
			
			Microscope *ms = microscope_get_microscope();   
			UI_MODULE_ERROR_HANDLER err_handler = NULL;
	
			if(ms->_stage == NULL)
				return -1;

			if (setupCamera(ss)<0)
				return -1;

			// DO WE NEED TO HAVE RTO CLOSED NOW? We get the rto from the microscope below ...
			// template code below to close the rto
	//		if (close_realtimeoverview() <0){  // if the realtime overview is open seperately, close it now, will interfere with stage scan
	//			// put camera back how it was, do not use Error, init not finished
	//			gci_camera_restore_state(ss->camera, &(ss->camera_state_cache));
	//			return -1;
	//		}

			err_handler = ui_module_get_error_handler(UIMODULE_CAST(ms->_stage));

			logger_log(UIMODULE_LOGGER(ss), LOGGER_INFORMATIONAL, "Stage Scan Overview started.");

			region_of_interest_clear_button_state(ss->roi);
			
			wherearewe = 1;

			// Setup the realtime overview
			ss->rto = microscope_get_realtime_overview(ss->ms);
			max_val = pow(2.0, (double)gci_camera_get_data_mode(ss->camera));
			realtime_overview_set_image_max_scale_value(ss->rto, max_val);

			realtime_overview_init(ss->rto);		
			realtime_overview_activate(ss->rto);
			realtime_overview_display(ss->rto);
			
			setupFOV (ss);
			
			wherearewe = 2;

			// store current speed
			stage_get_speed(ss->stage, ALL_AXIS, &speed_cache);
		
			// Check top left of region to make it is in the safe stage region, modify if neccessary
			stagescan_get_closest_point_within_safe_region(ss, ss->frame_roi_left, ss->frame_roi_top, &tmp_x, &tmp_y);
			ss->frame_roi_width -= fabs(tmp_x-ss->frame_roi_left);
			ss->frame_roi_height -= fabs(tmp_y-ss->frame_roi_top);
			ss->frame_roi_left = tmp_x;
			ss->frame_roi_top = tmp_y;

			wherearewe = 3;

			// Check bottm right of region to make it is in the safe stage region, modify width height if neccessary
			bottom = ss->frame_roi_top+ss->frame_roi_height;
			right  = ss->frame_roi_left+ss->frame_roi_width;
			stagescan_get_closest_point_within_safe_region(ss, right, bottom, &tmp_x, &tmp_y);
			ss->frame_roi_width -= fabs(tmp_x-right) + ss->um_to_accelerate;
			ss->frame_roi_height -= fabs(tmp_y-bottom) + ss->um_to_accelerate;

			wherearewe = 4;

			if(!stage_check_is_within_xy_limits_and_safe_region(ss->stage, ss->frame_roi_left, ss->frame_roi_top+ss->frame_roi_height)) {
				
				GCI_MessagePopup("Error", "Stage Scan Overview corner is outside the limits or safe region.");
				goto Error;
			}

			if(!stage_check_is_within_xy_limits_and_safe_region(ss->stage, ss->frame_roi_left+ss->frame_roi_width, ss->frame_roi_top+ss->frame_roi_height)) {
				
				GCI_MessagePopup("Error", "Stage Scan Overview corner is outside the limits or safe region.");
				goto Error;
			}

			if(!stage_check_is_within_xy_limits_and_safe_region(ss->stage,ss->frame_roi_left+ss->frame_roi_width, ss->frame_roi_top)) {
				
				GCI_MessagePopup("Error", "Stage Scan Overview corner is outside the limits or safe region.");
				goto Error;
			}

			if(!stage_check_is_within_xy_limits_and_safe_region(ss->stage, ss->frame_roi_left, ss->frame_roi_top)) {
				
				GCI_MessagePopup("Error", "Stage Scan Overview corner is outside the limits or safe region.");
				goto Error;
			}

			// visit the scan corners to init the rto
			if (stage_goto_xy_position(ss->stage, ss->frame_roi_left, ss->frame_roi_top+ss->frame_roi_height)<0){
				wherearewe = 5;
				goto Error;
			}

			if (stage_goto_xy_position(ss->stage, ss->frame_roi_left+ss->frame_roi_width, ss->frame_roi_top+ss->frame_roi_height)<0) {
				wherearewe = 6;
				goto Error;
			}

			if (stage_goto_xy_position(ss->stage, ss->frame_roi_left+ss->frame_roi_width, ss->frame_roi_top)<0) {
				wherearewe = 7;
				goto Error;
			}

			if (stage_goto_xy_position(ss->stage, ss->frame_roi_left, ss->frame_roi_top)<0) {
				wherearewe = 8;
				goto Error;
			}
		
			// set the speed for the stage scan adjusted for the current x fov
			GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_STAGE_SPEED, &(ss->overview_speed));  
			ss->stage_speed = ss->overview_speed / ss->overview_calib_fov * ss->fov_x;
			
			if (stage_set_speed(ss->stage, ALL_AXIS, ss->stage_speed)<0) {
				wherearewe = 9;
				goto Error;  
			}

			if (stage_set_acceleration(ss->stage, ALL_AXIS, ss->stage_acceleration)<0) {
				wherearewe = 10;
				goto Error;
			}
				
			stage_scan_dim_ui_controls(ss, 1);
			
			ss->pause_scan = 0;
			ss->abort_scan = 0; 

			stage_stop_timer(ss->stage); 
			microscope_disable_all_panels(ms, 1);
						
			GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_JOY_ON, &joyOn);
			
			if (joyOn)
				stage_set_joystick_off (ss->stage);
			
			SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_JOY_ON, 0);

			GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_PULSE_WIDTH, &ss->pulse_width);
			GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_POLARITY, &ss->polarity);

			ss->start_time = Timer();
			ss->total_lines = 0;
			
			// Overview uses a simple scan, images should be acquired by realtime overview whilst this is scanning
			if (stage_scan_single_frame_xy_simple(ss)<0) {
				wherearewe = 11;
				goto Error;
			}

			// Finished scanning
		
			if (joyOn) { // reset joystick control
				stage_set_joystick_on (ss->stage);
				SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_JOY_ON, joyOn);
			}

			// undim stage controls
			stage_scan_dim_ui_controls(ss, 0);
			
			// reset pause dimming
			SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_PAUSE, ATTR_LABEL_TEXT, "Pause");

			if (ss->msg_panel >= 0) {
				DiscardPanel(ss->msg_panel);
				ss->msg_panel = -1;
			}
			
			ss->pause_scan = 0;
			ss->abort_scan = 0; 
			
			// restart normal microscope timers
			stage_start_timer(ss->stage); 
			microscope_disable_all_panels(ms, 0); 

			// reset the stage speed to what it was
			stage_set_speed(ss->stage, ALL_AXIS, speed_cache);

			// put camera back how it was
			gci_camera_restore_state(ss->camera, &(ss->camera_state_cache));

			// set rto to just show stage cross
			realtime_overview_activate_only_stage(ss->rto);

			logger_log(UIMODULE_LOGGER(ss), LOGGER_INFORMATIONAL, "Stage Scan Overview completed.");

			}break;
		}
	return 0;

Error:
	// reset the stage speed to what it was
	stage_set_speed(ss->stage, ALL_AXIS, speed_cache);

	// put camera back how it was
	gci_camera_restore_state(ss->camera, &(ss->camera_state_cache));

	// log the error
	logger_log(UIMODULE_LOGGER(ss), LOGGER_INFORMATIONAL, "Stage Scan Overview failed to complete.");
	// popup message
	GCI_MessagePopup("Error", "Stage Scan Overview failed Error %d. Please check the log.", wherearewe);

	// set rto to just show stage cross
	realtime_overview_activate_only_stage(ss->rto);

	return 0;
}

int CVICALLBACK cbStageScanPause (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			char label[20], buffer[500] = "";
			stage_scan *ss = (stage_scan *) callbackData; 

			GetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_PAUSE, ATTR_LABEL_TEXT, label);
			if (!strcmp(label, "Resume")) {
				if (ss->msg_panel >= 0) {
					DiscardPanel(ss->msg_panel);
					ss->msg_panel = -1;
				}
				ss->pause_scan = 0;
				SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_PAUSE, ATTR_LABEL_TEXT, "Pause");
			}
			else {

				find_resource("StageScan_ui.uir", buffer); 
				ss->msg_panel = LoadPanel(0, buffer, MESSAGE); 

				DisplayPanel(ss->msg_panel);
				ss->pause_scan = 1;
				SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_PAUSE, ATTR_LABEL_TEXT, "Resume");
			}
			
			}break;
		}
	return 0;
}

int CVICALLBACK cbStageScanStop (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			stage_scan *ss = (stage_scan *) callbackData; 

			ss->abort_scan = 1;
			}break;
		}
	return 0;
}

int CVICALLBACK cbOverviewClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	stage_scan *ss = (stage_scan *) callbackData; 

	switch (event)
	{
		case EVENT_COMMIT:
		{
			stage_scan_overview_hide(ss);
			
			break;	
		}
	}

	return 0;
}

int CVICALLBACK cbStageScanClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	stage_scan *ss = (stage_scan *) callbackData; 

	switch (event)
	{
		case EVENT_COMMIT:
		{
			stage_scan_hide(ss);
			
			break;	
		}
	}

	return 0;
}

int CVICALLBACK cbStageScanAdvanced (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	stage_scan *ss = (stage_scan *) callbackData; 

	switch (event)
	{
		case EVENT_COMMIT:
		{
			ui_module_display_panel(UIMODULE_CAST(ss), ss->stage_scan_panel);
			
			break;	
		}
	}

	return 0;
}

int CVICALLBACK cbStageScanGotoPoint (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			}break;
		}
	return 0;
}


static void stage_scan_save_default_data(stage_scan* ss)
{
	FILE *fd;
    dictionary *d = dictionary_new(10);
	
	char path[GCI_MAX_PATHNAME_LEN], dir[GCI_MAX_PATHNAME_LEN];

	microscope_get_data_directory(ss->ms, dir);
	
    sprintf(path, "%s\\%s", dir, "StageScanData.ini");
	
	fd = fopen(path, "w");
	
	dictionary_set(d, "Stage Scan Data", NULL);

	region_of_interest_get_region(ss->roi, &(ss->frame_roi_left), &(ss->frame_roi_top),
		&(ss->frame_roi_width), &(ss->frame_roi_height));
	
	if(ss->roi_set) {
	
    	dictionary_setdouble(d, "Roi Left", ss->frame_roi_left);
    	dictionary_setdouble(d, "Roi Top", ss->frame_roi_top);
    	dictionary_setdouble(d, "Roi Width", ss->frame_roi_width);
		dictionary_setdouble(d, "Roi Height", ss->frame_roi_height);
	}
	
	dictionary_setdouble(d, "Acceleration", ss->stage_acceleration);      
	dictionary_setdouble(d, "Speed", ss->stage_speed); 
	dictionary_setdouble(d, "Fov X", ss->fov_x);   
	dictionary_setdouble(d, "Fov Y", ss->fov_y);   
	dictionary_setdouble(d, "Exposure", ss->exposure);   
	dictionary_setint(d, "Detectors", ss->detectors);
	dictionary_setint(d, "Scan Type", ss->scan_type);
	dictionary_setint(d, "Two Speed", ss->two_speed);
	dictionary_setdouble(d, "Stage Speed 2", ss->stage_speed_2);
	dictionary_setint(d, "Frames", ss->frames);   
	dictionary_setint(d, "Two Speed", ss->two_speed);   

	GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_PULSE_WIDTH, &ss->pulse_width);
	dictionary_setint(d, "Pulse Width", ss->pulse_width); 
	
	GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_POLARITY, &ss->polarity);
	dictionary_setint(d, "Polarity", ss->polarity);

	// Overview stuff
	dictionary_setdouble(d, "overview speed", ss->overview_speed);
	dictionary_setdouble(d, "overview calib fov", ss->overview_calib_fov);
	
	iniparser_save(d, fd); 
	
	fclose(fd);
	dictionary_del(d);
}


static void stage_scan_load_default_data(stage_scan* ss)
{
	dictionary* d = NULL;
	int file_size;
	char path[GCI_MAX_PATHNAME_LEN], dir[GCI_MAX_PATHNAME_LEN];

	microscope_get_data_directory(ss->ms, dir);
	
    sprintf(path, "%s\\%s", dir, "StageScanData.ini");
	
	if(FileExists(path, &file_size))
	{
		d = iniparser_load(path);  
	
		if(d == NULL)
			return;
	}
	else {
		d = dictionary_new(10);	
	}
	
    ss->frame_roi_width = dictionary_getint(d, "Stage Scan Data:Roi Width", -1);
		
	if(ss->frame_roi_width != -1) {
			
		// we have a previous saved roi
		ss->frame_roi_left = dictionary_getdouble(d, "Stage Scan Data:Roi Left", 0.0);       
        ss->frame_roi_top = dictionary_getdouble(d, "Stage Scan Data:Roi Top", 0.0);
        ss->frame_roi_width = dictionary_getdouble(d, "Stage Scan Data:Roi Width", 0.0);
        ss->frame_roi_height = dictionary_getdouble(d, "Stage Scan Data:Roi Height", 0.0);
        
		region_of_interest_set_region(ss->roi, ss->frame_roi_left, ss->frame_roi_top, 
			ss->frame_roi_width, ss->frame_roi_height);
		
		SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_X_START, ss->frame_roi_left);
		SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_Y_START, ss->frame_roi_top);
		SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_X_LENGTH, ss->frame_roi_width);
		SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_Y_LENGTH, ss->frame_roi_height);
		
		ss->roi_set = 1;
	}
	else {
		
		// No config read the values on the panel
		GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_X_START, &(ss->frame_roi_left));
		GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_Y_START, &(ss->frame_roi_top));
		GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_X_LENGTH, &(ss->frame_roi_width));
		GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_Y_LENGTH, &(ss->frame_roi_height));
		
		region_of_interest_set_region(ss->roi, ss->frame_roi_left, ss->frame_roi_top, 
			ss->frame_roi_width, ss->frame_roi_height);
		
		ss->roi_set = 1;    
	}
		
	// Acceleration
	ss->stage_acceleration = dictionary_getdouble(d, "Stage Scan Data:Acceleration", 0.0);
	
	if(ss->stage_acceleration != 0.0)
		SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_STAGE_ACCELERATION, ss->stage_acceleration);   
	else {
		// Failed to read from config a valid value so we read from the ui	
		GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_STAGE_ACCELERATION, &(ss->stage_acceleration));  
	}
	
	// FOV X
	ss->fov_x = dictionary_getdouble(d, "Stage Scan Data:Fov X", 0.0);
	
	if(ss->fov_x != 0.0)
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FOV_X, ss->fov_x);     
	else {
		// Failed to read from config a valid value so we read from the ui	
		GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_FOV_X, &(ss->fov_x));	
	}
	
	// FOV Y
	ss->fov_y = dictionary_getdouble(d, "Stage Scan Data:Fov Y", 0.0);
	
	if(ss->fov_y != 0.0)
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_FOV_Y, ss->fov_y);     
	else {
		// Failed to read from config a valid value so we read from the ui	
		GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_FOV_Y, &(ss->fov_y));	
	}
	
	// Exposure
	ss->exposure = dictionary_getdouble(d, "Stage Scan Data:Exposure", 0.0);
	
	if(ss->exposure != 0.0)
		SetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_EXPOSURE, ss->exposure*1000.0);     
	else {
		// Failed to read from config a valid value so we read from the ui	
		GetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_EXPOSURE, &(ss->exposure));	
		//ss->exposure /= 1000.0;	
	}
	
	// Speed
	ss->stage_speed = dictionary_getdouble(d, "Stage Scan Data:Speed", 0.0);
	
	if(ss->stage_speed != 0.0)
		SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_STAGE_SPEED, ss->stage_speed);  
	else {
		// Failed to read from config a valid value so we read from the ui	
		GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_STAGE_SPEED, &(ss->stage_speed)); 
		//ss->stage_speed = ss->fov_x/1000.0/ss->exposure;						//mm/sec     
	}
	
	// Detectors
	ss->detectors = dictionary_getint(d, "Stage Scan Data:Detectors", -1);

	if(ss->detectors != -1)
		SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_NO_DETECTORS, ss->detectors);  
	else {
		// Failed to read from config a valid value so we read from the ui	
		GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_NO_DETECTORS, &(ss->detectors));  
	}
	
	// Scan Type
	ss->scan_type = dictionary_getint(d, "Stage Scan Data:Scan Type", -1);
	
	if(ss->scan_type != -1)
		SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_SCAN_TYPE, ss->scan_type); 
	else {
		// Failed to read from config a valid value so we read from the ui	
		GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_SCAN_TYPE, &(ss->scan_type));  
	}
	
	// Two Speed
	ss->two_speed = dictionary_getint(d, "Stage Scan Data:Two Speed", -1);
	
	if(ss->two_speed != -1)
		SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_TWO_SPEED, ss->two_speed);    
	else {
		// Failed to read from config a valid value so we read from the ui	
		GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_TWO_SPEED, &(ss->two_speed));    
	}
	
	// Stage Speed 2
	ss->stage_speed_2 = dictionary_getdouble(d, "Stage Scan Data:Stage Speed 2", 0.0);
		
	if(ss->stage_speed_2  != 0.0)
		SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_STAGE_SPEED_2, ss->stage_speed_2);  
	else {
		// Failed to read from config a valid value so we read from the ui	
		GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_STAGE_SPEED_2, &(ss->stage_speed_2));   
	}
	
	// Frames
	ss->frames = dictionary_getint(d, "Stage Scan Data:Frames", -1);
	
	if(ss->frames != -1)
		SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_FRAMES, ss->frames);  
	else {
		// Failed to read from config a valid value so we read from the ui	
		GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_FRAMES, &(ss->frames));    
	}
	
	// Pulse Width
	ss->pulse_width = dictionary_getint(d, "Stage Scan Data:Pulse Width", -1);
	
	if(ss->pulse_width != -1)
		SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_PULSE_WIDTH, ss->pulse_width);
	else {
		// Failed to read from config a valid value so we read from the ui	
		GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_PULSE_WIDTH, &(ss->pulse_width)); 
	}
	
	// Polarity
	ss->polarity = dictionary_getint(d, "Stage Scan Data:Polarity", -1);
	
	if(ss->polarity != -1)
		SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_POLARITY, ss->polarity);   
	else {
		// Failed to read from config a valid value so we read from the ui	
		GetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_POLARITY, &(ss->polarity));   
	}

	// Overview stuff
	ss->overview_speed = dictionary_getdouble(d, "Stage Scan Data:overview speed", 3.0);
	ss->overview_calib_fov = dictionary_getdouble(d, "Stage Scan Data:overview calib fov", 330.0);

    dictionary_del(d); 
}

static void onCustomRegionOfInterestSelected (double left, double top, double width, double height, void *data)
{
	stage_scan *ss = (stage_scan *) data;

	ss->frame_roi_left   = left;
	ss->frame_roi_top    = top;
	ss->frame_roi_width  = width;
	ss->frame_roi_height = height;
	ss->roi_set = 1;      
	
	SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_X_START, ss->frame_roi_left);
	SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_Y_START, ss->frame_roi_top);
	SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_X_LENGTH, ss->frame_roi_width);
	SetCtrlVal (ss->stage_scan_panel, STAGE_SCAN_Y_LENGTH, ss->frame_roi_height);

	if (ss->scan_type == XY_SCAN) 
		stage_scan_calc_xy_frame_time(ss);
	else
		stage_scan_calc_yx_frame_time(ss);
}

void stage_scan_destroy(stage_scan *ss)
{
	if(ss == NULL)
		return;
	
	stage_scan_save_default_data(ss);
	
	if (ss->stage_signal_change_handler_id >= 0)
		stage_signal_xy_changed_handler_disconnect (ss->stage, ss->stage_signal_change_handler_id); 
	
	if (ss->master_camera_changed_handler_id >= 0)
		microscope_master_camera_changed_handler_disconnect(ss->ms, ss->master_camera_changed_handler_id);

	ui_module_destroy(UIMODULE_CAST(ss));

	ss->stage_scan_panel = -1;
	ss->stage = NULL;

	free(ss);
	ss = NULL;
}

void stage_scan_display_ui(stage_scan *ss)  
{
	ui_module_display_main_panel(UIMODULE_CAST(ss));
}

void stage_scan_display_overview_ui(stage_scan *ss)  
{
	// just make sure this has some sensible values
	gci_camera_save_state(ss->camera, &(ss->camera_state_cache));
	ui_module_display_panel(UIMODULE_CAST(ss), ss->overview_scan_panel);
}

void stage_scan_hide(stage_scan *ss)
{
	stage_scan_save_default_data(ss);
	ui_module_hide_panel(UIMODULE_CAST(ss), ss->stage_scan_panel);
}

void stage_scan_overview_hide(stage_scan *ss)
{
	stage_scan_save_default_data(ss);
	ui_module_hide_panel(UIMODULE_CAST(ss), ss->overview_scan_panel);
	region_of_interest_panel_hide(ss->roi);
}

int stage_scan_disable_ui(stage_scan *ss, int disable)
{
	SetPanelAttribute (ss->stage_scan_panel, ATTR_DIMMED, disable);
	return 0;
}

static void set_display_for_scan_type(stage_scan *ss)
{
	char image_path[GCI_MAX_PATHNAME_LEN];
	
	if (ss->scan_type == XY_SCAN) {
		
		if(find_resource("xy_scan.bmp", image_path) < 0) {
			GCI_MessagePopup("Error", "Cannot find xy_scan.bmp");		
			return;
		}
		
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_SCAN_TYPE, ATTR_IMAGE_FILE, image_path);
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_STAGE_SPEED_2, ATTR_LABEL_TEXT, "Y");
		stage_scan_calc_xy_frame_time(ss);
	}
	else {
		
		if(find_resource("yx_scan.bmp", image_path) < 0) {
			GCI_MessagePopup("Error", "Cannot find yx_scan.bmp");		
			return;
		}
		
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_SCAN_TYPE, ATTR_IMAGE_FILE, image_path);
		SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_STAGE_SPEED_2, ATTR_LABEL_TEXT, "X");
		stage_scan_calc_yx_frame_time(ss);
	}
}

static void OnStageScanPanelsClosedOrHidden (UIModule *module, void *data)
{
	stage_scan *ss = (stage_scan *) data; 

//	stage_signal_xy_changed_handler_disconnect  (ss->stage, ss->stage_signal_change_handler_id);
}

static void OnStageScanPanelsDisplayed (UIModule *module, int panel_id, void *data)
{
	stage_scan *ss = (stage_scan *) data; 

	ss->stage_signal_change_handler_id = stage_signal_xy_changed_handler_connect (ss->stage, OnStageRead, ss); 
}

static void OnMasterCameraChanged (Microscope* microscope, void *data)
{
	stage_scan *ss = (stage_scan *) data; 

    ss->camera = MICROSCOPE_MASTER_CAMERA(ss->ms);
}

stage_scan* stage_scan_new(void) 
{
	double pitch;
	stage_scan *ss = (stage_scan *) malloc (sizeof(stage_scan));

	ui_module_constructor(UIMODULE_CAST(ss), "Stage Scan");

	ss->ms = microscope_get_microscope();

	ss->camera = microscope_get_camera(ss->ms);
	ss->stage =  microscope_get_stage(ss->ms);

	ss->stage_scan_panel = -1;
	ss->overview_scan_panel = -1;
	ss->msg_panel = -1;
	ss->roi_set = 0;
	ss->rto = NULL;
	ss->stage_signal_change_handler_id = -1;

	ss->roi = microscope_get_region_of_interest(ss->ms);
	region_of_interest_selected_handler(ss->roi, onCustomRegionOfInterestSelected, ss) ;
	region_of_interest_panel_init(ss->roi);
	
	ss->abort_scan = 0;
	ss->pause_scan = 0;
	ss->pixels_x = 0;
	ss->pixels_y = 0;
	ss->total_pixels = 0;
	ss->stage_speed = 0.0;
	ss->stage_acceleration = 0.0;
	ss->two_speed = 0;
	ss->stage_speed_2 = 0.0;
	ss->um_to_accelerate = 0.0;
	ss->current_line = 0;
	ss->total_lines = 0;
	ss->scan_type = XY_SCAN;				
	ss->fov_x = 1.0;
	ss->fov_y = 1.0;
	ss->exposure = 1.0;
	ss->detectors = 1;
	ss->frame_time = 0;
	ss->frames = 1;
	ss->pulse_width = 1;			//ms
	ss->polarity = 1;				//active high

	// Main or Advanced panel
	ss->stage_scan_panel = ui_module_add_panel(UIMODULE_CAST(ss), "StageScan_ui.uir", STAGE_SCAN, 1);

	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_SET_ROI, cbStageScanSetROI, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, cbStageScanGotoPoint, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_STAGE_SPEED, cbSetStageSpeed, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_STAGE_ACCELERATION, cbSetStageAcceleration, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_JOY_ENABLE, cbStageScanJoystickEnable, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_FOV_X, cbSetFOV, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_EXPOSURE, cbSetScanParams, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_FOV_Y, cbSetFOV, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_NO_DETECTORS, cbSetNoDetectors, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_TWO_SPEED, cbStageScanTwoSpeed, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_STAGE_SPEED_2, cbStageScanTwoSpeed, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_SCAN_TYPE, cbStageScanSetScanType, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_START, cbStageScanStart, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_PAUSE, cbStageScanPause, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_STOP, cbStageScanStop, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_QUIT, cbStageScanClose, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_TEST, cbStageScanTestTrigOut, ss);
	InstallCtrlCallback (ss->stage_scan_panel, STAGE_SCAN_INITIALISE, cbStageScanStageReinitialise, ss);

	stage_get_pitch(ss->stage, XAXIS, &pitch); 
	
	SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_STAGE_SPEED, ATTR_MAX_VALUE, pitch*45.0);
	SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_STAGE_ACCELERATION, ATTR_MAX_VALUE, pitch*700.0);
	SetCtrlAttribute (ss->stage_scan_panel, STAGE_SCAN_STAGE_SPEED_2, ATTR_MAX_VALUE, pitch*45.0);

	// The simplified overview scan panel
	ss->overview_scan_panel = ui_module_add_panel(UIMODULE_CAST(ss), "StageScan_ui.uir", OVERVIEW, 1);

	InstallCtrlCallback (ss->overview_scan_panel, OVERVIEW_SET_ROI, cbStageScanSetROI, ss);
	InstallCtrlCallback (ss->overview_scan_panel, OVERVIEW_START, cbOverviewStart, ss);
	InstallCtrlCallback (ss->overview_scan_panel, OVERVIEW_PAUSE, cbStageScanPause, ss);
	InstallCtrlCallback (ss->overview_scan_panel, OVERVIEW_STOP, cbStageScanStop, ss);
	InstallCtrlCallback (ss->overview_scan_panel, OVERVIEW_QUIT, cbOverviewClose, ss);
	InstallCtrlCallback (ss->overview_scan_panel, OVERVIEW_ADVANCED, cbStageScanAdvanced, ss);

	stage_scan_load_default_data(ss);     

	set_display_for_scan_type(ss);
	
	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(ss), OnStageScanPanelsClosedOrHidden, ss);
	ui_module_panel_show_handler_connect (UIMODULE_CAST(ss), OnStageScanPanelsDisplayed, ss);

	ss->master_camera_changed_handler_id = microscope_master_camera_changed_handler_connect(ss->ms, OnMasterCameraChanged, ss);

	return ss;
}		


int CVICALLBACK cbStageScanTestTrigOut (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			
			GCI_MessagePopup("Sorry", "Not implemented.");
			
//			char command[50];
//			stage_scan *ss = (stage_scan *) callbackData; 

		
			//TODO
			//sprintf(command, "%d %d %d ot\n",ss->pulse_width, ss->polarity, 1); 
			//if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;  //Adds CR for us
			
			//sprintf(command, "1 setout"); 
			//GCI_ImagingStageSendCommand(command);  //Adds CR for us
			//Delay(2.0);
			//sprintf(command, "0 setout"); 
			//GCI_ImagingStageSendCommand(command);  //Adds CR for us
			}break;
		}
	return 0;
}

int CVICALLBACK cbStageScanSetScanType (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			stage_scan *ss = (stage_scan *) callbackData; 

			GetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_SCAN_TYPE, &ss->scan_type);
			set_display_for_scan_type(ss);
			}break;
		}
	return 0;
}

int CVICALLBACK cbStageScanTwoSpeed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			stage_scan *ss = (stage_scan *) callbackData; 

			GetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_TWO_SPEED, &ss->two_speed);
			GetCtrlVal(ss->stage_scan_panel, STAGE_SCAN_STAGE_SPEED_2, &ss->stage_speed_2);
			if (ss->scan_type == XY_SCAN) 
				stage_scan_calc_xy_frame_time(ss);
			else
				stage_scan_calc_yx_frame_time(ss);
			}break;
		}
	return 0;
}


static int stage_scan_single_frame_xy_simple(stage_scan *ss)
{
	// CORVUS SPECIFIC
	int rows, i, err, plotit = 1, status = 0;
	double y_move, left, right;
	double x=0.0, y=0.0, z=0.0, old_y;
	
	Delay(1.0);

	// turn off the camera window display
	gci_camera_set_no_display(ss->camera, 1);
	realtime_overview_set_no_display(ss->rto, 1);
	microscope_stop_all_timers(ss->ms);

	left = ss->frame_roi_left - ss->um_to_accelerate;
	right = ss->frame_roi_left + ss->frame_roi_width + ss->um_to_accelerate;
	y_move = ss->fov_y * ss->detectors * 0.9;
	rows = (int) (ss->frame_roi_height/y_move) + 1;
	
	//Go to start position
	//TODO
	//stage_set_default_speed(ss->stage);  
	
	err = stage_goto_xy_position(ss->stage, left, ss->frame_roi_top);
	
	if (err){
		gci_camera_set_no_display(ss->camera, 0);
		realtime_overview_set_no_display(ss->rto, 0);
		microscope_start_all_timers(ss->ms);
		return -1;
	}
	
	stage_set_speed(ss->stage, ALL_AXIS, ss->stage_speed);     
	
	//Don't wait for the default time of 5 seconds in case we want to abort.
	//TODO
	stage_set_timeout (ss->stage, 2.0);    
	
	ss->current_line = 0;
//	ss->start_time = Timer();
	ShowElapsedTime(ss);

	for (i=0, old_y=ss->frame_roi_top; i<rows; i+=2) {  // do 2 rows at a time
		if (ss->abort_scan) 
			break;
		if (ss->pause_scan) {
			
			stage_is_moving (ss->stage, &status);  
			
			if ((ss->msg_panel >= 0) && (status == 0)) {
				DiscardPanel(ss->msg_panel);
				ss->msg_panel = -1;
			}
			if (i > 1) i -= 1;
			ProcessSystemEvents();
			continue;
		}
		
		old_y = y;
		x = left;
		y = ss->frame_roi_top + i*y_move;
		stage_goto_xy_position(ss->stage, x, y);
		if (plotit && (ss->stage_scan_panel >= 0))
			PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, x, old_y, x, y, VAL_RED);
		
		x = right;
		stage_goto_xy_position(ss->stage, x, y);
		if (plotit && (ss->stage_scan_panel >= 0))
			PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, left, y, right, y, VAL_RED);
		realtime_overview_force_display_update(ss->rto);

		old_y = y;
		y = ss->frame_roi_top + (i+1)*y_move;
		stage_goto_xy_position(ss->stage, x, y);
		if (plotit && (ss->stage_scan_panel >= 0))
			PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, x, old_y, x, y, VAL_RED);

		x = left;
		stage_goto_xy_position(ss->stage, x, y);
		if (plotit && (ss->stage_scan_panel >= 0))
			PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, left, y, right, y, VAL_RED);
		realtime_overview_force_display_update(ss->rto);

		plotit = 1;
		
		ss->current_line += 2;
		ss->total_lines += 2;
		ShowElapsedTime(ss);
	}
	


//Abort:
	gci_camera_set_no_display(ss->camera, 0);
	realtime_overview_set_no_display(ss->rto, 0);
	microscope_start_all_timers(ss->ms);

	return 0;
}

//  ORIGINAL STAGE SCAN FLIM CODE BELOW, NEVER WORKED, ALL TEST CODE ...

/*
static int stage_scan_xy_trig_out_command(stage_scan *ss, int dir, int io_line, int *chars)
{   // sets up stage controller to output triggers at line start and/or end for xy scans
	// is also called to setup frame start trigger - HOW DOES THIS WORK?
	// CORVUS SPECIFIC
	
	char command[50];
	double line_start, line_end;
	int axis = 1, start_output = 1, end_output = 2;
	StageDirection horzDir, vertDir;
	
	horzDir = stage_get_axis_dir(ss->stage, XAXIS);    
	vertDir = stage_get_axis_dir(ss->stage, YAXIS);  
	
	if (dir == 1) {			 //left to right
		line_start = ss->frame_roi_left;
		line_end = (ss->frame_roi_left + ss->frame_roi_width);

		STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, line_start, XAXIS);     
		STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, line_end, XAXIS);     
		
		if (horzDir == STAGE_POSITIVE_TO_NEGATIVE) 		 //decreasing x counts
			dir = 0;
		else
			dir = 1;
	}
	else if (dir == -1) {	 //right to left 
		line_end = ss->frame_roi_left;
		line_start = (ss->frame_roi_left + ss->frame_roi_width);

		STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, line_start, XAXIS);     
		STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, line_end, XAXIS);   
		
		if (vertDir == STAGE_POSITIVE_TO_NEGATIVE)		 //increasing x counts
			dir = 1;
		else
			dir = 0;
	}
		
	//Line start signal
	sprintf(command, "%.2f %d %d %d %d %d wpot\n",line_start, dir, axis, ss->pulse_width, ss->polarity, io_line); 
	
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS)
		return -1;  //Adds CR for us
	
	*chars = strlen(command);	//needed if creating macro
	//Line end signal
	//sprintf(command, "%.2f %d %d %d %d %d wpot",line_end, dir, axis, ss->pulse_width, ss->polarity, io_line); 
	//if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;
	//*chars += strlen(command);
	
	return 0;
}*/
/*
static int stage_scan_xy_setup_macro_a(stage_scan *ss, double left, double right)
{   // sends a 'makro' to the stage controller for one line for xy scans:
	// CORVUS SPECIFIC

	// beginmakro
	// x relative move the width of the scan
	// trig_out_command for line start on line 1
	// trig_out_command for frame start on line 2
	// ge - blocking command
	// endmakro
	char command[50];
	int chars=0, nchars = 0;
	StageDirection horzDir, vertDir;
	
	horzDir = stage_get_axis_dir(ss->stage, XAXIS);    
	vertDir = stage_get_axis_dir(ss->stage, YAXIS);  

	STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, left, XAXIS);
	STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, right, XAXIS);
	
	sprintf(command, "beginmakro\n"); 
	
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS)
		return -1;  
	
	chars += strlen(command);

	sprintf(command, "%.2f 0.0 r\n",right-left);
	
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS)		 	//left to right
		return -1;
		
	chars += strlen(command);

	//Line start signal on I/O line 1
	if (stage_scan_xy_trig_out_command(ss, 1, 1, &nchars) < 0)
		return -1;  

	chars += nchars;
	
	//Frame start signal  on I/O line 2
	if (stage_scan_xy_trig_out_command(ss, 1, 2, &nchars) < 0)
		return -1;  
	
	chars += nchars;
	
	//Finish with a blocking command
	sprintf(command, "ge\n"); 
	
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS)
		return -1;
		
	chars += strlen(command);

	sprintf(command, "endmakro\n"); 
	
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS)
		return -1;  
	
	return 0;
}*/
/*
static int stage_scan_xy_setup_macro(stage_scan *ss, double left, double right, int *macro_rows)
{   // sends a 'makro' to the stage controller for n stripes for xy scans,
	// currently this is only 1 stripe becuase of the "max_cycles = 1" line - I think
	// CORVUS SPECIFIC
	
	// beginmakro
	// if "two_speed" set vel to stage_speed_2*1000.0
	// y relative move for y_move
	// if "two_speed" set vel to stage_speed*1000.0
	// x relative move for width
	// trig_out_command for line start on line 1
	// if "two_speed" set vel to stage_speed_2*1000.0
	// y relative move for y_move
	// if "two_speed" set vel to stage_speed*1000.0
	// x relative move for width
	// trig_out_command for line start on line 1
	// ge
	// endmakro
	
	char command[50];
	int rows, chars=0, nchars, cycle = 0, max_cycles;
	double y_move;
	StageDirection horzDir, vertDir;
	
	horzDir = stage_get_axis_dir(ss->stage, XAXIS);    
	vertDir = stage_get_axis_dir(ss->stage, YAXIS);  

	STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, left, XAXIS);
	STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, right, XAXIS);
	
	y_move = ss->fov_y * ss->detectors;

	STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, y_move, YAXIS);   
	rows = abs((int) (ss->frame_roi_height/y_move)); 
	
	sprintf(command, "beginmakro\n"); 
	
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS)   	//Adds CR for us 
		return -1;  		
		
	chars += strlen(command);

	while (1) {
		if (ss->two_speed == 1) {
			
			sprintf(command, "%.2f sv\n",ss->stage_speed_2*1000.0); 
			
			if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS)
				return -1;  	// y step speed
				
			chars += strlen(command);
		}
		sprintf(command, "0.0 %.2f r\n",y_move); 
		
		if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS)
			return -1;  		// y step
			
		chars += strlen(command);
	
		if (ss->two_speed == 1) {
			sprintf(command, "%.2f sv\n",ss->stage_speed*1000.0); 
			
			if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS)
				return -1;  	// x line speed
				
			chars += strlen(command);
		}
		
		sprintf(command, "%.2f 0.0 r\n",left-right); 
		
		if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS)
			return -1;  		//right to left
			
		chars += strlen(command);
	
		if (stage_scan_xy_trig_out_command(ss, -1, 1, &nchars) != STAGE_SUCCESS) return -1;  //trigger out points
		chars += nchars;
	
		if (ss->two_speed == 1) {
			sprintf(command, "%.2f sv\n",ss->stage_speed_2*1000.0); 
			
			if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS)
				return -1;  	// y step speed
				
			chars += strlen(command);
		}
		
		sprintf(command, "0.0 %.2f r\n",y_move); 
		
		if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS)
			return -1;  		// y step
			
		chars += strlen(command);
	
		if (ss->two_speed == 1) {
			
			sprintf(command, "%.2f sv\n",ss->stage_speed*1000.0); 
			
			if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS)
				return -1;  	// x line speed
				
			chars += strlen(command);
		}
		
		sprintf(command, "%.2f 0.0 r\n",right-left); 
		
		if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS)
			return -1;  		//left to right
			
		chars += strlen(command);
		
		if (stage_scan_xy_trig_out_command(ss, 1, 1, &nchars) != STAGE_SUCCESS) return -1;  //trigger out points
		chars += nchars;
	

		cycle++;
		if (cycle == 1) {
			max_cycles = 4000/chars;	//Corvus stack size
			//Want an exact division of the roi height
			max_cycles = (int) (1+max(1, (rows/2)/(ceil((double)rows/2.0/max_cycles))));
			max_cycles = 1;			 //So it can be interrupted more often
		}
	
		if (cycle == max_cycles) break;
	}
	
	//Finish with a blocking command
	sprintf(command, "ge\n"); 
	
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS)
		return -1;
		
	chars += strlen(command);

	sprintf(command, "endmakro\n"); 
	
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS)
		return -1;  

	*macro_rows = max_cycles*2;
	
	return 0;
}*/
/*
static int stage_scan_yx_trig_out_command(stage_scan *ss, int dir, int io_line, int *chars)
{   // now for yx scans, see xy above
	// CORVUS SPECIFIC
	
	char command[50];
	double line_start, line_end;
	int axis = 2, start_output = 1, end_output = 2;
	StageDirection horzDir, vertDir;
	
	horzDir = stage_get_axis_dir(ss->stage, XAXIS);    
	vertDir = stage_get_axis_dir(ss->stage, YAXIS);  

	if (dir == 1) {			 //top to bottom
		line_start = ss->frame_roi_top;
		line_end = (ss->frame_roi_top + ss->frame_roi_height);

		STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, line_start, YAXIS);     
		STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, line_end, YAXIS);
		
		if (vertDir == STAGE_POSITIVE_TO_NEGATIVE) 		 //decreasing y counts
			dir = 0;
		else
			dir = 1;
	}
	else if (dir == -1) {	 //bottom to top 
		line_end = ss->frame_roi_top;
		line_start = (ss->frame_roi_top + ss->frame_roi_height);

		STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, line_start, YAXIS);     
		STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, line_end, YAXIS);
		
		if (vertDir == STAGE_NEGATIVE_TO_POSITIVE)		 //increasing y counts
			dir = 1;
		else
			dir = 0;
	}
		
	//Line start signal
	sprintf(command, "%.2f %d %d %d %d %d wpot\n",line_start, dir, axis, ss->pulse_width, ss->polarity, io_line); 
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;  //Adds CR for us
	*chars = strlen(command);	//needed if creating macro
	//Line end signal
	sprintf(command, "%.2f %d %d %d %d %d wpot\n",line_end, dir, axis, ss->pulse_width, ss->polarity, io_line); 
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;
	*chars += strlen(command);
	
	return 0;
}*/
/*
static int stage_scan_yx_setup_macro_a(stage_scan *ss, double top, double bottom)
{   // now for yx scans, see xy above 
	// CORVUS SPECIFIC

	FILE *fp;
	char command[50];
	int chars=0, nchars;
	StageDirection horzDir, vertDir;
	
	horzDir = stage_get_axis_dir(ss->stage, XAXIS);    
	vertDir = stage_get_axis_dir(ss->stage, YAXIS);  
	
	STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, top, YAXIS);     
	STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, bottom, YAXIS);
	
	//Set up macro for the first top-bottom move.
	fp = fopen ("testa.txt", "w");

	sprintf(command, "beginmakro\n"); 
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;  //Adds CR for us
	chars += strlen(command);
	fprintf(fp, "%s\n", command);
	
	sprintf(command, "0.0, %.2f r\n", bottom-top);
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;  		  //top to bottom
	chars += strlen(command);
	fprintf(fp, "%s\n", command);
		
	//Line start signal on I/O line 1
	if (stage_scan_yx_trig_out_command(ss, 1, 1, &nchars) != STAGE_SUCCESS) return -1;  //trigger out points
	chars += nchars;
	
	//Frame start signal  on I/O line 2
	if (stage_scan_yx_trig_out_command(ss, 1, 2, &nchars) != STAGE_SUCCESS) return -1;  //trigger out points
	chars += nchars;
	
	//Finish with a blocking command
	sprintf(command, "ge\n"); 
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;
	chars += strlen(command);
	fprintf(fp, "%s\n", command);

	sprintf(command, "endmakro\n"); 
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;  
	fprintf(fp, "%s\n", command);
	fclose(fp);
	
	return 0;
}*/
/*
static int stage_scan_yx_setup_macro(stage_scan *ss, double top, double bottom, int *macro_rows)
{   // now for yx scans, see xy above 
	// CORVUS SPECIFIC

	FILE *fp;
	char command[50];
	int rows, chars=0, nchars, cycle = 0, max_cycles;
	double x_move;
	StageDirection horzDir, vertDir;
	
	horzDir = stage_get_axis_dir(ss->stage, XAXIS);    
	vertDir = stage_get_axis_dir(ss->stage, YAXIS);  
	
	STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, top, YAXIS);     
	STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, bottom, YAXIS);
	
	x_move = ss->fov_x * ss->detectors;
	STAGE_CORRECT_VAL_FOR_ORIENTATION(ss->stage, x_move, XAXIS);        
	rows = abs((int) (ss->frame_roi_width/x_move));

	fp = fopen ("test.txt", "w");

	sprintf(command, "beginmakro\n"); 
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;  //Adds CR for us
	chars += strlen(command);
	fprintf(fp, "%s\n", command);
	
	while (1) {
		if (ss->two_speed == 1) {
			sprintf(command, "%.2f sv\n",ss->stage_speed_2*1000.0); 
			if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;  		 // x step speed
			chars += strlen(command);
			fprintf(fp, "%s\n", command);
		}
		sprintf(command, "%.2f 0.0 r\n",x_move); 
		if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;  		 // x step
		chars += strlen(command);
		fprintf(fp, "%s\n", command);
	
		if (ss->two_speed == 1) {
			sprintf(command, "%.2f sv\n",ss->stage_speed*1000.0); 
			if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;  		 // x step speed
			chars += strlen(command);
			fprintf(fp, "%s\n", command);
		}
		sprintf(command, "0.0 %.2f r\n",top-bottom); 
		if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;  		 //bottom to top
		chars += strlen(command);
		fprintf(fp, "%s\n", command);
	
		if (stage_scan_yx_trig_out_command(ss, -1, 1, &nchars) != STAGE_SUCCESS) return -1;  //trigger out points
		chars += nchars;
	
		if (ss->two_speed == 1) {
			sprintf(command, "%.2f sv\n",ss->stage_speed_2*1000.0); 
			if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;  		 // x step speed
			chars += strlen(command);
			fprintf(fp, "%s\n", command);
		}
		sprintf(command, "%.2f 0.0 r\n",x_move);
		if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;  		 // x step
		chars += strlen(command);
		fprintf(fp, "%s\n", command);
	
		if (ss->two_speed == 1) {
			sprintf(command, "%.2f sv\n",ss->stage_speed*1000.0); 
			if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;  		 // y step
			chars += strlen(command);
			fprintf(fp, "%s\n", command);
		}
		sprintf(command, "0.0 %.2f r\n",bottom-top); 
		if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;  		  //top to bottom
		chars += strlen(command);
		fprintf(fp, "%s\n", command);
		
		if (stage_scan_yx_trig_out_command(ss, 1, 1, &nchars) != STAGE_SUCCESS) return -1;  //trigger out points
		chars += nchars;
	
		
		cycle++;
		if (cycle == 1) {
			max_cycles = 4000/chars;	//Corvus stack size
			//Want an exact division of the roi height
			max_cycles = (int) (1+max(1, (rows/2)/(ceil((double)rows/2.0/max_cycles))));
			max_cycles = 1;			 //So it can be interrupted more often
		}
	
		if (cycle == max_cycles) break;
	}
	
	sprintf(command, "ge\n"); 
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;
	chars += strlen(command);
	fprintf(fp, "%s\n", command);

	sprintf(command, "endmakro\n"); 
	if (RS232CorvusSend((CorvusXYStage *) ss->stage, command) != STAGE_SUCCESS) return -1;  
	fprintf(fp, "%s\n", command);
	fclose(fp);
	
	*macro_rows = max_cycles*2;
	
	return 0;
}*/
/*
static int stage_scan_first_row_xy(stage_scan *ss, double left, double right)
{
	// CORVUS SPECIFIC
	char response[200];
	double y;
	int status = 0;
		
	//First row is different - we have to send the frame start signal
	stage_scan_xy_setup_macro_a(ss, left, right);
	
	if (RS232CorvusSend((CorvusXYStage *) ss->stage,"startmakro\n") != STAGE_SUCCESS)
		return -1; 
	
	Delay (0.05);	  //wait for it to start

	//Ask for status. We won't get a response until the macro has finished.

	RS232CorvusSend((CorvusXYStage *) ss->stage,"st\n");

	while (STAGE_ReadString((CorvusXYStage *)ss->stage, response) < 0) {
		
		ProcessSystemEvents();
		if (ss->abort_scan) 
			break;
			
		ShowElapsedTime(ss);
	}	

	//On abort send ctrl C 
	ProcessSystemEvents();
	
	if (ss->abort_scan) {
		
		stage_abort_move(ss->stage);
		
		stage_is_moving (ss->stage, &status);      
		
		while (status != 0) {
			stage_abort_move(ss->stage);
		}
	}	

	y = ss->frame_roi_top+ss->fov_y/2.0;
	if (ss->stage_scan_panel >= 0) 
		PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, left, y, right, y, VAL_RED);

	//TODO
	//stage_set_current_position (ss->stage, right, y, 0.0);  
	
	return 0;
}*/
/*
static int stage_scan_single_frame_xy_using_macro(stage_scan *ss)
{
	// CORVUS SPECIFIC
	int macro_rows, rows, i, r, err, plotit = 0, status = 0;
	double y_move, left, right;
	double x=0.0, y=0.0, z=0.0;
	char response[200];
	
	left = ss->frame_roi_left - ss->um_to_accelerate;
	right = ss->frame_roi_left + ss->frame_roi_width + ss->um_to_accelerate;
	y_move = ss->fov_y * ss->detectors;
	rows = (int) (ss->frame_roi_height/y_move);
	
	//Go to start position
	//TODO
	//stage_set_default_speed(ss->stage);  
	
	err = stage_async_goto_xy_position(ss->stage, left, ss->frame_roi_top+ss->fov_y/2.0);
	
	if (err)
		return -1;
	
	stage_set_speed(ss->stage, ALL_AXIS, ss->stage_speed);     
	
	//Hide the expected timeout errors
	stage_log_errors(ss->stage, 0);      
	
	//During macro execution the Corvus doesn't respond. 
	//Don't wait for the default time of 5 seconds in case we want to abort.
	//TODO
	//stage_set_timeout (ss->stage, 0.5);    
	
	ss->current_line = 0;
//	ss->start_time = Timer();
	ShowElapsedTime(ss);

	//First row is different - we have to send the frame start signal
	stage_scan_first_row_xy(ss, left, right);
	if (ss->abort_scan) goto Abort;
	ss->current_line = 1;
	
	//clear Corvus command stack
	RS232CorvusSend((CorvusXYStage *) ss->stage,"clear\n");  

	//TODO
	//stage_set_timeout (ss->stage, 5.0);
	err = stage_scan_xy_setup_macro(ss, left, right, &macro_rows);
	
	if(err < 0) {
		GCI_MessagePopup("Error", "Falied to setup macro");
	}
	
	//TODO
	//stage_set_timeout (ss->stage, 1.5);  
	
	for (i=1; i<rows; i+=macro_rows) {
		if (ss->abort_scan) 
			break;
		if (ss->pause_scan) {
			
			stage_is_moving (ss->stage, &status);  
			
			if ((ss->msg_panel >= 0) && (status == 0)) {
				DiscardPanel(ss->msg_panel);
				ss->msg_panel = -1;
			}
			if (i > 1) i -= macro_rows;
			ProcessSystemEvents();
			continue;
		}
		
		if (RS232CorvusSend((CorvusXYStage *) ss->stage,"startmakro\n") != STAGE_SUCCESS)
			return -1;
		
		Delay (0.05);	  //wait for it to start
		
		//Plot lines scanned so far while it is doing the next ones
		if (plotit && (ss->stage_scan_panel >= 0)) {
			PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, left, y-y_move, right, y-y_move, VAL_RED);
			PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, left, y, right, y, VAL_RED);
			
			//SetGraphCursor (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, 1, x, y);
		}	

		//Ask for status. We won't get a response until the macro has finished.
		if(RS232CorvusSend((CorvusXYStage *) ss->stage,"st\n") == STAGE_ERROR)
			return -1;
		
//		Delay (0.25);	  //wait for it to start  
		
		
		
		//TODO
		//RS232CorvusBlockUntilFinished((CorvusXYStage *)ss->stage);     
		
		// This fails
			 
		while (STAGE_ReadString((CorvusXYStage *)ss->stage, response) != STAGE_SUCCESS) {
			ProcessSystemEvents();
			if (ss->abort_scan) 
				break;
			
			ShowElapsedTime(ss);
		}	
			
		
		
		//On abort send ctrl C for every move in the macro
		ProcessSystemEvents();
		
		if (ss->abort_scan) {
			for (r = 0; r<macro_rows; r++) {
				
				stage_abort_move(ss->stage);
				
				while (the_stage_is_moving(ss->stage) != 0) {
					stage_abort_move(ss->stage);
				}
			}	
			
			break;
		}	

		//Wait for the final X move to complete
		//while (GCI_MicroscopeStageIsMoving() != 0);

		//TODO
		stage_get_xy_position (ss->stage, &x, &y);
		//stage_set_current_position (ss->stage, x, y, z);  
		
		plotit = 1;
		
		ss->current_line += 2;
		ss->total_lines += 2;
		ShowElapsedTime(ss);
	}
	
	//Plot last two lines scanned
	if (ss->stage_scan_panel >= 0) {
		PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, left, y-y_move, right, y-y_move, VAL_RED);
		PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, left, y, right, y, VAL_RED);
		
		//SetGraphCursor (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, 1, x, y);
	}	

Abort:
	//clear Corvus command stack
	RS232CorvusSend((CorvusXYStage *) ss->stage,"clear\n");  

	//Restore normal stage settings
	
	//TODO
	//stage_set_timeout (ss->stage, 5.0);
	//TODO
	//stage_set_default_speed(ss->stage);  
	//TODO
	stage_log_errors(ss->stage, 1);	

	return 0;
}*/
/*
static int stage_scan_first_row_yx(stage_scan *ss, double top, double bottom)
{
	// CORVUS SPECIFIC
	char response[200];
	double x;
	
	//First row is different - we have to send the frame start signal
	stage_scan_yx_setup_macro_a(ss, top, bottom);
	
	if (RS232CorvusSend((CorvusXYStage *) ss->stage,"startmakro\n") != STAGE_SUCCESS)
		return -1;
	
	Delay (0.05);	  //wait for it to start

	//Ask for status. We won't get a response until the macro has finished.
	//RS232CorvusSend((CorvusXYStage *) ss->stage,"st")
		
	while (STAGE_ReadString((CorvusXYStage *)ss->stage, response) != STAGE_SUCCESS) {
		ProcessSystemEvents();
		if (ss->abort_scan) 
			break;
			
		ShowElapsedTime(ss);
	}	
		
	//On abort send ctrl C 
	ProcessSystemEvents();
	if (ss->abort_scan) {
		stage_abort_move(ss->stage);
		while (the_stage_is_moving(ss->stage) != 0) {
			stage_abort_move(ss->stage);
		}
	}	

	x = ss->frame_roi_left+ss->fov_x/2.0;
	if (ss->stage_scan_panel >= 0) 
		PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, x, top, x, bottom, VAL_RED);
	
	//TODO
	//stage_set_current_position (ss->stage,x, bottom, 0.0);
	
	return 0;
}*/
/*
static int stage_scan_single_frame_yx_using_macro(stage_scan *ss)
{
	// CORVUS SPECIFIC
	int macro_rows, rows, i, r, err, plotit = 0;
	double x_move, top, bottom;
	double x=0.0, y=0.0, z=0.0;
	char response[200];;
	
	top = ss->frame_roi_top - ss->um_to_accelerate;
	bottom = ss->frame_roi_top + ss->frame_roi_height + ss->um_to_accelerate;
	x_move = ss->fov_x * ss->detectors;
	rows = (int) (ss->frame_roi_width/x_move);
	
	//Go to start position
	//TODO
	//stage_set_default_speed(ss->stage); 
	
	err = stage_async_goto_xy_position(ss->stage, ss->frame_roi_left+ss->fov_x/2.0, top);
	
	if (err)
		return -1;
	
	stage_set_speed(ss->stage, ALL_AXIS, ss->stage_speed); 
	
	//TODO
	stage_log_errors(ss->stage, 0);	//Hide the expected timeout errors

	//During macro execution the Corvus doesn't respond. 
	//Don't wait for thr default time of 5 seconds in case we want to abort.
	//TODO
	//stage_set_timeout (ss->stage, 0.5);  

	ss->current_line = 0;
//	ss->start_time = Timer();
	ShowElapsedTime(ss);
	
	//First row is different - we have to send the frame start signal
	stage_scan_first_row_yx(ss, top, bottom);
	if (ss->abort_scan) goto Abort;
	ss->current_line = 1;
	
	//TODO
	//stage_set_timeout (ss->stage, 5.0);
	//clear Corvus command stack
	RS232CorvusSend((CorvusXYStage *) ss->stage,"clear\n");  
	err = stage_scan_yx_setup_macro(ss, top, bottom, &macro_rows);
	
	//TODO
	//stage_set_timeout (ss->stage, 0.5);  

	for (i=1; i<rows; i+=macro_rows) {
		if (ss->abort_scan) 
			break;
		if (ss->pause_scan) {
			if ((ss->msg_panel >= 0) && (the_stage_is_moving(ss->stage) == 0)) {
				DiscardPanel(ss->msg_panel);
				ss->msg_panel = -1;
			}
			if (i > 1) i -= macro_rows;
			ProcessSystemEvents();
			continue;
		}
		
		if (RS232CorvusSend((CorvusXYStage *) ss->stage,"startmakro\n") != STAGE_SUCCESS)
			return -1;  
		
		Delay (0.05);	  //wait for it to start
		
		if (plotit && (ss->stage_scan_panel >= 0)) {
			PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, x-x_move, top, x-x_move, bottom, VAL_RED);
			PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, x, top, x, bottom, VAL_RED);
			
			//SetGraphCursor (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, 1, x, y);
		}	

		//Ask for status. We won't get a response until the macro has finished.
		RS232CorvusSend((CorvusXYStage *) ss->stage,"st\n");
		
		while (STAGE_ReadString((CorvusXYStage *)ss->stage, response) != STAGE_SUCCESS) {
			
			ProcessSystemEvents();
			
			if (ss->abort_scan) 
				break;
			
			ShowElapsedTime(ss);
		}
		
		//On abort send ctrl C for every move in the macro
		ProcessSystemEvents();						// 60 ms
		
		if (ss->abort_scan) {
			
			for (r = 0; r<macro_rows; r++) {
				
				stage_abort_move(ss->stage);
				
				while (the_stage_is_moving(ss->stage) != 0) {
					stage_abort_move(ss->stage);
				}
			}	
			break;
		}	

		//Wait for the final Y move to complete
		while (the_stage_is_moving(ss->stage) != 0);	//10 ms
		
		stage_get_xy_position (ss->stage, &x, &y);		//pos read/plot 60 ms
		
		//TODO
		//stage_set_current_position (ss->stage, x, y, z);  
		plotit = 1;
		
		ss->current_line += 2;  //number of completed lines
		ss->total_lines += 2;
		ShowElapsedTime(ss);						//takes 0 ms
	}
	
	//Plot last two lines scanned
	if (ss->stage_scan_panel >= 0) {
		PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, x-x_move, top, x-x_move, bottom, VAL_RED);
		PlotLine (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, x, top, x, bottom, VAL_RED);
		
		//SetGraphCursor (ss->stage_scan_panel, STAGE_SCAN_XY_DISP, 1, x, y);
	}	

Abort:
	//clear Corvus command stack
	RS232CorvusSend((CorvusXYStage *) ss->stage,"clear");  

	//Restore normal stage settings
	//stage_set_timeout (ss->stage, 5.0);
	//TODO
	//stage_set_default_speed(ss->stage);  

	stage_log_errors(ss->stage, 1);	

	return 0;
}
*/
