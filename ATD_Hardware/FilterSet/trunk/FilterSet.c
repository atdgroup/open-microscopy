#include "HardWareTypes.h"

#include "FilterSet.h"
#include "FilterSetUI.h"
#include "device_list_ui.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "password.h"

#include "toolbox.h"
#include "asynctmr.h"

#include <userint.h>
#include <utility.h>
#include <ansi_c.h> 

int send_fluofilter_error_text (FilterSetCollection* filterset, char fmt[], ...)
{
	int ret;
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(filterset), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(filterset), message);  
	
	ret = ui_module_send_valist_error(UIMODULE_CAST(filterset), "Fluorescent filter manager error", fmt, ap);
	
	va_end(ap);  
	
	return ret;
}


static int default_error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)
{
	GCI_MessagePopup("FilterSetCollection Error", error_string);
	
	return UIMODULE_ERROR_NONE;
}


static int FILTERSET_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (FilterSetCollection*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (FilterSetCollection *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int FILTERSET_PTR_FILTERSET_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (FilterSetCollection*, FilterSet *, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (FilterSetCollection *) args[0].void_ptr_data,  (FilterSet *) args[1].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int FILTERSET_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (FilterSetCollection*, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (FilterSetCollection *) args[0].void_ptr_data,  (int) args[1].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int CVICALLBACK OnFilterTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
        {
			FilterSetCollection *filterset = (FilterSetCollection *) callbackData;
			int pos;
	
			if(filterset->_prevent_timer_callback == 1)
				return 0;

			#ifdef TIMER_DEBUG
			printf("Enter Timer Tick -- %s\n", __FUNCTION__);
			#endif

			if (filterset_get_current_filter_position(filterset, &pos) == FILTERSET_ERROR)
				goto Error;
  
			if(pos != filterset->_current_pos) {
				
				GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(filterset), "FilterChanged", GCI_VOID_POINTER, filterset, GCI_INT, pos); 		
				filterset->_current_pos = pos;
			
				// Update the ui
				if(filterset_load_active_filters_into_list_control(filterset,
					filterset->_main_ui_panel, FILTER_PNL_FILTER_POS) == FILTERSET_ERROR) {

					goto Error;
				}
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

int filterset_wait_for_stop_moving (FilterSetCollection* filterset, double timeout)
{
	double time = Timer();
	int pos, err;
	
	while ((Timer()-time) < timeout)
	{
		err = filterset_get_current_filter_position(filterset, &pos);

		if(err!=FILTERSET_ERROR && pos!=filterset->_current_pos) 
		{
			filterset->_current_pos = pos;
			return 0;
		}
	}
	
	return 0;
}

// Return an array of the active filters
FilterSet* filterset_get_active_filters(FilterSetCollection* filterset)
{
	int i, size = 0;
	FilterSet * device_node = NULL;
	FilterSet * set = NULL;
	CMDeviceNode *tmp = NULL;
	ListType list;

	size = device_conf_get_num_active_devices(DEVICE_CONF_CAST(filterset->dc));

	if(size < 1)
		return DEVICE_CONF_ERROR; 	

	list = device_conf_get_devices_in_use_list(DEVICE_CONF_CAST(filterset->dc));

	set = malloc(sizeof(FilterSet) * size);

	for(i=0; i < size; i++) {
	
		tmp = ListGetPtrToItem(list, i);  
		device_node = (FilterSet*) tmp->device;

		strcpy(set[i].name, tmp->name);
		set[i].position = tmp->position;
		strcpy(set[i].exc_name, device_node->exc_name);
		strcpy(set[i].emm_name, device_node->emm_name);
		strcpy(set[i].dic_name, device_node->dic_name);
		set[i].exc_min_nm = device_node->exc_min_nm;
		set[i].exc_max_nm = device_node->exc_max_nm;
		set[i].emm_min_nm = device_node->emm_min_nm;
		set[i].emm_max_nm = device_node->emm_max_nm;
		set[i].dichroic_nm = device_node->dichroic_nm;
	}

	return set;
}


int filterset_load_active_filters_into_list_control(FilterSetCollection* filterset, int panel, int ctrl)
{
	int pos;
	
	if(filterset_get_current_filter_position(filterset, &pos) == FILTERSET_ERROR)
		return FILTERSET_ERROR; 
	
	if(device_conf_load_active_devices_into_list_control(filterset->dc, panel, ctrl, pos) == DEVICE_CONF_ERROR) 
		return FILTERSET_ERROR; 
	
	return FILTERSET_SUCCESS;
}


static void OnConfigChanged(ModuleDeviceConfigurator* dc, void *data)
{
	FilterSetCollection* filterset = (FilterSetCollection* ) data;

	filterset_load_active_filters_into_list_control(filterset, filterset->_main_ui_panel, FILTER_PNL_FILTER_POS);
}


int filterset_signal_filter_changed_handler_connect(FilterSetCollection* filterset,
	FILTERSET_CHANGE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(filterset), "FilterChanged", handler, callback_data) == SIGNAL_ERROR) {
		return FILTERSET_ERROR;
	}
	
	return FILTERSET_SUCCESS; 
}


int filterset_signal_filter_config_changed_handler_connect(FilterSetCollection* filterset,
	FILTERSET_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(filterset->dc), "ConfigChanged", handler, callback_data) == SIGNAL_ERROR) {
		send_fluofilter_error_text(filterset, "Can not connect signal handler for ConfigChanged signal");
		return FILTERSET_ERROR;
	}

	return FILTERSET_SUCCESS;	
}


int filterset_is_initialised(FilterSetCollection* filterset)
{
	return (filterset->_initialised && filterset->_hw_initialised);	
}



// Converts one node to a string formaty used to save all data as an ini file.
int filterset_node_to_ini_fmt (ModuleDeviceConfigurator *conf, CMDeviceNode *node, char *buffer)
{
	FilterSet *filter = (FilterSet*) node->device;
	
	sprintf(buffer, "Id=%d\nName=%s\nPosition=%d\nExc-name=%s\nExc-min-nm=%d\nExc-max-nm=%d\n"
					"Dic-Name=%s\nDichroic-nm=%d\nEm-Name=%s\nEm-min-nm=%d\nEm-max-nm=%d\n\n\n",
					 node->id, node->name, node->position, filter->exc_name, filter->exc_min_nm,
					 filter->exc_max_nm, filter->dic_name, filter->dichroic_nm, filter->emm_name,
					 filter->emm_min_nm, filter->emm_max_nm);
		
	return DEVICE_CONF_SUCCESS;  
}


static char* construct_key(char *buffer, const char *section, const char* name)
{
	memset(buffer, 0, 1);
	
	sprintf(buffer, "%s:%s", section, name);
	
	return buffer;
}

int filterset_ini_fmt_to_node (ModuleDeviceConfigurator *conf, dictionary* ini, int section_number, CMDeviceNode *node)
{
	char *section = NULL, key[200];
	char *str_result = NULL;
		
	FilterSet* filter = node->device = malloc(sizeof(FilterSet));
	
	memset(filter, 0, sizeof(FilterSet));
	
	section = iniparser_getsecname(ini, section_number);   
	
	node->id = iniparser_getint(ini, construct_key(key, section, "Id"), -1);  
	
	if(node->id == -1)
		return DEVICE_CONF_ERROR;  	
	
	str_result = iniparser_getstring(ini, construct_key(key, section, "Name"), "Unknown");
	strncpy(node->name, str_result, strlen(str_result) + 1); 
		
	node->position = iniparser_getint(ini, construct_key(key, section, "Position"), -1);
	
	// Exc-Name
	str_result = iniparser_getstring(ini, construct_key(key, section, "Exc-Name"), "Unknown");
	strncpy(filter->exc_name, str_result, strlen(str_result) + 1); 
	
	// Exc-min-NM 
	if((filter->exc_min_nm = iniparser_getint(ini, construct_key(key, section, "Exc-min-nm"), -1)) < 0)
		return DEVICE_CONF_ERROR;  
			
	// Exc-max-NM 
	if((filter->exc_max_nm = iniparser_getint(ini, construct_key(key, section, "Exc-max-nm"), -1)) < 0)
		return DEVICE_CONF_ERROR;  
		
	// Dic-Name
	str_result = iniparser_getstring(ini, construct_key(key, section, "Dic-Name"), "Unknown");
	strncpy(filter->dic_name, str_result, strlen(str_result) + 1); 

	// Dichroic-NM  
	if((filter->dichroic_nm = iniparser_getint(ini, construct_key(key, section, "Dichroic-nm"), -1)) < 0)
		return DEVICE_CONF_ERROR;    
		
	// Emm-Name
	str_result = iniparser_getstring(ini, construct_key(key, section, "Em-Name"), "Unknown");
	strncpy(filter->emm_name, str_result, strlen(str_result) + 1); 

	// Em-min-NM
	if((filter->emm_min_nm = iniparser_getint(ini, construct_key(key, section, "Em-min-nm"), -1)) < 0)
		return DEVICE_CONF_ERROR;    
		
	// Em-max-NM  
	if((filter->emm_max_nm = iniparser_getint(ini, construct_key(key, section, "Em-max-nm"), -1)) < 0)
		return DEVICE_CONF_ERROR;   
		
	filter = NULL;
	
	return DEVICE_CONF_SUCCESS;  
}

int filterset_get_current_value_text(HardwareDevice* device, char* info)
{
	FilterSet item;
	
	if (info==NULL)
		return HARDWARE_ERROR;

	if(filterset_get_current_filterset((FilterSetCollection*)device, &item) == FILTERSET_ERROR)
		return HARDWARE_ERROR;
	
	if (item.name!=NULL){
		strncpy(info, item.name, UIMODULE_NAME_LEN);
		return HARDWARE_SUCCESS;
	}

	return HARDWARE_ERROR;
}


int filterset_hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode)
{
	FilterSetCollection* filterset = (FilterSetCollection*)device;
	int pos;
	FILE *fp = NULL;

	dictionary *d = dictionary_new(5);
	
	filterset_get_current_filter_position(filterset, &pos);

	fp = fopen(filepath, mode);
	
	if(fp == NULL)
		return FILTERSET_ERROR;

	dictionary_set(d, UIMODULE_GET_NAME(filterset), NULL);
	dictionary_setint(d, "Position", pos);      

	iniparser_save(d, fp); 
	
	fclose(fp);
	dictionary_del(d);

	return FILTERSET_SUCCESS;
}

int filterset_hardware_load_state_from_file (HardwareDevice* device, const char* filepath)
{
	FilterSetCollection* filterset = (FilterSetCollection*)device;
	dictionary* d = NULL;
	int pos, file_size, num_devices;    
	char buffer[500] = "";

	if(!FileExists(filepath, &file_size))
		return FILTERSET_ERROR;	 	
	
	d = iniparser_load(filepath);  

	num_devices = device_conf_get_num_active_devices(filterset->dc);

	if(d != NULL) {

		pos = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(filterset), "position"), -1); 

		if(pos >= 0 && pos <= num_devices)
			filterset_move_to_position(filterset, pos);
	}

    dictionary_del(d);

	return FILTERSET_SUCCESS;
}

FilterSetCollection* filterset_new(const char *name, const char *description, const char *data_dir, const char *data_file, size_t size)
{
	FilterSetCollection* filterset = (FilterSetCollection*) malloc(size);

	ui_module_set_error_handler(UIMODULE_CAST(filterset), default_error_handler, filterset);  
	
	filterset->_move_pos_thread_id = -1;
	filterset->_prevent_timer_callback = 0;
	filterset->_requested_pos = -1;
	filterset->_current_pos = -1;  
	filterset->_initialised = 0;   
	filterset->_hw_initialised = 0;  
	
	// Ok attach to the add and edit buttons 
	filterset->dc = device_conf_new();
	
	ui_module_set_data_dir(UIMODULE_CAST(filterset->dc), data_dir);
	device_conf_set_default_filename(filterset->dc, data_file);        
	device_conf_set_max_active_num_devices(filterset->dc, 10);
	
	filterset->dc->save_node_as_ini_fmt = filterset_node_to_ini_fmt; 
	filterset->dc->read_node_from_ini_fmt = filterset_ini_fmt_to_node;
		
	FILTERSET_VTABLE_PTR(filterset, hardware_init) = NULL;
	FILTERSET_VTABLE_PTR(filterset, initialise) = NULL;   
	FILTERSET_VTABLE_PTR(filterset, destroy) = NULL; 
	FILTERSET_VTABLE_PTR(filterset, move_to_filter_position) = NULL; 
	FILTERSET_VTABLE_PTR(filterset, get_current_filter_position) = NULL; 
	
	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(filterset), name);
	ui_module_set_description(UIMODULE_CAST(filterset), description);
	
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(filterset), hardware_get_current_value_text) = filterset_get_current_value_text; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(filterset), hardware_save_state_to_file) = filterset_hardware_save_state_to_file; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(filterset), hardware_load_state_from_file) = filterset_hardware_load_state_from_file; 

	return filterset;
}


static int filterset_is_timer_enabled(FilterSetCollection* filterset)
{
	int enabled = 0;
	
	#ifdef SINGLE_THREADED_POLLING  
	GetCtrlAttribute (filterset->_main_ui_panel, filterset->_timer, ATTR_ENABLED, &enabled);   
	#else
	GetAsyncTimerAttribute (filterset->_timer, ASYNC_ATTR_ENABLED,  &enabled);
	#endif	
	
	return enabled;
}


void filterset_stop_timer(FilterSetCollection* filterset)
{
	#ifdef SINGLE_THREADED_POLLING  
	SetCtrlAttribute (filterset->_main_ui_panel, filterset->_timer, ATTR_ENABLED, 0);  
	#else
	SetAsyncTimerAttribute (filterset->_timer, ASYNC_ATTR_ENABLED,  0); 
	#endif	
}

void filterset_start_timer(FilterSetCollection* filterset)
{
	#ifdef SINGLE_THREADED_POLLING
	SetCtrlAttribute (filterset->_main_ui_panel, filterset->_timer, ATTR_ENABLED, 1);
	#else
	SetAsyncTimerAttribute (filterset->_timer, ASYNC_ATTR_ENABLED,  1);    
	#endif	
}

int filterset_destroy(FilterSetCollection* filterset)
{
	filterset_stop_timer(filterset);

	CHECK_FILTERSET_VTABLE_PTR(filterset, destroy) 
  	
	CALL_FILTERSET_VTABLE_PTR(filterset, destroy) 

	device_conf_destroy(filterset->dc);      
	
	ui_module_destroy(UIMODULE_CAST(filterset));
	
  	free(filterset);
  	
  	return FILTERSET_SUCCESS;
}


int filterset_goto_default_position(FilterSetCollection* filterset)
{
	int pos=0;
	
	if(device_conf_get_default_position(filterset->dc, &pos) == DEVICE_CONF_ERROR)
		return FILTERSET_ERROR;
	
	if(filterset_move_to_position(filterset, pos) == FILTERSET_ERROR)
		return FILTERSET_ERROR;  	

	return FILTERSET_SUCCESS;
}

int filterset_get_number_of_filters(FilterSetCollection* filterset, int *number_of_filters)
{
	*number_of_filters = device_conf_get_num_active_devices(filterset->dc);
	
	return FILTERSET_SUCCESS;
}



// Virtual Methods

int filterset_hardware_initialise(FilterSetCollection* filterset)
{
	int status = UIMODULE_ERROR_NONE;

	CHECK_FILTERSET_VTABLE_PTR(filterset, hardware_init) 
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (filterset->vtable.hardware_init)(filterset) == FILTERSET_ERROR ) {
			status = send_fluofilter_error_text(filterset, "filterset_hardware_initialisefailed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return FILTERSET_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);

	filterset->_hw_initialised = 1;  
	
  	return FILTERSET_SUCCESS;	
}


int filterset_initialise(FilterSetCollection* filterset)
{
	int device_config_panel_id;
	
	filterset->_timer = -1;

	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(filterset), "FilterChanged", FILTERSET_PTR_INT_MARSHALLER); 
	
	filterset->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(filterset), "FilterSetUI.uir", FILTER_PNL, 1);   

    if ( InstallCtrlCallback (filterset->_main_ui_panel, FILTER_PNL_SETUP, OnFilterSetup, filterset) < 0)
		return FILTERSET_ERROR;
  	
  	if ( InstallCtrlCallback (filterset->_main_ui_panel, FILTER_PNL_CLOSE, OnFilterClose, filterset) < 0)
		return FILTERSET_ERROR;
		
  	if ( InstallCtrlCallback (filterset->_main_ui_panel, FILTER_PNL_FILTER_POS, OnFilterChanged, filterset) < 0)
		return FILTERSET_ERROR;
	
	filterset->_details_ui_panel = ui_module_add_panel(UIMODULE_CAST(filterset), "FilterSetUI.uir", EDIT_PANEL, 0);
	
	if ( InstallCtrlCallback (filterset->_details_ui_panel, EDIT_PANEL_OK_BUTTON, OnFilterAddEditOkClicked, filterset) < 0)
		return FILTERSET_ERROR;	
	
	device_config_panel_id = device_conf_get_panel_id(filterset->dc);
	
	if ( InstallCtrlCallback (device_config_panel_id, DLIST_CONF_ADD_BUTTON, OnFilterDetailsAdd, filterset) < 0)
		return FILTERSET_ERROR;
	
  	if ( InstallCtrlCallback (device_config_panel_id, DLIST_CONF_EDIT_BUTTON, OnFilterDetailsEdit, filterset) < 0)
		return FILTERSET_ERROR;
		
	ui_module_set_main_panel_title (UIMODULE_CAST(filterset));
	
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(filterset->dc), "ConfigChanged", OnConfigChanged, filterset) == SIGNAL_ERROR) {
		send_fluofilter_error_text(filterset, "Can not connect signal handler for ConfigChanged signal");
		return FILTERSET_ERROR;
	}

	if (get_device_double_param_from_ini_file(UIMODULE_GET_NAME(filterset), "TimerInterval", &(filterset->_timer_interval))<0) 
		filterset->_timer_interval=2.0;

	#ifdef SINGLE_THREADED_POLLING
		filterset->_timer = NewCtrl(filterset->_main_ui_panel, CTRL_TIMER, "", 0, 0);
		
		if ( InstallCtrlCallback (filterset->_main_ui_panel, filterset->_timer, OnFilterTimerTick, filterset) < 0)
			return FILTERSET_ERROR;
		
		SetCtrlAttribute(filterset->_main_ui_panel, filterset->_timer, ATTR_INTERVAL, filterset->_timer_interval);  
		SetCtrlAttribute(filterset->_main_ui_panel, filterset->_timer, ATTR_ENABLED, 0);
	#else
		filterset->_timer = NewAsyncTimer (filterset->_timer_interval, -1, 1, OnFilterTimerTick, filterset);
		SetAsyncTimerName(filterset->_timer, "FilterSetSlider");
		SetAsyncTimerAttribute (filterset->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
		
	if(device_conf_load_default_node_data(filterset->dc) == DEVICE_CONF_ERROR)
		return FILTERSET_ERROR; 
	
	// Update the ui
	if(filterset_load_active_filters_into_list_control(filterset, filterset->_main_ui_panel,
		FILTER_PNL_FILTER_POS) == FILTERSET_ERROR)  { 		
		return FILTERSET_ERROR;
	}
	
	// Call specific device initialisation if necessary.
	if( (filterset->vtable.initialise != NULL)) {
		
		if( (filterset->vtable.initialise)(filterset) == FILTERSET_ERROR )
			return FILTERSET_ERROR;  	
	}
	
	filterset->_initialised = 1;  
	
	return FILTERSET_SUCCESS;	
}


int filterset_get_current_filter_position(FilterSetCollection* filterset, int *position)
{
	int status = UIMODULE_ERROR_NONE;

	CHECK_FILTERSET_VTABLE_PTR(filterset, get_current_filter_position) 

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (filterset->vtable.get_current_filter_position)(filterset, position) == FILTERSET_ERROR ) {
			status = send_fluofilter_error_text(filterset, "filterset_get_current_filter_position failed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return FILTERSET_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
	

  	return FILTERSET_SUCCESS;
}

static int CVICALLBACK check_filter_moved_correctly(void *callback)
{
	FilterSetCollection* filterset = (FilterSetCollection*) callback;
	int pos=-1;
	double start_time = Timer();
	
	while((Timer() - start_time) < 10.0) {

		if(filterset->_abort_filter_move_check > 0)
			goto Completed;

		Delay(2.0);

		if (filterset_get_current_filter_position(filterset, &pos) == FILTERSET_SUCCESS) {
		
			if(pos == filterset->_requested_pos)
				goto Completed;
		}
	}

	filterset->_prevent_timer_callback = 0;           
	filterset->_abort_filter_move_check = 0;
	ui_module_send_error(UIMODULE_CAST(filterset), "Filterset Error", "Filterset failed to move to requested position within timeout");

	return FILTERSET_ERROR;

Completed:

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(filterset), "FilterChanged", GCI_VOID_POINTER, filterset, GCI_INT, pos); 		
	filterset->_current_pos = pos;
				
	filterset_load_active_filters_into_list_control(filterset, filterset->_main_ui_panel, FILTER_PNL_FILTER_POS);
	
	filterset->_prevent_timer_callback = 0;
	filterset->_abort_filter_move_check = 0;
	
	return FILTERSET_SUCCESS;
}

static int abort_filter_check_thread_completion(FilterSetCollection* filterset)
{
	int thread_status;
	double start_time = Timer();
	filterset->_abort_filter_move_check = 1;

	CmtGetThreadPoolFunctionAttribute (gci_thread_pool(), filterset->_move_pos_thread_id,
			ATTR_TP_FUNCTION_EXECUTION_STATUS, &thread_status);

	// Thread no longer executing. Must be shutdown time.
	if(thread_status >= kCmtThreadFunctionPostExecution) {
		filterset->_abort_filter_move_check = 0;
		return FILTERSET_SUCCESS;
	}

	while(filterset->_abort_filter_move_check == 1) {

		if((Timer() - start_time) > 10.0) {
			logger_log(UIMODULE_LOGGER(filterset), LOGGER_ERROR, "The filter manager failed to abort thread");
			filterset->_abort_filter_move_check = 0;
			return FILTERSET_ERROR;
		}

		if(thread_status >= kCmtThreadFunctionPostExecution) {
			filterset->_abort_filter_move_check = 0;
			return FILTERSET_SUCCESS;
		}

		Delay(0.1);
		ProcessSystemEvents();
	}

	filterset->_abort_filter_move_check = 0;

	return FILTERSET_SUCCESS;
}

int filterset_move_to_position(FilterSetCollection* filterset, int position)
{
	int status = UIMODULE_ERROR_NONE;
	double start_time = Timer();

	logger_log(UIMODULE_LOGGER(filterset), LOGGER_INFORMATIONAL, "%s move to pos %d", UIMODULE_GET_DESCRIPTION(filterset), position);
	
	CHECK_FILTERSET_VTABLE_PTR(filterset, move_to_filter_position) 

	filterset->_requested_pos = position;

	// Don't read until position has changed
	filterset->_prevent_timer_callback = 1;           
		
	do {
		status = UIMODULE_ERROR_NONE;
		if( (filterset->vtable.move_to_filter_position)(filterset, position) == FILTERSET_ERROR ) {
			status = send_fluofilter_error_text(filterset, "filterset_move_to_position failed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				goto ErrorLabel;    
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);
  
	if( filterset->_move_pos_thread_id > 0)
		abort_filter_check_thread_completion(filterset);

	CmtScheduleThreadPoolFunction (gci_thread_pool(), check_filter_moved_correctly, filterset, &filterset->_move_pos_thread_id);
	CmtWaitForThreadPoolFunctionCompletion(gci_thread_pool(),  filterset->_move_pos_thread_id, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	CmtReleaseThreadPoolFunctionID(gci_thread_pool(),  filterset->_move_pos_thread_id);
	filterset->_move_pos_thread_id = -1;

	filterset->_prevent_timer_callback = 0;  

  	return FILTERSET_SUCCESS;
	
	ErrorLabel:
	
		filterset->_prevent_timer_callback = 0;  
		
		return FILTERSET_ERROR;    
}

int filterset_get_current_filterset(FilterSetCollection* filterset, FilterSet *filter)
{
	int pos;
	CMDeviceNode* node = NULL;
	FilterSet *tmp;
	
	if(filterset_get_current_filter_position(filterset, &pos) == FILTERSET_ERROR)
		return FILTERSET_ERROR; 
		
	if((node = device_conf_get_node_at_position(filterset->dc, pos)) == NULL)
		return FILTERSET_ERROR; 
	
	tmp = (FilterSet *) node->device;
	
	memcpy(filter, tmp, sizeof(FilterSet));
	strcpy(filter->name, node->name);
	filter->position = node->position;
	
	strcpy(filter->exc_name, tmp->exc_name);
	strcpy(filter->dic_name, tmp->dic_name);
	strcpy(filter->emm_name, tmp->emm_name);
	
	return FILTERSET_SUCCESS; 
}

double filterset_get_average_emmision(FilterSet filter)
{
	return filter.emm_min_nm + (filter.emm_max_nm - filter.emm_min_nm) / 2.0;
}
