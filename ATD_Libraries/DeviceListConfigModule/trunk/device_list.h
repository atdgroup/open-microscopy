#ifndef __DEVICE_CONFIG_MANAGER__
#define __DEVICE_CONFIG_MANAGER__

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#endif

#include "signals.h"
#include "gci_ui_module.h"
#include "toolbox.h"
#include "iniparser.h"

#define DEVICE_CONF_ERROR 0
#define DEVICE_CONF_SUCCESS 1

#define DEVICE_CONF_CAST(obj) ((ModuleDeviceConfigurator *) (obj))    
#define DEVICE_CONF_VTABLE_PTR(ob, member) ((ob)->vtable.member)
#define DEVICE_CONF_MANAGER_VTABLE(ob, member) (*((ob)->vtable.member))


typedef struct _ModuleDeviceConfigurator ModuleDeviceConfigurator;

// This is the node that contains a reference to the device that
// inheriters define like say a specific objective.
typedef struct
{
	int id;
	int position; // Position of -1 means not in use
	char name[100];
	void *device;
	
} CMDeviceNode;


struct _ModuleDeviceConfigurator
{
    UIModule parent;	
	
	ListType not_in_use_list;			 // Devices that are not in use.
	ListType in_use_list;    			 // Devices that are in use.
	ListType stored_not_in_use_list;			 // Devices that are not in use.
	ListType stored_in_use_list;
	ListType defined_list;

	int (*save_node_as_ini_fmt) (ModuleDeviceConfigurator *conf, CMDeviceNode *node, char *buffer);  
	int (*read_node_from_ini_fmt) (ModuleDeviceConfigurator *conf, dictionary* ini, int section_number, CMDeviceNode *node);  
	int (*destroy_node) (ModuleDeviceConfigurator *conf, CMDeviceNode *node);  
	
	int config_panel_id;
	int max_allowed_devices;
	int max_in_use_allowed_devices;
	int active_tree_id;
	int data_modified;
	
	char filename[100];
	
	CMDeviceNode *default_node;
};


ModuleDeviceConfigurator* device_conf_new(void); 

void device_conf_display_panel(ModuleDeviceConfigurator *dc);

int device_conf_close_config_panel(ModuleDeviceConfigurator *dc);

int device_conf_set_default_filename(ModuleDeviceConfigurator * dc, const char *name);

int device_conf_get_panel_id(ModuleDeviceConfigurator * dc); 

void device_conf_destroy(ModuleDeviceConfigurator *dc); 

int device_conf_get_device_node_for_id(ModuleDeviceConfigurator *dc, CMDeviceNode **tmp, int id);

int device_conf_get_device_name_for_id(ModuleDeviceConfigurator *dc, char *name, int id);

int device_conf_get_device_name_for_pos(ModuleDeviceConfigurator *dc, char *name, int pos);

void device_conf_set_max_num__of_devices(ModuleDeviceConfigurator *dc, int number); 

void device_conf_set_max_active_num_devices(ModuleDeviceConfigurator *dc, int number); 

int  device_conf_get_num_defined_devices(ModuleDeviceConfigurator *dc);

int  device_conf_get_num_active_devices(ModuleDeviceConfigurator *dc); 

// Add a device to the configurator. device must continue to be valid. ie don't call free on it.
int device_conf_add_device(ModuleDeviceConfigurator *dc, void *device, const char* name, int pos);

ListType device_conf_get_in_use_devices(ModuleDeviceConfigurator *dc);

ListType device_conf_get_defined_devices_list(ModuleDeviceConfigurator *dc);

CMDeviceNode* device_conf_get_device_node_with_name(ModuleDeviceConfigurator *dc, const char* name);

void* device_conf_get_device_with_name(ModuleDeviceConfigurator *dc, const char* name); 

void* device_conf_get_device_at_position(ModuleDeviceConfigurator *dc, int pos);

void* device_conf_get_device_at_default_position(ModuleDeviceConfigurator *dc);

int device_conf_get_default_position(ModuleDeviceConfigurator *dc, int *pos);    

int device_conf_load_active_devices_into_list_control(ModuleDeviceConfigurator *dc, int panel, int ctrl, int device_pos);

ListType device_conf_get_devices_in_use_list(ModuleDeviceConfigurator *dc);

int device_conf_set_active_devices_list_control_to_pos(ModuleDeviceConfigurator *dc, int panel, int ctrl, int device_pos);

CMDeviceNode* device_conf_get_node_for_selected_item(ModuleDeviceConfigurator *dc);

CMDeviceNode* device_conf_get_node_at_position(ModuleDeviceConfigurator *dc, int pos);

int device_conf_save_node_data(ModuleDeviceConfigurator* dc, const char *filepath);

int device_conf_save_node_data_for_module(ModuleDeviceConfigurator* dc);

int device_conf_load_node_data(ModuleDeviceConfigurator* dc, const char *filepath);

int device_conf_load_default_node_data(ModuleDeviceConfigurator* dc);

CMDeviceNode* device_conf_get_active_device_array(ModuleDeviceConfigurator *dc, size_t device_size, size_t *size);

void device_conf_update_display(ModuleDeviceConfigurator *dc);

void device_conf_display_local_panel (ModuleDeviceConfigurator *dc, int panel);

/*
int device_conf_set_device_pos(ModuleDeviceConfigurator *conf, CMDeviceNode *, int pos); 

int device_conf_get_device(ModuleDeviceConfigurator *conf, CMDeviceNode *, int pos);

int device_conf_num_active_devices(ModuleDeviceConfigurator *conf);  

int device_conf_num_devices(ModuleDeviceConfigurator *conf); 

int device_conf_get_default_position(ModuleDeviceConfigurator *conf);  

int device_conf_place_devices_in_list(ModuleDeviceConfigurator *conf, int panel, int list);

int device_conf_save_to_ini_file(ModuleDeviceConfigurator *conf);  

int device_conf_load_from_ini_file(ModuleDeviceConfigurator *conf); 
  */

int  CVICALLBACK OnDefaultPosChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDeviceListCloseClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDeviceListDownArrow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDeviceListFileRecall(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDeviceListFileSave(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDeviceListLeftArrow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDeviceListRemoveClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDeviceListRightArrow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDeviceListTreeChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDeviceListUpArrow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif

 
