#ifndef __ROI__
#define __ROI__

typedef struct
{
	int id;
	
	int region_panel;
	int region_panel_tab1;
	int region_panel_tab2;
	int region_panel_tab3;
	int region_panel_tab4;
	int focal_panel;
	
	int focal_plane_valid;
	
	int left;
	int top;
	int width;
	int height;
	
	double focal_point_a;
	double focal_point_b;
	double focal_point_c;
	
	GciCamera *camera;
	
	signal_table signal_table;
} region_of_interest;


//struct roi;
//typedef struct roi region_of_interest;


typedef void (*REGION_SELECTED_EVENT_HANDLER) (int left, int top, int width, int height, void *data);

region_of_interest* region_of_interest_selection_new(GciCamera *camera);

int region_of_interest_panel_init(region_of_interest* roi);

int region_of_interest_panel_display(region_of_interest* roi);

int region_of_interest_panel_hide(region_of_interest* roi);

void region_of_interest_destroy(region_of_interest* roi);

void region_of_interest_set_region(region_of_interest* roi, int left, int top, int width, int height);

void region_of_interest_get_region(region_of_interest* roi, int *left, int *top, int *width, int *height);

int region_of_interest_setup_focal_plane(region_of_interest* roi, int doItOutside);

int region_of_interest_goto_stage_xy(region_of_interest* roi, double x, double y, double *z);

int region_of_interest_z_for_xy(region_of_interest* roi, double x, double y, double *z);

void region_of_interest_selected_handler(region_of_interest* roi, REGION_SELECTED_EVENT_HANDLER handler, void *callback_data) ;

int region_of_interest_show_focus_options(region_of_interest* roi);

#endif
