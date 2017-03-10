#ifndef __TIMELAPSE__
#define __TIMELAPSE__

#define TIMELAPSE_ERROR -1
#define TIMELAPSE_SUCCESS 0

#define MAX_NUMBER_OF_ACTIONS 20

#define REPEAT_CYCLES   0
#define REPEAT_HOURS    1
#define REPEAT_FOREVER  2

#define TABLE_COL_NUMBER 5

#include "Python.h"  

#include "HardwareTypes.h"
#include "gci_ui_module.h"
#include "timelapse.h"
#include "camera\gci_camera.h"
#include "stage\stage.h"
#include "StagePlate.h"
#include "Wizard.h"
#include "gci_types.h"
#include "mosaic.h"

typedef struct _timelapse timelapse;
typedef struct _TimelapseWellPlateDefiner TimelapseWellPlateDefiner;

// The master struct of information for each timelapse point
typedef struct
{
	GCI_FPOINT centre;
	int hasRegion;
	GCI_FPOINT regionSize;
	int hasFocalPlane;
	double focalPlane_a;
	double focalPlane_b;
	double focalPlane_c;

} TimelapseTableEntry;

typedef struct
{
	char name[500];
	int position;
	double exposure;
	double gain;
	double offset;

} TimelapseCubeOptions;

typedef struct
{
	timelapse *tl;
	char name[500];
	char description[500];
	char author[500];
	char category[500];
	char filepath[500];
	int  new_format;

} TimelapseScriptDetails;

#define TIMELAPSE_PLATE_TOP_LEFT     0
#define TIMELAPSE_PLATE_TOP_RIGHT    1
#define TIMELAPSE_PLATE_BOTTOM_LEFT  2
#define TIMELAPSE_PLATE_BOTTOM_RIGHT 3

#define TIMELAPSE_POLY_PT_1    4+1
#define TIMELAPSE_POLY_PT_2    4+2
#define TIMELAPSE_POLY_PT_3    4+3

#define TIMELAPSE_USEGLOBALFOCALPLANE 0
#define	TIMELAPSE_USEINDFOCALPLANE    1
#define	TIMELAPSE_USECONSTFOCALPLANE  2
#define	TIMELAPSE_USEAUTOEVERY        4

typedef struct
{
    int well_count;
    int horizontal_size;
    int vertical_size;

} TimelapseWellPlate;

typedef enum {TIMELAPSE_WELL_PLATE_CUSTOM=-1,
              TIMELAPSE_WELL_PLATE_6=0,
              TIMELAPSE_WELL_PLATE_12,
              TIMELAPSE_WELL_PLATE_24,
              TIMELAPSE_WELL_PLATE_48,
              TIMELAPSE_WELL_PLATE_96,
              TIMELAPSE_WELL_PLATE_396

             } TimelapsePredefinedWellPlate; 

/* mrowley - 081210 */
typedef struct
{
	unsigned short row;
	unsigned short col;

} TimelapseWCoord;

typedef struct
{
	int					 measured; /* 1 = valid measured value stored, could be extended to measured / fitted for use in reducing range for auto-focusing */
	TimelapseWCoord      coord;    /* the (i,j) coordinate on the plate of the well */
	GCI_FPOINT			 pt;

} TimelapseWPoint;

struct _TimelapseWellPlateDefiner
{
	timelapse *tl;

	/* mrowley - 081210 */
	TimelapseWPoint pts_corners[4]; //corners plus those measured in the centre
	TimelapseWPoint pts_others[3];
	
	double					poly_coeffs[7];
	unsigned short			poly_num_of_coeffs;
	TimelapseWCoord         poly_pts_template[3];
	int linear_fit;  // if there are not enough wells to do a poly fit

	int manual_or_auto_pnl;
	int custom_or_predefined_pnl;
	int custom_pnl;
	int direction_pnl;
	int plate_select_pnl;
	int use_selected_wells;
	
	int generate_from_stage_plate;
};


struct _timelapse
{
	UIModule parent;
	
	GciCamera *camera;
	
	Microscope *ms;
	XYStage* stage;
	Z_Drive *z_drive;
	IcsViewerWindow *window;         // to use crosshair on camera window
	MosaicWindow* mosaic_window;  // to use crosshair on stage scan or region scan window
	StagePlateModule* stage_plate_module;
	
	ListType points;
	ListType cube_options;
		
	GCI_FPOINT mouse_point; 
	
	int region_mode; // indicates that the region options are available and only region scripts should be displayed
	int focus_mode;
	int has_global_focalPlane;
	double global_focalPlane_a, global_focalPlane_b, global_focalPlane_c;

	int has_run;
	int plot;
	int panel_id;
	int stage_plate_menu_item;
	int revisit_panel;
	int regions_panel;
	int current_point;
	double start_time;
	volatile int active;	// This can / should be called from different threads etc
	
	ListType script_details;
	dictionary* script_details_index;

	int action_menubar;
	int action_menu;
	int _crosshair_signal_id;
	int _microscope_master_z_drive_changed_signal_id;
	int _microscope_master_camera_changed_signal_id;
	int _stage_plate_signal_plate_changed_signal_id;

	char start_date_time[200];
	char end_date_time[200];

	#ifdef MICROSCOPE_PYTHON_AUTOMATION
	PyObject*	 pModule;
	PyObject*    on_point_changed_callable;
	PyObject*    on_start_callable;  
	PyObject*    on_cycle_start_callable;
	PyObject*    on_abort_callable;
	#endif

	int  last_script_index;
	char last_script_filename[GCI_MAX_PATHNAME_LEN];
	char **action_filepaths[MAX_NUMBER_OF_ACTIONS][500];

	wizard* wiz;

	TimelapseWellPlateDefiner* wpd;
};

timelapse* timelapse_new(Microscope *ms);

void timelapse_display(timelapse *tl);

void timelapse_hide(timelapse *tl);

void timelapse_destroy(timelapse *tl);

void timelapse_add_point(timelapse *tl, TimelapseTableEntry pt);

void timelapse_add_point_xyz(timelapse *tl, double x, double y, double z);

int timelapse_edit_centre_point(timelapse *tl, int pos, GCI_FPOINT pt);

int timelapse_edit_ROI_size(timelapse *tl, int pos, GCI_FPOINT pt);

void timelapse_get_point(timelapse *tl, int position, TimelapseTableEntry *pt);          

ListType timelapse_get_point_list(timelapse *tl);

//void timelapse_setup_graph(timelapse *tl);

void timelapse_load_data_from_file(timelapse *tl, const char *filepath);

void timelapse_load_data_from_selected_filepath(timelapse *tl);

void timelapse_save_data(timelapse *tl, const char *filepath);

void timelapse_save_data_to_selected_filepath(timelapse *tl);

void timelapse_auto_save_data(timelapse *tl);

void timelapse_clear_auto_save_data(timelapse *tl);

int timelapse_setup_action_scripts(timelapse *tl);

int timelapse_setup_script(timelapse *tl);

int timelapse_call_python_on_point_changed(timelapse *tl, GCI_FPOINT pt, int current_point);

int timelapse_call_python_on_cycle_start(timelapse *tl);

int timelapse_call_python_on_start(timelapse *tl);

int timelapse_move_to_next_point(timelapse *tl);

int timelapse_move_to_previous_point(timelapse *tl);

int timelapse_move_to_point(timelapse *tl, int pos);

int timelapse_remove_point(timelapse *tl, int pos);

int timelapse_clear_points(timelapse *tl);

void timelapse_draw_points(timelapse *tl);

int timelapse_update_centre_point(timelapse *tl, int pos);

int timelapse_new_point(timelapse *tl);
int timelapse_new_region_WH(timelapse *tl, double width, double height);   
int timelapse_new_region_XYZWH(timelapse *tl, double X, double Y, double Z, double width, double height);   

int timelapse_perform_sequence(timelapse *tl);

int timelapse_perform_points(timelapse *tl);

void timelapse_update_revisit_buttons(timelapse *tl);

void timelapse_status(timelapse *tl, int *has_run, int *active, char *start_time, char *end_time);

void timelapse_get_cube_options(timelapse *tl, int position, TimelapseCubeOptions *pt);

void GenerateStagePlatePoints(timelapse *tl);

int GenerateStagePlatePointsWithDirInfo(timelapse *tl,
										int selected_points, int is_horizontal,
										StagePlateHorizontalStartPosition horz_start,
										StagePlateVerticalStartPosition vert_start,
										int shortest_path, int region, double xsize, double ysize);

void timelapse_set_repeat_ring(timelapse *tl, int value);
void timelapse_set_repeat_val(timelapse *tl, int value);
void timelapse_set_interval(timelapse *tl, int value);

void timelapse_set_vals_to_camera(timelapse *tl);
void timelapse_get_vals_from_camera(timelapse *tl);

void timelapse_region_mode_on(timelapse *tl);
void timelapse_region_mode_off(timelapse *tl);

void timelapse_setup_cube_options(timelapse *tl);

TimelapseWellPlateDefiner* timelapse_timelapse_well_plate_definer_new(timelapse *ts);

void timelapse_well_plate_definer_destroy(TimelapseWellPlateDefiner* wpd);

int calculate_well_z_position_for_xy(TimelapseWellPlateDefiner* wpd, double x, double y, GCI_FPOINT *pt);

int timelapse_well_plate_definer_export_points_to_file (TimelapseWellPlateDefiner* wpd, char *filepath);

//int calculate_well_position(TimelapseWellPlateDefiner* wpd, unsigned short i, unsigned short j, GCI_FPOINT *pt);

void check_mosaic_region_needed(timelapse *tl);

int CVICALLBACK OnTimelapseManualWellDefine (int panel, int control, int event,
        void *callbackData, int eventData1, int eventData2);

int InsertFilesInToList(timelapse *tl);

int  CVICALLBACK cbMessageOK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnClearAllClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDeleteClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDeleteClicked2(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnNewPointClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnDefinePointsClicked (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnUpdatePointClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnNextClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnPrevClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnGotoPointClicked (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnRepeatOptionChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnRevisitClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnRevisitCloseClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStartClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStopClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnTimeLapseActionChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnTimeLapseTableEdited(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnTimeLapseCloseClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnTimeLapseEdit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnTimeLapseHelp (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnTimeLapseLoadClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnTimeLapseSaveClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnTimeLapseTableEdited(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnTimeLapseTimerTick(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnUseCrossHairClicked (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnUseCrossHairClicked_2 (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
//int  CVICALLBACK OnUseCrossHairClicked_3 (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
void CVICALLBACK MenuItemChanged(int menuBarHandle, int menuItemID, void *callbackData,  int panelHandle);
int  CVICALLBACK OnTimeLapseMenuRingChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnRegionTypeClicked (int panel, int control, int event,	void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnNewRegionClicked (int panel, int control, int event,	void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnRegionsCloseClicked (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnUseRegionsClicked (int panel, int control, int event,	void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnFocusTypeClicked (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnFocalPlanesClicked (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnUseDefaultCubeOptions (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnCubeSelect (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnCubeOptionsChanged (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnGetCameraValsClicked (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnSetCameraValsClicked (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

// Image window crosshair handler 
void OnCrosshairClicked (IcsViewerWindow *window, const Point p1, const Point p2, void* data);
void OnCrosshairMosaicClicked (IcsViewerWindow *window, const Point p1, const Point p2, void* data);

#endif










