#include "fluorescent_cubes.h"
#include "fluorescent_cubes_ui.h"
#include "gci_utils.h"
#include "string_utils.h"

#include <utility.h>
#include <userint.h>
#include "asynctmr.h"


////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Fluorescent cube control.
////////////////////////////////////////////////////////////////////////////

int CVICALLBACK OnFluorCubeClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:

			//SetAsyncTimerAttribute (cube_manager->_timer, ASYNC_ATTR_ENABLED,  0);
			
			cube_manager_hide_main_ui (cube_manager);
			cube_manager_hide_config_ui (cube_manager);
			
			break;
		}
	return 0;
}


int CVICALLBACK OnCubeSetup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*) callbackData;  
	
	switch (event)
		{
		case EVENT_COMMIT:
			
			cube_manager_display_config_ui(cube_manager); 

			break;
		}
	return 0;
}

int CVICALLBACK OnCubeAddEditOkClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*) callbackData;  
	int index, add_or_edit;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_ADDOREDIT, &add_or_edit);
			
			// If 1 we edit
			if(add_or_edit)
				GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_INDEX, &index);  
			else 
				index = cube_manager_add_cube(cube_manager);

			cube_manager_edit_cube(cube_manager, index);     
			break;
		}
	return 0;
}

int CVICALLBACK OnCubeConfigCloseClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*)callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			if (cube_manager->_data_modified == 1) {
				if (!ConfirmPopup("Warning", "Exit without saving changes?")) 
					break;
			}

			cube_manager->_data_modified = 0;
			HidePanel(cube_manager->_cube_table_panel);
			//cube_manager->_cube_table_panel = -1;
			break;
		}
	return 0;
}

int CVICALLBACK OnCubeFileSave (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char path[500], fname[500];
	FluoCubeManager* cube_manager = (FluoCubeManager*)callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(cube_manager->_cube_table_panel, CUBE_CONF_DEFAULT, &cube_manager->_default_pos);
			GetProjectDir (path);
			strcat(path, "\\Microscope Data");
			if (LessLameFileSelectPopup (cube_manager->_cube_table_panel, path, "CubeData.xml", "*.xml",
							 "Cube Backup", VAL_SAVE_BUTTON, 0, 1, 1, 0, fname) > 0) {
				cube_manager_save_cube_data(cube_manager, fname);
				cube_manager->_data_modified = 0;
			}
			
			break;
		}
	return 0;
}

int CVICALLBACK OnCubeFileRecall (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char path[500], fname[500];
	FluoCubeManager* cube_manager = (FluoCubeManager*)callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetProjectDir (path);
			strcat(path, "\\Microscope Data");
			if (LessLameFileSelectPopup (cube_manager->_cube_table_panel, path, "CubeData.xml", "*.xml",
							 "Cubes", VAL_OK_BUTTON, 0, 1, 1, 0, fname) == 1) {
				cube_manager_load_cube_file(cube_manager, fname);
				SetCtrlVal(cube_manager->_cube_table_panel, CUBE_CONF_DEFAULT, cube_manager->_default_pos);
				cube_manager->_data_modified = 0;
			}
			
			break;
		}
	return 0;
}

int CVICALLBACK OnCubeDownArrow (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*)callbackData;
	int index, id, below_id, number_of_items;
	FluoCube *cube1, *cube2;

	switch (event)
		{
		case EVENT_COMMIT:
			GetNumListItems (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, &number_of_items);

			if(number_of_items < 1)
				return 0;

			GetActiveTreeItem (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, &index);
		
			// We have already selected the bottom entry
			if(index >= cube_manager->number_of_present_cubes - 1)  {
			
				if(GetValueFromIndex (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, index, &id) < 0)
					return -1;
				cube1 = cube_manager_get_cube_ptr_for_id(cube_manager, id);
				//cube_manager_get_cube(cube_manager, id, cube1);
				if (cube1->position < CUBE_TURRET_SIZE) {
					cube1->position ++;	
					cube_manager_load_all_possible_cubes_into_ui(cube_manager); // Update Main UI   
				}
				
				SetActiveTreeItem (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, cube_manager->number_of_present_cubes - 1, VAL_TOGGLE_SELECTION_OF_ITEM);
				SetActiveCtrl (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE);
				cube_manager->_data_modified = 1;
				return 0;
			}
		
			if(GetValueFromIndex (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, index, &id) < 0)
				return -1;
				
			// Get the id of the item below the selected one.
			if(GetValueFromIndex (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, index + 1, &below_id) < 0)
				return -1;
				
			//cube_manager_get_cube(cube_manager, id, cube1);
			//cube_manager_get_cube(cube_manager, below_id, cube2);
			cube1 = cube_manager_get_cube_ptr_for_id(cube_manager, id);
			cube2 = cube_manager_get_cube_ptr_for_id(cube_manager, below_id);
			if (cube2->position == cube1->position+1) {
				cube_manager_switch_active_position(cube_manager, below_id, id); 
				SetActiveTreeItem (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, index + 1, VAL_TOGGLE_SELECTION_OF_ITEM);
			}
			else {
				cube1->position ++;	
				cube_manager_load_all_possible_cubes_into_ui(cube_manager); // Update Main UI   
				SetActiveTreeItem (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, index, VAL_TOGGLE_SELECTION_OF_ITEM);
			}

			SetActiveCtrl (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE);
			cube_manager->_data_modified = 1;
			break;
		}
	return 0;
}

int CVICALLBACK OnCubeUpArrow (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*)callbackData;
	int index, id, above_id, number_of_items;
	FluoCube *cube1, *cube2;

	switch (event)
		{
		case EVENT_COMMIT:
			GetNumListItems (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, &number_of_items);

			if(number_of_items < 1)
				return 0;

			GetActiveTreeItem (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, &index);
		
			// We have already selected the top entry
			if(index < 1)  {
			
				if(GetValueFromIndex (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, index, &id) < 0)
					return -1;
				cube1 = cube_manager_get_cube_ptr_for_id(cube_manager, id);
				if (cube1->position > 1) {
					cube1->position --;	
					cube_manager_load_all_possible_cubes_into_ui(cube_manager); // Update Main UI   
				}
				
				SetActiveTreeItem (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, 0, VAL_TOGGLE_SELECTION_OF_ITEM);
				SetActiveCtrl (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE);

				cube_manager->_data_modified = 1;
				
				return 0;
			}
		
			if(GetValueFromIndex (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, index, &id) < 0)
				return -1;
				
			// Get the id of the item above the selected one.
			if(GetValueFromIndex (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, index - 1, &above_id) < 0)
				return -1;
				
			cube1 = cube_manager_get_cube_ptr_for_id(cube_manager, id);
			cube2 = cube_manager_get_cube_ptr_for_id(cube_manager, above_id);
			//cube_manager_get_cube(cube_manager, id, cube1);
			//cube_manager_get_cube(cube_manager, above_id, cube2);
			if (cube2->position == cube1->position-1) {
				cube_manager_switch_active_position(cube_manager, above_id, id); 
				SetActiveTreeItem (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, index - 1, VAL_TOGGLE_SELECTION_OF_ITEM);
			}
			else {
				cube1->position --;	
				cube_manager_load_all_possible_cubes_into_ui(cube_manager); // Update Main UI   
				SetActiveTreeItem (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, index, VAL_TOGGLE_SELECTION_OF_ITEM);
			}

			cube_manager->_data_modified = 1;
			
			SetActiveCtrl (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE);
			break;
		}
	return 0;
}

int CVICALLBACK OnCubeRightArrow (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*)callbackData;
	int index, id;

	switch (event)
		{
		case EVENT_COMMIT:
			GetActiveTreeItem (cube_manager->_cube_table_panel, CUBE_CONF_ALL_TREE, &index);
		
			if(index < 0)
				return 0;
		
			if(GetValueFromIndex (cube_manager->_cube_table_panel, CUBE_CONF_ALL_TREE, index, &id) < 0)
				return -1;
				
			cube_manager_change_to_active(cube_manager, id); 

			break;
		}
	return 0;
}

int CVICALLBACK OnCubeLeftArrow (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*)callbackData;
	int index, id;

	switch (event)
		{
		case EVENT_COMMIT:
			GetActiveTreeItem (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, &index);
		
			if(index < 0)
				return 0;
		
			if(GetValueFromIndex (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, index, &id) < 0)
				return -1;
				
			cube_manager_remove_obj_at_active_position(cube_manager, id); 
			
			break;
		}
	return 0;
}

int CVICALLBACK OnCubeDetailsAdd (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*)callbackData;

	switch (event)
		{
		case EVENT_COMMIT:

			cube_manager_add_cube_ui(cube_manager); 
			break;
		}
	return 0;
}

int CVICALLBACK OnCubeRemoveClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*)callbackData;
	int index, id;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetActiveTreeItem (cube_manager->_cube_table_panel, CUBE_CONF_ALL_TREE, &index);
		
			if(index < 0)
				return 0;
		
			if(GetValueFromIndex (cube_manager->_cube_table_panel, CUBE_CONF_ALL_TREE, index, &id) < 0)
				return -1;
				
			cube_manager_remove_cube(cube_manager, id);
			
			cube_manager_load_all_possible_cubes_into_ui(cube_manager);
			break;
		}
	return 0;
}

int CVICALLBACK OnCubeDetailsEdit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*)callbackData;
	int index, id, count; 

	switch (event)
		{
		case EVENT_COMMIT:

			GetNumListItems (cube_manager->_cube_table_panel, CUBE_CONF_ALL_TREE, &count);

			if(!count)
				return -1;

			GetActiveTreeItem (cube_manager->_cube_table_panel, CUBE_CONF_ALL_TREE, &index);

			if(GetValueFromIndex (cube_manager->_cube_table_panel, CUBE_CONF_ALL_TREE, index, &id) < 0)
				return -1;

			cube_manager_edit_cube_ui(cube_manager, id); 
			break;
		}
	return 0;
}

int CVICALLBACK OnCubeTreeValueChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	//FluoCubeManager* cube_manager = (FluoCubeManager*)callbackData;

	switch (event)
		{
		case EVENT_COMMIT:

			//cube_manager->_active_tree_ctrl = control;

			break;
		}
	return 0;
}

int CVICALLBACK OnCubeEditActive (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	FluoCubeManager* cube_manager = (FluoCubeManager*)callbackData;
	int index, id, count; 

	switch (event)
		{
		case EVENT_COMMIT:
			GetNumListItems (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, &count);
			if(!count)
				return -1;

			GetActiveTreeItem (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, &index);

			if(GetValueFromIndex (cube_manager->_cube_table_panel, CUBE_CONF_ACTIVE_TREE, index, &id) < 0)
				return -1;

			cube_manager_edit_cube_ui(cube_manager, id); 
			break;
		}
	return 0;
}

int CVICALLBACK OnCubeChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	FluoCubeManager* manager = (FluoCubeManager*)callbackData;
	int index, count; 
	
	switch (event)
		{
		case EVENT_COMMIT:

			GetNumListItems (manager->_main_ui_panel, CUBE_INFO_TURRET_POS, &count);

			if(!count)
				return -1;

			GetCtrlAttribute (manager->_main_ui_panel, CUBE_INFO_TURRET_POS, ATTR_CTRL_INDEX, &index);

			cube_manager_move_to_position(manager, index+1);
			
			break;
		}
	return 0;
}
