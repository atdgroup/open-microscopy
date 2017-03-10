#include "optical_calibration_uir.h"
#include "optical_calibration.h"
#include "OpticalCalibrationDevice.h"

#include "gci_utils.h"
#include "string_utils.h"

#include "microscope.h"

#include <utility.h>
#include <ansi_c.h>
#include <userint.h>
#include "toolbox.h"

static int get_objective_calibration_factor_from_tree_for_id(optical_calibration* cal, int id, float *factor)
{
	int i, tmp_id, number_of_rows;
	char factor_str[50] = "";

	*factor = 1.0;

	GetNumListItems (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CAL_TREE, &number_of_rows);  
	
    for(i=0; i < number_of_rows; i++) {
  		
		if(GetValueFromIndex (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CAL_TREE, i, &tmp_id) < 0)
			return OBJ_CALIBRATION_ERROR;   
		
		if(tmp_id == id) {

			if(GetTreeCellAttribute (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CAL_TREE, i, 1, ATTR_LABEL_TEXT, factor_str))
				return OBJ_CALIBRATION_ERROR;   
		
			sscanf(factor_str, "%f", factor);

			break;
		}
	}

	return OBJ_CALIBRATION_SUCCESS;
}

static void OnObjectiveConfigChanged(ObjectiveManager* manager, void *data)
{
	optical_calibration* cal = (optical_calibration*) data;

	optical_calibration_read_data(cal);        
}


int optical_calibration_write_data(optical_calibration* cal)
{
	ListType objectives_list;
	CMDeviceNode *node = NULL;
	float factor = 1.0;
	char camera_name[50] = "";
	int i, number_of_objectives = 0;

	objectives_list = device_conf_get_defined_devices_list(cal->objective_manager->dc);    

	number_of_objectives = objective_manager_get_number_of_defined_objectives(cal->objective_manager);   
	
    for(i=1; i <= number_of_objectives; i++) {
	
		node = ListGetPtrToItem(objectives_list, i); 
	
		get_objective_calibration_factor_from_tree_for_id(cal, node->id, &factor);

		objective_set_calibration_for_objective_id(cal->objective_manager, node->id,
					OPTICAL_CALIBRATION_GET_DEFAULT_DEVICE_NAME, factor);
	}

	cal->changed = 0;

	device_conf_save_node_data_for_module(cal->objective_manager->dc);

	return OBJ_CALIBRATION_SUCCESS;  
}

ObjectiveManager*  optical_calibration_get_objective_manager(optical_calibration* cal)
{
	return cal->objective_manager;
}

int  optical_calibration_get_selected_index(optical_calibration* cal)
{
	return cal->selected_index;
}

// This should go through each of the ACTIVE defined objectives as find the calibration for each.
// Or set the cal factor to one if it is not yet calibrated.
int optical_calibration_read_data(optical_calibration* cal) 
{
	int i, number_of_objectives, current_objective_pos, count = 0, current_position_present = 0, index;
	char factor_str[200];
	double factor = 0.0;
	CMDeviceNode *node = NULL;  
	ListType objectives_list;
	
	number_of_objectives = objective_manager_get_number_of_defined_objectives(cal->objective_manager);   
	
	// Clear the control
	if(ClearListCtrl (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CAL_TREE) < 0)
		return OBJ_CALIBRATION_ERROR;
	
	objective_manager_get_current_position(cal->objective_manager,&current_objective_pos); 
	
	objectives_list = device_conf_get_defined_devices_list(cal->objective_manager->dc);    
	
    for(i=1, count=1; i <= number_of_objectives; i++) {
	
		node = ListGetPtrToItem(objectives_list, i); 
		
		// Node not active.
		if(node->position == -1)
			continue;
		
		index = InsertTreeItem (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CAL_TREE, VAL_SIBLING, 0, VAL_LAST, node->name, 0, 0, node->id);

		objective_get_calibration_for_objective_id(cal->objective_manager, node->id,
			OPTICAL_CALIBRATION_GET_DEFAULT_DEVICE_NAME, &factor);

		sprintf(factor_str, "%f", factor);
		
		if(SetTreeCellAttribute (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CAL_TREE, index, 1, ATTR_LABEL_TEXT, factor_str))
			goto CAL_ERROR; 
		
		if(node->position == current_objective_pos)
			current_position_present = count;
		
		count++;
		
	}
	
	// Highlight the row of the selected objective.	
	if(current_position_present) {
		SetActiveCtrl(UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CAL_TREE);
		SetActiveTreeItem (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CAL_TREE, current_position_present - 1, VAL_REPLACE_SELECTION_WITH_ITEM); 
	}
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(cal), "CalibrationChanged", GCI_VOID_POINTER, cal);  
	
    return OBJ_CALIBRATION_SUCCESS;
	
	CAL_ERROR:
		
		return OBJ_CALIBRATION_ERROR;   
}

static int VOID_OPTICAL_CALIBRATION_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (optical_calibration *, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (optical_calibration *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

static void on_imagewindow_profile(GCIWindow *window, const Point p1, const Point p2, void *callback_data)
{
	unsigned int pixel_len;
	optical_calibration* cal = (optical_calibration*) callback_data;
	
	pixel_len = (unsigned int) floor(sqrt(pow(abs(p1.x - p2.x), 2.0) + pow(abs(p1.y - p2.y), 2.0)) + 0.5);
	
	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_PIXEL_NUMERIC, pixel_len);
}

static int on_optical_calibration_display_setup(optical_calibration* cal)
{
	optical_calibration_read_data(cal);    
	
	optical_calibration_device_setup_device_for_calibration (optical_calibration_device_get_default_device(), on_imagewindow_profile, cal);

	cal->window = optical_calibration_device_get_calibration_window (optical_calibration_device_get_default_device());

	return OBJ_CALIBRATION_SUCCESS;
}

int optical_calibration_display_panel(optical_calibration* cal)
{
	on_optical_calibration_display_setup(cal);
	
	ui_module_display_panel(UIMODULE_CAST(cal), UIMODULE_MAIN_PANEL_ID(cal));
	
	return OBJ_CALIBRATION_SUCCESS;
}

void OnOpticalCalMainPanelDisplayed (UIModule *module, void *data)
{
	optical_calibration* cal = (optical_calibration*) data;
	
	on_optical_calibration_display_setup(cal);      	
}

void OpticalCalibrationDevicePreSetAsDefault (OpticalCalibrationDevice* device, void *data)
{
	// This is called just before the default calibration device is changed.
	optical_calibration* cal = (optical_calibration*) data;

	if(!ui_module_main_panel_is_visible(UIMODULE_CAST(cal)))
		return;

	optical_calibration_device_deinitilise_device_for_calibration(optical_calibration_device_get_default_device());
}


void OpticalCalibrationDeviceSetAsDefault (OpticalCalibrationDevice* device, void *data)
{
	optical_calibration* cal = (optical_calibration*) data;

	if(!ui_module_main_panel_is_visible(UIMODULE_CAST(cal)))
		return;

	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_ACTIVE_DEVICE_TXT, OPTICAL_CALIBRATION_GET_DEFAULT_DEVICE_NAME);

	// Display the optical configuration devices ui
	ui_module_display_main_panel(UIMODULE_CAST(device));

	on_optical_calibration_display_setup(cal);
}

optical_calibration* optical_calibration_new(Microscope *ms)
{
	optical_calibration *cal = (optical_calibration *) malloc (sizeof(optical_calibration));		
	
	int i=0, device_index = 0, number_of_devices = 0;

	char data_dir[GCI_MAX_PATHNAME_LEN] = "";
	char temp_dir[GCI_MAX_PATHNAME_LEN] = "";
	char default_filename[GCI_MAX_PATHNAME_LEN] = "";
	
	cal->window = NULL;
	cal->changed = 0;
	cal->ms = ms;
    cal->objective_manager = microscope_get_objective(ms);
	
	if(cal->objective_manager == NULL)
		return NULL;

	ui_module_constructor(UIMODULE_CAST(cal), "OpticalCalibration");  

	cal->_objective_manager_config_changed_signal_id = objective_manager_signal_config_changed_handler_connect(cal->objective_manager,  OnObjectiveConfigChanged, cal); 
	
	UIMODULE_MAIN_PANEL_ID(cal) = ui_module_add_panel(UIMODULE_CAST(cal), "optical_calibration_uir.uir", CAL_PANEL, 1);
	
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(cal), "CalibrationChanged", VOID_OPTICAL_CALIBRATION_PTR_MARSHALLER);      
	
	if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CANCEL_BUTTON, OnCancelClicked, cal) < 0)
		return NULL;	
	
	if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_UM_NUMERIC, OnNumericChanged, cal) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_PIXEL_NUMERIC, OnNumericChanged, cal) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_OK_BUTTON, OnOkClicked, cal) < 0)
		return NULL;	
	
	if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_SET_BUTTON, OnCalibrationSet, cal) < 0)
		return NULL;	
	
	if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_SAVE_BUTTON, OnSaveClicked, cal) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CAL_TREE, OnCalTreeEvent, cal) < 0)
		return NULL;
	
	//if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CALIBRATION_DEV_RING, OnCalibrationDeviceChanged, cal) < 0)
	//	return NULL;

	if(ui_module_main_panel_show_mainpanel_handler_connect (UIMODULE_CAST(cal), OnOpticalCalMainPanelDisplayed, cal) == UI_MODULE_ERROR)
		return NULL;

	/*
	// Insert into calibration device ring
	ClearListCtrl(UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CALIBRATION_DEV_RING);

	number_of_devices = optical_calibration_device_number_of_devices();

	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_ACTIVE_DEVICE_TXT, OPTICAL_CALIBRATION_GET_DEFAULT_DEVICE_NAME);
	
	for(i=0; i < number_of_devices; i++) {

		OpticalCalibrationDevice* device = optical_calibration_device_at_index(i);

		//InsertListItem (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CALIBRATION_DEV_RING, -1, UIMODULE_GET_NAME(device), 0);  

		optical_calibration_device_pre_set_as_default_handler(device, OpticalCalibrationDevicePreSetAsDefault, cal);
		optical_calibration_device_set_as_default_handler(device, OpticalCalibrationDeviceSetAsDefault, cal);
	}
*/

	/*
	for(i=0; i < number_of_devices; i++) {
		
		OpticalCalibrationDevice* device = optical_calibration_device_at_index(i);

		if(strcmp(UIMODULE_GET_NAME(device), OPTICAL_CALIBRATION_GET_DEFAULT_DEVICE_NAME) == 0) {
			SetCtrlIndex(UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CALIBRATION_DEV_RING, 0);
			cal->device = device;
			break;
		}
	}
	*/

	return cal;
}


int optical_calibration_initialise(optical_calibration* cal)
{
	optical_calibration_read_data(cal);        
	
	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_ACTIVE_DEVICE_TXT, OPTICAL_CALIBRATION_GET_DEFAULT_DEVICE_NAME);

	return OBJ_CALIBRATION_SUCCESS; 
}

int optical_calibration_signal_calibration_changed_handler_connect (optical_calibration* cal,
	OPTICAL_CALIBRATION_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(cal), "CalibrationChanged", handler, callback_data) == SIGNAL_ERROR) {
		
		ui_module_send_error(UIMODULE_CAST(cal), "Optical Calibration Error", "%s",
			"Can not connect signal handler for calibration changed signal");
		
		return OBJ_CALIBRATION_ERROR;
	}

	return OBJ_CALIBRATION_SUCCESS;
}


void optical_calibration_hide(optical_calibration* cal)
{
	optical_calibration_device_deinitilise_device_for_calibration (optical_calibration_device_get_default_device());

	ui_module_hide_all_panels(UIMODULE_CAST(cal)); 
}


void optical_calibration_destroy(optical_calibration* cal)
{
	optical_calibration_device_deinitilise_device_for_calibration (optical_calibration_device_get_default_device());
		
	if (cal->_objective_manager_config_changed_signal_id >= 0)
		objective_manager_signal_config_changed_handler_disconnect(cal->objective_manager, cal->_objective_manager_config_changed_signal_id);

	ui_module_destroy(UIMODULE_CAST(cal));
	
	free(cal);
}

int optical_calibration_get_calibration_factor_for_current_objective(optical_calibration* cal, double *factor)
{
	Objective obj;
	OpticalCalibrationDevice* default_device = NULL;

	*factor = 0.0;

	if(objective_manager_get_current_objective(cal->objective_manager, &obj) == OBJECTIVE_MANAGER_ERROR)
		return OBJ_CALIBRATION_ERROR;;

	if(objective_get_calibration(&obj, OPTICAL_CALIBRATION_GET_DEFAULT_DEVICE_NAME, factor) == OBJECTIVE_MANAGER_ERROR)
		return OBJ_CALIBRATION_ERROR;
	
    return OBJ_CALIBRATION_SUCCESS;
}
