#include "HardWareTypes.h"

#include "OpticalPath.h"
#include "OpticalPathUI.h"
#include "device_list_ui.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "password.h"

#include "toolbox.h"
#include "asynctmr.h"

#include <userint.h>
#include <utility.h>
#include <ansi_c.h> 

#define ENABLE_OPTICAL_PATH_STATUS_POLLING

int send_optical_path_error_text (OpticalPathManager* opm, char fmt[], ...)
{
	int ret;
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(opm), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(opm), message);  
	
	ret = ui_module_send_valist_error(UIMODULE_CAST(opm), "Optical Path manager error", fmt, ap);
	
	va_end(ap);  
	
	return ret;
}


static int default_error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)     
{
	GCI_MessagePopup("OpticalPathManager Error", error_string); 
	
	return UIMODULE_ERROR_NONE;    
}


static void optical_path_manager_read_or_write_main_panel_registry_settings(OpticalPathManager *opm, int write)
{
	ui_module_read_or_write_registry_settings(UIMODULE_CAST(opm), write);   
}


static int OPTICAL_PATH_MANAGER_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (OpticalPathManager*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (OpticalPathManager *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int OPTICAL_PATH_MANAGER_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (OpticalPathManager*, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (OpticalPathManager *) args[0].void_ptr_data, (int) args[1].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int CVICALLBACK OnOpticalPathManagerTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
        {
			OpticalPathManager *opm = (OpticalPathManager *) callbackData;
			int pos;
	
			if(opm->_moving)
				goto Success;
			
			#ifdef TIMER_DEBUG
			printf("Enter Timer Tick -- %s\n", __FUNCTION__);
			#endif

			if (optical_path_get_current_position(opm, &pos) == OPTICAL_PATH_MANAGER_ERROR)
				goto Error;
  
			if(pos != opm->_old_pos) {
				
				GCI_Signal_Emit_From_MainThread(UIMODULE_SIGNAL_TABLE(opm), "OpticalPathManagerChanged", GCI_VOID_POINTER, opm, GCI_INT, pos); 		
				opm->_old_pos = pos;
				
				// Update the ui
				if(optical_path_load_active_paths_into_list_control(opm, opm->_main_ui_panel, OPATH_PNL_TURRET_POS) == OPTICAL_PATH_MANAGER_ERROR)   	
					goto Error;
			}

            break;
		}
    }
    
Success:

	#ifdef TIMER_DEBUG
	printf("Exit Timer Tick -- %s\n", __FUNCTION__);
	#endif

	return 0;

Error:

	#ifdef TIMER_DEBUG
	printf("Exit Timer Tick -- %s\n", __FUNCTION__);
	#endif

	return -1;
}


int optical_path_load_active_paths_into_list_control(OpticalPathManager* opm, int panel, int ctrl)
{
	int pos;
	
	if(optical_path_get_current_position(opm, &pos) == OPTICAL_PATH_MANAGER_ERROR)
		return OPTICAL_PATH_MANAGER_ERROR; 
	
	if(device_conf_load_active_devices_into_list_control(opm->dc, panel, ctrl, pos) == DEVICE_CONF_ERROR) 
		return OPTICAL_PATH_MANAGER_ERROR; 
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_set_list_control_to_pos(OpticalPathManager* opm, int panel, int ctrl, int pos)       
{
	CMDeviceNode* tmp = NULL;
	
	int i, size = ListNumItems(opm->dc->in_use_list);
	
	if(size < 1)
		return OPTICAL_PATH_MANAGER_ERROR;    	
	
	for(i=1; i <= size; i++) {
	
		tmp = ListGetPtrToItem(opm->dc->in_use_list, i);  

		if(tmp->position == pos)
			SetCtrlIndex(panel, ctrl, i-1); 
	}
	
	return OPTICAL_PATH_MANAGER_SUCCESS;    
}


static void OnConfigChanged(ModuleDeviceConfigurator* dc, void *data)
{
	OpticalPathManager* opm = (OpticalPathManager* ) data;

	optical_path_load_active_paths_into_list_control(opm, opm->_main_ui_panel, OPATH_PNL_TURRET_POS);
}


static int optical_path_init (OpticalPathManager* opm)
{
	int device_config_panel_id;
	
	opm->_timer = -1;

	ui_module_set_error_handler(UIMODULE_CAST(opm), default_error_handler, opm);
	
	opm->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(opm), "OpticalPathUI.uir", OPATH_PNL, 1);

    if ( InstallCtrlCallback (opm->_main_ui_panel, OPATH_PNL_SETUP, OnOpticalPathSetup, opm) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
  	
    if ( InstallCtrlCallback (opm->_main_ui_panel, OPATH_PNL_CALIBRATE, OnOpticalPathCalibrate, opm) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
  	
  	if ( InstallCtrlCallback (opm->_main_ui_panel, OPATH_PNL_CLOSE, OnOpticalPathClose, opm) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (opm->_main_ui_panel, OPATH_PNL_TURRET_POS, OnOpticalPathChanged, opm) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
	
//	if ( InstallCtrlCallback (opm->_main_ui_panel, OPATH_PNL_INIT, OnOpticalPathInit, opm) < 0)
//		return OPTICAL_PATH_MANAGER_ERROR;

	opm->_details_ui_panel = ui_module_add_panel(UIMODULE_CAST(opm), "OpticalPathUI.uir", PATH_EDIT, 0);
	
	if ( InstallCtrlCallback (opm->_details_ui_panel, PATH_EDIT_OK_BUTTON, OnOpticalPathAddEditOkClicked , opm) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;   
	
	device_config_panel_id = device_conf_get_panel_id(opm->dc);
	
	if ( InstallCtrlCallback (device_config_panel_id, DLIST_CONF_ADD_BUTTON, OnOpticalPathDetailsAdd, opm) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (device_config_panel_id, DLIST_CONF_EDIT_BUTTON, OnOpticalPathDetailsEdit, opm) < 0)
		return OPTICAL_PATH_MANAGER_ERROR;
		
	ui_module_set_main_panel_title (UIMODULE_CAST(opm));
	
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(opm->dc), "ConfigChanged", OnConfigChanged, opm) == SIGNAL_ERROR) {
		send_optical_path_error_text(opm, "Can not connect signal handler for ConfigChanged signal");
		return OPTICAL_PATH_MANAGER_ERROR;
	}

	if (get_device_double_param_from_ini_file(UIMODULE_GET_NAME(opm), "TimerInterval", &(opm->_timer_interval))<0) 
		opm->_timer_interval=4.0;

	#ifdef SINGLE_THREADED_POLLING
		opm->_timer = NewCtrl(opm->_main_ui_panel, CTRL_TIMER, "", 0, 0);
		
		if ( InstallCtrlCallback (opm->_main_ui_panel, opm->_timer, OnOpticalPathManagerTimerTick, opm) < 0)
			return OPTICAL_PATH_MANAGER_ERROR;
		
		SetCtrlAttribute(opm->_main_ui_panel, opm->_timer, ATTR_INTERVAL, opm->_timer_interval);  
		SetCtrlAttribute(opm->_main_ui_panel, opm->_timer, ATTR_ENABLED, 0);
	#else
		opm->_timer = NewAsyncTimer (opm->_timer_interval, -1, 1, OnOpticalPathManagerTimerTick, opm);
		SetAsyncTimerName(opm->_timer, "OpticalPath");
		SetAsyncTimerAttribute (opm->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif

	
  	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_signal_changed_handler_connect(OpticalPathManager* opm,
	OPTICAL_PATH_MANAGER_CHANGE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(opm), "OpticalPathManagerChanged", handler, callback_data) == SIGNAL_ERROR) {
		return OPTICAL_PATH_MANAGER_ERROR;
	}
	
	return OPTICAL_PATH_MANAGER_SUCCESS; 
}

int optical_path_signal_pre_change_handler_connect(OpticalPathManager* opm,
	OPTICAL_PATH_MANAGER_CHANGE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(opm), "OpticalPathManagerPreChange", handler, callback_data) == SIGNAL_ERROR) {
		return OPTICAL_PATH_MANAGER_ERROR;
	}
	
	return OPTICAL_PATH_MANAGER_SUCCESS; 
}



int optical_path_signal_config_changed_handler_connect(OpticalPathManager* opm,
	OPTICAL_PATH_MANAGER_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(opm->dc), "ConfigChanged", handler, callback_data) == SIGNAL_ERROR) {
		send_optical_path_error_text(opm, "Can not connect signal handler for ConfigChanged signal");
		return OPTICAL_PATH_MANAGER_ERROR;
	}

	return OPTICAL_PATH_MANAGER_SUCCESS;	
}

int optical_path_is_initialised(OpticalPathManager* opm)
{
	return (opm->_initialised && opm->_hw_initialised);	
}


// Converts one node to a string formaty used to save all data as an ini file.
int optical_path_node_to_ini_fmt (ModuleDeviceConfigurator *conf, CMDeviceNode *node, char *buffer)
{
	sprintf(buffer, "Id=%d\nName=%s\nPosition=%d\n\n\n", node->id, node->name, node->position);
		
	return DEVICE_CONF_SUCCESS;  
}


static char* construct_key(char *buffer, const char *section, const char* name)
{
	memset(buffer, 0, 1);
	
	sprintf(buffer, "%s:%s", section, name);
	
	return buffer;
}

int optical_path_ini_fmt_to_node (ModuleDeviceConfigurator *conf, dictionary* ini, int section_number, CMDeviceNode *node)
{
	char *section = NULL, key[200];
	char *str_result = NULL;
		
	section = iniparser_getsecname(ini, section_number);   

	node->id = iniparser_getint(ini, construct_key(key, section, "Id"), -1);       
	
	str_result = iniparser_getstring(ini, construct_key(key, section, "Name"), "Unknown");
	strncpy(node->name, str_result, strlen(str_result) + 1); 
		
	node->position = iniparser_getint(ini, construct_key(key, section, "Position"), -1);

	node->device = NULL;
	
	return DEVICE_CONF_SUCCESS;  
}


int optical_path_hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode)
{
	OpticalPathManager* opm = (OpticalPathManager*)device;
	int pos;
	FILE *fp = NULL;

	dictionary *d = dictionary_new(5);
	
	optical_path_get_current_position(opm, &pos);

	fp = fopen(filepath, mode);
	
	if(fp == NULL)
		return OPTICAL_PATH_MANAGER_ERROR;

	dictionary_set(d, UIMODULE_GET_NAME(opm), NULL);
	dictionary_setint(d, "Position", pos);      

	iniparser_save(d, fp); 
	
	fclose(fp);
	dictionary_del(d);

	return OPTICAL_PATH_MANAGER_SUCCESS;
}

int optical_path_hardware_load_state_from_file (HardwareDevice* device, const char* filepath)
{
	OpticalPathManager* opm = (OpticalPathManager*)device;
	dictionary* d = NULL;
	int pos, file_size, num_devices;    
	char buffer[500] = "";

	if(!FileExists(filepath, &file_size))
		return OPTICAL_PATH_MANAGER_ERROR;	 	
	
	d = iniparser_load(filepath);  

	num_devices = device_conf_get_num_active_devices(opm->dc);

	if(d != NULL) {

		pos = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(opm), "position"), -1); 

		if(pos >= 0 && pos <= num_devices)
			optical_path_move_to_position(opm, pos);
	}

    dictionary_del(d);

	return OPTICAL_PATH_MANAGER_SUCCESS;
}

int optical_path_get_current_path_name(OpticalPathManager* optical_path_manager, char *name)
{
	CMDeviceNode* node = NULL;
	int pos;
	
	if(optical_path_get_current_position(optical_path_manager, &pos) == OPTICAL_PATH_MANAGER_ERROR)
		return OPTICAL_PATH_MANAGER_ERROR;
		
	if((node = device_conf_get_node_at_position(optical_path_manager->dc, pos)) == NULL)
		return OPTICAL_PATH_MANAGER_ERROR;
	
	strncpy(name, node->name, UIMODULE_NAME_LEN);
	
	return OPTICAL_PATH_MANAGER_SUCCESS; 
}


int optical_path_manager_get_current_value_text(HardwareDevice* device, char* info)
{
	if (info==NULL)
		return OPTICAL_PATH_MANAGER_ERROR;

	return (optical_path_get_current_path_name((OpticalPathManager*)device, info));
}

OpticalPathManager* optical_path_manager_new(const char *name, const char *description, const char* data_dir, const char *data_file, size_t size)
{
	OpticalPathManager* opm = (OpticalPathManager*) malloc(size);
	
	opm->_initialised = 0;   
	
	// Ok attach to the add and edit buttons 
	opm->dc = device_conf_new();
	opm->_moving = 0;   
	opm->_requested_pos = 1;
	opm->_old_pos = -1;

	device_conf_set_default_filename(opm->dc, data_file);        
	device_conf_set_max_active_num_devices(opm->dc, 4);
	
	opm->dc->save_node_as_ini_fmt = optical_path_node_to_ini_fmt; 
	opm->dc->read_node_from_ini_fmt = optical_path_ini_fmt_to_node;
		
	OPTICAL_PATH_MANAGER_VTABLE_PTR(opm, hw_init) = NULL;     
	OPTICAL_PATH_MANAGER_VTABLE_PTR(opm, destroy) = NULL; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(opm, move_to_optical_path_position) = NULL; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(opm, get_current_optical_path_position) = NULL; 
	OPTICAL_PATH_MANAGER_VTABLE_PTR(opm, setup_optical_path) = NULL;  
	OPTICAL_PATH_MANAGER_VTABLE_PTR(opm, hide_optical_path_calib) = NULL;  

	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(opm), name);
	ui_module_set_description(UIMODULE_CAST(opm), description);
	ui_module_set_data_dir(UIMODULE_CAST(opm->dc), data_dir);
	
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(opm), hardware_save_state_to_file) = optical_path_hardware_save_state_to_file; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(opm), hardware_load_state_from_file) = optical_path_hardware_load_state_from_file; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(opm), hardware_get_current_value_text) = optical_path_manager_get_current_value_text; 

	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(opm), "OpticalPathManagerChanged", OPTICAL_PATH_MANAGER_PTR_INT_MARSHALLER);        
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(opm), "OpticalPathManagerPreChange", OPTICAL_PATH_MANAGER_PTR_INT_MARSHALLER);  

	optical_path_init (opm);
	
	return opm;
}

void optical_path_stop_timer(OpticalPathManager* opm)
{
	#ifdef SINGLE_THREADED_POLLING   
	SetCtrlAttribute (opm->_main_ui_panel, opm->_timer, ATTR_ENABLED, 0);
	#else
	SetAsyncTimerAttribute (opm->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
}

void optical_path_start_timer(OpticalPathManager* opm)
{
	#ifdef SINGLE_THREADED_POLLING   
	SetCtrlAttribute (opm->_main_ui_panel, opm->_timer, ATTR_ENABLED, 1);
	#else
	SetAsyncTimerAttribute (opm->_timer, ASYNC_ATTR_ENABLED,  1);
	#endif
}

int optical_path_destroy(OpticalPathManager* opm)
{
	CHECK_OPTICAL_PATH_MANAGER_VTABLE_PTR(opm, destroy) 
  	
	CALL_OPTICAL_PATH_MANAGER_VTABLE_PTR(opm, destroy) 

	optical_path_stop_timer(opm);
	
	#ifndef SINGLE_THREADED_POLLING   
	DiscardAsyncTimer(opm->_timer);
	#endif

	device_conf_destroy(opm->dc);      
	
	ui_module_destroy(UIMODULE_CAST(opm));
	
  	free(opm);
  	
  	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_goto_default_position(OpticalPathManager* opm)
{
	int pos=0;
	
	if(device_conf_get_default_position(opm->dc, &pos) == DEVICE_CONF_ERROR)
		return OPTICAL_PATH_MANAGER_ERROR;
	
	if(optical_path_move_to_position(opm, pos) == OPTICAL_PATH_MANAGER_ERROR)
		return OPTICAL_PATH_MANAGER_ERROR;  	

	return OPTICAL_PATH_MANAGER_SUCCESS;
}

int optical_path_get_number_of_positions(OpticalPathManager* opm, int *number_of_positions)
{
	*number_of_positions = device_conf_get_num_active_devices(opm->dc);
	
	return OPTICAL_PATH_MANAGER_SUCCESS;
}


// Virtual Methods


int optical_path_hardware_initialise(OpticalPathManager* opm)
{
	int status = UIMODULE_ERROR_NONE;    
	
	CHECK_OPTICAL_PATH_MANAGER_VTABLE_PTR(opm, hw_init) 

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (opm->vtable.hw_init)(opm, 1) == OPTICAL_PATH_MANAGER_ERROR ) {
			status = send_optical_path_error_text(opm, "optical_path_initialise failed"); 
		
			if(status == UIMODULE_ERROR_IGNORE) {
				opm->_hw_initialised = 0;    
				return OPTICAL_PATH_MANAGER_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);

	opm->_hw_initialised = 1;
	
  	return OPTICAL_PATH_MANAGER_SUCCESS;	
}


int optical_path_initialise(OpticalPathManager* opm, int move_to_default)
{
	int default_pos = 1;
	int status = UIMODULE_ERROR_NONE;    
	
	if(device_conf_load_default_node_data(opm->dc) == DEVICE_CONF_ERROR)
		return OPTICAL_PATH_MANAGER_ERROR;    
	
	if(optical_path_load_active_paths_into_list_control(opm, opm->_main_ui_panel, OPATH_PNL_TURRET_POS) == OPTICAL_PATH_MANAGER_ERROR)   	
		return OPTICAL_PATH_MANAGER_ERROR;  
	
	device_conf_get_default_position(opm->dc, &default_pos);  
	
	if(move_to_default) {
		if(optical_path_move_to_position(opm, default_pos) == OPTICAL_PATH_MANAGER_ERROR)
			return OPTICAL_PATH_MANAGER_ERROR; 
	}
		
	opm->_initialised = 1;
	
  	return OPTICAL_PATH_MANAGER_SUCCESS;	
}

int optical_path_get_current_position(OpticalPathManager* opm, int *position)
{
	int status = UIMODULE_ERROR_NONE;    
	
	CHECK_OPTICAL_PATH_MANAGER_VTABLE_PTR(opm, get_current_optical_path_position) 

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (opm->vtable.get_current_optical_path_position)(opm, position) == OPTICAL_PATH_MANAGER_ERROR ) {
			status = send_optical_path_error_text(opm, "optical_path_get_current_position failed"); 
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return OPTICAL_PATH_MANAGER_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	/*
	if( (opm->vtable.get_current_optical_path_position)(opm, position) == OPTICAL_PATH_MANAGER_ERROR ) {
		send_optical_path_error_text(opm, "optical_path_get_current_position failed");
		
		return OPTICAL_PATH_MANAGER_ERROR;
	}
	*/
	
  	return OPTICAL_PATH_MANAGER_SUCCESS;
}

int optical_path_move_to_position(OpticalPathManager* opm, int position)
{
	int status = UIMODULE_ERROR_NONE;    
	
	if(opm == NULL)
		return OPTICAL_PATH_MANAGER_ERROR;

	#ifdef VERBOSE_DEBUG
		printf("optical_path_move_to_position 1\n");
	#endif

	logger_log(UIMODULE_LOGGER(opm), LOGGER_INFORMATIONAL, "%s move to pos %d", UIMODULE_GET_DESCRIPTION(opm), position);

	CHECK_OPTICAL_PATH_MANAGER_VTABLE_PTR(opm, move_to_optical_path_position)  

	#ifdef VERBOSE_DEBUG
		printf("optical_path_move_to_position 2\n");
	#endif

	GCI_Signal_Emit_From_MainThread(UIMODULE_SIGNAL_TABLE(opm), "OpticalPathManagerPreChange", GCI_VOID_POINTER, opm, GCI_INT, position); 

	#ifdef VERBOSE_DEBUG
		printf("optical_path_move_to_position 3\n");
	#endif

	opm->_moving = 1;
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (opm->vtable.move_to_optical_path_position)(opm, position) == OPTICAL_PATH_MANAGER_ERROR ) {
			status = send_optical_path_error_text(opm, "optical_path_move_to_position failed");               
		
			if(status == UIMODULE_ERROR_IGNORE) {
				opm->_moving = 0; 
				return OPTICAL_PATH_MANAGER_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	#ifdef VERBOSE_DEBUG
		printf("optical_path_move_to_position 4\n");
	#endif

	GCI_Signal_Emit_From_MainThread(UIMODULE_SIGNAL_TABLE(opm), "OpticalPathManagerChanged", GCI_VOID_POINTER, opm, GCI_INT, position); 		      
	
	opm->_moving = 0; 
	
	/*	
	if( (opm->vtable.move_to_optical_path_position)(opm, position) == OPTICAL_PATH_MANAGER_ERROR ) {
		send_optical_path_error_text(opm, "optical_path_move_to_position failed");
		return OPTICAL_PATH_MANAGER_ERROR;
	}
	*/

	SetCtrlVal(opm->_main_ui_panel, OPATH_PNL_TURRET_POS, position);
	
	#ifdef VERBOSE_DEBUG
		printf("optical_path_move_to_position 5\n");
	#endif

  	return OPTICAL_PATH_MANAGER_SUCCESS;
}


int optical_path_manager_display_calib_ui(OpticalPathManager* opm)
{
	int status = UIMODULE_ERROR_NONE;    
	
	CHECK_OPTICAL_PATH_MANAGER_VTABLE_PTR(opm, setup_optical_path)   

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*opm->vtable.setup_optical_path)(opm) == OPTICAL_PATH_MANAGER_ERROR ) {
			status = send_optical_path_error_text(opm, "setup_optical_path failed");              
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return OPTICAL_PATH_MANAGER_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	/*	
	if( (*opm->vtable.setup_optical_path)(opm) == OPTICAL_PATH_MANAGER_ERROR ) {
		send_optical_path_error_text(opm, "setup_optical_path failed");
		return OPTICAL_PATH_MANAGER_ERROR;
	}
	*/
	
  	return OPTICAL_PATH_MANAGER_SUCCESS;
}

int optical_path_manager_hide_calib_ui(OpticalPathManager* opm)
{
	int status = UIMODULE_ERROR_NONE;    
	
	CHECK_OPTICAL_PATH_MANAGER_VTABLE_PTR(opm, hide_optical_path_calib)     

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*opm->vtable.hide_optical_path_calib)(opm) == OPTICAL_PATH_MANAGER_ERROR ) {
			status = send_optical_path_error_text(opm, "hide_optical_path_calib failed");            
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return OPTICAL_PATH_MANAGER_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
		
	/*
	if( (*opm->vtable.hide_optical_path_calib)(opm) == OPTICAL_PATH_MANAGER_ERROR ) {
		send_optical_path_error_text(opm, "hide_optical_path_calib failed");
		return OPTICAL_PATH_MANAGER_ERROR;
	}
	*/
	
  	return OPTICAL_PATH_MANAGER_SUCCESS;
}

