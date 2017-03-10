#ifndef __STAGE_SCAN__
#define __STAGE_SCAN__

#include "gci_ui_module.h"
#include "icsviewer_window.h"
#include "camera\gci_camera.h" 
#include "microscope.h"
#include "realtime_overview.h"

////////////////////////////////////////////////////////////////////
// Module to perform XY stage scan
// Ros Locke - November 2006
////////////////////////////////////////////////////////////////////

typedef enum {XY_SCAN, YX_SCAN} ScanType;

typedef struct _stage_scan stage_scan;  

struct _stage_scan
{
	UIModule parent;
	
	int lock;
	int stage_scan_panel;
	int overview_scan_panel;
	int msg_panel;
	
	Microscope *ms;
	GciCamera *camera;
	XYStage *stage;
	realtime_overview *rto;
	region_of_interest *roi; 
	
	volatile int pause_scan;
	volatile int abort_scan;

	int stage_signal_change_handler_id;
	int master_camera_changed_handler_id;
	int pixels_x;
	int pixels_y;
	int total_pixels;
	
	double stage_speed;				//mm/sec
	double stage_acceleration;		//mm/sec/sec
	int two_speed;
	double stage_speed_2;			//mm/sec
	double um_to_accelerate;
	int current_line;
	int total_lines;
	
	int scan_type;
	double fov_x;					//um
	double fov_y;					//um
	double exposure;				//secs
	int detectors;
									//secs
	double start_time;				//secs
	double frame_time;
	int frames;

	double frame_roi_left;			//um
	double frame_roi_top;			//um 
	double frame_roi_width;			//um 
	double frame_roi_height; 		//um 

	int	pulse_width;				//um 
	int polarity;
	
	int roi_set;

	double overview_speed;
	double overview_calib_fov;
	
	CameraState camera_state_cache;
};

stage_scan* stage_scan_new(void);

void stage_scan_reset(stage_scan *ss);

void stage_scan_destroy(stage_scan *ss);

void stage_scan_hide(stage_scan *ss);
void stage_scan_overview_hide(stage_scan *ss);

void stage_scan_display_ui(stage_scan *ss);
void stage_scan_display_overview_ui(stage_scan *ss);

int stage_scan_disable_ui(stage_scan *ss, int disable);

int stage_scan_autoscan(stage_scan *ss);

void stage_scan_dim_ui_controls(stage_scan *ss, int dim);

int  CVICALLBACK cbSetFOV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSetNoDetectors(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSetScanParams(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSetStageAcceleration(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSetStageSpeed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbStageScanClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbStageScanGotoPoint(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbStageScanJoystickEnable(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbStageScanPause(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbStageScanSetROI(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbStageScanSetScanType(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbStageScanStageReinitialise(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbStageScanStart(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbStageScanStop(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbStageScanTestTrigOut(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbStageScanTwoSpeed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

int  CVICALLBACK cbOverviewStart(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbOverviewClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK cbOverviewSetCamera (int panel, int control, int event,	void *callbackData, int eventData1, int eventData2);

#endif
