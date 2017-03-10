#include "HardWareTypes.h"

#include "PmtSet.h"
#include "PmtSetUI.h"
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

int send_pmtset_error_text (PmtSet* pmtset, char fmt[], ...)
{
	int ret;
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(pmtset), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(pmtset), message);  
	
	ret = ui_module_send_valist_error(UIMODULE_CAST(pmtset), "Optical Path manager error", fmt, ap);
	
	va_end(ap);  
	
	return ret;
}


static int default_error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)     
{
	GCI_MessagePopup("PmtSet Error", error_string); 
	
	return UIMODULE_ERROR_NONE;    
}


static void pmtset_read_or_write_main_panel_registry_settings(PmtSet *pmtset, int write)
{
	ui_module_read_or_write_registry_settings(UIMODULE_CAST(pmtset), write);   
}


static int PMTSET_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (PmtSet*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (PmtSet *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int PMTSET_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (PmtSet*, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (PmtSet *) args[0].void_ptr_data, (int) args[1].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int CVICALLBACK OnPmtSetTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
        {
			PmtSet *pmtset = (PmtSet *) callbackData;
			int pos;
	
			if(pmtset->_moving)
				goto Success;
			
			#ifdef TIMER_DEBUG
			printf("Enter Timer Tick -- %s\n", __FUNCTION__);
			#endif

			if (pmtset_get_current_position(pmtset, &pos) == PMTSET_ERROR)
				goto Error;
  
			if(pos != pmtset->_old_pos) {
				
				GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(pmtset), "PmtSetChanged", GCI_VOID_POINTER, pmtset, GCI_INT, pos); 		
				pmtset->_old_pos = pos;
				
				// Update the ui
				if(pmtset_load_active_paths_into_list_control(pmtset, pmtset->_main_ui_panel,
					PMT_PNL_PMT_POS) == PMTSET_ERROR)   	
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


int pmtset_load_active_paths_into_list_control(PmtSet* pmtset, int panel, int ctrl)
{
	int pos;
	
	if(pmtset_get_current_position(pmtset, &pos) == PMTSET_ERROR)
		return PMTSET_ERROR; 
	
	if(device_conf_load_active_devices_into_list_control(pmtset->dc, panel, ctrl, pos) == DEVICE_CONF_ERROR) 
		return PMTSET_ERROR; 
	
	return PMTSET_SUCCESS;
}


int pmtset_set_list_control_to_pos(PmtSet* pmtset, int panel, int ctrl, int pos)       
{
	CMDeviceNode* tmp = NULL;
	
	int i, size = ListNumItems(pmtset->dc->in_use_list);
	
	if(size < 1)
		return PMTSET_ERROR;    	
	
	for(i=1; i <= size; i++) {
	
		tmp = ListGetPtrToItem(pmtset->dc->in_use_list, i);  

		if(tmp->position == pos)
			SetCtrlIndex(panel, ctrl, i-1); 
	}
	
	return PMTSET_SUCCESS;    
}


static void OnConfigChanged(ModuleDeviceConfigurator* dc, void *data)
{
	PmtSet* pmtset = (PmtSet* ) data;

	pmtset_load_active_paths_into_list_control(pmtset, pmtset->_main_ui_panel, PMT_PNL_PMT_POS);
}


static int pmtset_init (PmtSet* pmtset)
{
	int device_config_panel_id;
	
	pmtset->_timer = -1;

	ui_module_set_error_handler(UIMODULE_CAST(pmtset), default_error_handler, pmtset);
	
	pmtset->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(pmtset), "PmtSetUI.uir", PMT_PNL, 1);

    if ( InstallCtrlCallback (pmtset->_main_ui_panel, PMT_PNL_SETUP, OnPmtSetSetup, pmtset) < 0)
		return PMTSET_ERROR;
  	
    if ( InstallCtrlCallback (pmtset->_main_ui_panel, PMT_PNL_CALIBRATE, OnPmtSetCalibrate, pmtset) < 0)
		return PMTSET_ERROR;
  	
  	if ( InstallCtrlCallback (pmtset->_main_ui_panel, PMT_PNL_CLOSE, OnPmtSetClose, pmtset) < 0)
		return PMTSET_ERROR;
		
  	if ( InstallCtrlCallback (pmtset->_main_ui_panel, PMT_PNL_PMT_POS, OnPmtSetChanged, pmtset) < 0)
		return PMTSET_ERROR;
	
//	if ( InstallCtrlCallback (pmtset->_main_ui_panel, PMT_PNL_INIT, OnPmtSetInit, pmtset) < 0)
//		return PMTSET_ERROR;

	pmtset->_details_ui_panel = ui_module_add_panel(UIMODULE_CAST(pmtset), "PmtSetUI.uir", PATH_EDIT, 0);
	
	if ( InstallCtrlCallback (pmtset->_details_ui_panel, PATH_EDIT_OK_BUTTON, OnPmtSetAddEditOkClicked , pmtset) < 0)
		return PMTSET_ERROR;   
	
	device_config_panel_id = device_conf_get_panel_id(pmtset->dc);
	
	if ( InstallCtrlCallback (device_config_panel_id, DLIST_CONF_ADD_BUTTON, OnPmtSetDetailsAdd, pmtset) < 0)
		return PMTSET_ERROR;
		
  	if ( InstallCtrlCallback (device_config_panel_id, DLIST_CONF_EDIT_BUTTON, OnPmtSetDetailsEdit, pmtset) < 0)
		return PMTSET_ERROR;
		
	ui_module_set_main_panel_title (UIMODULE_CAST(pmtset));
	
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(pmtset->dc), "ConfigChanged", OnConfigChanged, pmtset) == SIGNAL_ERROR) {
		send_pmtset_error_text(pmtset, "Can not connect signal handler for ConfigChanged signal");
		return PMTSET_ERROR;
	}

	if (get_device_double_param_from_ini_file(UIMODULE_GET_NAME(pmtset), "TimerInterval", &(pmtset->_timer_interval))<0) 
		pmtset->_timer_interval=4.0;

	#ifdef SINGLE_THREADED_POLLING
		pmtset->_timer = NewCtrl(pmtset->_main_ui_panel, CTRL_TIMER, "", 0, 0);
		
		if ( InstallCtrlCallback (pmtset->_main_ui_panel, pmtset->_timer, OnPmtSetTimerTick, pmtset) < 0)
			return PMTSET_ERROR;
		
		SetCtrlAttribute(pmtset->_main_ui_panel, pmtset->_timer, ATTR_INTERVAL, pmtset->_timer_interval);  
		SetCtrlAttribute(pmtset->_main_ui_panel, pmtset->_timer, ATTR_ENABLED, 0);
	#else
		pmtset->_timer = NewAsyncTimer (pmtset->_timer_interval, -1, 1, OnPmtSetTimerTick, pmtset);
		SetAsyncTimerName(pmtset->_timer, "PmtSet");
		SetAsyncTimerAttribute (pmtset->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif

	
  	return PMTSET_SUCCESS;
}


int pmtset_signal_changed_handler_connect(PmtSet* pmtset,
	PMTSET_CHANGE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(pmtset), "PmtSetChanged", handler, callback_data) == SIGNAL_ERROR) {
		return PMTSET_ERROR;
	}
	
	return PMTSET_SUCCESS; 
}

int pmtset_signal_pre_change_handler_connect(PmtSet* pmtset,
	PMTSET_CHANGE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(pmtset), "PmtSetPreChange", handler, callback_data) == SIGNAL_ERROR) {
		return PMTSET_ERROR;
	}
	
	return PMTSET_SUCCESS; 
}



int pmtset_signal_config_changed_handler_connect(PmtSet* pmtset,
	PMTSET_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(pmtset->dc), "ConfigChanged", handler, callback_data) == SIGNAL_ERROR) {
		send_pmtset_error_text(pmtset, "Can not connect signal handler for ConfigChanged signal");
		return PMTSET_ERROR;
	}

	return PMTSET_SUCCESS;	
}

int pmtset_is_initialised(PmtSet* pmtset)
{
	return (pmtset->_initialised && pmtset->_hw_initialised);	
}


// Converts one node to a string formaty used to save all data as an ini file.
int pmtset_node_to_ini_fmt (ModuleDeviceConfigurator *conf, CMDeviceNode *node, char *buffer)
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

int pmtset_ini_fmt_to_node (ModuleDeviceConfigurator *conf, dictionary* ini, int section_number, CMDeviceNode *node)
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

int pmtset_get_current_value_text(HardwareDevice* device, char* info)
{
	PmtSet *pmt = (PmtSet*)device;
	int pos;
	
	if (info==NULL)
		return PMTSET_ERROR;

	pmtset_get_current_position(pmt, &pos);
	pmtset_get_device_name_for_pos(pmt, info, pos);

	return PMTSET_SUCCESS;
}

int pmtset_hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode)
{
	PmtSet *pmt = (PmtSet*)device;
	int pos;
	FILE *fp = NULL;

	dictionary *d = dictionary_new(5);
	
	pmtset_get_current_position(pmt, &pos);

	fp = fopen(filepath, mode);
	
	if(fp == NULL)
		return PMTSET_ERROR;

	dictionary_set(d, UIMODULE_GET_NAME(pmt), NULL);
	dictionary_setint(d, "Position", pos);      

	iniparser_save(d, fp); 
	
	fclose(fp);
	dictionary_del(d);

	return PMTSET_SUCCESS;
}

int pmtset_hardware_load_state_from_file (HardwareDevice* device, const char* filepath)
{
	PmtSet *pmt = (PmtSet*)device;
	dictionary* d = NULL;
	int pos, file_size, num_devices;    
	char buffer[500] = "";

	if(!FileExists(filepath, &file_size))
		return PMTSET_ERROR;	 	
	
	d = iniparser_load(filepath);  

	num_devices = device_conf_get_num_active_devices(pmt->dc);

	if(d != NULL) {

		pos = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(pmt), "position"), -1); 

		if(pos >= 0 && pos <= num_devices)
			pmtset_move_to_position(pmt, pos);
	}

    dictionary_del(d);

	return PMTSET_SUCCESS;
}

PmtSet* pmtset_new(const char *name, const char *description, const char* data_dir, const char *data_file, size_t size)
{
	PmtSet* pmtset = (PmtSet*) malloc(size);
	
	pmtset->_initialised = 0;   
	
	// Ok attach to the add and edit buttons 
	pmtset->dc = device_conf_new();
	pmtset->_moving = 0;   
	pmtset->_requested_pos = 1;
	pmtset->_old_pos = -1;

	device_conf_set_default_filename(pmtset->dc, data_file);        
	device_conf_set_max_active_num_devices(pmtset->dc, 4);
	
	pmtset->dc->save_node_as_ini_fmt = pmtset_node_to_ini_fmt; 
	pmtset->dc->read_node_from_ini_fmt = pmtset_ini_fmt_to_node;
		
	PMTSET_VTABLE_PTR(pmtset, hw_init) = NULL;     
	PMTSET_VTABLE_PTR(pmtset, destroy) = NULL; 
	PMTSET_VTABLE_PTR(pmtset, move_to_pmtset_position) = NULL; 
	PMTSET_VTABLE_PTR(pmtset, get_current_pmtset_position) = NULL; 
	PMTSET_VTABLE_PTR(pmtset, setup_pmtset) = NULL;  
	PMTSET_VTABLE_PTR(pmtset, hide_pmtset_calib) = NULL;  

	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(pmtset), name);
	ui_module_set_description(UIMODULE_CAST(pmtset), description);
	ui_module_set_data_dir(UIMODULE_CAST(pmtset->dc), data_dir);
	
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(pmtset), hardware_get_current_value_text) = pmtset_get_current_value_text; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(pmtset), hardware_save_state_to_file) = pmtset_hardware_save_state_to_file; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(pmtset), hardware_load_state_from_file) = pmtset_hardware_load_state_from_file; 

	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(pmtset), "PmtSetChanged", PMTSET_PTR_INT_MARSHALLER);        
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(pmtset), "PmtSetPreChange", PMTSET_PTR_INT_MARSHALLER);  

	pmtset_init (pmtset);
	
	return pmtset;
}

void pmtset_stop_timer(PmtSet* pmtset)
{
	#ifdef SINGLE_THREADED_POLLING   
	SetCtrlAttribute (pmtset->_main_ui_panel, pmtset->_timer, ATTR_ENABLED, 0);
	#else
	SetAsyncTimerAttribute (pmtset->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
}

void pmtset_start_timer(PmtSet* pmtset)
{
	#ifdef SINGLE_THREADED_POLLING   
	SetCtrlAttribute (pmtset->_main_ui_panel, pmtset->_timer, ATTR_ENABLED, 1);
	#else
	SetAsyncTimerAttribute (pmtset->_timer, ASYNC_ATTR_ENABLED,  1);
	#endif
}

int pmtset_destroy(PmtSet* pmtset)
{
	CHECK_PMTSET_VTABLE_PTR(pmtset, destroy) 
  	
	CALL_PMTSET_VTABLE_PTR(pmtset, destroy) 

	pmtset_stop_timer(pmtset);
	
	#ifndef SINGLE_THREADED_POLLING   
	DiscardAsyncTimer(pmtset->_timer);
	#endif

	device_conf_destroy(pmtset->dc);      
	
	ui_module_destroy(UIMODULE_CAST(pmtset));
	
  	free(pmtset);
  	
  	return PMTSET_SUCCESS;
}


int pmtset_goto_default_position(PmtSet* pmtset)
{
	int pos=0;
	
	if(device_conf_get_default_position(pmtset->dc, &pos) == DEVICE_CONF_ERROR)
		return PMTSET_ERROR;
	
	if(pmtset_move_to_position(pmtset, pos) == PMTSET_ERROR)
		return PMTSET_ERROR;  	

	return PMTSET_SUCCESS;
}

int pmtset_get_number_of_positions(PmtSet* pmtset, int *number_of_positions)
{
	*number_of_positions = device_conf_get_num_active_devices(pmtset->dc);
	
	return PMTSET_SUCCESS;
}


// Virtual Methods


int pmtset_hardware_initialise(PmtSet* pmtset)
{
	int status = UIMODULE_ERROR_NONE;    
	
	CHECK_PMTSET_VTABLE_PTR(pmtset, hw_init) 

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (pmtset->vtable.hw_init)(pmtset, 1) == PMTSET_ERROR ) {
			status = send_pmtset_error_text(pmtset, "pmtset_initialise failed"); 
		
			if(status == UIMODULE_ERROR_IGNORE) {
				pmtset->_hw_initialised = 0;    
				return PMTSET_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);

	pmtset->_hw_initialised = 1;
	
  	return PMTSET_SUCCESS;	
}


int pmtset_initialise(PmtSet* pmtset)
{
	int default_pos = 1;
	int status = UIMODULE_ERROR_NONE;    
	
	if(device_conf_load_default_node_data(pmtset->dc) == DEVICE_CONF_ERROR)
		return PMTSET_ERROR;    
	
	if(pmtset_load_active_paths_into_list_control(pmtset, pmtset->_main_ui_panel, PMT_PNL_PMT_POS) == PMTSET_ERROR)   	
		return PMTSET_ERROR;  
	
	device_conf_get_default_position(pmtset->dc, &default_pos);  
	
	pmtset->_initialised = 1;
	
  	return PMTSET_SUCCESS;	
}

int pmtset_get_current_position(PmtSet* pmtset, int *position)
{
	int status = UIMODULE_ERROR_NONE;    
	
	CHECK_PMTSET_VTABLE_PTR(pmtset, get_current_pmtset_position) 

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (pmtset->vtable.get_current_pmtset_position)(pmtset, position) == PMTSET_ERROR ) {
			status = send_pmtset_error_text(pmtset, "pmtset_get_current_position failed"); 
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return PMTSET_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	/*
	if( (pmtset->vtable.get_current_pmtset_position)(pmtset, position) == PMTSET_ERROR ) {
		send_pmtset_error_text(pmtset, "pmtset_get_current_position failed");
		
		return PMTSET_ERROR;
	}
	*/
	
  	return PMTSET_SUCCESS;
}

int pmtset_get_device_name_for_pos(PmtSet* pmtset, char *name, int pos)
{
	if(device_conf_get_device_name_for_pos(pmtset->dc, name, pos) == DEVICE_CONF_ERROR)
		return PMTSET_ERROR;


	return PMTSET_SUCCESS;
}

int pmtset_move_to_position(PmtSet* pmtset, int position)
{
	int status = UIMODULE_ERROR_NONE;    
	
	logger_log(UIMODULE_LOGGER(pmtset), LOGGER_INFORMATIONAL, "%s move to pos %d", UIMODULE_GET_DESCRIPTION(pmtset), position);

	CHECK_PMTSET_VTABLE_PTR(pmtset, move_to_pmtset_position)  

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(pmtset), "PmtSetPreChange", GCI_VOID_POINTER, pmtset, GCI_INT, position); 

	pmtset->_moving = 1;
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (pmtset->vtable.move_to_pmtset_position)(pmtset, position) == PMTSET_ERROR ) {
			status = send_pmtset_error_text(pmtset, "pmtset_move_to_position failed");               
		
			if(status == UIMODULE_ERROR_IGNORE) {
				pmtset->_moving = 0; 
				return PMTSET_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(pmtset), "PmtSetChanged", GCI_VOID_POINTER, pmtset, GCI_INT, position); 		      
	
	pmtset->_moving = 0; 
	
	/*	
	if( (pmtset->vtable.move_to_pmtset_position)(pmtset, position) == PMTSET_ERROR ) {
		send_pmtset_error_text(pmtset, "pmtset_move_to_position failed");
		return PMTSET_ERROR;
	}
	*/

	SetCtrlVal(pmtset->_main_ui_panel, PMT_PNL_PMT_POS, position);
	
  	return PMTSET_SUCCESS;
}


int pmtset_display_calib_ui(PmtSet* pmtset)
{
	int status = UIMODULE_ERROR_NONE;    
	
	CHECK_PMTSET_VTABLE_PTR(pmtset, setup_pmtset)   

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*pmtset->vtable.setup_pmtset)(pmtset) == PMTSET_ERROR ) {
			status = send_pmtset_error_text(pmtset, "setup_pmtset failed");              
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return PMTSET_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	
	/*	
	if( (*pmtset->vtable.setup_pmtset)(pmtset) == PMTSET_ERROR ) {
		send_pmtset_error_text(pmtset, "setup_pmtset failed");
		return PMTSET_ERROR;
	}
	*/
	
  	return PMTSET_SUCCESS;
}

int pmtset_hide_calib_ui(PmtSet* pmtset)
{
	int status = UIMODULE_ERROR_NONE;    
	
	CHECK_PMTSET_VTABLE_PTR(pmtset, hide_pmtset_calib)     

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (*pmtset->vtable.hide_pmtset_calib)(pmtset) == PMTSET_ERROR ) {
			status = send_pmtset_error_text(pmtset, "hide_pmtset_calib failed");            
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return PMTSET_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
		
	/*
	if( (*pmtset->vtable.hide_pmtset_calib)(pmtset) == PMTSET_ERROR ) {
		send_pmtset_error_text(pmtset, "hide_pmtset_calib failed");
		return PMTSET_ERROR;
	}
	*/
	
  	return PMTSET_SUCCESS;
}

