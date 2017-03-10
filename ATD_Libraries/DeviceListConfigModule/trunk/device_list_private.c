#include "device_list.h"
#include "device_list_private.h"
#include "device_list_ui.h"
#include "string_utils.h"
#include "gci_utils.h"

#include "toolbox.h"
#include <userint.h>
#include <utility.h>
#include <ansi_c.h> 



/*

static int dc_copy_objective(ObjectiveManager* dc, Objective* src, Objective* obj)
{
	obj->_id = src->_id;
	obj->_active = src->_active;
	obj->_turret_position = src->_turret_position;
	
	strcpy(obj->_objective_name, src->_objective_name);
	strcpy(obj->_objective_medium, src->_objective_medium);
	strcpy(obj->_magnification_str, src->_magnification_str);
	strcpy(obj->_condenser, src->_condenser);
			 
	obj->_numerical_aperture = src->_numerical_aperture;
	obj->_working_distance = src->_working_distance;
	obj->_back_aperture = src->_back_aperture;
	obj->_focus_position = src->_focus_position;
	obj->_illumination = src->_illumination;
	obj->_aperture_stop = src->_aperture_stop;
	obj->_field_stop = src->_field_stop;
	
	return dc_SUCCESS;
}


Objective* dc_get_objective_ptr_for_turret_position(ObjectiveManager* dc, int position)
{
	int i, size;
	Objective *obj;
	
	size = ListNumItems (dc->_list);
	
	for(i=1; i <= size; i++) {
	
		obj = ListGetPtrToItem (dc->_list, i);

		if(obj->_turret_position == position)
			return obj;	
		
	}
	
	return NULL;
}




int CVICALLBACK active_tree_sort_cmp(int panel, int control, int item1, int item2, int keyCol, void *callbackData)
{
	ObjectiveManager* dc = (ObjectiveManager*) callbackData;
	int objective_id1;
	int objective_id2;
	Objective *objective1;
	Objective *objective2; 
	
	GetValueFromIndex (panel, control, item1, &objective_id1);
	GetValueFromIndex (panel, control, item2, &objective_id2);
	
	objective1 = dc_get_objective_ptr_for_id(dc, objective_id1); 
	objective2 = dc_get_objective_ptr_for_id(dc, objective_id2); 
	
	return objective1->_turret_position - objective2->_turret_position;
}





static int dc_load_active_objectives_into_main_list(ObjectiveManager* dc)
{
	int i, size;
	Objective objective;
	
	if(ClearListCtrl (dc->_main_ui_panel, OBJ_PANEL_TURRET_POS) < 0)
		return DEVICE_CONF_ERROR;
		
	for(i=1; i <= TURRET_SIZE; i++) 
		InsertListItem (dc->_main_ui_panel, OBJ_PANEL_TURRET_POS, -1, "empty", -1);
	
	if(dc->_number_of_objectives_in_turret > 1)
		ListQuickSort (dc->_list, sort_by_turret); 
		
	size = ListNumItems (dc->_list);
	
	for(i=1; i <= size; i++) {
	
		ListGetItem (dc->_list, &objective, i);

		if(objective._active)
			ReplaceListItem (dc->_main_ui_panel, OBJ_PANEL_TURRET_POS, objective._turret_position-1, objective._objective_name, objective._id);
	}
	
	return dc_SUCCESS;
}




static int find_first_free_turret_pos(ObjectiveManager* dc)
{
	int i, size, active_pos = 1;
	Objective *objective;
	
	ListQuickSort (dc->_list, sort_by_turret);

	size = ListNumItems (dc->_list);
	
	for(i = 1; i <= size; i++) {     
	
		objective = ListGetPtrToItem (dc->_list, i); 
		
		if(!objective->_active)
			continue;
			
		if(objective->_turret_position != active_pos)
			break;
			
		active_pos++;
	}
	
	if(active_pos <= TURRET_SIZE)
		return active_pos;

	return 0;
}


int dc_switch_turret_position(ObjectiveManager* dc, int id1, int id2)
{
	int temp_turret_pos;
	Objective *objective1, *objective2;
	
	objective1 = dc_get_objective_ptr_for_id(dc, id1);    
	objective2 = dc_get_objective_ptr_for_id(dc, id2);
		
	temp_turret_pos = objective1->_turret_position; 
	objective1->_turret_position = objective2->_turret_position;
	objective2->_turret_position = temp_turret_pos;
	
	// Update Main UI
	dc_load_all_possible_objectives_into_ui(dc);    

	return dc_SUCCESS;  
}


int dc_add_objective_to_turret(ObjectiveManager* dc, int id)
{
	int i, size, turret_pos;
	int panel, pnl, ctrl;
	Objective *objective;

	//Insert an objective. Ask for an unused position to be entered.
	
	if(dc->_number_of_objectives_in_turret >= TURRET_SIZE)
		return DEVICE_CONF_ERROR;	
			  
	panel = FindAndLoadUIR(0, "objectives_ui.uir", INSERT_OBJ);

	size = ListNumItems (dc->_list);
	for(i = 1; i <= size; i++) {     
		objective = ListGetPtrToItem (dc->_list, i); 
		
		if(!objective->_active) continue;
			
		ReplaceListItem (panel, INSERT_OBJ_TURRET_POS, objective->_turret_position-1, "Occupied", -1);
	}
	
	InstallPopup(panel);
	while (1) {
		GetUserEvent (1, &pnl, &ctrl);
		if (pnl == panel) {
			if (ctrl == INSERT_OBJ_CANCEL) {
				DiscardPanel(panel);
				return dc_SUCCESS;
			}
			if (ctrl == INSERT_OBJ_OK) {
				GetCtrlVal(panel, INSERT_OBJ_TURRET_POS, &turret_pos);
				if (turret_pos > 0)
					break;
				GCI_MessagePopup("Error", "That position is already occupied. Please try again.");
			}
		}
	}

	DiscardPanel(panel);
	
	dc->_number_of_objectives_in_turret++;
	
	//turret_pos = find_first_free_turret_pos(dc);

	objective = dc_get_objective_ptr_for_id(dc, id);     

	objective->_active = 1;
	objective->_turret_position = turret_pos;
	
	dc_load_all_possible_objectives_into_ui(dc);
	dc->_data_modified = 1;
	
	return dc_SUCCESS;
}


int dc_add_objective(ObjectiveManager* dc)
{
	int size = ListNumItems (dc->_list);
	Objective obj;
	
	obj._id = ++size;
	obj._active = 0;
	obj._turret_position = 0;
	
	ListInsertItem(dc->_list, &obj, END_OF_LIST);
	
	return obj._id;
}


int dc_get_objective_pos_for_id(ObjectiveManager* dc, int id)
{
	int i, size;
	Objective *obj;
	
	size = ListNumItems (dc->_list);
	
	for(i=1; i <= size; i++) {
	
		obj = ListGetPtrToItem (dc->_list, i);

		if(obj->_id == id)
			return i;	
		
	}
	
	return -1;
}

int dc_remove_objective(ObjectiveManager* dc, int id)
{
	Objective objective;
	int index;
	
	index = dc_get_objective_pos_for_id(dc, id);
	if (index < 1) return DEVICE_CONF_ERROR;

	ListGetItem (dc->_list, &objective, index);

	if(objective._active )
		dc->_number_of_objectives_in_turret--; 
			    
	ListRemoveItem (dc->_list, 0, index);       

	dc->_data_modified = 1;

	return dc_SUCCESS;
}





 */





static int CVICALLBACK node_tree_pos_sort_cmp(int panel, int control, int item1, int item2, int keyCol, void *callbackData)
{
	ModuleDeviceConfigurator* dc = (ModuleDeviceConfigurator*) callbackData;

	int id1, id2;
	CMDeviceNode *node1, *node2;
	
	GetValueFromIndex (panel, control, item1, &id1);
	GetValueFromIndex (panel, control, item2, &id2);
	
	if(device_conf_get_device_node_for_id(dc, &node1, id1) == DEVICE_CONF_ERROR)
		return DEVICE_CONF_ERROR; 
	
	if(device_conf_get_device_node_for_id(dc, &node2, id2) == DEVICE_CONF_ERROR)
		return DEVICE_CONF_ERROR;
	
	return node1->position - node2->position;
}


static int CVICALLBACK node_tree_id_sort_cmp(int panel, int control, int item1, int item2, int keyCol, void *callbackData)
{
	ModuleDeviceConfigurator* dc = (ModuleDeviceConfigurator*) callbackData;

	int id1, id2;
	CMDeviceNode *node1, *node2;
	
	GetValueFromIndex (panel, control, item1, &id1);
	GetValueFromIndex (panel, control, item2, &id2);
	
	if(device_conf_get_device_node_for_id(dc, &node1, id1) == DEVICE_CONF_ERROR)
		return DEVICE_CONF_ERROR; 
	
	if(device_conf_get_device_node_for_id(dc, &node2, id2) == DEVICE_CONF_ERROR)
		return DEVICE_CONF_ERROR;
	
	return node1->id - node2->id;
}


// Display all the devices that are not in use in a tree control.
int device_conf_display_not_used_devices_in_list(ModuleDeviceConfigurator *dc)
{
	int i, size;
	CMDeviceNode node;
	
	// Clear the control
	if(ClearListCtrl (dc->config_panel_id, DLIST_CONF_ALL_TREE) < 0)
		return DEVICE_CONF_ERROR;
	
	size = ListNumItems (dc->not_in_use_list);
	
	for(i=1; i <= size; i++) {
	
		ListGetItem (dc->not_in_use_list, &node, i);

		InsertTreeItem (dc->config_panel_id, DLIST_CONF_ALL_TREE, VAL_SIBLING, 0, VAL_NEXT, node.name, 0, 0, node.id);
	}
	
	// Putting nodes into the tree in order does not appear to be enough
	// CVI unsorts them. Why must we use this software.
	if(size > 1)     
		SortTreeItems (dc->config_panel_id, DLIST_CONF_ALL_TREE, 0, 0, 0, 0, node_tree_id_sort_cmp, dc);
	
	
	// If there are no items that are not used. Then dim the right arrow button.
	if(size == 0)
		SetCtrlAttribute(dc->config_panel_id, DLIST_CONF_RIGHT_ARROW, ATTR_DIMMED, 1);
	else
		SetCtrlAttribute(dc->config_panel_id, DLIST_CONF_RIGHT_ARROW, ATTR_DIMMED, 0);     
	
	
	return DEVICE_CONF_SUCCESS; 
}


// Display all the devices that are not in use in a tree control.
int device_conf_display_used_devices_in_list(ModuleDeviceConfigurator *dc)
{
	int i, size, index;
	char pos_text[10];
	CMDeviceNode *node = NULL;
	
	// Clear the control
	if(ClearListCtrl (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE) < 0)
		return DEVICE_CONF_ERROR;
	
	size = ListNumItems (dc->in_use_list);
	
	
	for(i=1; i <= size; i++) {
	
		node = ListGetPtrToItem (dc->in_use_list, i);

		sprintf(pos_text, "%d", node->position); 
			
		index = InsertTreeItem (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, VAL_SIBLING, 0, VAL_NEXT, pos_text, 0, 0, node->id);

		if(SetTreeCellAttribute (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, index, 1, ATTR_LABEL_TEXT, node->name)) {
				
			return DEVICE_CONF_ERROR;  
		}
	}
	
	// Putting nodes into the tree in order does not appear to be enough
	// CVI unsorts them. Why must we use this software.
	if(size > 1)
		SortTreeItems (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, 0, 0, 0, 0, node_tree_pos_sort_cmp, dc);
	
	// If there are no items that are not used. Then dim the left arrow button.
	if(size == 0)
		SetCtrlAttribute(dc->config_panel_id, DLIST_CONF_LEFT_ARROW, ATTR_DIMMED, 1);
	else
		SetCtrlAttribute(dc->config_panel_id, DLIST_CONF_LEFT_ARROW, ATTR_DIMMED, 0);     
	
	return DEVICE_CONF_SUCCESS; 
}

static int device_conf_get_list_index_for_node_from_in_use_list(ModuleDeviceConfigurator *dc, CMDeviceNode *node)
{
	int i;
	CMDeviceNode *tmp = NULL;

	for(i=1; i <= ListNumItems(dc->in_use_list); i++) {
		
		tmp = ListGetPtrToItem(dc->in_use_list, i);
		
		if(tmp->id == node->id)
			return i;
	}

	return -1;
}

static int device_conf_get_list_index_for_node_from_not_in_use_list(ModuleDeviceConfigurator *dc, CMDeviceNode *node)
{
	int i;
	CMDeviceNode *tmp = NULL;

	// Not active
	for(i=1; i <= ListNumItems(dc->not_in_use_list); i++) {
		
		tmp = ListGetPtrToItem(dc->not_in_use_list, i);
		
		if(tmp->id == node->id)
			return i;
	}

	return -1;
}

int device_conf_get_list_index_for_node(ModuleDeviceConfigurator *dc, CMDeviceNode *node)
{
	int i;
	CMDeviceNode *tmp = NULL;
	
	if(node->position == -1) {
	
		// Not active
		for(i=1; i <= ListNumItems(dc->not_in_use_list); i++) {
		
			tmp = ListGetPtrToItem(dc->not_in_use_list, i);
		
			if(tmp->id == node->id)
				return i;
		}
	}
	else {
		
		for(i=1; i <= ListNumItems(dc->in_use_list); i++) {
		
			tmp = ListGetPtrToItem(dc->in_use_list, i);
		
			if(tmp->id == node->id)
				return i;
		}
	}
	
	return -1;
}


int device_conf_set_device_pos(ModuleDeviceConfigurator *dc, int id, int pos)
{
	CMDeviceNode *tmp = NULL, node;
	int i, index;
	
	// Get the device node for the id
	
	// No device with that id
	if(device_conf_get_device_node_for_id(dc, &tmp, id) == DEVICE_CONF_ERROR)
		return DEVICE_CONF_ERROR;
	
	// Find the position in the two list that the node is at so we can remove it.
	//index = device_conf_get_list_index_for_node(dc, tmp);
	
	// If pos is -1 then we are removing the device from the in_use list
	if(pos == -1) {
		
		// Find the position in the two list that the node is at so we can remove it.
		index = device_conf_get_list_index_for_node_from_in_use_list(dc, tmp);

		if(index < 0)
			return DEVICE_CONF_ERROR;

		ListRemoveItem(dc->in_use_list, &node, index);
		node.position = -1;
		
		ListInsertItem(dc->not_in_use_list, &node, END_OF_LIST);  
		
		// Update display
		device_conf_display_used_devices_in_list(dc);
		device_conf_display_not_used_devices_in_list(dc);
		
		return DEVICE_CONF_SUCCESS;
	}
	
	// Ok we are trying to add the device to the in use list.
	
	// Find the position in the two list that the node is at so we can remove it.
	index = device_conf_get_list_index_for_node_from_not_in_use_list(dc, tmp);

	if(index < 0)
		return DEVICE_CONF_ERROR;

	// Are we allowed to add anymore to the active list ?
	if(ListNumItems(dc->in_use_list) >= dc->max_in_use_allowed_devices)
		return DEVICE_CONF_ERROR;
	
	// Check whether the position is occupied ?
	for(i=1; i <= ListNumItems(dc->in_use_list); i++) {
		
		tmp = ListGetPtrToItem(dc->in_use_list, i);
		
		if(tmp->position == pos)
			return DEVICE_CONF_ERROR;
	}
	
	ListRemoveItem(dc->not_in_use_list, &node, index);

	// Changed node position data
	node.position = pos;
	
	// Ok lets add the device to the in use list.
	ListInsertInOrder (dc->in_use_list, &node, node_sort);  
	
	// Update display 
	device_conf_display_used_devices_in_list(dc);
	device_conf_display_not_used_devices_in_list(dc); 
	
	return DEVICE_CONF_SUCCESS; 
}


int device_conf_swap_pos_in_use_devices(ModuleDeviceConfigurator *dc, int first_id, int second_id)
{
	CMDeviceNode *node1 = NULL, *node2 = NULL;
	int tmp;
	
	// Make sure the two ids are in the in use list
	
	if(device_conf_get_device_node_for_id(dc, &node1, first_id) == DEVICE_CONF_ERROR)
		return DEVICE_CONF_ERROR;
	
	if(device_conf_get_device_node_for_id(dc, &node2, second_id) == DEVICE_CONF_ERROR)
		return DEVICE_CONF_ERROR;
	
	if(node1->position == -1 || node2->position == -1)
		return DEVICE_CONF_ERROR;	
	
	// Change positions.
	tmp = node2->position;
	node2->position = node1->position;
	node1->position = tmp;
	
	device_conf_display_used_devices_in_list(dc);
	
	return DEVICE_CONF_SUCCESS;   
}
