#ifndef _REALTIME_OVERVIEW__
#define _REALTIME_OVERVIEW__

#include "gci_ui_module.h"
#include "camera\gci_camera.h" 
#include "stage\stage.h" 
#include "mosaic.h"        

typedef struct _Microscope Microscope;  

#define REALTIME_OVERVIEW_SUCCESS 1
#define REALTIME_OVERVIEW_ERROR 0

#define REALTIME_OVERVIEW_BASE         			(WM_USER + 70)
#define REALTIME_OVERVIEW_MOSAIC_UPDATE 		(REALTIME_OVERVIEW_BASE + 1)

typedef struct _realtime_overview
{
	UIModule parent;
	
	int timer;
	HWND window_hwnd;
	int activate;
	int panel_id;	
	int option_panel_id;
	int enable_button_id;
	int bpp;
	int cam_enter_callback_id;
	int cam_exit_callback_id;
	int cam_post_capture_callback_id;
	int mosaic_click_callback_id;
	int stage_wait_for_stop_completion_id;
	int master_camera_changed_callback_id;
	int got_pre_capture_stage_position;
	int got_post_capture_stage_position;
	FREE_IMAGE_TYPE type;
	int image_width;
	int image_height;
	double image_max;
	double fov_x;
	double fov_y;
	double cache_size_x;
	double cache_size_y;
	double true_microns_per_pixel;
    Rect region;

	double stage_x_before_image;
	double stage_y_before_image;
	double stage_x;
	double stage_y;
	double stage_weight;

	Microscope *ms;
	GciCamera *camera;  
	XYStage *stage;
	MosaicWindow* mosaic_window;

	FIBITMAP* mosaic;  

	char filename[50];

} realtime_overview;

realtime_overview* realtime_overview_new(void);
int realtime_overview_init(realtime_overview* rto);
void realtime_overview_activate(realtime_overview* rto);
void realtime_overview_activate_only_stage(realtime_overview* rto);
void realtime_overview_update_microns_per_pixel(realtime_overview* rto);
// On deactivate stop the time and do nother on camera signals
void realtime_overview_deactivate(realtime_overview* rto);
int realtime_overview_destroy(realtime_overview* rto); 
void realtime_overview_display(realtime_overview* rto);            
void realtime_overview_hide(realtime_overview* rto);
void realtime_overview_set_no_display(realtime_overview* rto, int no_display);
void realtime_overview_force_display_update(realtime_overview* rto);
void realtime_overview_set_image_max_scale_value (realtime_overview* rto, double max);
void realtime_overview_set_close_handler(realtime_overview* roi,
										 void (*close_handler) (IcsViewerWindow *window, void *data), void *callback_data);

#endif

 
