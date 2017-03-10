#ifndef __STAGE_PLATE_MANAGER__
#define __STAGE_PLATE_MANAGER__

#include "HardWareTypes.h"      
#include "HardWareDevice.h" 

#include "signals.h"
#include "gci_types.h"
#include "gci_ui_module.h"
#include "device_list.h"
#include "toolbox.h" 
#include "stage/stage.h"

#define STAGE_PLATE_MODULE_SUCCESS 0
#define STAGE_PLATE_MODULE_ERROR -1

#define STAGE_PLATE_MODULE_VTABLE_PTR(ob, member) ((ob)->vtable.member)
#define STAGE_PLATE_MODULE_VTABLE(ob, member) (*((ob)->vtable.member))

#define STAGEPLATE_MAX_NUMBER_OF_WELLS 400
#define STAGE_PLATE_NAME_SIZE 50

#define CHECK_STAGE_PLATE_MODULE_VTABLE_PTR(ob, member) if(STAGE_PLATE_MODULE_VTABLE_PTR(ob, member) == NULL) { \
    send_fluocube_error_text(ob, "member not implemented"); \
    return STAGE_PLATE_MODULE_ERROR; \
}  

#define CALL_STAGE_PLATE_MODULE_VTABLE_PTR(ob, member) if(STAGE_PLATE_MODULE_VTABLE(ob, member)(ob) == STAGE_PLATE_MODULE_ERROR ) { \
	send_fluocube_error_text(ob, "member failed");  \
	return STAGE_PLATE_MODULE_ERROR; \
}

typedef enum {PLATE_WELLPLATE, PLATE_SLIDE, PLATE_DISH} PlateType;

typedef enum {PLATE_SEL_DIALOG_NONE, PLATE_SEL_DIALOG_RECT, PLATE_SEL_DIALOG_WELLS} PlateDialogResult;

typedef enum {TL_WG_START_LEFT = 1, TL_WG_START_RIGHT = -1} StagePlateHorizontalStartPosition;
typedef enum {TL_WG_START_TOP = 1, TL_WG_START_BOTTOM = -1} StagePlateVerticalStartPosition;

typedef struct
{
    float  cx;
    float  cy;
	float  radius;

} WellRegion;

typedef struct
{
	WellRegion region;
	BYTE selected;
	int	   row;
	int	   col;

} Well;

typedef struct _StagePlate StagePlate;

struct _StagePlate
{
	char		name[STAGE_PLATE_NAME_SIZE];
	int			position;
	int			_wells_subgroup_selected;

	PlateType	type;
	int			rows;
    int			cols;
	double		x_offset;
	double		y_offset;
	double		x_size;
	double		y_size;
	double		x_spacing;
	double		y_spacing;

	StageShape	safe_region_shape;
	GCI_FPOINT	safe_left_top;
	GCI_FPOINT	safe_right_bottom;
	GCI_FPOINT	safe_center;
	float		safe_radius;

	double      microns_per_pixel;
};

typedef struct
{
	float scale_left;
	float scale_top;
	float scale_width;
	float scale_height;
	Rect draw_rect;
	int canvas;
	int panel_id;
	StagePlateModule* stage_plate_module;

} ScaledCanvas;

struct _StagePlateModule
{
  HardwareDevice parent; 
  
  ModuleDeviceConfigurator* dc;

  XYStage	 *stage;

  int	 	 _main_ui_panel;
  int		 _current_pos;
  
  // This panel is the add / edit panel
  int	 	 _details_ui_panel;
  int		 _region_selection_panel;
  int		 _region_selection_gen_opt_panel;
  HWND       _region_selection_hwnd;
  PlateDialogResult _region_selection_result;

  // We only want to maintain one array of wells at any one time
  // We can only have one place selected at a time.
  int		 _current_number_of_wells;
  Well		 wells[STAGEPLATE_MAX_NUMBER_OF_WELLS];

  double	 stage_aspect_ratio;

  ScaledCanvas*		 _region_selection_canvas;
  ScaledCanvas*		 _edit_canvas;
};

int CVICALLBACK OnStagePlateDetailsAdd (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK OnStagePlateDetailsEdit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

StagePlateModule* stage_plate_new(const char *name, const char *description, const char *data_dir, const char *data_file);

void stage_plate_set_stage(StagePlateModule* stage_plate_module, XYStage *stage);

void stage_plate_set_error_handler(StagePlateModule* stage_plate_module, UI_MODULE_ERROR_HANDLER handler, void *callback_data);

int  stage_plate_destroy(StagePlateModule* stage_plate_module);

int  stage_plate_initialise(StagePlateModule* stage_plate_module);
int  stage_plate_move_to_position_no_emit(StagePlateModule* stage_plate_module, int position);
int  stage_plate_move_to_position(StagePlateModule* stage_plate_module, int position);
int  stage_plate_goto_default_plate(StagePlateModule* stage_plate_module);
int  stage_plate_get_number_of_plates(StagePlateModule* stage_plate_module, int *number_of_cubes);
int  stage_plate_get_current_plate_position(StagePlateModule* stage_plate_module, int *position);
int  stage_plate_get_current_plate(StagePlateModule* stage_plate_module, StagePlate *plate);
int  stage_plate_get_plate_for_position(StagePlateModule* stage_plate_module, int position, StagePlate* plate);
int  stage_current_plate_has_valid_safe_region(StagePlateModule* stage_plate_module);
int  stage_current_plate_is_valid(StagePlateModule* stage_plate_module);
void draw_current_region_selection_dialog(StagePlateModule* stage_plate_module, StagePlate *plate);

PlateDialogResult stage_plate_display_region_selection_dialog(StagePlateModule* stage_plate_module);

int  stage_plate_load_active_plates_into_list_control(StagePlateModule* stage_plate_module, int panel, int ctrl);
int  stage_plate_get_entire_region_of_interest(StagePlateModule* stage_plate_module, RECT *rect);
int  stage_plate_get_entire_region_of_interest_for_plate(StagePlateModule* stage_plate_module, StagePlate *plate, RECT *rect);
int  stage_plate_get_topleft_and_bottomright_centres_for_plate(StagePlateModule* stage_plate_module, StagePlate *plate, POINT *top_left, POINT *bottom_right);
int  stage_plate_get_all_wells_for_plate(StagePlateModule* stage_plate_module, StagePlate *plate, Well *wells);
int  stage_plate_get_well_positions_for_plate(StagePlateModule* stage_plate_module, StagePlate *plate, Well *wells);
int  stage_plate_get_well_positions(StagePlateModule* stage_plate_module, Well *wells);
int  stage_plate_get_rows_cols_for_selected_wells_on_plate(StagePlateModule* stage_plate_module, StagePlate *plate, int *rows, int *cols);
//int	 stage_plate_get_selected_well_positions(StagePlateModule* stage_plate_module, Well *wells);

void stage_plate_clear_selected_wells(StagePlateModule* stage_plate_module);
void stage_plate_select_all_wells(StagePlateModule* stage_plate_module);

int  stage_plate_get_topleft_and_bottomright_centres_for_selected_wells_on_plate(StagePlateModule* stage_plate_module,
				StagePlate *plate, POINT *top_left, POINT *bottom_right);

int  stage_plate_are_wells_selected(StagePlateModule* stage_plate_module);

int stage_plate_generate_canvas_list_of_wells(StagePlateModule* stage_plate_module, StagePlate *plate);

// Client must free the returned array
StagePlate* stage_plate_get_active_plates(StagePlateModule* stage_plate_module);

int __cdecl plate_point_sort(const void *lhs_ptr, const void *rhs_ptr);

void plate_points_set_sorting_params(int is_horizontal, StagePlateHorizontalStartPosition horz_start,
														StagePlateVerticalStartPosition vert_start,
														int shortest_path);

// Drawing
ScaledCanvas* scaled_canvas_new(StagePlateModule* stage_plate_module, int panel, const char *label, int top, int left);
void scaled_canvas_set_scale_rect(ScaledCanvas* canvas, double left, double top, double width, double height);
void draw_plate_on_canvas (StagePlateModule* stage_plate_module, ScaledCanvas* canvas, StagePlate *plate);
void stage_plate_draw_plate_for_plate_ui_values(StagePlateModule* stage_plate_module);
void stage_plate_setup_stage_safe_region(StagePlateModule* stage_plate_module, StagePlate *plate);

Rect CanvasGetRectForWell(StagePlateModule* stage_plate_module, int panel, int control,
						 double cx, double cy, double width, double height);
// Signals
typedef void (*STAGE_PLATE_MODULE_EVENT_HANDLER) (StagePlateModule* stage_plate_module, void *data); 
typedef void (*STAGE_PLATE_MODULE_PLATE_EVENT_HANDLER) (StagePlateModule* stage_plate_module, StagePlate plate, void *data);
typedef void (*STAGE_PLATE_MODULE_CHANGE_EVENT_HANDLER) (StagePlateModule* stage_plate_module, int pos, void *data); 

int stage_plate_signal_close_handler_connect (StagePlateModule* stage_plate_module,
	STAGE_PLATE_MODULE_EVENT_HANDLER handler, void *callback_data);

int stage_plate_signal_plate_changed_handler_connect(StagePlateModule* stage_plate_module,
	STAGE_PLATE_MODULE_CHANGE_EVENT_HANDLER handler, void *callback_data);
int stage_plate_signal_plate_changed_handler_disconnect(StagePlateModule* stage_plate_module, int id);

int stage_plate_signal_plate_config_changed_handler_connect(StagePlateModule* stage_plate_module,
	STAGE_PLATE_MODULE_EVENT_HANDLER handler, void *callback_data);

int  CVICALLBACK OnStagePlateAddEditOkClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStagePlateChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStagePlateSetup(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStagePlateClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStagePlateItemChanged (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStagePlateTestClicked (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCanvasEvent (int panelHandle, int controlID, int event, void *callbackData, int eventData1, int eventData2);

#endif

 
