#include "HardWareTypes.h"

#include "objectives.h"
#include "objectives_private.h"
#include "ObjectivesUI.h"
#include "device_list_ui.h" 

#include "string_utils.h"
#include "gci_utils.h"
#include "password.h"

#include "toolbox.h"

#include "asynctmr.h"

#include <userint.h>
#include <utility.h>

#include <ansi_c.h> 


int send_objectives_error_text (ObjectiveManager* objective_manager, char fmt[], ...)
{
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(objective_manager), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(objective_manager), message);  
	
	ui_module_send_valist_error(UIMODULE_CAST(objective_manager), "Objective Manager Error", fmt, ap);
	
	va_end(ap);  
	
	return OBJECTIVE_MANAGER_SUCCESS;
}

static int default_error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)
{
	GCI_MessagePopup("ObjectiveManager error", error_string); 
	
	return UIMODULE_ERROR_NONE;    
}

static void objective_manager_read_or_write_main_panel_registry_settings(ObjectiveManager *objective_manager, int write)
{
	ui_module_read_or_write_registry_settings(UIMODULE_CAST(objective_manager), write);   
}


static int OBJECTIVE_MANAGER_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (ObjectiveManager*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (ObjectiveManager *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


static int OBJECTIVE_MANAGER_PTR_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (ObjectiveManager*, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (ObjectiveManager *) args[0].void_ptr_data, (int) args[1].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


int CVICALLBACK OnObjectivesTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
        {
			ObjectiveManager *objective_manager = (ObjectiveManager *) callbackData;

			static int old_pos = -1;
			int pos;

			#ifdef TIMER_DEBUG
			printf("Enter Timer Tick -- %s\n", __FUNCTION__);
			#endif

			if (objective_manager_get_current_position(objective_manager, &pos) == OBJECTIVE_MANAGER_ERROR)
				goto Error;
  
			if(pos != old_pos) {
				
				GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(objective_manager), "EndObjectiveChange", GCI_VOID_POINTER, objective_manager, GCI_INT, pos); 		
				old_pos = pos;
				
				// Update the ui
				if(objective_manager_load_active_objectives_into_list_control(objective_manager,
					objective_manager->_main_ui_panel, OBJ_PANEL_TURRET_POS) == OBJECTIVE_MANAGER_ERROR)   	
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


// Return an array of the active cubes
Objective* objective_manager_get_active_cubes(ObjectiveManager* objective_manager)
{
	int i, size = 0;
	Objective * device_node = NULL;
	Objective * objectives = NULL;
	CMDeviceNode *tmp = NULL;
	ListType list;

	size = device_conf_get_num_active_devices(DEVICE_CONF_CAST(objective_manager->dc));

	if(size < 1)
		return DEVICE_CONF_ERROR; 	

	list = device_conf_get_devices_in_use_list(DEVICE_CONF_CAST(objective_manager->dc));

	objectives = malloc(sizeof(Objective) * size);

	for(i=0; i < size; i++) {
	
		tmp = ListGetPtrToItem(list, i);  
		device_node = (Objective*) tmp->device;

		objectives[i]._active = device_node->_active;
		objectives[i]._turret_position = device_node->_turret_position;
		strcpy(objectives[i]._objective_name,device_node->_objective_name);
		strcpy(objectives[i]._objective_medium,device_node->_objective_medium);
		strcpy(objectives[i]._magnification_str,device_node->_magnification_str);
		strcpy(objectives[i]._numerical_aperture,device_node->_numerical_aperture);
		strcpy(objectives[i]._working_distance,device_node->_working_distance);
		strcpy(objectives[i]._back_aperture,device_node->_back_aperture);
		strcpy(objectives[i]._focus_position,device_node->_focus_position);
		strcpy(objectives[i]._illumination,device_node->_illumination);
		strcpy(objectives[i]._condenser,device_node->_condenser);
		strcpy(objectives[i]._aperture_stop,device_node->_aperture_stop);
		strcpy(objectives[i]._field_stop,device_node->_field_stop);
	}

	return objectives;
}


int objective_manager_load_active_objectives_into_list_control(ObjectiveManager* objective_manager, int panel, int ctrl)
{
	int pos;
	
	if(objective_manager_get_current_position(objective_manager, &pos) == OBJECTIVE_MANAGER_ERROR)
		return OBJECTIVE_MANAGER_ERROR; 
	
	if(device_conf_load_active_devices_into_list_control(objective_manager->dc, panel, ctrl, pos) == DEVICE_CONF_ERROR) 
		return OBJECTIVE_MANAGER_ERROR; 
	
	return OBJECTIVE_MANAGER_SUCCESS;
}


void OnObjectivePanelDisplayed (UIModule *module, void *data)
{
	ObjectiveManager *objective_manager = (ObjectiveManager *) data;
	
	// Update the ui
	objective_manager_load_active_objectives_into_list_control(objective_manager, objective_manager->_main_ui_panel, OBJ_PANEL_TURRET_POS);
}


static void OnConfigChanged(ModuleDeviceConfigurator* dc, void *data)
{
	ObjectiveManager *objective_manager = (ObjectiveManager *) data;     
	
	objective_manager_load_active_objectives_into_list_control(objective_manager, objective_manager->_main_ui_panel, OBJ_PANEL_TURRET_POS);
}


// Converts one node to a string formaty used to save all data as an ini file.
static int objective_manager_node_to_ini_fmt (ModuleDeviceConfigurator *conf, CMDeviceNode *node, char *buffer)
{
	Objective *objective  = (Objective *) node->device;
	int i;
	char calibrations_str[500] = "", temp[500] = "";

	sprintf(buffer, "Id=%d\nName=%s\nPosition=%d\nMedium=%s\nMagnification=%s\nNumerical-Aperture=%s\nWorking-Distance=%s\nBack-Aperture=%s\n"
					"Focus-Position=%s\nIllumination=%s\nCondenser=%s\nAperture-Stop=%s\nField-Stop=%s\n",
					 node->id, node->name, node->position, objective->_objective_medium, objective->_magnification_str, objective->_numerical_aperture,
					 objective->_working_distance, objective->_back_aperture, objective->_focus_position, objective->_illumination,
					 objective->_condenser, objective->_aperture_stop, objective->_field_stop);

	if (objective->_calibrations == NULL)
		goto FINISHED;
	
	strcpy(calibrations_str, "Calibrations=(");

	for (i=0 ; i < objective->_calibrations->size; i++) {
        if (objective->_calibrations->key[i]) {
			sprintf(temp, "%s:%s,", objective->_calibrations->key[i], objective->_calibrations->val[i]);
			strcat(calibrations_str, temp);
        }
	}

	calibrations_str[strlen(calibrations_str) - 1] = ')'; // Remove last ,
	strcat(buffer, calibrations_str);

FINISHED:

	strcat(buffer, "\n\n");

	return DEVICE_CONF_SUCCESS;  
}


static char* construct_key(char *buffer, const char *section, const char* name)
{
	memset(buffer, 0, 1);
	
	sprintf(buffer, "%s:%s", section, name);
	
	return buffer;
}


static int get_calibration_from_str_format(const char *str, char *module_name, double *calibration)
{
	char buffer[500] = "";
	char *section = NULL;

	strncpy(buffer, str, 499);
		
	section	= strtok (buffer, ":");
  
	if(section == NULL)
		return -1;  

	strcpy(module_name, section);

	section = strtok (NULL, ":");

	if(section == NULL)
		return -1;  
		
	*calibration = strtod(section, NULL);

	return 0;
}

static int objective_manager_destroy_node(ModuleDeviceConfigurator *conf, CMDeviceNode *node)
{
	Objective *objective = (Objective *) node->device;

	dictionary_del(objective->_calibrations);
	objective->_calibrations = NULL;

	return 0;
}

static int objective_manager_ini_fmt_to_node (ModuleDeviceConfigurator *conf, dictionary* ini, int section_number, CMDeviceNode *node)
{
	char *section = NULL, key[200];
	char *str_result = NULL;
	char calibrations[10][50];
	char module[500] = "";
	double calib = 0.0;
	int i=0, number_of_calibrations = 0;

	Objective *objective = node->device = malloc(sizeof(Objective));
	memset(objective, 0, sizeof(Objective));
	
	objective->_calibrations = dictionary_new(5);

	section = iniparser_getsecname(ini, section_number);   
	
	sscanf(section, "node%d", &(node->id));

	str_result = iniparser_getstring(ini, construct_key(key, section, "Name"), "Unknown Name");
	strncpy(node->name, str_result, strlen(str_result) + 1); 
	strncpy(objective->_objective_name, node->name, OBJ_STR_LEN - 1);	

	node->position = iniparser_getint(ini, construct_key(key, section, "Position"), -1);
			
	str_result = iniparser_getstring(ini, construct_key(key, section, "Medium"), "Unknown Medium");
	strcpy(objective->_objective_medium, str_result);   
		
	str_result = iniparser_getstring(ini, construct_key(key, section, "Magnification"), "Unknown Magnification");
	strcpy(objective->_magnification_str, str_result);  
	
	str_result = iniparser_getstring(ini, construct_key(key, section, "Numerical-Aperture"), "0.0");
	strcpy(objective->_numerical_aperture, str_result);
	   
	str_result = iniparser_getstring(ini, construct_key(key, section, "Working-Distance"), "0.0");
    strcpy(objective->_working_distance, str_result);
		
	str_result = iniparser_getstring(ini, construct_key(key, section, "Back-Aperture"), "0.0");
	strcpy(objective->_back_aperture, str_result);
	
	str_result = iniparser_getstring(ini, construct_key(key, section, "Focus-Position"), "0.0");
	strcpy(objective->_focus_position, str_result); 
	
	str_result = iniparser_getstring(ini, construct_key(key, section, "Illumination"), "0.0");
	strcpy(objective->_illumination, str_result);
	
	str_result = iniparser_getstring(ini, construct_key(key, section, "Condenser"), "Unknown Condensor");
	strcpy(objective->_condenser, str_result);       

	str_result = iniparser_getstring(ini, construct_key(key, section, "Aperture-Stop"), "0.0");
	strcpy(objective->_aperture_stop, str_result);
	
	str_result = iniparser_getstring(ini, construct_key(key, section, "Field-Stop"), "0.0");
	strcpy(objective->_field_stop, str_result);

	str_result = iniparser_getstring(ini, construct_key(key, section, "Calibrations"), "");

	if(strcmp(str_result, "") == 0)
		return DEVICE_CONF_SUCCESS;  

	str_result++; // Remove (

	section = strtok (str_result, ",");

	if(section == NULL)
		return DEVICE_CONF_SUCCESS;  

	while (section != NULL) {
		
		strncpy(calibrations[i++], section, 49);
		number_of_calibrations++;

		section = strtok (NULL, ",");
	}

	for(i=0; i < number_of_calibrations; i++) {

		get_calibration_from_str_format(calibrations[i], module, &calib);
		objective_set_calibration(objective, module, calib);
	}

	return DEVICE_CONF_SUCCESS;  
}

int objective_manager_get_current_value_text(HardwareDevice* device, char* info)
{
	if (info==NULL)
		return OBJECTIVE_MANAGER_ERROR;

	return (objective_manager_get_current_objective_name((ObjectiveManager*)device, info));
}


int objective_hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode)
{
	ObjectiveManager* objective_manager = (ObjectiveManager*)device;
	int pos;
	FILE *fp = NULL;

	dictionary *d = dictionary_new(5);
	
	objective_manager_get_current_position(objective_manager, &pos);

	fp = fopen(filepath, mode);
	
	if(fp == NULL)
		return OBJECTIVE_MANAGER_ERROR;

	dictionary_set(d, UIMODULE_GET_NAME(objective_manager), NULL);
	dictionary_setint(d, "Position", pos);      

	iniparser_save(d, fp); 
	
	fclose(fp);
	dictionary_del(d);

	return OBJECTIVE_MANAGER_SUCCESS;
}

int objective_hardware_load_state_from_file (HardwareDevice* device, const char* filepath)
{
	ObjectiveManager* objective_manager = (ObjectiveManager*)device;
	dictionary* d = NULL;
	int pos, file_size, num_devices;    
	char buffer[500] = "";

	if(!FileExists(filepath, &file_size))
		return OBJECTIVE_MANAGER_ERROR;	 	
	
	d = iniparser_load(filepath);  

	num_devices = device_conf_get_num_active_devices(objective_manager->dc);

	if(d != NULL) {

		pos = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(objective_manager), "position"), -1); 

		if(pos >= 0 && pos <= num_devices)
			objective_manager_move_to_position(objective_manager, pos);
	}

    dictionary_del(d);

	return OBJECTIVE_MANAGER_SUCCESS;
}

ObjectiveManager* objective_manager_new(const char *name, const char *description, const char *data_dir, const char *data_file, size_t size)
{
	ObjectiveManager* objective_manager = (ObjectiveManager*) malloc(size);
	int device_config_panel_id;

	objective_manager->_initialised = 0;    

	// Ok attach to the add and edit buttons 
	objective_manager->dc = device_conf_new();
	
	ui_module_set_data_dir(UIMODULE_CAST(objective_manager->dc), data_dir);
	device_conf_set_default_filename(objective_manager->dc, data_file);        

	objective_manager->dc->save_node_as_ini_fmt = objective_manager_node_to_ini_fmt; 
	objective_manager->dc->read_node_from_ini_fmt = objective_manager_ini_fmt_to_node;
	objective_manager->dc->destroy_node = objective_manager_destroy_node;

	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(objective_manager), name);
	ui_module_set_description(UIMODULE_CAST(objective_manager), description); 
	
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(objective_manager), hardware_get_current_value_text) = objective_manager_get_current_value_text; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(objective_manager), hardware_save_state_to_file) = objective_hardware_save_state_to_file; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(objective_manager), hardware_load_state_from_file) = objective_hardware_load_state_from_file; 

	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, hw_init) = NULL;      
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, destroy) = NULL; 
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, get_current_turret_position) = NULL; 
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, move_to_turret_position) = NULL; 
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, get_current_turret_position) = NULL;  
													
	objective_manager->_timer = -1;

	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(objective_manager), "Close", OBJECTIVE_MANAGER_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(objective_manager), "ObjectiveManagerOff", OBJECTIVE_MANAGER_PTR_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(objective_manager), "ObjectiveManagerOn", OBJECTIVE_MANAGER_PTR_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(objective_manager), "ObjectiveChange", OBJECTIVE_MANAGER_PTR_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(objective_manager), "StartObjectiveChange", OBJECTIVE_MANAGER_PTR_INT_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(objective_manager), "EndObjectiveChange", OBJECTIVE_MANAGER_PTR_INT_MARSHALLER); 

	ui_module_set_error_handler(UIMODULE_CAST(objective_manager), default_error_handler, objective_manager);
	
	objective_manager->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(objective_manager), "ObjectivesUI.uir", OBJ_PANEL, 1); 
	objective_manager->_details_ui_panel = ui_module_add_panel(UIMODULE_CAST(objective_manager), "ObjectivesUI.uir", EDIT_PANEL, 0);      
	
	if(objective_manager->_main_ui_panel <= 0)
		return NULL;
	
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(objective_manager->dc), "ConfigChanged", OnConfigChanged, objective_manager) == SIGNAL_ERROR) {
		send_objectives_error_text(objective_manager, "Can not connect signal handler for ConfigChanged signal");
		return NULL;
	}
	
	if ( InstallCtrlCallback (objective_manager->_main_ui_panel, OBJ_PANEL_TURRET_POS, OnObjectiveChanged, objective_manager) < 0)
		return NULL;	
		
	if ( InstallCtrlCallback (objective_manager->_main_ui_panel, OBJ_PANEL_CONFIG_BUTTON, OnObjectivesConfig, objective_manager) < 0)
		return NULL;	
		
	if ( InstallCtrlCallback (objective_manager->_main_ui_panel, OBJ_PANEL_CLOSE_BUTTON, OnObjectivesCloseClicked, objective_manager) < 0)
		return NULL;

	if ( InstallCtrlCallback (objective_manager->_details_ui_panel, EDIT_PANEL_OK_BUTTON, OnObjectivesAddEditOkClicked, objective_manager) < 0)
		return NULL;	
		
	device_config_panel_id = device_conf_get_panel_id(objective_manager->dc);       
	
	if ( InstallCtrlCallback (device_config_panel_id, DLIST_CONF_ADD_BUTTON, OnObjectivesDetailsAdd, objective_manager) < 0)
		return NULL;
	
  	if ( InstallCtrlCallback (device_config_panel_id, DLIST_CONF_EDIT_BUTTON, OnObjectivesDetailsEdit, objective_manager) < 0)
		return NULL;
	
	ui_module_set_main_panel_title (UIMODULE_CAST(objective_manager));
	
	if (get_device_double_param_from_ini_file(UIMODULE_GET_NAME(objective_manager), "TimerInterval", &(objective_manager->_timer_interval))<0) 
		objective_manager->_timer_interval=2.0;

	objective_manager->_timer = NewAsyncTimer (objective_manager->_timer_interval, -1, 1, OnObjectivesTimerTick, objective_manager);
	SetAsyncTimerName(objective_manager->_timer, "Objectives");
	SetAsyncTimerAttribute (objective_manager->_timer, ASYNC_ATTR_ENABLED,  0);

	ui_module_main_panel_show_mainpanel_handler_connect(UIMODULE_CAST(objective_manager), OnObjectivePanelDisplayed, objective_manager);

	return objective_manager;
}

int objective_manager_signal_close_handler_connect (ObjectiveManager* objective_manager, OBJECTIVE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(objective_manager), "Close", handler, callback_data) == SIGNAL_ERROR) {
		send_objectives_error_text(objective_manager, "Can not connect signal handler for Close signal");
		return OBJECTIVE_MANAGER_ERROR;
	}

	return OBJECTIVE_MANAGER_SUCCESS;
}

int objective_set_calibration(Objective* objective, const char* module_name, double calibration)
{
	if (objective->_calibrations==NULL)
		objective->_calibrations = dictionary_new(5);
	
	dictionary_setdouble(objective->_calibrations, module_name, calibration);

	return OBJECTIVE_MANAGER_SUCCESS;
}

int objective_get_calibration(Objective* objective, const char* module_name, double *calibration)
{
	*calibration = 1.0;

	if(objective->_calibrations == NULL)
		return OBJECTIVE_MANAGER_ERROR;

	*calibration = dictionary_getdouble(objective->_calibrations, module_name, 1.0);

	return OBJECTIVE_MANAGER_SUCCESS;
}

int objective_set_calibration_for_objective_id(ObjectiveManager* objective_manager, int id, const char* module_name, double calibration)
{
	Objective *obj;
	CMDeviceNode* node = NULL;

	if(device_conf_get_device_node_for_id(objective_manager->dc, &node, id) == DEVICE_CONF_ERROR)   
		return OBJECTIVE_MANAGER_ERROR; 

	obj = (Objective*) node->device;

	objective_set_calibration(obj, module_name, calibration);

	return OBJECTIVE_MANAGER_SUCCESS;
}

int objective_get_calibration_for_objective_id(ObjectiveManager* objective_manager, int id, const char* module_name, double *calibration)
{
	Objective *obj;
	CMDeviceNode* node = NULL;

	if(device_conf_get_device_node_for_id(objective_manager->dc, &node, id) == DEVICE_CONF_ERROR)   
		return OBJECTIVE_MANAGER_ERROR; 

	obj = (Objective*) node->device;

	memcpy(obj, node->device, sizeof(Objective));

	objective_get_calibration(obj, module_name, calibration);

	return OBJECTIVE_MANAGER_SUCCESS;
}

int objective_manager_signal_start_change_handler_connect (ObjectiveManager* objective_manager, OBJECTIVE_CHANGE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(objective_manager), "StartObjectiveChange", handler, callback_data) == SIGNAL_ERROR) {
		send_objectives_error_text(objective_manager, "Can not connect signal handler for Start Change signal");
		return OBJECTIVE_MANAGER_ERROR;
	}

	return OBJECTIVE_MANAGER_SUCCESS;
}

int objective_manager_signal_end_change_handler_connect (ObjectiveManager* objective_manager, OBJECTIVE_CHANGE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(objective_manager), "EndObjectiveChange", handler, callback_data) == SIGNAL_ERROR) {
		send_objectives_error_text(objective_manager, "Can not connect signal handler for End Change signal");
		return OBJECTIVE_MANAGER_ERROR;
	}

	return OBJECTIVE_MANAGER_SUCCESS;
}


int objective_manager_is_initialised(ObjectiveManager* objective_manager)
{
	return (objective_manager->_initialised && objective_manager->_hw_initialised);	
}


void objective_manager_stop_timer(ObjectiveManager* objective_manager)
{
	SetAsyncTimerAttribute (objective_manager->_timer, ASYNC_ATTR_ENABLED,  0);
}

void objective_manager_start_timer(ObjectiveManager* objective_manager)
{  
	SetAsyncTimerAttribute (objective_manager->_timer, ASYNC_ATTR_ENABLED,  1);
}


int objective_manager_destroy(ObjectiveManager* objective_manager)
{
	objective_manager_stop_timer(objective_manager);
	DiscardAsyncTimer(objective_manager->_timer);
	
	CHECK_OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, destroy) 
  	
	CALL_OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, destroy) 

	objective_manager_read_or_write_main_panel_registry_settings(objective_manager, 1);

	device_conf_destroy(objective_manager->dc);
  	
	ui_module_destroy(UIMODULE_CAST(objective_manager));

	free(objective_manager);

  	return OBJECTIVE_MANAGER_SUCCESS;
}



// Virtual Methods


int objective_manager_hardware_initialise(ObjectiveManager* objective_manager)
{
	int status = UIMODULE_ERROR_NONE;    

	CHECK_OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, hw_init) 

	do {
		status = UIMODULE_ERROR_NONE;
		
		if( (objective_manager->vtable.hw_init)(objective_manager) == OBJECTIVE_MANAGER_ERROR ) {
			status = send_objectives_error_text(objective_manager, "objective_manager_initialise failed");

			if(status == UIMODULE_ERROR_IGNORE) {
				objective_manager->_hw_initialised = 0;
				return OBJECTIVE_MANAGER_ERROR;  
			}
		}
	}
	while(status == UIMODULE_ERROR_RETRY);

	objective_manager->_hw_initialised = 1;
	
  	return OBJECTIVE_MANAGER_SUCCESS;  	
}


int objective_manager_initialise(ObjectiveManager* objective_manager, int move_to_default)
{
	int default_pos = 1;
	int status = UIMODULE_ERROR_NONE;    

	if(device_conf_load_default_node_data(objective_manager->dc) == DEVICE_CONF_ERROR)
		return OBJECTIVE_MANAGER_ERROR; 
	
	objective_manager_load_active_objectives_into_list_control(objective_manager,
		objective_manager->_main_ui_panel, OBJ_PANEL_TURRET_POS);
		
	device_conf_get_default_position(objective_manager->dc, &default_pos);  

	if(move_to_default) {
		if(objective_manager_move_to_position(objective_manager, default_pos) == OBJECTIVE_MANAGER_ERROR)
			return OBJECTIVE_MANAGER_ERROR; 
	}

	objective_manager->_initialised = 1;
	
  	return OBJECTIVE_MANAGER_SUCCESS;  	
}



int objective_manager_move_to_position(ObjectiveManager* objective_manager, int position)
{
	CHECK_OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, move_to_turret_position) 
  	
	logger_log(UIMODULE_LOGGER(objective_manager), LOGGER_INFORMATIONAL, "%s move to pos %d", UIMODULE_GET_DESCRIPTION(objective_manager), position);

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(objective_manager), "StartObjectiveChange",
		GCI_VOID_POINTER, objective_manager, GCI_INT, position); 

	if( (*objective_manager->vtable.move_to_turret_position)(objective_manager, position) == OBJECTIVE_MANAGER_ERROR ) {
		logger_log(UIMODULE_LOGGER(objective_manager), LOGGER_INFORMATIONAL, "%s move to pos %d failed", UIMODULE_GET_DESCRIPTION(objective_manager), position);

		return OBJECTIVE_MANAGER_ERROR;
	}
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(objective_manager), "EndObjectiveChange",
		GCI_VOID_POINTER, objective_manager, GCI_INT, position); 
	
	SetCtrlVal(objective_manager->_main_ui_panel, OBJ_PANEL_TURRET_POS, position);

	return OBJECTIVE_MANAGER_SUCCESS; 
}


int objective_manager_get_current_objective_name(ObjectiveManager* objective_manager, char *name)
{
	CMDeviceNode* node = NULL;
	Objective *obj = NULL;
	int pos;
	
	if(objective_manager_get_current_position(objective_manager, &pos) == OBJECTIVE_MANAGER_ERROR)
		return OBJECTIVE_MANAGER_ERROR;
		
	if((node = device_conf_get_node_at_position(objective_manager->dc, pos)) == NULL)
		return OBJECTIVE_MANAGER_ERROR;
	
	// Copy name of node to objective structure
	strncpy(name, node->name, UIMODULE_NAME_LEN);
	
	return OBJECTIVE_MANAGER_SUCCESS; 
}


int objective_manager_get_current_position(ObjectiveManager* objective_manager, int *position)
{
	CHECK_OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, get_current_turret_position)
	
	if( (*objective_manager->vtable.get_current_turret_position)(objective_manager, position) == OBJECTIVE_MANAGER_ERROR ) {
		send_objectives_error_text(objective_manager, "get_current_turret_position failed");
		return OBJECTIVE_MANAGER_ERROR;
	}
	
	return OBJECTIVE_MANAGER_SUCCESS; 
}


int objective_manager_get_number_of_defined_objectives(ObjectiveManager* objective_manager)
{
	return device_conf_get_num_defined_devices(objective_manager->dc);
}

int objective_manager_get_number_of_active_objectives(ObjectiveManager* objective_manager)
{
	return device_conf_get_num_active_devices(objective_manager->dc);
}


Objective*  objective_manager_get_objective_with_name(ObjectiveManager* objective_manager, const char* name)
{
	return device_conf_get_device_with_name(objective_manager->dc, name);  	
}


int objective_manager_get_current_objective(ObjectiveManager* objective_manager, Objective* objective)
{
	int pos;
	CMDeviceNode* node = NULL;
	Objective *obj = NULL, *tmp = NULL;
								   
	if(objective_manager_get_current_position(objective_manager, &pos) == OBJECTIVE_MANAGER_ERROR)
		return OBJECTIVE_MANAGER_ERROR;
		
	if((node = device_conf_get_node_at_position(objective_manager->dc, pos)) == NULL)
		return OBJECTIVE_MANAGER_ERROR;
	
	tmp = (Objective *) node->device;
	
	memcpy(objective, tmp, sizeof(Objective));
	strcpy(objective->_objective_name, node->name);
	objective->_turret_position = node->position;
	
	return OBJECTIVE_MANAGER_SUCCESS; 
}


int objective_manager_goto_default_position(ObjectiveManager* objective_manager)
{
	int pos=0;
	
	if(device_conf_get_default_position(objective_manager->dc, &pos) == DEVICE_CONF_ERROR)
		return OBJECTIVE_MANAGER_ERROR;
	
	if(objective_manager_move_to_position(objective_manager, pos) == OBJECTIVE_MANAGER_ERROR)
		return OBJECTIVE_MANAGER_ERROR;  	

	return OBJECTIVE_MANAGER_SUCCESS;
}

int objective_manager_signal_config_changed_handler_connect(ObjectiveManager* objective_manager,
	OBJECTIVE_EVENT_HANDLER handler, void *callback_data)
{
	int id;
	if( (id=GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(objective_manager->dc), "ConfigChanged", handler, callback_data)) == SIGNAL_ERROR) {
		send_objectives_error_text(objective_manager, "Can not connect signal handler for ConfigChanged signal");
		return OBJECTIVE_MANAGER_ERROR;
	}

	return id;	
}

int objective_manager_signal_config_changed_handler_disconnect(ObjectiveManager* objective_manager, int id)
{
	if( GCI_Signal_Disconnect(UIMODULE_SIGNAL_TABLE(objective_manager->dc), "ConfigChanged", id) == SIGNAL_ERROR) {
		send_objectives_error_text(objective_manager, "Can not disconnect signal handler for ConfigChanged signal");
		return OBJECTIVE_MANAGER_ERROR;
	}

	return OBJECTIVE_MANAGER_SUCCESS;	
}


int objective_manager_enable_dynamic_objective_options(ObjectiveManager* objective_manager)
{
	SetPanelAttribute(objective_manager->_details_ui_panel, ATTR_HEIGHT, 280);

	SetCtrlAttribute(objective_manager->_details_ui_panel, EDIT_PANEL_FOCUS_POSITION, ATTR_VISIBLE, 1);
	SetCtrlAttribute(objective_manager->_details_ui_panel, EDIT_PANEL_ILLUMINATION, ATTR_VISIBLE, 1);
	SetCtrlAttribute(objective_manager->_details_ui_panel, EDIT_PANEL_APERTURE_STOP, ATTR_VISIBLE, 1);
	SetCtrlAttribute(objective_manager->_details_ui_panel, EDIT_PANEL_FIELD_STOP, ATTR_VISIBLE, 1);

	SetCtrlAttribute(objective_manager->_details_ui_panel, EDIT_PANEL_OK_BUTTON, ATTR_TOP, 245);

	return OBJECTIVE_MANAGER_SUCCESS;
}
