#include "HardWareTypes.h"

#include "CubeSlider.h"
#include "CubeSliderUI.h"
#include "device_list_ui.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "password.h"

#include "toolbox.h"
#include "asynctmr.h"

#include <userint.h>
#include <utility.h>
#include <ansi_c.h> 

int send_fluocube_error_text (FluoCubeManager* cube_manager, char fmt[], ...)
{
	int ret;
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(cube_manager), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(cube_manager), message);  
	
	ret = ui_module_send_valist_error(UIMODULE_CAST(cube_manager), "Fluorescent cube manager error", fmt, ap);
	
	va_end(ap);  
	
	return ret;
}


static int default_error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)
{
	GCI_MessagePopup("FluoCubeManager Error", error_string);
	
	return UIMODULE_ERROR_NONE;
}


static int CUBE_MANAGER_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (FluoCubeManager*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (FluoCubeManager *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int CUBE_MANAGER_PTR_CUBE_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (FluoCubeManager*, FluoCube *, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (FluoCubeManager *) args[0].void_ptr_data,  (FluoCube *) args[1].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int CUBE_MANAGER_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (FluoCubeManager*, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (FluoCubeManager *) args[0].void_ptr_data,  (int) args[1].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int CVICALLBACK OnCubeTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
        {
			FluoCubeManager *cube_manager = (FluoCubeManager *) callbackData;
			int pos;
	
			if(cube_manager->_prevent_timer_callback == 1)
				return 0;

			#ifdef TIMER_DEBUG
			printf("Enter Timer Tick -- %s\n", __FUNCTION__);
			#endif

			if (cube_manager_get_current_cube_position(cube_manager, &pos) == CUBE_MANAGER_ERROR)
				goto Error;
  
			if(pos != cube_manager->_current_pos) {
				
				GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(cube_manager), "FluoCubeChanged", GCI_VOID_POINTER, cube_manager, GCI_INT, pos); 		
				cube_manager->_current_pos = pos;
			
				// Update the ui
				if(cube_manager_load_active_cubes_into_list_control(cube_manager, cube_manager->_main_ui_panel, CUBE_INFO_TURRET_POS) == CUBE_MANAGER_ERROR)   	
					goto Error;
			}
		
            break;
		}
    }
    
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

int cube_manager_wait_for_stop_moving (FluoCubeManager* cube_manager, double timeout)
{
	double time = Timer();
	int pos, err;
	
	while ((Timer()-time) < timeout)
	{
		err = cube_manager_get_current_cube_position(cube_manager, &pos);

		if(err!=CUBE_MANAGER_ERROR && pos!=cube_manager->_current_pos) 
		{
			cube_manager->_current_pos = pos;
			return 0;
		}
	}
	
	return 0;
}

// Return an array of the active cubes
FluoCube* cube_manager_get_active_cubes(FluoCubeManager* cube_manager)
{
	int size = 0;
	
	return (FluoCube *) device_conf_get_active_device_array(cube_manager->dc, sizeof(FluoCube), &size);  
}


int cube_manager_load_active_cubes_into_list_control(FluoCubeManager* cube_manager, int panel, int ctrl)
{
	int pos;
	
	if(cube_manager_get_current_cube_position(cube_manager, &pos) == CUBE_MANAGER_ERROR)
		return CUBE_MANAGER_ERROR; 
	
	if(device_conf_load_active_devices_into_list_control(cube_manager->dc, panel, ctrl, pos) == DEVICE_CONF_ERROR) 
		return CUBE_MANAGER_ERROR; 
	
	return CUBE_MANAGER_SUCCESS;
}


static void OnConfigChanged(ModuleDeviceConfigurator* dc, void *data)
{
	FluoCubeManager* cube_manager = (FluoCubeManager* ) data;

	cube_manager_load_active_cubes_into_list_control(cube_manager, cube_manager->_main_ui_panel, CUBE_INFO_TURRET_POS);
}


int cube_manager_signal_cube_changed_handler_connect(FluoCubeManager* cube_manager,
	CUBE_MANAGER_CHANGE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(cube_manager), "FluoCubeChanged", handler, callback_data) == SIGNAL_ERROR) {
		return CUBE_MANAGER_ERROR;
	}
	
	return CUBE_MANAGER_SUCCESS; 
}


int cube_manager_signal_cube_config_changed_handler_connect(FluoCubeManager* cube_manager,
	CUBE_MANAGER_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(cube_manager->dc), "ConfigChanged", handler, callback_data) == SIGNAL_ERROR) {
		send_fluocube_error_text(cube_manager, "Can not connect signal handler for ConfigChanged signal");
		return CUBE_MANAGER_ERROR;
	}

	return CUBE_MANAGER_SUCCESS;	
}


int cube_manager_is_initialised(FluoCubeManager* cube_manager)
{
	return (cube_manager->_initialised && cube_manager->_hw_initialised);	
}



// Converts one node to a string formaty used to save all data as an ini file.
int cube_manager_node_to_ini_fmt (ModuleDeviceConfigurator *conf, CMDeviceNode *node, char *buffer)
{
	FluoCube *cube = (FluoCube*) node->device;
	
	sprintf(buffer, "Id=%d\nName=%s\nPosition=%d\nExc-min-nm=%d\nExc-max-nm=%d\nDichroic-nm=%d\nEm-min-nm=%d\nEm-max-nm=%d\n\n\n",
					 node->id, node->name, node->position, cube->exc_min_nm, cube->exc_max_nm, cube->dichroic_nm, cube->emm_min_nm, cube->emm_max_nm);
		
	return DEVICE_CONF_SUCCESS;  
}


static char* construct_key(char *buffer, const char *section, const char* name)
{
	memset(buffer, 0, 1);
	
	sprintf(buffer, "%s:%s", section, name);
	
	return buffer;
}

int cube_manager_ini_fmt_to_node (ModuleDeviceConfigurator *conf, dictionary* ini, int section_number, CMDeviceNode *node)
{
	char *section = NULL, key[200];
	char *str_result = NULL;
		
	FluoCube* cube = node->device = malloc(sizeof(FluoCube));
	
	memset(cube, 0, sizeof(FluoCube));
	
	section = iniparser_getsecname(ini, section_number);   
	
	node->id = iniparser_getint(ini, construct_key(key, section, "Id"), -1);  
	
	if(node->id == -1)
		return DEVICE_CONF_ERROR;  	
	
	str_result = iniparser_getstring(ini, construct_key(key, section, "Name"), "Unknown");
	strncpy(node->name, str_result, strlen(str_result) + 1); 
		
	node->position = iniparser_getint(ini, construct_key(key, section, "Position"), -1);
			
	// Exc-min-NM 
	if((cube->exc_min_nm = iniparser_getint(ini, construct_key(key, section, "Exc-min-nm"), -1)) < 0)
		return DEVICE_CONF_ERROR;  
			
	// Exc-max-NM 
	if((cube->exc_max_nm = iniparser_getint(ini, construct_key(key, section, "Exc-max-nm"), -1)) < 0)
		return DEVICE_CONF_ERROR;  
		
	// Dichroic-NM  
	if((cube->dichroic_nm = iniparser_getint(ini, construct_key(key, section, "Dichroic-nm"), -1)) < 0)
		return DEVICE_CONF_ERROR;    
		
	// Em-min-NM
	if((cube->emm_min_nm = iniparser_getint(ini, construct_key(key, section, "Em-min-nm"), -1)) < 0)
		return DEVICE_CONF_ERROR;    
		
	// Em-max-NM  
	if((cube->emm_max_nm = iniparser_getint(ini, construct_key(key, section, "Em-max-nm"), -1)) < 0)
		return DEVICE_CONF_ERROR;   
		
	cube = NULL;
	
	return DEVICE_CONF_SUCCESS;  
}

int cube_manager_get_current_value_text(HardwareDevice* device, char* info)
{
	FluoCube item;
	
	if (info==NULL)
		return CUBE_MANAGER_ERROR;
	
	cube_manager_get_current_cube((FluoCubeManager*)device, &item);
	
	if (item.name!=NULL){
		strcpy(info, item.name);
		return CUBE_MANAGER_SUCCESS;
	}

	return CUBE_MANAGER_ERROR;
}


int cube_manager_hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*)device;
	int pos;
	FILE *fp = NULL;

	dictionary *d = dictionary_new(5);
	
	cube_manager_get_current_cube_position(cube_manager, &pos);

	fp = fopen(filepath, mode);
	
	if(fp == NULL)
		return CUBE_MANAGER_ERROR;

	dictionary_set(d, UIMODULE_GET_NAME(cube_manager), NULL);
	dictionary_setint(d, "Position", pos);      

	iniparser_save(d, fp); 
	
	fclose(fp);
	dictionary_del(d);

	return CUBE_MANAGER_SUCCESS;
}

int cube_manager_hardware_load_state_from_file (HardwareDevice* device, const char* filepath)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*)device;
	dictionary* d = NULL;
	int pos, file_size, num_devices;    
	char buffer[500] = "";

	if(!FileExists(filepath, &file_size))
		return CUBE_MANAGER_ERROR;	 	
	
	d = iniparser_load(filepath);  

	num_devices = device_conf_get_num_active_devices(cube_manager->dc);

	if(d != NULL) {

		pos = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(cube_manager), "position"), -1); 

		if(pos >= 0 && pos <= num_devices)
			cube_manager_move_to_position(cube_manager, pos);
	}

    dictionary_del(d);

	return CUBE_MANAGER_SUCCESS;
}

FluoCubeManager* cube_manager_new(const char *name, const char *description, const char *data_dir, const char *data_file, size_t size)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*) malloc(size);

    memset(cube_manager, 0, size);
    
	ui_module_set_error_handler(UIMODULE_CAST(cube_manager), default_error_handler, cube_manager);  
	
	cube_manager->_move_pos_thread_id = -1;
	cube_manager->_prevent_timer_callback = 0;
	cube_manager->_requested_pos = -1;
	cube_manager->_current_pos = -1;  
	cube_manager->_initialised = 0;   
	cube_manager->_hw_initialised = 0;  
	
	// Ok attach to the add and edit buttons 
	cube_manager->dc = device_conf_new();
	
	ui_module_set_data_dir(UIMODULE_CAST(cube_manager->dc), data_dir);
	device_conf_set_default_filename(cube_manager->dc, data_file);        
	device_conf_set_max_active_num_devices(cube_manager->dc, 10);
	
	cube_manager->dc->save_node_as_ini_fmt = cube_manager_node_to_ini_fmt; 
	cube_manager->dc->read_node_from_ini_fmt = cube_manager_ini_fmt_to_node;
		
	CUBE_MANAGER_VTABLE_PTR(cube_manager, hardware_init) = NULL;
	CUBE_MANAGER_VTABLE_PTR(cube_manager, initialise) = NULL;   
	CUBE_MANAGER_VTABLE_PTR(cube_manager, destroy) = NULL; 
	CUBE_MANAGER_VTABLE_PTR(cube_manager, move_to_cube_position) = NULL; 
	CUBE_MANAGER_VTABLE_PTR(cube_manager, get_current_cube_position) = NULL; 
	
	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(cube_manager), name);
	ui_module_set_description(UIMODULE_CAST(cube_manager), description);
	
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(cube_manager), hardware_get_current_value_text) = cube_manager_get_current_value_text; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(cube_manager), hardware_save_state_to_file) = cube_manager_hardware_save_state_to_file; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(cube_manager), hardware_load_state_from_file) = cube_manager_hardware_load_state_from_file; 

	return cube_manager;
}


static int cube_manager_is_timer_enabled(FluoCubeManager* cube_manager)
{
	int enabled = 0;
	
	#ifdef SINGLE_THREADED_POLLING  
	GetCtrlAttribute (cube_manager->_main_ui_panel, cube_manager->_timer, ATTR_ENABLED, &enabled);   
	#else
	GetAsyncTimerAttribute (cube_manager->_timer, ASYNC_ATTR_ENABLED,  &enabled);
	#endif	
	
	return enabled;
}


void cube_manager_stop_timer(FluoCubeManager* cube_manager)
{
	#ifdef SINGLE_THREADED_POLLING  
	SetCtrlAttribute (cube_manager->_main_ui_panel, cube_manager->_timer, ATTR_ENABLED, 0);  
	#else
	SetAsyncTimerAttribute (cube_manager->_timer, ASYNC_ATTR_ENABLED,  0); 
	#endif	
}

void cube_manager_start_timer(FluoCubeManager* cube_manager)
{
	#ifdef SINGLE_THREADED_POLLING
	SetCtrlAttribute (cube_manager->_main_ui_panel, cube_manager->_timer, ATTR_ENABLED, 1);
	#else
	SetAsyncTimerAttribute (cube_manager->_timer, ASYNC_ATTR_ENABLED,  1);    
	#endif	
}

int cube_manager_destroy(FluoCubeManager* cube_manager)
{
	cube_manager_stop_timer(cube_manager);

	CHECK_CUBE_MANAGER_VTABLE_PTR(cube_manager, destroy) 
  	
	CALL_CUBE_MANAGER_VTABLE_PTR(cube_manager, destroy) 

	device_conf_destroy(cube_manager->dc);      
	
	ui_module_destroy(UIMODULE_CAST(cube_manager));
	
  	free(cube_manager);
  	
  	return CUBE_MANAGER_SUCCESS;
}


int cube_manager_goto_default_position(FluoCubeManager* cube_manager)
{
	int pos=0;
	
	if(device_conf_get_default_position(cube_manager->dc, &pos) == DEVICE_CONF_ERROR)
		return CUBE_MANAGER_ERROR;
	
	if(cube_manager_move_to_position(cube_manager, pos) == CUBE_MANAGER_ERROR)
		return CUBE_MANAGER_ERROR;  	

	return CUBE_MANAGER_SUCCESS;
}

int cube_manager_get_number_of_cubes(FluoCubeManager* cube_manager, int *number_of_cubes)
{
	*number_of_cubes = device_conf_get_num_active_devices(cube_manager->dc);
	
	return CUBE_MANAGER_SUCCESS;
}



// Virtual Methods

int cube_manager_hardware_initialise(FluoCubeManager* cube_manager)
{
	int status = UIMODULE_ERROR_NONE;

	CHECK_CUBE_MANAGER_VTABLE_PTR(cube_manager, hardware_init) 
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (cube_manager->vtable.hardware_init)(cube_manager) == CUBE_MANAGER_ERROR ) {
			status = send_fluocube_error_text(cube_manager, "cube_manager_hardware_initialisefailed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return CUBE_MANAGER_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);

	cube_manager->_hw_initialised = 1;  
	
  	return CUBE_MANAGER_SUCCESS;	
}


int cube_manager_initialise(FluoCubeManager* cube_manager, int move_to_default)
{
	int device_config_panel_id;
	
	cube_manager->_timer = -1;

	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(cube_manager), "FluoCubeChanged", CUBE_MANAGER_PTR_INT_MARSHALLER); 
	
	cube_manager->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(cube_manager), "CubeSliderUI.uir", CUBE_INFO, 1);   

    if ( InstallCtrlCallback (cube_manager->_main_ui_panel, CUBE_INFO_SETUP, OnCubeSetup, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
  	
  	if ( InstallCtrlCallback (cube_manager->_main_ui_panel, CUBE_INFO_CLOSE, OnFluorCubeClose, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (cube_manager->_main_ui_panel, CUBE_INFO_TURRET_POS, OnCubeChanged, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
	
	cube_manager->_details_ui_panel = ui_module_add_panel(UIMODULE_CAST(cube_manager), "CubeSliderUI.uir", EDIT_PANEL, 0);
	
	if ( InstallCtrlCallback (cube_manager->_details_ui_panel, EDIT_PANEL_OK_BUTTON, OnCubeAddEditOkClicked, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;	
	
	device_config_panel_id = device_conf_get_panel_id(cube_manager->dc);
	
	if ( InstallCtrlCallback (device_config_panel_id, DLIST_CONF_ADD_BUTTON, OnCubeDetailsAdd, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
	
  	if ( InstallCtrlCallback (device_config_panel_id, DLIST_CONF_EDIT_BUTTON, OnCubeDetailsEdit, cube_manager) < 0)
		return CUBE_MANAGER_ERROR;
		
	ui_module_set_main_panel_title (UIMODULE_CAST(cube_manager));
	
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(cube_manager->dc), "ConfigChanged", OnConfigChanged, cube_manager) == SIGNAL_ERROR) {
		send_fluocube_error_text(cube_manager, "Can not connect signal handler for ConfigChanged signal");
		return CUBE_MANAGER_ERROR;
	}

	if (get_device_double_param_from_ini_file(UIMODULE_GET_NAME(cube_manager), "TimerInterval", &(cube_manager->_timer_interval))<0) 
		cube_manager->_timer_interval=2.0;

	#ifdef SINGLE_THREADED_POLLING
		cube_manager->_timer = NewCtrl(cube_manager->_main_ui_panel, CTRL_TIMER, "", 0, 0);
		
		if ( InstallCtrlCallback (cube_manager->_main_ui_panel, cube_manager->_timer, OnCubeTimerTick, cube_manager) < 0)
			return CUBE_MANAGER_ERROR;
		
		SetCtrlAttribute(cube_manager->_main_ui_panel, cube_manager->_timer, ATTR_INTERVAL, cube_manager->_timer_interval);  
		SetCtrlAttribute(cube_manager->_main_ui_panel, cube_manager->_timer, ATTR_ENABLED, 0);
	#else
		cube_manager->_timer = NewAsyncTimer (cube_manager->_timer_interval, -1, 1, OnCubeTimerTick, cube_manager);
		SetAsyncTimerName(cube_manager->_timer, "CubeSlider");
		SetAsyncTimerAttribute (cube_manager->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
		
	if(device_conf_load_default_node_data(cube_manager->dc) == DEVICE_CONF_ERROR)
		return CUBE_MANAGER_ERROR; 
	
	// Update the ui
	if(cube_manager_load_active_cubes_into_list_control(cube_manager, cube_manager->_main_ui_panel, CUBE_INFO_TURRET_POS) == CUBE_MANAGER_ERROR)   		
		return CUBE_MANAGER_ERROR;
	
	// Call specific device initialisation if necessary.
	if( (cube_manager->vtable.initialise != NULL)) {
		
		if( (cube_manager->vtable.initialise)(cube_manager) == CUBE_MANAGER_ERROR )
			return CUBE_MANAGER_ERROR;  	
	}
		
	if(move_to_default) {
		
		if(cube_manager_goto_default_position(cube_manager) == CUBE_MANAGER_ERROR)
			return CUBE_MANAGER_ERROR;
	}
	
	cube_manager->_initialised = 1;  
	
	return CUBE_MANAGER_SUCCESS;	
}


int cube_manager_get_current_cube_position(FluoCubeManager* cube_manager, int *position)
{
	int status = UIMODULE_ERROR_NONE;

	CHECK_CUBE_MANAGER_VTABLE_PTR(cube_manager, get_current_cube_position) 

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (cube_manager->vtable.get_current_cube_position)(cube_manager, position) == CUBE_MANAGER_ERROR ) {
			status = send_fluocube_error_text(cube_manager, "cube_manager_get_current_cube_position failed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return CUBE_MANAGER_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	

  	return CUBE_MANAGER_SUCCESS;
}

static int CVICALLBACK check_cube_moved_correctly(void *callback)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*) callback;
	int pos=-1;
	double start_time = Timer();
	
	while((Timer() - start_time) < 10.0) {

		if(cube_manager->_abort_cube_move_check > 0)
			goto Completed;

		Delay(2.0);

		if (cube_manager_get_current_cube_position(cube_manager, &pos) == CUBE_MANAGER_SUCCESS) {
		
			if(pos == cube_manager->_requested_pos)
				goto Completed;
		}
	}

	cube_manager->_prevent_timer_callback = 0;           
	cube_manager->_abort_cube_move_check = 0;
	ui_module_send_error(UIMODULE_CAST(cube_manager), "Cube Error", "Cube slider failed to move to requested position within timeout");

	return CUBE_MANAGER_ERROR;

Completed:

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(cube_manager), "FluoCubeChanged", GCI_VOID_POINTER, cube_manager, GCI_INT, pos); 		
	cube_manager->_current_pos = pos;
				
	cube_manager_load_active_cubes_into_list_control(cube_manager, cube_manager->_main_ui_panel, CUBE_INFO_TURRET_POS);
	
	cube_manager->_prevent_timer_callback = 0;
	cube_manager->_abort_cube_move_check = 0;
	
	return CUBE_MANAGER_SUCCESS;
}

static int abort_cube_check_thread_completion(FluoCubeManager* cube_manager)
{
	int thread_status;
	double start_time = Timer();
	cube_manager->_abort_cube_move_check = 1;

	CmtGetThreadPoolFunctionAttribute (gci_thread_pool(), cube_manager->_move_pos_thread_id,
			ATTR_TP_FUNCTION_EXECUTION_STATUS, &thread_status);

	// Thread no longer executing. Must be shutdown time.
	if(thread_status >= kCmtThreadFunctionPostExecution) {
		cube_manager->_abort_cube_move_check = 0;
		return CUBE_MANAGER_SUCCESS;
	}

	while(cube_manager->_abort_cube_move_check == 1) {

		if((Timer() - start_time) > 10.0) {
			logger_log(UIMODULE_LOGGER(cube_manager), LOGGER_ERROR, "The cube manager failed to abort thread");
			cube_manager->_abort_cube_move_check = 0;
			return CUBE_MANAGER_ERROR;
		}

		if(thread_status >= kCmtThreadFunctionPostExecution) {
			cube_manager->_abort_cube_move_check = 0;
			return CUBE_MANAGER_SUCCESS;
		}

		Delay(0.1);
		ProcessSystemEvents();
	}

	cube_manager->_abort_cube_move_check = 0;

	return CUBE_MANAGER_SUCCESS;
}

int cube_manager_move_to_position(FluoCubeManager* cube_manager, int position)
{
	int status = UIMODULE_ERROR_NONE;
	double start_time = Timer();

	logger_log(UIMODULE_LOGGER(cube_manager), LOGGER_INFORMATIONAL, "%s move to pos %d", UIMODULE_GET_DESCRIPTION(cube_manager), position);
	
	CHECK_CUBE_MANAGER_VTABLE_PTR(cube_manager, move_to_cube_position) 

	cube_manager->_requested_pos = position;

	// Don't read until position has changed
	cube_manager->_prevent_timer_callback = 1;           
		
	do {
		status = UIMODULE_ERROR_NONE;
		if( (cube_manager->vtable.move_to_cube_position)(cube_manager, position) == CUBE_MANAGER_ERROR ) {
			status = send_fluocube_error_text(cube_manager, "cube_manager_move_to_position failed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				goto CUBE_ERROR;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
  
	if( cube_manager->_move_pos_thread_id > 0)
		abort_cube_check_thread_completion(cube_manager);

	CmtScheduleThreadPoolFunction (gci_thread_pool(), check_cube_moved_correctly, cube_manager, &cube_manager->_move_pos_thread_id);
	CmtWaitForThreadPoolFunctionCompletion(gci_thread_pool(),  cube_manager->_move_pos_thread_id, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	CmtReleaseThreadPoolFunctionID(gci_thread_pool(),  cube_manager->_move_pos_thread_id);
	cube_manager->_move_pos_thread_id = -1;

	cube_manager->_prevent_timer_callback = 0;  

  	return CUBE_MANAGER_SUCCESS;
	
	CUBE_ERROR:
	
		cube_manager->_prevent_timer_callback = 0;  
		
		return CUBE_MANAGER_ERROR;    
}

int cube_manager_get_current_cube(FluoCubeManager* cube_manager, FluoCube *cube)
{
	int pos;
	CMDeviceNode* node = NULL;
	FluoCube *tmp;
	
	if(cube_manager_get_current_cube_position(cube_manager, &pos) == CUBE_MANAGER_ERROR)
		return CUBE_MANAGER_ERROR; 
		
	if((node = device_conf_get_node_at_position(cube_manager->dc, pos)) == NULL)
		return CUBE_MANAGER_ERROR; 
	
	tmp = (FluoCube *) node->device;
	
	memcpy(cube, tmp, sizeof(FluoCube));
	strcpy(cube->name, node->name);
	cube->position = node->position;
	
	return CUBE_MANAGER_SUCCESS; 
}

double cube_manager_get_average_emmision(FluoCube cube)
{
	return cube.emm_min_nm + (cube.emm_max_nm - cube.emm_min_nm) / 2.0;
}
