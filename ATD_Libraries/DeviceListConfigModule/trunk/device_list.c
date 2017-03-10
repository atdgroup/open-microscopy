#include "device_list.h"
#include "device_list_private.h" 
#include "device_list_ui.h"
#include "string_utils.h"
#include "gci_utils.h"

#include "toolbox.h"
#include <userint.h>
#include <utility.h>
#include <ansi_c.h> 
#include <string.h>

static int DEVICE_CONF_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (ModuleDeviceConfigurator*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (ModuleDeviceConfigurator *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

int device_conf_set_default_filename(ModuleDeviceConfigurator * dc, const char *name)
{
	strcpy(dc->filename, name);
	
	return DEVICE_CONF_SUCCESS;   
}


// This code use to save as xml but the cvi xml library doesn't work when the ms xml library is used.
// The ms xml library is used by libraries we depend on. For now I am writing
// the cube data to an ini file.

int device_conf_save_node_data(ModuleDeviceConfigurator* dc, const char *filepath)
{
	int i;
	CMDeviceNode *tmp = NULL;  
	char section_buffer[100], buffer[2000];
	char temp_dir[GCI_MAX_PATHNAME_LEN] = "", temp_filepath[GCI_MAX_PATHNAME_LEN] = "";

	if(GetTempPath(GCI_MAX_PATHNAME_LEN-1, temp_dir) == 0)
		return DEVICE_CONF_ERROR;

	if(GetTempFileName(temp_dir, "device_list", 0, temp_filepath) == 0)
		return DEVICE_CONF_ERROR;
	
	memset(section_buffer, 0, 1);
	memset(buffer, 0, 1);   
	
	// Write the global section
	if( dc->default_node != NULL) {
		sprintf(buffer, "Default=%d\n\n", dc->default_node->position);
		str_change_char(buffer, '\n', '\0');  
	}
	
	if(!WritePrivateProfileSection("Global", buffer, temp_filepath))
			return DEVICE_CONF_ERROR;  
	
	memset(buffer, 0, 1);
	
	
	for(i=1; i <= ListNumItems(dc->not_in_use_list); i++) {
	
		tmp = ListGetPtrToItem(dc->not_in_use_list, i);     
		
		sprintf(section_buffer, "Node%d", tmp->id);
		
		if(dc->save_node_as_ini_fmt (dc, tmp, buffer) == DEVICE_CONF_ERROR)
			return DEVICE_CONF_ERROR;
		
		str_change_char(buffer, '\n', '\0');
		
		if(!WritePrivateProfileSection(section_buffer, buffer, temp_filepath))
			return DEVICE_CONF_ERROR;
	}	
	
	for(i=1; i <= ListNumItems(dc->in_use_list); i++) {
	
		tmp = ListGetPtrToItem(dc->in_use_list, i);     
		
		sprintf(section_buffer, "Node%d", tmp->id);
		
		if(dc->save_node_as_ini_fmt (dc, tmp, buffer) == DEVICE_CONF_ERROR)
			return DEVICE_CONF_ERROR;
		
		str_change_char(buffer, '\n', '\0');
		
		if(!WritePrivateProfileSection(section_buffer, buffer, temp_filepath))
			return DEVICE_CONF_ERROR;
	}		

	if(CopyFile (temp_filepath, filepath) < 0)
		return DEVICE_CONF_ERROR;

	return DEVICE_CONF_SUCCESS;     
}

int device_conf_save_node_data_for_module(ModuleDeviceConfigurator* dc)
{
	char path[GCI_MAX_PATHNAME_LEN];
	int ret;

	ui_module_get_data_dir(UIMODULE_CAST(dc), path);

	strcat(path, "\\");
	strcat(path, dc->filename);

	ret = device_conf_save_node_data(dc, path);   

	if (DEVICE_CONF_SUCCESS==ret) {
		GCIDialog(0, "Configuration Saved", IDI_INFORMATION, GCI_OK_BUTTON, "Configuration saved successfully to:\n%s", path);

		// This variable determines if you get a popup asking if you want
		// to exit without saving.
		dc->data_modified = 0;

		GCI_Signal_Emit_From_MainThread(UIMODULE_SIGNAL_TABLE(dc), "ConfigChanged", GCI_VOID_POINTER, dc);
	}
	else {
		GCIDialog(0, "Configuration Save Error", IDI_EXCLAMATION, GCI_OK_BUTTON, "Configuration FAILED to save to:\n%s", path);
	}

	return DEVICE_CONF_SUCCESS;
}

int device_conf_load_node_data(ModuleDeviceConfigurator* dc, const char *filepath)   
{
	CMDeviceNode node;    
	dictionary* ini = iniparser_load  (filepath);  
	
	int i=1, default_pos, number_of_items = iniparser_getnsec  (ini);  
	char key[200];
	
	memset(key, 0, 1);

	ListClear(dc->not_in_use_list);  
	ListClear(dc->in_use_list); 
	
	if(ClearListCtrl (dc->config_panel_id, DLIST_CONF_ALL_TREE) < 0)
		goto Error;   
	
	if(ClearListCtrl (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE) < 0)
		goto Error;   

	// 0 is the global section.
	for(i=1; i < number_of_items; i++) {
	
		if(dc->read_node_from_ini_fmt (dc, ini, i, &node) == DEVICE_CONF_ERROR)
			return DEVICE_CONF_ERROR;

		if(device_conf_add_device(dc, node.device, node.name, node.position) == DEVICE_CONF_ERROR)
		   goto Error;
	}	
	
	
	// Default position
	default_pos = iniparser_getint(ini, "Global:Default", -1);
	
	if(default_pos == -1)
		goto Error;  
	
	dc->default_node = device_conf_get_node_at_position(dc, default_pos);
	 
	if(dc->default_node == NULL)
		goto Error;  
	
	iniparser_freedict(ini);    
	
	// Update display 
	device_conf_display_used_devices_in_list(dc);
	device_conf_display_not_used_devices_in_list(dc); 
	
	SetCtrlVal(dc->config_panel_id, DLIST_CONF_DEFAULT, default_pos);
	
	return DEVICE_CONF_SUCCESS;
	
	Error:
	
		iniparser_freedict(ini);
		
		return DEVICE_CONF_ERROR;
}


int device_conf_load_default_node_data(ModuleDeviceConfigurator* dc)
{
	char path[GCI_MAX_PATHNAME_LEN];
	
	ui_module_get_data_dir(UIMODULE_CAST(dc), path);
	strcat(path, "\\");
	strcat(path, dc->filename);
	
	return device_conf_load_node_data(dc, path);	
}

ModuleDeviceConfigurator* device_conf_new(void)
{
	ModuleDeviceConfigurator* dc = (ModuleDeviceConfigurator*) malloc(sizeof(ModuleDeviceConfigurator));
	
	memset(dc, 0, sizeof(ModuleDeviceConfigurator));
	memset(dc->filename, 0, 1);
	
	dc->data_modified = 0;  
	dc->save_node_as_ini_fmt = NULL; 
	dc->read_node_from_ini_fmt = NULL; 
	dc->destroy_node = NULL; 

	dc->not_in_use_list = ListCreate (sizeof(CMDeviceNode));
	dc->in_use_list = ListCreate (sizeof(CMDeviceNode));
	dc->defined_list = ListCreate (sizeof(CMDeviceNode)); 
	
	// Allow a large number of devices by default.
	dc->max_allowed_devices = 100;
	dc->max_in_use_allowed_devices = 100;
	
	ui_module_constructor(UIMODULE_CAST(dc), "DeviceListModule");
	
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(dc), "ConfigChanged", DEVICE_CONF_PTR_MARSHALLER); 
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(dc), "EditAddPanelDisplayed", DEVICE_CONF_PTR_MARSHALLER); 
	
	dc->config_panel_id = ui_module_add_panel(UIMODULE_CAST(dc), "device_list_ui.uir", DLIST_CONF, 1);

	SetCtrlAttribute(dc->config_panel_id, DLIST_CONF_DEFAULT, ATTR_MAX_VALUE, 100);	

	if ( InstallCtrlCallback (dc->config_panel_id, DLIST_CONF_LEFT_ARROW, OnDeviceListLeftArrow, dc) < 0)
		return NULL;
		
	if ( InstallCtrlCallback (dc->config_panel_id, DLIST_CONF_RIGHT_ARROW, OnDeviceListRightArrow, dc) < 0)
		return NULL;
  
	if ( InstallCtrlCallback (dc->config_panel_id, DLIST_CONF_QUIT_BUTTON, OnDeviceListCloseClicked, dc) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (dc->config_panel_id, DLIST_CONF_UP_ARROW, OnDeviceListUpArrow, dc) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (dc->config_panel_id, DLIST_CONF_DOWN_ARROW, OnDeviceListDownArrow, dc) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (dc->config_panel_id, DLIST_CONF_ALL_TREE, OnDeviceListTreeChanged, dc) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, OnDeviceListTreeChanged, dc) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (dc->config_panel_id, DLIST_CONF_SAVE_OBJ_BUTTON, OnDeviceListFileSave, dc) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (dc->config_panel_id, DLIST_CONF_LOAD_OBJ_BUTTON, OnDeviceListFileRecall , dc) < 0)
		return NULL;
	
	if ( InstallCtrlCallback (dc->config_panel_id, DLIST_CONF_REMOVE_BUTTON, OnDeviceListRemoveClicked, dc) < 0)
		return NULL;

	return dc;
}


int device_conf_get_panel_id(ModuleDeviceConfigurator * dc)
{
	return dc->config_panel_id;
}

void device_conf_set_max_num__of_devices(ModuleDeviceConfigurator *dc, int number)
{
	dc->max_allowed_devices = number;
	SetCtrlAttribute(dc->config_panel_id, DLIST_CONF_DEFAULT, ATTR_MAX_VALUE, number);
}

void device_conf_set_max_active_num_devices(ModuleDeviceConfigurator *dc, int number)
{
	dc->max_in_use_allowed_devices = number;	
}

int device_conf_close_config_panel(ModuleDeviceConfigurator *dc)
{
	if (dc->data_modified == 1) {
		SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
		if (ConfirmPopup("Warning", "Exit without saving changes?\nYour changes will be lost.")) {

			device_conf_restore_current_state(dc);
			ui_module_hide_all_panels(UIMODULE_CAST(dc));
			return 0;
		}
		else {
			return -1;
		}
	}
	
	ui_module_hide_all_panels(UIMODULE_CAST(dc));

	return 0;
}

void device_conf_display_panel(ModuleDeviceConfigurator *dc)
{
	device_conf_save_current_state(dc);

	// Update display 
	device_conf_display_used_devices_in_list(dc);
	device_conf_display_not_used_devices_in_list(dc); 
	
	if(dc->default_node != NULL)
		SetCtrlVal(dc->config_panel_id, DLIST_CONF_DEFAULT, dc->default_node->position);  

	SetCtrlAttribute(dc->config_panel_id, DLIST_CONF_EDIT_BUTTON, ATTR_DIMMED, 1); 
	SetCtrlAttribute(dc->config_panel_id, DLIST_CONF_REMOVE_BUTTON, ATTR_DIMMED, 1);  
	
	ui_module_display_main_panel_without_registry(UIMODULE_CAST(dc));	
}

void device_conf_destroy(ModuleDeviceConfigurator *dc)
{
	int i;
	CMDeviceNode *node = NULL;
	
	ui_module_destroy(UIMODULE_CAST(dc)); 
	
	for(i=1; i <= ListNumItems(dc->not_in_use_list); i++) {
		node = ListGetPtrToItem(dc->not_in_use_list, i);

		if(dc->destroy_node != NULL)
			dc->destroy_node (dc, node);

		free(node->device);
		node->device = NULL;
	}
	
	for(i=1; i <= ListNumItems(dc->in_use_list); i++) {
		node = ListGetPtrToItem(dc->in_use_list, i);  

		if(dc->destroy_node != NULL)
			dc->destroy_node (dc, node);

		free(node->device);
		node->device = NULL;
	}
	
	ListDispose(dc->not_in_use_list);
	ListDispose(dc->in_use_list); 
	ListDispose(dc->defined_list);

	free(dc);
}


int CVICALLBACK node_sort(void *item1, void *item2)
{
	CMDeviceNode *node1 = (CMDeviceNode *) item1;
	CMDeviceNode *node2 = (CMDeviceNode *) item2;
	
	return node1->position - node2->position;
}

void device_conf_save_current_state(ModuleDeviceConfigurator *dc)
{
	dc->stored_in_use_list = ListCopy(dc->in_use_list);
	dc->stored_not_in_use_list = ListCopy(dc->not_in_use_list);
}

void device_conf_restore_current_state(ModuleDeviceConfigurator *dc)
{
	dc->in_use_list = ListCopy(dc->stored_in_use_list);
	dc->not_in_use_list = ListCopy(dc->stored_not_in_use_list);
}

// Add a device to the configurator. device must continue to be valid. ie don't call free on it.
int device_conf_add_device(ModuleDeviceConfigurator *dc, void *device, const char* name, int pos)
{
	CMDeviceNode node;
	
	int total_nodes = ListNumItems(dc->in_use_list) +  ListNumItems(dc->not_in_use_list);
	
	// Must not exceed max allow devices
	if(total_nodes >= dc->max_allowed_devices)
		return DEVICE_CONF_ERROR;

	node.device = device;
	strcpy(node.name, name);
	node.id = total_nodes + 1;
	node.position = pos;
	
	if(ListNumItems(dc->in_use_list) >= dc->max_in_use_allowed_devices)
		node.position = -1;	// Put in not used list
	
	if(node.position != -1)
		ListInsertInOrder (dc->in_use_list, &node, node_sort);   
	else
		ListInsertItem(dc->not_in_use_list, &node, END_OF_LIST);
	
	// Update display 
	device_conf_display_used_devices_in_list(dc);
	device_conf_display_not_used_devices_in_list(dc); 
	
	return DEVICE_CONF_SUCCESS;   
}


ListType device_conf_get_in_use_devices(ModuleDeviceConfigurator *dc)
{
	return dc->in_use_list;	
}

ListType device_conf_get_defined_devices_list(ModuleDeviceConfigurator *dc)
{
	ListClear(dc->defined_list);
	ListAppend(dc->defined_list, dc->not_in_use_list);
	ListAppend(dc->defined_list, dc->in_use_list);

	return dc->defined_list;	
}

CMDeviceNode* device_conf_get_device_node_with_name(ModuleDeviceConfigurator *dc, const char* name)
{
	int i;
	CMDeviceNode *tmp = NULL;  
	
	for(i=1; i <= ListNumItems(dc->in_use_list); i++) {
		
		tmp = ListGetPtrToItem(dc->in_use_list, i);
		
		if(strcmp(tmp->name, name) == 0)
			return tmp;
	}
	
	for(i=1; i <= ListNumItems(dc->not_in_use_list); i++) {
		
		tmp = ListGetPtrToItem(dc->not_in_use_list, i);
		
		if(strcmp(tmp->name, name) == 0)
			return tmp;
	}
	
	return NULL;  	
}

void* device_conf_get_device_with_name(ModuleDeviceConfigurator *dc, const char* name)
{
	return device_conf_get_device_node_with_name(dc, name)->device;	
}

void* device_conf_get_device_at_position(ModuleDeviceConfigurator *dc, int pos)
{
	CMDeviceNode* tmp = device_conf_get_node_at_position(dc, pos);
	
	if(tmp == NULL)
		return NULL;
	
	return tmp->device;
}


void* device_conf_get_device_at_default_position(ModuleDeviceConfigurator *dc)
{
	return dc->default_node->device;  
}

int device_conf_get_default_position(ModuleDeviceConfigurator *dc, int *pos)
{
	*pos = dc->default_node->position;
	
	return DEVICE_CONF_SUCCESS;    	
}


int device_conf_load_active_devices_into_list_control(ModuleDeviceConfigurator *dc, int panel, int ctrl, int device_pos)
{
	int i, size, count = 0, default_index = 1;
	CMDeviceNode *tmp = NULL;
	
	if(ClearListCtrl (panel, ctrl) < 0)
		return DEVICE_CONF_ERROR;
		
	size = ListNumItems(dc->in_use_list);
	
	if(size < 1)
		return DEVICE_CONF_ERROR; 	
	
	if(size > 1)
		ListQuickSort (dc->in_use_list, node_sort); 
		
	for(i=1; i <= size; i++) {
	
		tmp = ListGetPtrToItem(dc->in_use_list, i);  

		if(tmp->position == device_pos)
			default_index = i;	
		
		// Position starts at 1
		InsertListItem (panel, ctrl, count++, tmp->name, tmp->position);  
	}
	
	SetCtrlIndex(panel, ctrl, default_index-1);     
	
	return DEVICE_CONF_SUCCESS;
}

ListType device_conf_get_devices_in_use_list(ModuleDeviceConfigurator *dc)
{
	return dc->in_use_list;
}

// Items in list are sorted by pos
int device_conf_set_active_devices_list_control_to_pos(ModuleDeviceConfigurator *dc, int panel, int ctrl, int device_pos)
{
	int i, size, default_index = 0; // was_panel_dimmed = 0;
	CMDeviceNode *tmp = NULL;
		
	//GetPanelAttribute (panel, ATTR_DIMMED, &was_panel_dimmed);

	size = ListNumItems(dc->in_use_list);
	
	if(size < 1)
		return DEVICE_CONF_ERROR; 	

	for(i=1; i <= size; i++) {
	
		tmp = ListGetPtrToItem(dc->in_use_list, i);  

		if(tmp->position == device_pos)
			default_index = i - 1;	
	}
	
	// This seems to undimmed a control if the panel is dimmed, can't seem to undim.
	SetCtrlIndex(panel, ctrl, default_index);     
	
	//if(was_panel_dimmed) {
	//	SetPanelAttribute(panel, ATTR_DIMMED, 0);
	//	SetPanelAttribute(panel, ATTR_DIMMED, 1);
	//	ProcessSystemEvents();
	//}

	return DEVICE_CONF_SUCCESS;
}

int  device_conf_get_num_defined_devices(ModuleDeviceConfigurator *dc)
{
	return ListNumItems(dc->in_use_list) + ListNumItems(dc->not_in_use_list);	
}

int  device_conf_get_num_active_devices(ModuleDeviceConfigurator *dc)
{
	return ListNumItems(dc->in_use_list);	
}

CMDeviceNode * device_conf_get_node_for_selected_item(ModuleDeviceConfigurator *dc)
{
	CMDeviceNode *tmp = NULL;  
	
	if(device_conf_get_device_node_for_id(dc, &tmp, dc->active_tree_id) == DEVICE_CONF_ERROR)
		return NULL;
	
	return tmp;  
}


int device_conf_get_device_node_for_id(ModuleDeviceConfigurator *dc, CMDeviceNode **tmp, int id)
{
	// Search in use list and not in use list for device id.
	
	int i, size = ListNumItems(dc->not_in_use_list);
	CMDeviceNode *node = NULL;
	
	for(i=1; i <= size; i++) {
		
		node = ListGetPtrToItem (dc->not_in_use_list, i);

		if(node->id == id) {
			*tmp = node;
			return DEVICE_CONF_SUCCESS;
		}
	}
	
	size = ListNumItems(dc->in_use_list);
	
	for(i=1; i <= size; i++) {
		
		node = ListGetPtrToItem (dc->in_use_list, i);

		if(node->id == id) {
			*tmp = node;
			return DEVICE_CONF_SUCCESS;
		}
	}
	
	return DEVICE_CONF_ERROR;  
}


int device_conf_get_device_name_for_id(ModuleDeviceConfigurator *dc, char *name, int id)
{
	CMDeviceNode *node = NULL;

	return device_conf_get_device_node_for_id(dc, &node, id);
}

int device_conf_remove_node_for_selected_item(ModuleDeviceConfigurator *dc)
{
	if(device_conf_remove_device_node_for_id(dc, dc->active_tree_id) == DEVICE_CONF_ERROR)
		return DEVICE_CONF_ERROR;
	
	return DEVICE_CONF_SUCCESS;  
}

int device_conf_remove_device_node_for_id(ModuleDeviceConfigurator *dc, int id)
{
	// Search in use list and not in use list for device id.
	
	int i, size = ListNumItems(dc->not_in_use_list);
	CMDeviceNode *node = NULL;
	
	for(i=1; i <= size; i++) {
		
		node = ListGetPtrToItem (dc->not_in_use_list, i);

		if(node->id == id) {
			ListRemoveItem(dc->not_in_use_list, NULL, i);
			goto SUCCESS;    
		}
	}
	
	size = ListNumItems(dc->in_use_list);
	
	for(i=1; i <= size; i++) {
		
		node = ListGetPtrToItem (dc->in_use_list, i);

		if(node->id == id) {
			ListRemoveItem(dc->in_use_list, NULL, i);          
			goto SUCCESS;
		}
	}
	
	return DEVICE_CONF_ERROR;
	
	SUCCESS:
	
		// Update display 
		device_conf_display_used_devices_in_list(dc);
		device_conf_display_not_used_devices_in_list(dc); 
	
		return DEVICE_CONF_SUCCESS;   
}



void device_conf_update_display(ModuleDeviceConfigurator *dc)
{
	device_conf_display_used_devices_in_list(dc);
	device_conf_display_not_used_devices_in_list(dc); 
}

void device_conf_display_local_panel (ModuleDeviceConfigurator *dc, int panel)
{
	int top, left;
	
	GetPanelAttribute(dc->config_panel_id, ATTR_LEFT, &left);
	GetPanelAttribute(dc->config_panel_id, ATTR_TOP,  &top);
	
	SetPanelAttribute(panel, ATTR_TOP,  top+10);
	SetPanelAttribute(panel, ATTR_LEFT, left+10);
	
	GCI_Signal_Emit_From_MainThread(UIMODULE_SIGNAL_TABLE(dc), "EditAddPanelDisplayed", GCI_VOID_POINTER, dc);

	DisplayPanel(panel);
}
	
CMDeviceNode* device_conf_get_node_at_position(ModuleDeviceConfigurator *dc, int pos)
{
	int i, size = ListNumItems(dc->in_use_list);
	CMDeviceNode *tmp = NULL;  
	
	for(i=1; i <= size; i++) {
		
		tmp = ListGetPtrToItem(dc->in_use_list, i);
		
		if(tmp->position == pos)
			return tmp;
	}
	
	return NULL;  
}

int device_conf_get_device_name_for_pos(ModuleDeviceConfigurator *dc, char *name, int pos)
{
	CMDeviceNode *node = device_conf_get_node_at_position(dc, pos);

	if(node == NULL)
		return DEVICE_CONF_ERROR;

	strncpy(name, node->name, UIMODULE_NAME_LEN);
	
	return DEVICE_CONF_SUCCESS;
}

// Return an array of the active cubes
CMDeviceNode* device_conf_get_active_device_array(ModuleDeviceConfigurator *dc, size_t device_size, size_t *size)
{
	unsigned int i;
	CMDeviceNode *device_array = NULL;
	CMDeviceNode *tmp = NULL;    
	
	*size = device_conf_get_num_active_devices(dc);
	
	if(*size < 1)
		return NULL;
	
	device_array = (CMDeviceNode *) malloc(device_size * *size);

	for(i=1; i <= *size; i++) {
	
		tmp = ListGetPtrToItem(dc->in_use_list, i);    

		memcpy( device_array + ((i-1) * device_size), tmp->device, device_size);
	}
	
	return device_array;	
}
