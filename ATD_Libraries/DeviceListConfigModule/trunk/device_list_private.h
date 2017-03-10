#ifndef __DEVICE_CONFIG_MANAGER_PRIVATE__
#define __DEVICE_CONFIG_MANAGER_PRIVATE__

#include "device_list.h"


int CVICALLBACK node_sort(void *item1, void *item2);

int GetIdFromActiveTreeItem(ModuleDeviceConfigurator* dc, int tree);

int device_conf_get_list_index_for_node(ModuleDeviceConfigurator *dc, CMDeviceNode *node);

int device_conf_display_not_used_devices_in_list(ModuleDeviceConfigurator *dc);

int device_conf_display_used_devices_in_list(ModuleDeviceConfigurator *dc);

int device_conf_place_devices_in_list(ModuleDeviceConfigurator *conf, int panel, int list);

int device_conf_set_device_pos(ModuleDeviceConfigurator *dc, int id, int pos);

int device_conf_swap_pos_in_use_devices(ModuleDeviceConfigurator *dc, int first_id, int second_id);

int device_conf_remove_device_node_for_id(ModuleDeviceConfigurator *dc, int id);

int device_conf_remove_node_for_selected_item(ModuleDeviceConfigurator *dc);

void device_conf_save_current_state(ModuleDeviceConfigurator *dc);

void device_conf_restore_current_state(ModuleDeviceConfigurator *dc);

#endif

 
