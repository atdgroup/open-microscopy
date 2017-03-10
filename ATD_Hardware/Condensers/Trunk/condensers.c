#include "condensers.h"
#include "condensers_ui.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "password.h"

#include "device_list.h"
#include "device_list_ui.h"

#include "toolbox.h"
#include "asynctmr.h"

#include <userint.h>
#include <utility.h>
#include <ansi_c.h> 

int send_condenser_error_text (CondenserManager* condenser_manager, char fmt[], ...)
{
	int ret;
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(condenser_manager), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(condenser_manager), message);  
	
	ret = ui_module_send_valist_error(UIMODULE_CAST(condenser_manager), "Condenser manager error", fmt, ap);
	
	va_end(ap);  
	
	return ret;
}


static void error_handler (char *error_string, CondenserManager *condenser_manager)
{
	GCI_MessagePopup("CondenserManager Error", error_string); 
}

static int CONDENSER_MANAGER_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (CondenserManager*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (CondenserManager *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int CONDENSER_MANAGER_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (CondenserManager*, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (CondenserManager *) args[0].void_ptr_data, (int) args[1].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int CONDENSER_MANAGER_PTR_CONDENSER_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (CondenserManager*, Condenser *, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (CondenserManager *) args[0].void_ptr_data,  (Condenser *) args[1].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int CVICALLBACK OnCondenserTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	CondenserManager *condenser_manager = (CondenserManager *) callbackData;
	
    switch (event)
    {
        case EVENT_TIMER_TICK:
        {
        	CondenserManager *condenser_manager = (CondenserManager *) callbackData;
			int pos;
	
			if(condenser_manager->_prevent_timer_callback == 1)
				return 0;

			if (condenser_manager_get_current_condenser_position(condenser_manager, &pos) == CONDENSER_MANAGER_ERROR)
				return -1;
  
			if(pos != condenser_manager->_current_pos) {
				
				GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(condenser_manager), "CondenserChanged", GCI_VOID_POINTER, condenser_manager, GCI_INT, pos); 		
				condenser_manager->_current_pos = pos;
			
				// Update the ui
				if(condenser_manager_load_active_condensers_into_list_control(condenser_manager, condenser_manager->_main_ui_panel, CONDENSER_TURRET_POS) == CONDENSER_MANAGER_ERROR)   	
					return -1;
			}
        	
            break;
        }
    }
    
    return 0;
}

static int CVICALLBACK sort_by_turret(void *item1, void *item2)
{
	Condenser *condenser1 = (Condenser *) item1;
	Condenser *condenser2 = (Condenser *) item2;
	
	return condenser1->position - condenser2->position;
}

int condenser_manager_load_active_condensers_into_list_control(CondenserManager* condenser_manager, int panel, int ctrl)
{
	int pos;
	
	if(condenser_manager_get_current_condenser_position(condenser_manager, &pos) == CONDENSER_MANAGER_ERROR)
		return CONDENSER_MANAGER_ERROR; 
	
	if(device_conf_load_active_devices_into_list_control(condenser_manager->dc, panel, ctrl, pos) == DEVICE_CONF_ERROR) 
		return CONDENSER_MANAGER_ERROR; 
	
	return CONDENSER_MANAGER_SUCCESS;
}

static void OnConfigChanged(ModuleDeviceConfigurator* dc, void *data)
{
	CondenserManager* condenser_manager = (CondenserManager* ) data;

	condenser_manager_load_active_condensers_into_list_control(condenser_manager,
		condenser_manager->_main_ui_panel, CONDENSER_TURRET_POS);
}

static void OnPanelsClosedOrHidden (UIModule *module, void *data)
{
	CondenserManager* condenser_manager = (CondenserManager*) data; 
 
	condenser_manager_stop_timer(condenser_manager);
}

static void OnPanelsDisplayed (UIModule *module, int panel_id, void *data)
{
	CondenserManager* condenser_manager = (CondenserManager*) data; 

	condenser_manager_start_timer(condenser_manager);
}

int condenser_manager_initialise (CondenserManager* condenser_manager)
{
	int device_config_panel_id;
	
	condenser_manager->_timer = -1;

	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(condenser_manager), "CondenserChanged", CONDENSER_MANAGER_PTR_INT_MARSHALLER); 

	condenser_manager->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(condenser_manager), "condensers_ui.uir", CONDENSER, 1);   

    if ( InstallCtrlCallback (condenser_manager->_main_ui_panel, CONDENSER_SETUP, OnCondenserSetup, condenser_manager) < 0)
		return CONDENSER_MANAGER_ERROR;
  	
  	if ( InstallCtrlCallback (condenser_manager->_main_ui_panel, CONDENSER_CLOSE, OnCondenserClose, condenser_manager) < 0)
		return CONDENSER_MANAGER_ERROR;
		
  	if ( InstallCtrlCallback (condenser_manager->_main_ui_panel, CONDENSER_TURRET_POS, OnCondenserChanged, condenser_manager) < 0)
		return CONDENSER_MANAGER_ERROR;
		

	condenser_manager->_details_ui_panel = ui_module_add_panel(UIMODULE_CAST(condenser_manager), "condensers_ui.uir", EDIT_PANEL, 0);   

	if ( InstallCtrlCallback (condenser_manager->_details_ui_panel, EDIT_PANEL_OK_BUTTON, OnCondenserAddEditOkClicked, condenser_manager) < 0)
		return CONDENSER_MANAGER_ERROR;	
	
	device_config_panel_id = device_conf_get_panel_id(condenser_manager->dc);
	
	if ( InstallCtrlCallback (device_config_panel_id, DLIST_CONF_ADD_BUTTON, OnCondenserDetailsAdd, condenser_manager) < 0)
		return CONDENSER_MANAGER_ERROR;
	
  	if ( InstallCtrlCallback (device_config_panel_id, DLIST_CONF_EDIT_BUTTON, OnCondenserDetailsEdit, condenser_manager) < 0)
		return CONDENSER_MANAGER_ERROR;
		
	ui_module_set_main_panel_title (UIMODULE_CAST(condenser_manager));
	
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(condenser_manager->dc), "ConfigChanged", OnConfigChanged, condenser_manager) == SIGNAL_ERROR) {
		send_condenser_error_text(condenser_manager, "Can not connect signal handler for ConfigChanged signal");
		return CONDENSER_MANAGER_ERROR;
	}

	if (get_device_double_param_from_ini_file(UIMODULE_GET_NAME(condenser_manager), "TimerInterval", &(condenser_manager->_timer_interval))<0) 
		condenser_manager->_timer_interval=2.0;

	#ifdef SINGLE_THREADED_POLLING
		condenser_manager->_timer = NewCtrl(condenser_manager->_main_ui_panel, CTRL_TIMER, "", 0, 0);
		
		if ( InstallCtrlCallback (condenser_manager->_main_ui_panel, condenser_manager->_timer, OnCondenserTimerTick, condenser_manager) < 0)
			return CONDENSER_MANAGER_ERROR;
		
		SetCtrlAttribute(condenser_manager->_main_ui_panel, condenser_manager->_timer, ATTR_INTERVAL, condenser_manager->_timer_interval);  
		SetCtrlAttribute(condenser_manager->_main_ui_panel, condenser_manager->_timer, ATTR_ENABLED, 0);
	#else
		condenser_manager->_timer = NewAsyncTimer (condenser_manager->_timer_interval, -1, 1, OnCondenserTimerTick, condenser_manager);
		SetAsyncTimerName(condenser_manager->_timer, "CubeSlider");
		SetAsyncTimerAttribute (condenser_manager->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
		
	if(device_conf_load_default_node_data(condenser_manager->dc) == DEVICE_CONF_ERROR)
		return CONDENSER_MANAGER_ERROR; 
	
	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(condenser_manager), OnPanelsClosedOrHidden, condenser_manager);
	ui_module_panel_show_handler_connect (UIMODULE_CAST(condenser_manager), OnPanelsDisplayed, condenser_manager);

	// Update the ui
	if(condenser_manager_load_active_condensers_into_list_control(condenser_manager, condenser_manager->_main_ui_panel,
		CONDENSER_TURRET_POS) == CONDENSER_MANAGER_ERROR)   		
		return CONDENSER_MANAGER_ERROR;
	
	// Call specific device initialisation if necessary.
	if( (condenser_manager->vtable.initialise != NULL)) {
		
		if( (condenser_manager->vtable.initialise)(condenser_manager) == CONDENSER_MANAGER_ERROR )
			return CONDENSER_MANAGER_ERROR;  	
	}
	
	return CONDENSER_MANAGER_SUCCESS;
}

int condenser_manager_is_initialised(CondenserManager* condenser_manager)
{
	return condenser_manager->_initialised;	
}

int condenser_manager_signal_condenser_changed_handler_connect(CondenserManager* condenser_manager,
	CONDENSER_MANAGER_CHANGE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(condenser_manager), "CondenserChanged", handler, callback_data) == SIGNAL_ERROR) {
		return CONDENSER_MANAGER_ERROR;
	}
	
	return CONDENSER_MANAGER_SUCCESS; 
}


static int condenser_manager_copy_condenser(CondenserManager* condenser_manager, Condenser* src, Condenser* condenser)
{
	strcpy(condenser->name, src->name);
	
	return CONDENSER_MANAGER_SUCCESS;
}


int condenser_manager_hardware_initialise(CondenserManager* condenser_manager)
{
	int status = UIMODULE_ERROR_NONE;

	CHECK_CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, hardware_init) 
		
	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (condenser_manager->vtable.hardware_init)(condenser_manager) == CONDENSER_MANAGER_ERROR ) {
			status = send_condenser_error_text(condenser_manager, "condenser_manager_hardware_initialisefailed");
		
			if(status == UIMODULE_ERROR_IGNORE) {
				return CONDENSER_MANAGER_ERROR; 
			}
		}
		
	} 
	while(status == UIMODULE_ERROR_RETRY);

	condenser_manager->_hw_initialised = 1;  
	
  	return CONDENSER_MANAGER_SUCCESS;	
}

int condenser_manager_hardware_is_initialised(CondenserManager* condenser_manager)
{
	return condenser_manager->_hw_initialised;	
}

int condenser_manager_get_current_value_text(HardwareDevice* device, char* info)
{
	Condenser item;
	
	if (info==NULL)
		return CONDENSER_MANAGER_ERROR;
	
	condenser_manager_get_current_condenser((CondenserManager*)device, &item);
	
	if (item.name!=NULL){
		strcpy(info, item.name);
		return CONDENSER_MANAGER_SUCCESS;
	}

	return CONDENSER_MANAGER_ERROR;
}


int condenser_manager_hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode)
{
	CondenserManager* condenser_manager = (CondenserManager*)device;
	int pos;
	FILE *fp = NULL;

	dictionary *d = dictionary_new(5);
	
	condenser_manager_get_current_condenser_position(condenser_manager, &pos);

	fp = fopen(filepath, mode);
	
	if(fp == NULL)
		return CONDENSER_MANAGER_ERROR;

	dictionary_set(d, UIMODULE_GET_NAME(condenser_manager), NULL);
	dictionary_setint(d, "Position", pos);      

	iniparser_save(d, fp); 
	
	fclose(fp);
	dictionary_del(d);

	return CONDENSER_MANAGER_SUCCESS;
}

int condenser_manager_hardware_load_state_from_file (HardwareDevice* device, const char* filepath)
{
	CondenserManager* condenser_manager = (CondenserManager*)device;
	dictionary* d = NULL;
	int pos, file_size, num_devices;    
	char buffer[500] = "";

	if(!FileExists(filepath, &file_size))
		return CONDENSER_MANAGER_ERROR;	 	
	
	d = iniparser_load(filepath);  

	num_devices = device_conf_get_num_active_devices(condenser_manager->dc);

	if(d != NULL) {

		pos = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(condenser_manager), "position"), -1); 

		if(pos >= 0 && pos <= num_devices)
			condenser_manager_move_to_position(condenser_manager, pos);
	}

    dictionary_del(d);

	return CONDENSER_MANAGER_SUCCESS;
}

// Converts one node to a string formaty used to save all data as an ini file.
static int condenser_manager_node_to_ini_fmt (ModuleDeviceConfigurator *conf, CMDeviceNode *node, char *buffer)
{
	Condenser *condenser = (Condenser*) node->device;
	
	sprintf(buffer, "Id=%d\nName=%s\nPosition=%d\n\n",
					 node->id, node->name, node->position);
		
	return DEVICE_CONF_SUCCESS;  
}

static int condenser_manager_ini_fmt_to_node (ModuleDeviceConfigurator *conf, dictionary* ini, int section_number, CMDeviceNode *node)
{
	char *section = NULL, key[200];
	char *str_result = NULL;
		
	Condenser *condenser = node->device = malloc(sizeof(Condenser));
	
	memset(condenser, 0, sizeof(Condenser));
	
	section = iniparser_getsecname(ini, section_number);   
	
	node->id = iniparser_getint(ini, dictionary_get_section_key(key, section, "Id"), -1);  
	
	if(node->id == -1)
		return DEVICE_CONF_ERROR;  	
	
	str_result = iniparser_getstring(ini, dictionary_get_section_key(key, section, "Name"), "Unknown");
	strncpy(node->name, str_result, strlen(str_result) + 1); 
		
	node->position = iniparser_getint(ini, dictionary_get_section_key(key, section, "Position"), -1);
	
	strcpy(condenser->name, node->name);
	condenser->position = node->position;

	condenser = NULL;
	
	return DEVICE_CONF_SUCCESS;  
}

CondenserManager* condenser_manager_new(char *name, char *description, const char *data_dir, const char *data_file, size_t size)
{
	CondenserManager* condenser_manager = (CondenserManager*) malloc(size);
	
	// Ok attach to the add and edit buttons 
	condenser_manager->dc = device_conf_new();

	ui_module_set_data_dir(UIMODULE_CAST(condenser_manager->dc), data_dir);
	device_conf_set_default_filename(condenser_manager->dc, data_file);

	CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, hardware_init) = NULL;
	CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, initialise) = NULL; 
	CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, destroy) = NULL; 
	CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, move_to_condenser_position) = NULL; 
	CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, get_current_condenser_position) = NULL; 

	condenser_manager->dc->save_node_as_ini_fmt = condenser_manager_node_to_ini_fmt; 
	condenser_manager->dc->read_node_from_ini_fmt = condenser_manager_ini_fmt_to_node;

    hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(condenser_manager), name);
	ui_module_set_description(UIMODULE_CAST(condenser_manager), description);
	
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(condenser_manager), hardware_get_current_value_text) = condenser_manager_get_current_value_text; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(condenser_manager), hardware_save_state_to_file) = condenser_manager_hardware_save_state_to_file; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(condenser_manager), hardware_load_state_from_file) = condenser_manager_hardware_load_state_from_file; 

	return condenser_manager;
}

void condenser_manager_stop_timer(CondenserManager* condenser_manager)
{
	#ifdef ENABLE_CONDENSER_STATUS_POLLING 
	SetAsyncTimerAttribute (condenser_manager->_timer, ASYNC_ATTR_ENABLED,  0);
	#endif
}

void condenser_manager_start_timer(CondenserManager* condenser_manager)
{
	#ifdef ENABLE_CONDENSER_STATUS_POLLING  
	SetAsyncTimerAttribute (condenser_manager->_timer, ASYNC_ATTR_ENABLED,  1);
	#endif 
}

int condenser_manager_destroy(CondenserManager* condenser_manager)
{
	condenser_manager_stop_timer(condenser_manager);

	CHECK_CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, destroy) 
  	
	CALL_CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, destroy) 

	device_conf_destroy(condenser_manager->dc);      
	
	ui_module_destroy(UIMODULE_CAST(condenser_manager));
	
  	free(condenser_manager);

  	return CONDENSER_MANAGER_SUCCESS;
}

int condenser_manager_get_number_of_condensers(CondenserManager* condenser_manager, int *number_of_condensers)
{
	*number_of_condensers = device_conf_get_num_active_devices(condenser_manager->dc);
	
	return CONDENSER_MANAGER_SUCCESS;
}


int condenser_manager_get_current_condenser_position(CondenserManager* condenser_manager, int *position)
{
	CHECK_CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, get_current_condenser_position) 

	if( (*condenser_manager->vtable.get_current_condenser_position)(condenser_manager, position) == CONDENSER_MANAGER_ERROR ) {
		send_condenser_error_text(condenser_manager, "condenser_manager_get_current_condenser_position failed");
		return CONDENSER_MANAGER_ERROR;
	}

  	return CONDENSER_MANAGER_SUCCESS;
}

int condenser_manager_move_to_position(CondenserManager* condenser_manager, int position)
{
	CHECK_CONDENSER_MANAGER_VTABLE_PTR(condenser_manager, move_to_condenser_position) 

	if( (*condenser_manager->vtable.move_to_condenser_position)(condenser_manager, position) == CONDENSER_MANAGER_ERROR ) {
		send_condenser_error_text(condenser_manager, "condenser_manager_move_to_position failed");
		return CONDENSER_MANAGER_ERROR;
	}

	SetCtrlVal(condenser_manager->_main_ui_panel, CONDENSER_TURRET_POS, position);

  	return CONDENSER_MANAGER_SUCCESS;
}

int condenser_manager_get_current_condenser(CondenserManager* condenser_manager, Condenser *condenser)
{
	int pos;
	CMDeviceNode* node = NULL;
	Condenser *tmp;
	
	if(condenser_manager_get_current_condenser_position(condenser_manager, &pos) == CONDENSER_MANAGER_ERROR)
		return CONDENSER_MANAGER_ERROR; 
		
	if((node = device_conf_get_node_at_position(condenser_manager->dc, pos)) == NULL)
		return CONDENSER_MANAGER_ERROR; 
	
	tmp = (Condenser *) node->device;
	
	memcpy(condenser, tmp, sizeof(Condenser));
	strcpy(condenser->name, node->name);
	condenser->position = node->position;
	
	return CONDENSER_MANAGER_SUCCESS;             
}

Condenser* condenser_manager_get_condenser_with_name(CondenserManager* condenser_manager, const char* name) 
{
	return device_conf_get_device_with_name(condenser_manager->dc, name);
}
