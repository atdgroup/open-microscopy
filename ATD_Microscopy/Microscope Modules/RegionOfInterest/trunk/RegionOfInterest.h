#ifndef __ROI__
#define __ROI__

#include "HardwareTypes.h"
#include "gci_ui_module.h"
#include "camera\gci_camera.h"
#include "stage\stage.h"


typedef struct _region_of_interest
{
	UIModule parent;
	
	int id;
	int _master_camera_changed_signal_id;
	int _stage_plate_signal_plate_changed_signal_id;

	int region_panel;
	int region_panel_tab1;
	int region_panel_tab2;
	int region_panel_tab3;
	int region_panel_tab4;

	int focal_plane_valid;
	
	double left;
	double top;
	double width;
	double height;
	
	double focal_point_a;
	double focal_point_b;
	double focal_point_c;
	double focal_point_c_original;  // to store values for region based timelapse, so that offset for cubes can be applied properly
	
	Microscope *ms;
	GciCamera *camera;
	XYStage *stage;

} region_of_interest;



typedef void (*REGION_SELECTED_EVENT_HANDLER) (double left, double top, double width, double height, void *data);

region_of_interest* region_of_interest_selection_new(GciCamera *camera);

int region_of_interest_panel_init(region_of_interest* roi);

void region_of_interest_clear_button_state(region_of_interest* roi);

int region_of_interest_display(region_of_interest* roi);

int region_of_interest_panel_display(region_of_interest* roi, int parent_panel_id);

int region_of_interest_panel_hide(region_of_interest* roi);

void region_of_interest_destroy(region_of_interest* roi);

void region_of_interest_set_region(region_of_interest* roi, double left, double top, double width, double height);

void region_of_interest_get_region(region_of_interest* roi, double *left, double *top, double *width, double *height);

int region_of_interest_setup_focal_plane(region_of_interest* roi, int doItOutside);

int region_of_interest_goto_stage_xy(region_of_interest* roi, double x, double y);

int region_of_interest_goto_stage_xy_advanced(region_of_interest* roi, double x, double y,
											  double wait_time, double z_speed, double z_accel, double start_z);

int region_of_interest_z_for_xy(region_of_interest* roi, double x, double y, double *z);

int region_of_interest_selected_handler(region_of_interest* roi, REGION_SELECTED_EVENT_HANDLER handler, void *callback_data) ;

int region_of_interest_show_focus_options(region_of_interest* roi);

void region_of_interest_set_focus_points(region_of_interest* roi, double a, double b, double c);

void region_of_interest_get_focus_points(region_of_interest* roi, double *a, double *b, double *c);

int region_of_interest_is_focal_plane_valid(region_of_interest* roi);

int  CVICALLBACK cbRoiSetFocalPlane(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiSetFocalPlaneOffset (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab1_dimChange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab1_pointChange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab1_SetEnd(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab1_SetStart(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab2_pointChange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab2_pointSetup(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab3_SetCentre(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab3_SetRadius(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab3_valChange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab4_GoPos(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onRoiCancel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onRoiOk(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onGetCoordsFromStagePlate(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif
