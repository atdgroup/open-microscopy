#include "device_list.h"
#include "device_list_private.h"
#include "device_list_ui.h" 
#include "gci_utils.h"
#include "string_utils.h"
#include <utility.h>
#include <ansi_c.h>
#include <userint.h>


static int IsPosPossible(ModuleDeviceConfigurator* dc, int pos)
{
	int i;
	CMDeviceNode *tmp = NULL;  
	
	for(i=1; i <= ListNumItems(dc->in_use_list); i++) {
		
		tmp = ListGetPtrToItem(dc->in_use_list, i);
		
		if(tmp->position == pos)
			return 1;
	}
	
	return 0;  
}


static int GetDefaultNode(ModuleDeviceConfigurator* dc)
{
	int default_pos;
			
	GetCtrlVal(dc->config_panel_id, DLIST_CONF_DEFAULT, &default_pos);
			
	// Is the default position actually allowed.
	if(!IsPosPossible(dc, default_pos)) {
			
		GCI_MessagePopup("Warning", "The default position chosen does not seem possible.");
				
		// Lets set the default pos to the first availiable
		dc->default_node = (CMDeviceNode *) ListGetPtrToItem(dc->in_use_list, 1);
		default_pos = 1;
	}
	else {
		
		dc->default_node = device_conf_get_node_at_position(dc, default_pos);
		
		if(dc->default_node == NULL) {
		
			GCI_MessagePopup("Warning", "The default position chosen does not seem possible.");
			return DEVICE_CONF_ERROR;
		}
	}		
		
	SetCtrlVal(dc->config_panel_id, DLIST_CONF_DEFAULT, default_pos);
		
	return DEVICE_CONF_SUCCESS;
}


int CVICALLBACK OnDeviceListLeftArrow (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	// We are making an in use device (ie has a position) no in use (set position = -1)
	switch (event)
	{
		case EVENT_COMMIT:{
			
			ModuleDeviceConfigurator* dc = (ModuleDeviceConfigurator*) callbackData;
			int found_id;
	
			if(device_conf_set_device_pos(dc, dc->active_tree_id, -1) == DEVICE_CONF_ERROR)
				return -1;
			
			// Un used devices should be sorted by id in the tree control
			// (They have no position)
			if(GetTreeItemFromValue (dc->config_panel_id, DLIST_CONF_ALL_TREE, VAL_ALL, 0, 0, VAL_NEXT_PLUS_SELF, 0, &found_id, dc->active_tree_id) < 0)
				return -1;
				
				
			if(found_id == -1) {
					
				printf("It is claimed id %d does not exist in all list\n", dc->active_tree_id);	
			}
				
			SetActiveTreeItem (dc->config_panel_id, DLIST_CONF_ALL_TREE, found_id, VAL_REPLACE_SELECTION_WITH_ITEM);		
			SetActiveCtrl(dc->config_panel_id, DLIST_CONF_ALL_TREE); 

			//SetCtrlAttribute(dc->config_panel_id, DLIST_CONF_LEFT_ARROW, ATTR_DIMMED, 1);
			//SetCtrlAttribute(dc->config_panel_id, DLIST_CONF_RIGHT_ARROW, ATTR_DIMMED, 0);

			dc->data_modified = 1;       
			
			}break;
	}
	
	return 0;
}



int CVICALLBACK OnDeviceListRightArrow (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	// Ok we are going to make a device active.
	
	switch (event)
	{
		case EVENT_COMMIT:{

			ModuleDeviceConfigurator* dc = (ModuleDeviceConfigurator*) callbackData;
			int i, j, count = 0, position_taken, found_id, panel_id, pnl, ctrl, pos;
			char full_path[GCI_MAX_PATHNAME_LEN] = "";
			char buf[15];
			CMDeviceNode *node;
	
			if(ListNumItems(dc->in_use_list) >= dc->max_in_use_allowed_devices) {
				GCI_MessagePopup("Error", "Maximum number of devices already in use");
				return -1;
			}

			if(find_resource("device_list_ui.uir", full_path) < 0) {
				ui_module_send_error(UIMODULE_CAST(dc), "Uir load error", "Can not find file %s", full_path);
				return -1;  
			}
	
			// Pop up a dialog displaying the allowed positions.
			panel_id = LoadPanel(0, full_path, INSERT_DEV);
			
			// Fill the list box with the availiable positions
			for(i=1; i <= dc->max_in_use_allowed_devices; i++) {
				
				position_taken = 0;
				
				// Ok this search is n*n but we should have very small lists.
				for(j=1; j <= ListNumItems(dc->in_use_list); j++) {
					
					node = ListGetPtrToItem(dc->in_use_list, j);
					
					if(node->position == i)
						position_taken = 1;	
				}
				
				if(!position_taken) {
					sprintf(buf, "Position %d", i);
				
					if(InsertListItem (panel_id, INSERT_DEV_POS, count++, buf, i) < 0)
						return -1;
				}
				
			}

			InstallPopup(panel_id);
			
			while (1) {
				
				GetUserEvent (1, &pnl, &ctrl);
		
				if(pnl != panel_id)
					return -1;
				
				// Do nothing - Canceled operation
				if (ctrl == INSERT_DEV_CANCEL) {
					DiscardPanel(panel_id);
					return 0;
				}
					
				if (ctrl == INSERT_DEV_OK) {
					
					GetCtrlVal(panel_id, INSERT_DEV_POS, &pos);
				
					if (pos > 0) {
						DiscardPanel(panel_id);         
						break;
					}
				}
			}
	
			if(device_conf_set_device_pos(dc, dc->active_tree_id, pos) == DEVICE_CONF_ERROR)
				return -1;

			// At what index is the device been sorted to ?		
			if(GetTreeItemFromValue (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, VAL_ALL, 0, VAL_FIRST, VAL_NEXT_PLUS_SELF, 0, &found_id, dc->active_tree_id) < 0)
				return -1;
				
			if(found_id == -1) {
					
				int count;
				int item;
				int id; 
				
				printf("Something has gone wrong\n");
					
				GetNumListItems (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, &count);
					
				GetTreeItem (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, VAL_ALL, 0, VAL_FIRST,
					VAL_NEXT_PLUS_SELF, 0, &item);

				if(item == -1)
					printf("There is a big problem here\n");
				else {
					
					GetValueFromIndex (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, item, &id);

					printf("node id %d \n", id);
				}
			}
				
			SetActiveTreeItem (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, found_id, VAL_REPLACE_SELECTION_WITH_ITEM);
			SetActiveCtrl(dc->config_panel_id, DLIST_CONF_ACTIVE_TREE);  
			
			//i = SetCtrlAttribute(dc->config_panel_id, DLIST_CONF_LEFT_ARROW, ATTR_DIMMED, 0);
			//i = SetCtrlAttribute(dc->config_panel_id, DLIST_CONF_RIGHT_ARROW, ATTR_DIMMED, 1);

			dc->data_modified = 1;       
			
			}break;
	}
	
	return 0;
}



// We are swapping the position of the list item with the one above.
int CVICALLBACK OnDeviceListUpArrow (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			ModuleDeviceConfigurator* dc = (ModuleDeviceConfigurator*) callbackData;
			int index, id, above_id, number_of_items;

			GetNumListItems (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, &number_of_items);

			if(number_of_items < 2)
				return -1;
		
			SetActiveCtrl(dc->config_panel_id, DLIST_CONF_ACTIVE_TREE);   
			GetActiveTreeItem (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, &index);
		
			// We have already selected the top entry
			if(index < 1)
				return -1;
	
			if(GetValueFromIndex (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, index, &id) < 0)
				return -1;
				
			// Get the id of the item above the selected one.
			if(GetValueFromIndex (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, index - 1, &above_id) < 0)
				return -1;
				
			if(device_conf_swap_pos_in_use_devices(dc, id, above_id) == DEVICE_CONF_ERROR)
				return -1;

			SetActiveTreeItem (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, index - 1, VAL_REPLACE_SELECTION_WITH_ITEM);
			
			dc->data_modified = 1;       
			
			}break;
		}
		
	return 0;
}


// We are swapping the position of the list item with the one above.
int CVICALLBACK OnDeviceListDownArrow (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			ModuleDeviceConfigurator* dc = (ModuleDeviceConfigurator*) callbackData;
			int index, id, below_id, number_of_items;

			GetNumListItems (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, &number_of_items);

			if(number_of_items < 2)
				return -1;
		
			SetActiveCtrl(dc->config_panel_id, DLIST_CONF_ACTIVE_TREE);   
			GetActiveTreeItem (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, &index);
		
			// We have already selected the top entry
			if(index + 1 >= ListNumItems(dc->in_use_list))
				return -1;
	
			if(GetValueFromIndex (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, index, &id) < 0)
				return -1;
				
			// Get the id of the item above the selected one.
			if(GetValueFromIndex (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, index + 1, &below_id) < 0)
				return -1;
				
			if(device_conf_swap_pos_in_use_devices(dc, id, below_id) == DEVICE_CONF_ERROR)
				return -1;

			SetActiveTreeItem (dc->config_panel_id, DLIST_CONF_ACTIVE_TREE, index + 1, VAL_REPLACE_SELECTION_WITH_ITEM);
			
			dc->data_modified = 1;       
			
			}break;
		}
		
	return 0;
}


int CVICALLBACK OnDeviceListCloseClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ModuleDeviceConfigurator* dc = (ModuleDeviceConfigurator*) callbackData;
		
			device_conf_close_config_panel(dc);

			break;
		}
	}
	
	return 0;
}



int CVICALLBACK OnDeviceListTreeChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_VAL_CHANGED:
		case EVENT_GOT_FOCUS:
		{
			ModuleDeviceConfigurator* dc = (ModuleDeviceConfigurator*) callbackData; 
			int row_index, count;
	
			if(event == EVENT_GOT_FOCUS) {
				if(control == DLIST_CONF_ALL_TREE) {

					SetCtrlAttribute(panel, DLIST_CONF_LEFT_ARROW, ATTR_DIMMED, 1);
					SetCtrlAttribute(panel, DLIST_CONF_RIGHT_ARROW, ATTR_DIMMED, 0);
					SetCtrlAttribute(panel, DLIST_CONF_UP_ARROW, ATTR_DIMMED, 1);
					SetCtrlAttribute(panel, DLIST_CONF_DOWN_ARROW, ATTR_DIMMED, 1);
				}
				else if(control == DLIST_CONF_ACTIVE_TREE) {

					SetCtrlAttribute(panel, DLIST_CONF_LEFT_ARROW, ATTR_DIMMED, 0);
					SetCtrlAttribute(panel, DLIST_CONF_RIGHT_ARROW, ATTR_DIMMED, 1);
					SetCtrlAttribute(panel, DLIST_CONF_UP_ARROW, ATTR_DIMMED, 0);
					SetCtrlAttribute(panel, DLIST_CONF_DOWN_ARROW, ATTR_DIMMED, 0);
				}
			}

			GetNumListItems (panel, control, &count);
				
			if(count < 1)
				return -1;
			
			if(GetActiveCtrl(panel) != control)
				return -1;
			
			GetActiveTreeItem (panel, control, &row_index);
			
			if(GetValueFromIndex (dc->config_panel_id, control, row_index, &(dc->active_tree_id)) < 0)
				return -1;
			
			SetCtrlAttribute(dc->config_panel_id, DLIST_CONF_EDIT_BUTTON, ATTR_DIMMED, 0); 
			SetCtrlAttribute(dc->config_panel_id, DLIST_CONF_REMOVE_BUTTON, ATTR_DIMMED, 0);  
	
			break;   
		}
	}
	
	return 0;
}

int CVICALLBACK OnDeviceListFileSave (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
		
			ModuleDeviceConfigurator* dc = (ModuleDeviceConfigurator*) callbackData;  
	
			GetDefaultNode(dc);    

			device_conf_save_node_data_for_module(dc);
	
			break;
			}
		}
	return 0;
}

int CVICALLBACK OnDeviceListFileRecall (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			char path[GCI_MAX_PATHNAME_LEN];
			ModuleDeviceConfigurator* dc = (ModuleDeviceConfigurator*) callbackData;  
	
			ui_module_get_data_dir(UIMODULE_CAST(dc), path);
			
			if (LessLameFileSelectPopup (dc->config_panel_id, path, dc->filename, "*.ini",
							 "Load Device Configuration", VAL_OK_BUTTON, 0, 1, 1, 0, path) == 1)
			{
				device_conf_load_node_data(dc, path);  
				
				if(dc->default_node != NULL) {
					SetCtrlVal(dc->config_panel_id, DLIST_CONF_DEFAULT, dc->default_node->position);
				}

				dc->data_modified = 0;
			}

			}break;
		}
	return 0;
}


int CVICALLBACK OnDeviceListRemoveClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
		{
			ModuleDeviceConfigurator* dc = (ModuleDeviceConfigurator*) callbackData;  
			
			device_conf_remove_node_for_selected_item(dc);       

			dc->data_modified = 1;       
			
			}break;
		}
	}
	
	return 0;
}


int CVICALLBACK OnDefaultPosChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
		{
			ModuleDeviceConfigurator* dc = (ModuleDeviceConfigurator*) callbackData; 
			
			GetDefaultNode(dc);     
			
			dc->data_modified = 1;       
			
			}break;
		}
	}
	
	return 0;
}
