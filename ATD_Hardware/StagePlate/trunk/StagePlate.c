#include "HardWareTypes.h"

#include "StagePlate.h"
#include "StagePlateUI.h"
#include "device_list_ui.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "password.h"

#include "toolbox.h"
#include "asynctmr.h"

#include <userint.h>
#include <utility.h>
#include <ansi_c.h> 

#define PIXEL_SIZE_OF_CLOSE_BUTTON_HEIGHT 35

static int create_all_wells_for_plate(StagePlateModule* stage_plate_module, StagePlate *plate);

static int default_error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)
{
	GCI_MessagePopup("StagePlateModule Error", error_string);
	
	return UIMODULE_ERROR_NONE;
}

static int STAGE_PLATE_MODULE_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (StagePlateModule*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (StagePlateModule *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

static int STAGE_PLATE_MODULE_PTR_PLATE_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (StagePlateModule*, StagePlate *, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (StagePlateModule *) args[0].void_ptr_data,  (StagePlate *) args[1].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

static int STAGE_PLATE_MODULE_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (StagePlateModule*, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (StagePlateModule *) args[0].void_ptr_data,  (int) args[1].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

// Return an array of the active plates
StagePlate* stage_plate_get_active_plates(StagePlateModule* stage_plate_module)
{
	int i, size = 0;
	StagePlate * device_node = NULL;
	StagePlate * set = NULL;
	CMDeviceNode *tmp = NULL;
	ListType list;

	size = device_conf_get_num_active_devices(DEVICE_CONF_CAST(stage_plate_module->dc));

	if(size < 1)
		return DEVICE_CONF_ERROR; 	

	list = device_conf_get_devices_in_use_list(DEVICE_CONF_CAST(stage_plate_module->dc));

	set = malloc(sizeof(StagePlate) * size);

	for(i=0; i < size; i++) {
	
		tmp = ListGetPtrToItem(list, i);  
		device_node = (StagePlate*) tmp->device;

		strcpy(set[i].name, tmp->name);
	}

	return set;
}

int stage_plate_load_active_plates_into_list_control(StagePlateModule* stage_plate_module, int panel, int ctrl)
{
	int pos;
	
	if(stage_plate_get_current_plate_position(stage_plate_module, &pos) == STAGE_PLATE_MODULE_ERROR)
		return STAGE_PLATE_MODULE_ERROR; 
	
	if(device_conf_load_active_devices_into_list_control(stage_plate_module->dc, panel, ctrl, pos) == DEVICE_CONF_ERROR) 
		return STAGE_PLATE_MODULE_ERROR; 
	
	return STAGE_PLATE_MODULE_SUCCESS;
}

static void OnConfigChanged(ModuleDeviceConfigurator* dc, void *data)
{
	StagePlateModule* stage_plate_module = (StagePlateModule* ) data;

	stage_plate_load_active_plates_into_list_control(stage_plate_module, stage_plate_module->_main_ui_panel, PLATE_INFO_POS);
}

static void OnEditAddPanelDisplayed(ModuleDeviceConfigurator* dc, void *data)
{
	StagePlateModule* stage_plate_module = (StagePlateModule* ) data;

	stage_plate_draw_plate_for_plate_ui_values(stage_plate_module);
}

int stage_plate_signal_plate_changed_handler_connect(StagePlateModule* stage_plate_module,
	STAGE_PLATE_MODULE_CHANGE_EVENT_HANDLER handler, void *callback_data)
{
	int id;
	if( (id=GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(stage_plate_module), "StagePlateChanged", handler, callback_data)) == SIGNAL_ERROR) {
		return STAGE_PLATE_MODULE_ERROR;
	}
	
	return id; 
}

int stage_plate_signal_plate_changed_handler_disconnect(StagePlateModule* stage_plate_module, int id)
{
	if( GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(stage_plate_module), "StagePlateChanged", id) == SIGNAL_ERROR) {
		return STAGE_PLATE_MODULE_ERROR;
	}
	
	return STAGE_PLATE_MODULE_SUCCESS; 
}

int stage_plate_signal_plate_config_changed_handler_connect(StagePlateModule* stage_plate_module,
	STAGE_PLATE_MODULE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(stage_plate_module->dc), "ConfigChanged", handler, callback_data) == SIGNAL_ERROR) {
		hardware_device_send_error_text(HARDWARE_DEVICE_CAST(stage_plate_module), "Can not connect signal handler for ConfigChanged signal");
		return STAGE_PLATE_MODULE_ERROR;
	}

	return STAGE_PLATE_MODULE_SUCCESS;	
}

// Converts one node to a string format used to save all data as an ini file.
int stage_plate_node_to_ini_fmt (ModuleDeviceConfigurator *conf, CMDeviceNode *node, char *buffer)
{
	StagePlate *plate = (StagePlate*) node->device;
	
	sprintf(buffer, "Id=%d\nName=%s\nPosition=%d\nType=%d\nX-Size=%f\nY-Size=%f\nX-Offset=%f\n"
					"Y-Offset=%f\nX-Spacing=%f\nY-Spacing=%f\nRows=%d\nCols=%d\n"
					"SafeRegionShape=%d\nSafeLeft=%f\nSafeTop=%f\nSafeRight=%f\nSafeBottom=%f\n"
					"SafeCenterX=%f\nSafeCenterY=%f\nSafeRadius=%f\n\n\n",
					 node->id, node->name, node->position, plate->type, plate->x_size, plate->y_size, 
					 plate->x_offset, plate->y_offset,
                     plate->x_spacing, plate->y_spacing,
					 plate->rows, plate->cols,
					 plate->safe_region_shape, plate->safe_left_top.x, plate->safe_left_top.y,
					 plate->safe_right_bottom.x, plate->safe_right_bottom.y,
					 plate->safe_center.x, plate->safe_center.y, plate->safe_radius);
		
	return DEVICE_CONF_SUCCESS;  
}


static char* construct_key(char *buffer, const char *section, const char* name)
{
	memset(buffer, 0, 1);
	
	sprintf(buffer, "%s:%s", section, name);
	
	return buffer;
}

int stage_plate_ini_fmt_to_node (ModuleDeviceConfigurator *conf, dictionary* ini, int section_number, CMDeviceNode *node)
{
	char *section = NULL, key[200];
	char *str_result = NULL;
		
	StagePlate* plate = node->device = malloc(sizeof(StagePlate));
	
	memset(plate, 0, sizeof(StagePlate));
	
	section = iniparser_getsecname(ini, section_number);   
	
	node->id = iniparser_getint(ini, construct_key(key, section, "Id"), -1);  
	
	if(node->id == -1)
		return DEVICE_CONF_ERROR;  	
	
	str_result = iniparser_getstring(ini, construct_key(key, section, "Name"), "Unknown");
	strncpy(node->name, str_result, strlen(str_result) + 1); 
		
	node->position = iniparser_getint(ini, construct_key(key, section, "Position"), -1);
			
	// type
	if((plate->type = iniparser_getint(ini, construct_key(key, section, "Type"), -1)) < 0)
		return DEVICE_CONF_ERROR;  

	// x_size
	if((plate->x_size = iniparser_getdouble(ini, construct_key(key, section, "X-Size"), -1)) < 0)
		return DEVICE_CONF_ERROR;  
			
	// y_size
	if((plate->y_size = iniparser_getdouble(ini, construct_key(key, section, "Y-Size"), -1)) < 0)
		return DEVICE_CONF_ERROR;  

	// x_offset
	plate->x_offset = iniparser_getdouble(ini, construct_key(key, section, "X-Offset"), 9999999);
	
	// y_offset
	plate->y_offset = iniparser_getdouble(ini, construct_key(key, section, "Y-Offset"), -9999999);
		
	// x_spacing
	if((plate->x_spacing = iniparser_getdouble(ini, construct_key(key, section, "X-Spacing"), -1)) < 0)
		return DEVICE_CONF_ERROR;    
		
	// y_spacing
	if((plate->y_spacing = iniparser_getdouble(ini, construct_key(key, section, "Y-Spacing"), -1)) < 0)
		return DEVICE_CONF_ERROR;    
		
	// rows
	if((plate->rows = iniparser_getint(ini, construct_key(key, section, "Rows"), -1)) < 0)
		return DEVICE_CONF_ERROR;   
        
    // cols
	if((plate->cols = iniparser_getint(ini, construct_key(key, section, "Cols"), -1)) < 0)
		return DEVICE_CONF_ERROR;   
		
	if((plate->safe_region_shape = iniparser_getint(ini, construct_key(key, section, "SafeRegionShape"), -1)) < 0)
		return DEVICE_CONF_ERROR; 

	// safe left
	plate->safe_left_top.x = (float) iniparser_getdouble(ini, construct_key(key, section, "SafeLeft"), 0.0);

	// safe top
	plate->safe_left_top.y = (float) iniparser_getdouble(ini, construct_key(key, section, "SafeTop"), 0.0);
 
	// safe right
	plate->safe_right_bottom.x = (float) iniparser_getdouble(ini, construct_key(key, section, "SafeRight"), 0.0); 

	// safe bottom
	plate->safe_right_bottom.y = (float) iniparser_getdouble(ini, construct_key(key, section, "SafeBottom"), 0.0);
	
	plate->safe_center.x = (float) iniparser_getdouble(ini, construct_key(key, section, "SafeCenterX"), 0.0);

	plate->safe_center.y = (float) iniparser_getdouble(ini, construct_key(key, section, "SafeCenterY"), 0.0);

	plate->safe_radius = (float) iniparser_getdouble(ini, construct_key(key, section, "SafeRadius"), 0.0);

	plate = NULL;
	
	return DEVICE_CONF_SUCCESS;  
}

int stage_plate_current_value_text(HardwareDevice* device, char* info)
{
	StagePlate item;
	
	if (info==NULL)
		return STAGE_PLATE_MODULE_ERROR;

	if (stage_plate_get_current_plate((StagePlateModule*)device, &item)==STAGE_PLATE_MODULE_ERROR)
		return STAGE_PLATE_MODULE_ERROR;
	
	if (item.name!=NULL){
		strncpy(info, item.name, UIMODULE_NAME_LEN);
		return STAGE_PLATE_MODULE_SUCCESS;
	}

	return STAGE_PLATE_MODULE_ERROR;
}


int stage_plate_hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode)
{
	StagePlateModule* stage_plate_module = (StagePlateModule*)device;
	int pos;
	FILE *fp = NULL;

	dictionary *d = dictionary_new(5);
	
	stage_plate_get_current_plate_position(stage_plate_module, &pos);

	fp = fopen(filepath, mode);
	
	if(fp == NULL)
		return STAGE_PLATE_MODULE_ERROR;

	dictionary_set(d, UIMODULE_GET_NAME(stage_plate_module), NULL);
	dictionary_setint(d, "Position", pos);      

	iniparser_save(d, fp); 
	
	fclose(fp);
	dictionary_del(d);

	return STAGE_PLATE_MODULE_SUCCESS;
}

int stage_plate_hardware_load_state_from_file (HardwareDevice* device, const char* filepath)
{
	StagePlateModule* stage_plate_module = (StagePlateModule*)device;
	dictionary* d = NULL;
	int pos, file_size, num_devices;    
	char buffer[500] = "";

	if(!FileExists(filepath, &file_size))
		return STAGE_PLATE_MODULE_ERROR;	 	
	
	d = iniparser_load(filepath);  

	num_devices = device_conf_get_num_active_devices(stage_plate_module->dc);

	if(d != NULL) {

		pos = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(stage_plate_module), "position"), -1); 

		if(pos >= 0 && pos <= num_devices)
			stage_plate_move_to_position(stage_plate_module, pos);
	}

    dictionary_del(d);

	return STAGE_PLATE_MODULE_SUCCESS;
}


void stage_plate_clear_selected_wells(StagePlateModule* stage_plate_module)
{
    int i;
    Well *well = NULL;

    for (i=0; i < stage_plate_module->_current_number_of_wells; i++)
    {
		well = &stage_plate_module->wells[i];

		well->selected = 0;
    }
        
    return;
}

void stage_plate_select_all_wells(StagePlateModule* stage_plate_module)
{
    int i;
    Well *well = NULL;

    for (i=0; i < stage_plate_module->_current_number_of_wells; i++)
    {
		well = &stage_plate_module->wells[i];

		well->selected = 1;
    }
        
    return;
}

void stage_plate_draw_plate(StagePlateModule* stage_plate_module, StagePlate plate)
{
	draw_plate_on_canvas (stage_plate_module, stage_plate_module->_edit_canvas, &plate);
}

void stage_plate_draw_plate_for_plate_ui_values(StagePlateModule* stage_plate_module)
{
	StagePlate plate;

	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_TYPE, &(plate.type));
	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_ROWS, &(plate.rows)); 
	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_COLS, &(plate.cols)); 
	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_XSIZE, &(plate.x_size)); 
	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_YSIZE, &(plate.y_size)); 
	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_XOFFSET, &(plate.x_offset)); 
	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_YOFFSET, &(plate.y_offset)); 
	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_XSPACING, &(plate.x_spacing)); 
	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_YSPACING, &(plate.y_spacing)); 
	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_L, &(plate.safe_left_top.x)); 
	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_T, &(plate.safe_left_top.y)); 
	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_R, &(plate.safe_right_bottom.x)); 
	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_B, &(plate.safe_right_bottom.y)); 
	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_SHAPE, &(plate.safe_region_shape)); 
	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_CX, &(plate.safe_center.x)); 
	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_CY, &(plate.safe_center.y)); 
	GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_RADIUS, &(plate.safe_radius)); 

	// dim ctrls not in use, safe_region_shape = 1 = circle
	SetCtrlAttribute(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_L, ATTR_DIMMED, plate.safe_region_shape);
	SetCtrlAttribute(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_T, ATTR_DIMMED, plate.safe_region_shape);
	SetCtrlAttribute(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_R, ATTR_DIMMED, plate.safe_region_shape);
	SetCtrlAttribute(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_B, ATTR_DIMMED, plate.safe_region_shape);
	SetCtrlAttribute(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_CX, ATTR_DIMMED, !plate.safe_region_shape);
	SetCtrlAttribute(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_CY, ATTR_DIMMED, !plate.safe_region_shape);
	SetCtrlAttribute(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_RADIUS, ATTR_DIMMED, !plate.safe_region_shape);

	// this create all wells will put into into stage_plate_module that may not reflect the current plate!
	create_all_wells_for_plate(stage_plate_module, &plate);
	// make the drawing
	stage_plate_draw_plate(stage_plate_module, plate);
	// now reset the info in the module for the current plate as a workaround.
	stage_plate_get_current_plate(stage_plate_module, &plate);
	create_all_wells_for_plate(stage_plate_module, &plate);
}

void draw_current_region_selection_dialog(StagePlateModule* stage_plate_module, StagePlate *plate)
{
	int width, height;

	GetPanelAttribute(stage_plate_module->_region_selection_panel, ATTR_WIDTH, &width);
	GetPanelAttribute(stage_plate_module->_region_selection_panel, ATTR_HEIGHT, &height); 

	SetCtrlAttribute(stage_plate_module->_region_selection_panel, stage_plate_module->_region_selection_canvas->canvas,
		ATTR_WIDTH, width - 20);

	SetCtrlAttribute(stage_plate_module->_region_selection_panel, stage_plate_module->_region_selection_canvas->canvas,
		ATTR_HEIGHT, height - PIXEL_SIZE_OF_CLOSE_BUTTON_HEIGHT - 40);

	// Move Close Button
	SetCtrlAttribute(stage_plate_module->_region_selection_panel,
		ROI_SELECT_OK_BUTTON, ATTR_LEFT, width - 70);

	SetCtrlAttribute(stage_plate_module->_region_selection_panel, ROI_SELECT_OK_BUTTON,
		ATTR_TOP, height - 35);  

	draw_plate_on_canvas (stage_plate_module, stage_plate_module->_region_selection_canvas, plate);
}

void stage_plate_get_window_height_for_stage_aspect_ratio(StagePlateModule* stage_plate_module, int width, int *height)
{
	*height = (int) (width / stage_plate_module->stage_aspect_ratio);
}


LRESULT CALLBACK StagePlateRegionChooserWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	LONG_PTR data = GetWindowLongPtr (hwnd, GWLP_USERDATA); 
	
	StagePlateModule* stage_plate_module = (StagePlateModule*) data;
	
	switch(message) {
			
		case WM_EXITSIZEMOVE:
    	{
			StagePlate plate;

			stage_plate_get_current_plate(stage_plate_module, &plate);

			draw_current_region_selection_dialog(stage_plate_module, &plate);

			break;
		}

		case WM_SIZE:
    	{
			StagePlate plate;
	
			stage_plate_get_current_plate(stage_plate_module, &plate);

			draw_current_region_selection_dialog(stage_plate_module, &plate);

			break;
		}

		case WM_SIZING:
		{
			// Keep aspect ratio to that of the stage limits.

			int width, height;
			RECT *rect = (RECT*)lParam;

			width = rect->right - rect->left;
		
			stage_plate_get_window_height_for_stage_aspect_ratio(stage_plate_module, width, &height);

			rect->right = rect->left + width;
			rect->bottom = rect->top + height + PIXEL_SIZE_OF_CLOSE_BUTTON_HEIGHT;

			return 1;
    	}
	
      	default:
		
        	break;
   	}

	return CallWindowProc ((WNDPROC) ui_module_get_original_wndproc_ptr(UIMODULE_CAST(stage_plate_module)),
							hwnd, message, wParam, lParam);
}

StagePlateModule* stage_plate_new(const char *name, const char *description, const char *data_dir, const char *data_file)
{
	int window_handle, left, top, width, height;

	StagePlateModule* stage_plate_module = (StagePlateModule*) malloc(sizeof(StagePlateModule));

	ui_module_set_error_handler(UIMODULE_CAST(stage_plate_module), default_error_handler, stage_plate_module);  
	
	//stage_plate_module->shape_regions = ListCreate(sizeof(Well));
	stage_plate_module->stage = NULL;
	stage_plate_module->_current_pos = -1; 
	
	// Ok attach to the add and edit buttons 
	stage_plate_module->dc = device_conf_new();
	
	ui_module_set_data_dir(UIMODULE_CAST(stage_plate_module->dc), data_dir);
	device_conf_set_default_filename(stage_plate_module->dc, data_file);        
	device_conf_set_max_active_num_devices(stage_plate_module->dc, 10);
	
	stage_plate_module->dc->save_node_as_ini_fmt = stage_plate_node_to_ini_fmt; 
	stage_plate_module->dc->read_node_from_ini_fmt = stage_plate_ini_fmt_to_node;
		
	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(stage_plate_module), name); 
	ui_module_set_description(UIMODULE_CAST(stage_plate_module), description);
	
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(stage_plate_module), hardware_get_current_value_text) = stage_plate_current_value_text; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(stage_plate_module), hardware_save_state_to_file) = stage_plate_hardware_save_state_to_file; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(stage_plate_module), hardware_load_state_from_file) = stage_plate_hardware_load_state_from_file; 

	stage_plate_module->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(stage_plate_module), "StagePlateUI.uir", PLATE_INFO, 1);   

	stage_plate_module->_details_ui_panel = ui_module_add_panel(UIMODULE_CAST(stage_plate_module), "StagePlateUI.uir", EDIT_PANEL, 0);
	
	GetCtrlAttribute(stage_plate_module->_details_ui_panel, EDIT_PANEL_CANVAS_BORDER, ATTR_LEFT, &left);
	GetCtrlAttribute(stage_plate_module->_details_ui_panel, EDIT_PANEL_CANVAS_BORDER, ATTR_TOP, &top);
	GetCtrlAttribute(stage_plate_module->_details_ui_panel, EDIT_PANEL_CANVAS_BORDER, ATTR_WIDTH, &width);
	GetCtrlAttribute(stage_plate_module->_details_ui_panel, EDIT_PANEL_CANVAS_BORDER, ATTR_HEIGHT, &height);

	stage_plate_module->_edit_canvas = scaled_canvas_new(stage_plate_module,
		stage_plate_module->_details_ui_panel, "", left, top);

	SetCtrlAttribute(stage_plate_module->_details_ui_panel, stage_plate_module->_edit_canvas->canvas, 
		ATTR_LEFT, left);

	SetCtrlAttribute(stage_plate_module->_details_ui_panel, stage_plate_module->_edit_canvas->canvas, 
		ATTR_TOP, top);

	SetCtrlAttribute(stage_plate_module->_details_ui_panel, stage_plate_module->_edit_canvas->canvas, 
		ATTR_WIDTH, width);

	SetCtrlAttribute(stage_plate_module->_details_ui_panel, stage_plate_module->_edit_canvas->canvas, 
		ATTR_HEIGHT, height);

	stage_plate_module->_region_selection_panel = ui_module_add_panel(UIMODULE_CAST(stage_plate_module), "StagePlateUI.uir", ROI_SELECT, 0);
	//stage_plate_module->_region_selection_gen_opt_panel = ui_module_add_panel_as_child(UIMODULE_CAST(stage_plate_module), "StagePlateUI.uir", GEN_OPT, stage_plate_module->_region_selection_panel);
	
	stage_plate_module->_region_selection_canvas = scaled_canvas_new(stage_plate_module,
		stage_plate_module->_region_selection_panel, "", 30, 10);

	GetPanelAttribute (stage_plate_module->_region_selection_panel, ATTR_SYSTEM_WINDOW_HANDLE, &window_handle);   
	stage_plate_module->_region_selection_hwnd = (HWND) window_handle;

	ui_module_set_window_proc(UIMODULE_CAST(stage_plate_module), stage_plate_module->_region_selection_panel, (LONG_PTR) StagePlateRegionChooserWndProc);

	hardware_device_set_as_manual(HARDWARE_DEVICE_CAST(stage_plate_module));

	return stage_plate_module;
}

void stage_plate_set_stage(StagePlateModule* stage_plate_module, XYStage *stage)
{	
	double xmax, xmin, ymax, ymin, xrange, yrange;
	
	stage_plate_module->stage = stage;

	xmax = stage_plate_module->stage->_limits.max_x;
	xmin = stage_plate_module->stage->_limits.min_x;
	ymax = stage_plate_module->stage->_limits.max_y;
	ymin = stage_plate_module->stage->_limits.min_y;
	
	stage_plate_module->stage_aspect_ratio = (xmax - xmin) / (ymax - ymin);

// Setup the scaled canvas's to the stagelimits plus a bit

	xrange = xmax - xmin;
	yrange = ymax - ymin;
	
	xmin = xmin - xrange/20.0;
	ymin = ymin - yrange/20.0;
	
	xrange += xrange/10.0;
	yrange += yrange/10.0;

	scaled_canvas_set_scale_rect(stage_plate_module->_region_selection_canvas,
		xmin, ymin, xrange, yrange);

	scaled_canvas_set_scale_rect(stage_plate_module->_edit_canvas,
		xmin, ymin, xrange, yrange);
}

int stage_plate_destroy(StagePlateModule* stage_plate_module)
{
	device_conf_destroy(stage_plate_module->dc);      
	
	//ListDispose(stage_plate_module->shape_regions);

	ui_module_restore_cvi_wnd_proc(stage_plate_module->_region_selection_panel);

	ui_module_destroy(UIMODULE_CAST(stage_plate_module));
	
  	free(stage_plate_module);
  	
  	return STAGE_PLATE_MODULE_SUCCESS;
}


int stage_plate_goto_default_position(StagePlateModule* stage_plate_module)
{
	int pos=0;
	
	if(device_conf_get_default_position(stage_plate_module->dc, &pos) == DEVICE_CONF_ERROR)
		return STAGE_PLATE_MODULE_ERROR;
	
	if(stage_plate_move_to_position(stage_plate_module, pos) == STAGE_PLATE_MODULE_ERROR)
		return STAGE_PLATE_MODULE_ERROR;  	

	return STAGE_PLATE_MODULE_SUCCESS;
}

int stage_plate_get_number_of_plates(StagePlateModule* stage_plate_module, int *number_of_plates)
{
	*number_of_plates = device_conf_get_num_active_devices(stage_plate_module->dc);
	
	return STAGE_PLATE_MODULE_SUCCESS;
}

int stage_plate_initialise(StagePlateModule* stage_plate_module)
{
	int device_config_panel_id;

	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(stage_plate_module), "StagePlateChanged", STAGE_PLATE_MODULE_PTR_INT_MARSHALLER); 
	
    if ( InstallCtrlCallback (stage_plate_module->_main_ui_panel, PLATE_INFO_SETUP, OnStagePlateSetup, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;
  	
  	if ( InstallCtrlCallback (stage_plate_module->_main_ui_panel, PLATE_INFO_CLOSE, OnStagePlateClose, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;
		
  	if ( InstallCtrlCallback (stage_plate_module->_main_ui_panel, PLATE_INFO_POS, OnStagePlateChanged, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;
	
	if ( InstallCtrlCallback (stage_plate_module->_details_ui_panel, EDIT_PANEL_OK_BUTTON, OnStagePlateAddEditOkClicked, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;	
	
	device_config_panel_id = device_conf_get_panel_id(stage_plate_module->dc);
	
	if ( InstallCtrlCallback (device_config_panel_id, DLIST_CONF_ADD_BUTTON, OnStagePlateDetailsAdd, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;
	
  	if ( InstallCtrlCallback (device_config_panel_id, DLIST_CONF_EDIT_BUTTON, OnStagePlateDetailsEdit, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;

	if ( InstallCtrlCallback (stage_plate_module->_details_ui_panel, EDIT_PANEL_COLS, OnStagePlateItemChanged, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;

	if ( InstallCtrlCallback (stage_plate_module->_details_ui_panel, EDIT_PANEL_ROWS, OnStagePlateItemChanged, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;

	if ( InstallCtrlCallback (stage_plate_module->_details_ui_panel, EDIT_PANEL_TYPE, OnStagePlateItemChanged, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;

	if ( InstallCtrlCallback (stage_plate_module->_details_ui_panel, EDIT_PANEL_XOFFSET, OnStagePlateItemChanged, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;

	if ( InstallCtrlCallback (stage_plate_module->_details_ui_panel, EDIT_PANEL_YOFFSET, OnStagePlateItemChanged, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;

	if ( InstallCtrlCallback (stage_plate_module->_details_ui_panel, EDIT_PANEL_XSIZE, OnStagePlateItemChanged, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;

	if ( InstallCtrlCallback (stage_plate_module->_details_ui_panel, EDIT_PANEL_YSIZE, OnStagePlateItemChanged, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;

	if ( InstallCtrlCallback (stage_plate_module->_details_ui_panel, EDIT_PANEL_XSPACING, OnStagePlateItemChanged, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;

	if ( InstallCtrlCallback (stage_plate_module->_details_ui_panel, EDIT_PANEL_YSPACING, OnStagePlateItemChanged, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;

	if ( InstallCtrlCallback (stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_SHAPE, OnStagePlateItemChanged, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;
	
	if ( InstallCtrlCallback (stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_L, OnStagePlateItemChanged, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;

	if ( InstallCtrlCallback (stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_T, OnStagePlateItemChanged, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;

	if ( InstallCtrlCallback (stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_R, OnStagePlateItemChanged, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;

	if ( InstallCtrlCallback (stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_B, OnStagePlateItemChanged, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;

	if ( InstallCtrlCallback (stage_plate_module->_details_ui_panel, EDIT_PANEL_TEST_BUTTON, OnStagePlateTestClicked, stage_plate_module) < 0)
		return STAGE_PLATE_MODULE_ERROR;

	if ( InstallCtrlCallback (stage_plate_module->_region_selection_panel,
		stage_plate_module->_region_selection_canvas->canvas, OnCanvasEvent, stage_plate_module->_region_selection_canvas) < 0)
		return STAGE_PLATE_MODULE_ERROR;

	ui_module_set_main_panel_title (UIMODULE_CAST(stage_plate_module));
	
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(stage_plate_module->dc), "ConfigChanged", OnConfigChanged, stage_plate_module) == SIGNAL_ERROR) {
		hardware_device_send_error_text(HARDWARE_DEVICE_CAST(stage_plate_module), "Can not connect signal handler for ConfigChanged signal");
		return STAGE_PLATE_MODULE_ERROR;
	}
	
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(stage_plate_module->dc), "EditAddPanelDisplayed", OnEditAddPanelDisplayed, stage_plate_module) == SIGNAL_ERROR) {
		hardware_device_send_error_text(HARDWARE_DEVICE_CAST(stage_plate_module), "Can not connect signal handler for EditAddPanelDisplayed signal");
		return STAGE_PLATE_MODULE_ERROR;
	}

	if(device_conf_load_default_node_data(stage_plate_module->dc) == DEVICE_CONF_ERROR)
		return STAGE_PLATE_MODULE_ERROR; 
	
	// Update the ui
	if(stage_plate_load_active_plates_into_list_control(stage_plate_module, stage_plate_module->_main_ui_panel, PLATE_INFO_POS) == STAGE_PLATE_MODULE_ERROR)   		
		return STAGE_PLATE_MODULE_ERROR;
	
	stage_plate_goto_default_position(stage_plate_module);

	return STAGE_PLATE_MODULE_SUCCESS;	
}


int stage_plate_get_current_plate_position(StagePlateModule* stage_plate_module, int *position)
{
    *position = stage_plate_module->_current_pos;

  	return STAGE_PLATE_MODULE_SUCCESS;
}


static int create_all_wells_for_plate(StagePlateModule* stage_plate_module, StagePlate *plate)
{
	int count = 0, c, r;

	// Erase array of Wells
	memset(stage_plate_module->wells, 0, sizeof(Well) * STAGEPLATE_MAX_NUMBER_OF_WELLS);

	stage_plate_module->_current_number_of_wells = plate->cols *  plate->rows;

	for(c=0; c < plate->cols; c++) {
		for(r=0; r < plate->rows; r++) {

			// Get all wells for plate
			stage_plate_module->wells[count].selected = 1;
			stage_plate_module->wells[count].row = r;
			stage_plate_module->wells[count].col = c;
			stage_plate_module->wells[count].region.cx = (float) (plate->x_offset + (c * plate->x_spacing *1000));
			stage_plate_module->wells[count].region.cy  = (float) (plate->y_offset + (r * plate->y_spacing *1000));
			stage_plate_module->wells[count].region.radius = (float) (plate->x_size / 2 * 1000.0);

			count++;		
		}
	}

	return count; 
}

void stage_plate_setup_stage_safe_region(StagePlateModule* stage_plate_module, StagePlate *plate)
{
	if(plate->safe_region_shape == STAGE_SHAPE_RECTANGLE) {
		Roi roi;

		roi.min_x = plate->safe_left_top.x;
		roi.max_x = plate->safe_right_bottom.x;
		roi.min_y = plate->safe_left_top.y;
		roi.max_y = plate->safe_right_bottom.y;
		roi.min_z = -DBL_MAX;
		roi.max_z = DBL_MAX;

		stage_set_safe_region_rectangle(stage_plate_module->stage, roi);
	}		
	else if(plate->safe_region_shape == STAGE_SHAPE_CIRCLE) {
		Coord coord;

		coord.x = plate->safe_center.x;
		coord.y = plate->safe_center.y;
		coord.z = 0.0;

		stage_set_safe_region_circle(stage_plate_module->stage, plate->safe_radius, coord);
	}
}

int stage_plate_move_to_position_no_emit(StagePlateModule* stage_plate_module, int position)
{
	int status = UIMODULE_ERROR_NONE;
	double start_time = Timer();
	StagePlate plate;

	logger_log(UIMODULE_LOGGER(stage_plate_module), LOGGER_INFORMATIONAL, "%s changed", UIMODULE_GET_DESCRIPTION(stage_plate_module));
	stage_plate_module->_current_pos = position;
	
	stage_plate_get_current_plate(stage_plate_module, &plate);
	create_all_wells_for_plate(stage_plate_module, &plate);
	draw_current_region_selection_dialog(stage_plate_module, &plate);

	stage_plate_setup_stage_safe_region(stage_plate_module, &plate);

	SetCtrlVal(stage_plate_module->_main_ui_panel, PLATE_INFO_POS, position);

  	return STAGE_PLATE_MODULE_SUCCESS;
}

int stage_plate_move_to_position(StagePlateModule* stage_plate_module, int position)
{
	stage_plate_move_to_position_no_emit(stage_plate_module, position);

//	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(stage_plate_module), "StagePlateChanged", GCI_VOID_POINTER, stage_plate_module);
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(stage_plate_module), "StagePlateChanged", GCI_VOID_POINTER, stage_plate_module, GCI_INT, position);

  	return STAGE_PLATE_MODULE_SUCCESS;
}

int stage_plate_get_current_plate(StagePlateModule* stage_plate_module, StagePlate *plate)
{
	int pos;
	CMDeviceNode* node = NULL;
	StagePlate *tmp;
	
	if(stage_plate_get_current_plate_position(stage_plate_module, &pos) == STAGE_PLATE_MODULE_ERROR)
		return STAGE_PLATE_MODULE_ERROR; 
		
	if((node = device_conf_get_node_at_position(stage_plate_module->dc, pos)) == NULL)
		return STAGE_PLATE_MODULE_ERROR; 
	
	tmp = (StagePlate *) node->device;
	
	memcpy(plate, tmp, sizeof(StagePlate));
	strncpy(plate->name, node->name, STAGE_PLATE_NAME_SIZE-1);  // we know is no bigger
	plate->position = node->position;
	
	return STAGE_PLATE_MODULE_SUCCESS; 
}

int  stage_plate_get_plate_for_position(StagePlateModule* stage_plate_module, int position, StagePlate* plate)
{
	CMDeviceNode* node = NULL;
	StagePlate *tmp;
	
	if((node = device_conf_get_node_at_position(stage_plate_module->dc, position)) == NULL)
		return STAGE_PLATE_MODULE_ERROR; 
	
	tmp = (StagePlate *) node->device;
	
	memcpy(plate, tmp, sizeof(StagePlate));
	strncpy(plate->name, node->name, STAGE_PLATE_NAME_SIZE-1);  // we know is no bigger

	plate->position = node->position;
	
	return STAGE_PLATE_MODULE_SUCCESS; 
}

int stage_current_plate_has_valid_safe_region(StagePlateModule* stage_plate_module)
{
	StagePlate plate;

	stage_plate_get_current_plate(stage_plate_module, &plate);

	if(plate.safe_left_top.x != 0.0 || plate.safe_left_top.y != 0.0
		|| plate.safe_right_bottom.x != 0.0 || plate.safe_right_bottom.y != 0.0) {

		return 1;
	}

	return 0;
}

int stage_current_plate_is_valid(StagePlateModule* stage_plate_module)
{
	StagePlate plate;

	stage_plate_get_current_plate(stage_plate_module, &plate);

	if(plate.cols == 0 || plate.rows == 0)
		return 0;

	if(plate.x_size == 0 || plate.y_size == 0)
		return 0;

	if(plate.type == PLATE_WELLPLATE) {
		if(plate.x_spacing == 0 || plate.y_spacing == 0)
			return 0;
	}

	return 1;
}

int stage_plate_get_entire_region_of_interest_for_plate(StagePlateModule* stage_plate_module, StagePlate *plate, RECT *rect)
{
	rect->left = (long) (plate->x_offset - plate->x_size/2.0*1000);
	rect->top = (long) (plate->y_offset -  plate->y_size/2.0*1000);
	rect->right  = (long) (rect->left + (plate->cols - 1)*plate->x_spacing*1000 + plate->x_size*1000);
	rect->bottom = (long) (rect->top  + (plate->rows - 1)*plate->y_spacing*1000 + plate->y_size*1000); 

	return 0;
}

int stage_plate_are_wells_selected(StagePlateModule* stage_plate_module)
{
	int i;
    Well *well = NULL;

    for (i=0; i < stage_plate_module->_current_number_of_wells; i++)
    {
		well = &stage_plate_module->wells[i];

        if (well->selected)
        {
            return 1;
        }	
    }
        
    return 0;
}

int stage_plate_get_topleft_and_bottomright_centres_for_plate(StagePlateModule* stage_plate_module, StagePlate *plate, POINT *top_left, POINT *bottom_right)
{
	if(stage_plate_are_wells_selected(stage_plate_module)) {
	
		// Return the top left and bottom right of the boundary of selected wells
		return stage_plate_get_topleft_and_bottomright_centres_for_selected_wells_on_plate(
			stage_plate_module, plate, top_left, bottom_right);
	
	}
	else {
		top_left->x = (int)plate->x_offset;
		top_left->y = (int)plate->y_offset;

		bottom_right->x = top_left->x + (int)((double)(plate->cols - 1)*plate->x_spacing*1000.0);
		bottom_right->y = top_left->y + (int)((double)(plate->rows - 1)*plate->y_spacing*1000.0);
	}

	return 0;
}

int stage_plate_get_rows_cols_for_selected_wells_on_plate(StagePlateModule* stage_plate_module, StagePlate *plate, int *rows, int *cols)
{
	if(stage_plate_are_wells_selected(stage_plate_module)) {
	
		// Return the rows and cols of the boundary of selected wells
		int number_of_wells = plate->rows * plate->cols;

		Well* wells = (Well*) malloc(sizeof(Well) * number_of_wells);

		number_of_wells = stage_plate_get_well_positions (stage_plate_module, wells);
		
		plate_points_set_sorting_params(1, TL_WG_START_LEFT, TL_WG_START_TOP, 0);
		qsort(wells, number_of_wells, sizeof(Well), plate_point_sort); 

		if(number_of_wells < 2)
		{
			if(number_of_wells == 1){
				*rows = 1;
				*cols = 1;
			}
			else{
				*rows = 0;
				*cols = 0;
			}
			return 0;
		}

		*rows = abs(wells[number_of_wells - 1].row - wells[0].row) + 1;
		*cols = abs(wells[number_of_wells - 1].col - wells[0].col) + 1;

		free(wells);
	}
	else {
		*rows = plate->rows;
		*cols = plate->cols;
	}

	return 0;
}

int stage_plate_get_entire_region_of_interest(StagePlateModule* stage_plate_module, RECT *rect)
{
	StagePlate plate;
	Microscope *ms = NULL;
	GciCamera *camera = NULL;

	stage_plate_get_current_plate(stage_plate_module, &plate);

	return stage_plate_get_entire_region_of_interest_for_plate(stage_plate_module, &plate, rect);
}

int stage_plate_get_all_wells_for_plate(StagePlateModule* stage_plate_module, StagePlate *plate, Well *wells)
{
	int count = 0, i = 0;

	for(i=0; i < stage_plate_module->_current_number_of_wells; i++) {		
		wells[count] = stage_plate_module->wells[i];	
		count++;
	}

	return count; 
}

int stage_plate_get_well_positions_for_plate(StagePlateModule* stage_plate_module, StagePlate *plate, Well *wells)
{
	int count = 0, i = 0;

	for(i=0; i < stage_plate_module->_current_number_of_wells; i++) {
			
		// Choose the selected wells
		if(stage_plate_module->wells[i].selected) {
			wells[count] = stage_plate_module->wells[i];	
			count++;
		}
	}

	return count; 
}

/*
int stage_plate_get_selected_well_positions(StagePlateModule* stage_plate_module, Well *wells)
{
	int count = 0;
	int i;
    Well *well = NULL;

	int size = ListNumItems(stage_plate_module->shape_regions);

    for (i=1; i <= size; i++)
    {
		well = (Well*) ListGetPtrToItem(stage_plate_module->shape_regions, i);

        if (well->selected)
        {
            wells[count] = *well;
			count++;
        }	
    }
        
    return count;
}
*/

//stage_plate_module->_wells_subgroup_selected

int stage_plate_get_topleft_and_bottomright_centres_for_selected_wells_on_plate(StagePlateModule* stage_plate_module,
																				StagePlate *plate, POINT *top_left, POINT *bottom_right)
{
	int number_of_wells = plate->rows * plate->cols;

	Well* wells = (Well*) malloc(sizeof(Well) * number_of_wells);

	number_of_wells = stage_plate_get_well_positions (stage_plate_module, wells);
	
	plate_points_set_sorting_params(1, TL_WG_START_LEFT, TL_WG_START_TOP, 0);
	qsort(wells, number_of_wells, sizeof(Well), plate_point_sort); 

	if(number_of_wells < 2)
		return 0;

	top_left->x = (int)wells[0].region.cx;
	top_left->y = (int)wells[0].region.cy;

	bottom_right->x = (int)wells[number_of_wells - 1].region.cx;
	bottom_right->y = (int)wells[number_of_wells - 1].region.cy;

	free(wells);

	return 0;
}


int stage_plate_get_well_positions(StagePlateModule* stage_plate_module, Well *wells)
{
	int count = 0;
	StagePlate plate;

	stage_plate_get_current_plate(stage_plate_module, &plate);

	return stage_plate_get_well_positions_for_plate(stage_plate_module, &plate, wells);

}


PlateDialogResult stage_plate_display_region_selection_dialog(StagePlateModule* stage_plate_module)
{
	int panel_width, panel_height, tmp_pnl, button_pressed_id;
	StagePlate plate;

	// Each time the dialog is displayed select all the wells to select again
//	stage_plate_select_all_wells(stage_plate_module);

	GetPanelAttribute(stage_plate_module->_region_selection_panel, ATTR_WIDTH, &panel_width);

	stage_plate_get_window_height_for_stage_aspect_ratio(stage_plate_module, panel_width, &panel_height);

	SetPanelAttribute(stage_plate_module->_region_selection_panel, ATTR_HEIGHT, panel_height);

	SendMessage(stage_plate_module->_region_selection_hwnd, WM_EXITSIZEMOVE, 0,0);
	ProcessDrawEvents();

	stage_plate_get_current_plate(stage_plate_module, &plate);

	draw_plate_on_canvas (stage_plate_module, stage_plate_module->_region_selection_canvas, &plate);

	stage_plate_module->_region_selection_result = PLATE_SEL_DIALOG_NONE;  // reset this and we can tell if anything changed

//	DisplayPanel (stage_plate_module->_region_selection_panel);  
	InstallPopup (stage_plate_module->_region_selection_panel);

	while(1) {

		GetUserEvent (0, &tmp_pnl, &button_pressed_id);

		if(button_pressed_id == ROI_SELECT_OK_BUTTON) {
			break;
		}

		ProcessSystemEvents();
		ProcessDrawEvents();
	}

//	HidePanel(stage_plate_module->_region_selection_panel);
	RemovePopup(0);
	return stage_plate_module->_region_selection_result;
}