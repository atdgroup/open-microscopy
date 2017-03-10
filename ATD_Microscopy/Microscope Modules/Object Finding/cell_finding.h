#ifndef __CELL_FINDING__
#define __CELL_FINDING__

#include "HardwareTypes.h"
#include "cell.h"
#include "cellmap.h"
#include "RegionOfInterest.h"
#include "mosaic.h"

typedef enum {CF_DYNAMIC_THRESHOLD = 0, CF_SIMPLE_THRESHOLD = 1} CF_THRESHOLD;

typedef struct _cell_finder cell_finder; 

typedef struct {

	int 	valid;
	FIARECT 	rect; 
	
} Box;


struct _cell_finder
{
	UIModule parent;
	
	int setup_panel_id;
	int criteria_panel_id;
	int show_processing;   	
	int charm_enabled;
	int save_images;
	int save_data;
	int save_data_to_timelapse;
	
	int number_of_cells;
	int number_of_boxes;
	int number_of_singles;
	int number_of_clusters;
	int number_of_grot;

	double stage_x;
	double stage_y;
	double stage_z;

	CF_THRESHOLD threshold_type;
	double min_cell_diameter;
	double max_cell_diameter;
	double cell_diameter_sumation;
	double min_cell_width;
	double min_cell_height;
	double max_cell_width;
	double max_cell_height;
	double max_cell_width_percentage_of_diameter;
	double max_cell_height_percentage_of_diameter;
	double box_cell_width;
	double box_cell_height;
	double box_width_percentage_larger;
	
	double overlap_percentage;		
	int overlap_vert_pixels;
	int overlap_horz_pixels;
	double overlap_vert_microns;
	double overlap_horz_microns;
	
	double frame_roi_left;
	double frame_roi_top;
	double frame_roi_width;
	double frame_roi_height; 

	int current_row;
	int current_col;
	int number_of_frames_in_row;
	int number_of_frames_in_col;
	
	double threshold_percentage;

	int    pre_process;
	
	Box *boxes;
	CellMap *map;
	region_of_interest *roi; 
	
	GciCamera *camera;
	Microscope *ms;
	
	char data_file_path[GCI_MAX_PATHNAME_LEN];
	
};


cell_finder* cell_finder_new(void);     
void cell_finder_destroy(cell_finder* cf);
void cell_finder_set_frame_overlap(cell_finder *cf, double percentage);
void cell_finder_display(cell_finder* cf);
void cell_finder_clear_map(cell_finder *cf);
int cell_finder_save_timelapse_file(cell_finder* cf, char *filepath); 

FIBITMAP* cell_finder_find_cells(cell_finder *cf, FIBITMAP *frame_image, 
								 double stage_left_offset, double stage_top_offset,double stage_z,
								 int validate_horz_overlap, int validate_vert_overlap);

int  CVICALLBACK cbCellsPreProcess(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCellFindingThreshold(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onCellInfoClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onCellInfoOk(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnClosePressed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMaxCellDiameterChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMinCellDiameterChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSaveDataChecked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnShowProcessingChecked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnEnableCharmChecked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSimpleThresholdTypeChecked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnTestPressed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCharmeOptimizerClicked (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCellFindingAdvancedPressed (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCharmeParametersClicked (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCriteriaCloseClicked (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnPreProcessClicked (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif
