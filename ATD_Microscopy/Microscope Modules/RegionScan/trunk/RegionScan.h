#ifndef _REGION_SCAN__
#define _REGION_SCAN__

#include "HardwareTypes.h" 
#include "gci_ui_module.h"
#include "icsviewer_window.h"
#include "mosaic.h"

typedef struct _region_of_interest region_of_interest;
typedef struct _region_scan region_scan;

typedef enum {RS_SAVE = 1, RS_SAVE_DISPLAY = 2, RS_DISPLAY_ONLY = 3} RS_ACTION; 

struct _region_scan
{
	UIModule parent;
	
	HWND window_hwnd;
	LONG_PTR uimodule_func_ptr;
	int _master_camera_changed_signal_id;
	int has_run;
	int process_queue;		// Thread safe queue
	int roi_set;
	int lock;
	int panel_id;
	int frames_in_x;
	int frames_in_y;
	int total_frames;
	int tile_count;
	int read_callback_runner_thread_id;
	int debug_thread_finished;
	int acquisition_thread_id;
	int process_thread_id;
	int destroying;
	int frames_done;
	int cell_finding_enabled;
	int region_selected_handler_id;
	int perform_swautofocus_every_point;
	int setup_of_mosaic_metadata;
	volatile int pause_scan;
	volatile int stop_scan; 
	volatile int acquisition_done; 
	float fov_x;
	float fov_y;
	float nominal_overlap_percent;
	float x_overlap_percent;
	float y_overlap_percent;
	float x_overlap;
	float y_overlap;
	double roi_left;
	double roi_top;
	double roi_width;
	double roi_height;
	double start_time; 
	double stage_dwell;
	char output_filename[50];
	char output_dir[GCI_MAX_PATHNAME_LEN];
	char extension[10];
	char start_date_time[200];
	char end_date_time[200];

	RS_ACTION action;
	//FILE* seq1_fp;
	FILE* seq2_fp; 
		
	Microscope *ms;
	GciCamera *camera;  
	region_of_interest *roi;
	IcsViewerWindow *image_window;
	MosaicWindow* mosaic_window;

	FIBITMAP* mosaic;   
};

region_scan* region_scan_new(void);
int region_scan_destroy(region_scan* rs); 
void region_scan_display(region_scan* rs);            
void region_scan_hide(region_scan* rs);

int regionscan_start(region_scan *rs, RS_ACTION action, char *output_dir,
								 char *filename_prefix, char *filename_ext);

void regionscan_stop(region_scan *rs);

void regionscan_status(region_scan *rs, int *has_run, int *active, double *percentage_complete, char *start_time, char *end_time);

void regionscan_set_roi(region_scan* rs, double left, double top, double width, double height);

void regionscan_set_perform_swautofocus_every_point(region_scan* rs, int val);

void regionscan_save_metadata(GCIWindow *window, char *filename, char *extension, void* callback);

#endif

 
