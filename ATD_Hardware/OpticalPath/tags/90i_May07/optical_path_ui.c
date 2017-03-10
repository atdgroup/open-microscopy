#include "optical_path.h"
#include "optical_path_ui.h"
#include "gci_utils.h"
#include "string_utils.h"

#include <utility.h>
#include <userint.h>
#include "asynctmr.h"


////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Optical path control.
////////////////////////////////////////////////////////////////////////////

int CVICALLBACK OnOpticalPathClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	OpticalPathManager* optical_path_manager = (OpticalPathManager*) callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:

			//SetAsyncTimerAttribute (optical_path_manager->_timer, ASYNC_ATTR_ENABLED,  0);
			
			optical_path_manager_hide_main_ui(optical_path_manager);
			optical_path_manager_hide_config_ui(optical_path_manager);
			GCI_Signal_Emit(&(optical_path_manager->signal_table), "Close", GCI_VOID_POINTER, optical_path_manager);
			
			break;
		}
	return 0;
}


int CVICALLBACK OnOpticalPathSetup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	OpticalPathManager* optical_path_manager = (OpticalPathManager*) callbackData;  
	
	switch (event)
		{
		case EVENT_COMMIT:
			
			optical_path_manager_display_config_ui(optical_path_manager); 

			break;
		}
	return 0;
}

int CVICALLBACK OnOpticalPathAddEditOkClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	OpticalPathManager* optical_path_manager = (OpticalPathManager*) callbackData;  
	int index, add_or_edit;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlVal(optical_path_manager->_details_ui_panel, PATH_DETS_ADDOREDIT, &add_or_edit);
			
			// If 1 we edit
			if(add_or_edit)
				GetCtrlVal(optical_path_manager->_details_ui_panel, PATH_DETS_INDEX, &index);  
			else 
				index = optical_path_manager_add_optical_path(optical_path_manager);

			optical_path_manager_edit_optical_path(optical_path_manager, index);     
			break;
		}
	return 0;
}

int CVICALLBACK OnOpticalPathConfigCloseClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	OpticalPathManager* optical_path_manager = (OpticalPathManager*)callbackData;

	switch (event)
		{
		case EVENT_COMMIT:
			if (optical_path_manager->_data_modified == 1) {
				if (!ConfirmPopup("Warning", "Exit without saving changes?")) 
					break;
			}

			optical_path_manager->_data_modified = 0;
			HidePanel(optical_path_manager->_optical_path_table_panel);
			break;
		}
	return 0;
}

int CVICALLBACK OnOpticalPathFileSave (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char path[500], fname[500];
	OpticalPathManager* optical_path_manager = (OpticalPathManager*)callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetProjectDir (path);
			strcat(path, "\\Microscope Data");
			if (LessLameFileSelectPopup (optical_path_manager->_optical_path_table_panel, path, "OpticalPathData.xml", "*.xml",
							 "OpticalPath Backup", VAL_SAVE_BUTTON, 0, 1, 1, 0, fname) > 0) {
				optical_path_manager_save_optical_path_data(optical_path_manager, fname);
				optical_path_manager->_data_modified = 0;
			}
			
			break;
		}
	return 0;
}

int CVICALLBACK OnOpticalPathFileRecall (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char path[500], fname[500];
	OpticalPathManager* optical_path_manager = (OpticalPathManager*)callbackData;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetProjectDir (path);
			strcat(path, "\\Microscope Data");
			if (LessLameFileSelectPopup (optical_path_manager->_optical_path_table_panel, path, "OpticalPathData.xml", "*.xml",
							 "OpticalPath Recall", VAL_OK_BUTTON, 0, 1, 1, 0, fname) == 1) {
				optical_path_manager_load_optical_path_file(optical_path_manager, fname);
				optical_path_manager->_data_modified = 0;
			}
			
			break;
		}
	return 0;
}

int CVICALLBACK OnOpticalPathDownArrow (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	OpticalPathManager* optical_path_manager = (OpticalPathManager*)callbackData;
	int index, id, below_id, number_of_items;
	OpticalPath *optical_path1, *optical_path2;

	switch (event)
		{
		case EVENT_COMMIT:
			GetNumListItems (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, &number_of_items);

			if(number_of_items < 1)
				return 0;

			GetActiveTreeItem (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, &index);
		
			// We have already selected the bottom entry
			if(index >= optical_path_manager->number_of_present_optical_paths - 1)  {
			
				if(GetValueFromIndex (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, index, &id) < 0)
					return -1;
				optical_path1 = optical_path_manager_get_optical_path_ptr_for_id(optical_path_manager, id);
				if (optical_path1->position < OPTICAL_PATH_TURRET_SIZE) {
					optical_path1->position ++;	
					optical_path_manager_load_all_possible_optical_paths_into_ui(optical_path_manager); // Update Main UI   
				}
				
				SetActiveTreeItem (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, optical_path_manager->number_of_present_optical_paths - 1, VAL_TOGGLE_SELECTION_OF_ITEM);
				SetActiveCtrl (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE);
				optical_path_manager->_data_modified = 1;
				return 0;
			}
		
			if(GetValueFromIndex (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, index, &id) < 0)
				return -1;
				
			// Get the id of the item below the selected one.
			if(GetValueFromIndex (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, index + 1, &below_id) < 0)
				return -1;
				
			optical_path1 = optical_path_manager_get_optical_path_ptr_for_id(optical_path_manager, id);
			optical_path2 = optical_path_manager_get_optical_path_ptr_for_id(optical_path_manager, below_id);
			if (optical_path2->position == optical_path1->position+1) {
				optical_path_manager_switch_active_position(optical_path_manager, below_id, id); 
				SetActiveTreeItem (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, index + 1, VAL_TOGGLE_SELECTION_OF_ITEM);
			}
			else {
				optical_path1->position ++;	
				optical_path_manager_load_all_possible_optical_paths_into_ui(optical_path_manager); // Update Main UI   
				SetActiveTreeItem (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, index, VAL_TOGGLE_SELECTION_OF_ITEM);
			}

			SetActiveCtrl (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE);
			optical_path_manager->_data_modified = 1;
			break;
		}
	return 0;
}

int CVICALLBACK OnOpticalPathUpArrow (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	OpticalPathManager* optical_path_manager = (OpticalPathManager*)callbackData;
	int index, id, above_id, number_of_items;
	OpticalPath *optical_path1, *optical_path2;

	switch (event)
		{
		case EVENT_COMMIT:
			GetNumListItems (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, &number_of_items);

			if(number_of_items < 1)
				return 0;

			GetActiveTreeItem (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, &index);
		
			// We have already selected the top entry
			if(index < 1)  {
			
				if(GetValueFromIndex (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, index, &id) < 0)
					return -1;
				optical_path1 = optical_path_manager_get_optical_path_ptr_for_id(optical_path_manager, id);
				if (optical_path1->position > 1) {
					optical_path1->position --;	
					optical_path_manager_load_all_possible_optical_paths_into_ui(optical_path_manager); // Update Main UI   
				}
				
				SetActiveTreeItem (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, 0, VAL_TOGGLE_SELECTION_OF_ITEM);
				SetActiveCtrl (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE);

				optical_path_manager->_data_modified = 1;
				
				return 0;
			}
		
			if(GetValueFromIndex (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, index, &id) < 0)
				return -1;
				
			// Get the id of the item above the selected one.
			if(GetValueFromIndex (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, index - 1, &above_id) < 0)
				return -1;
				
			optical_path1 = optical_path_manager_get_optical_path_ptr_for_id(optical_path_manager, id);
			optical_path2 = optical_path_manager_get_optical_path_ptr_for_id(optical_path_manager, above_id);
			if (optical_path2->position == optical_path1->position-1) {
				optical_path_manager_switch_active_position(optical_path_manager, above_id, id); 
				SetActiveTreeItem (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, index - 1, VAL_TOGGLE_SELECTION_OF_ITEM);
			}
			else {
				optical_path1->position --;	
				optical_path_manager_load_all_possible_optical_paths_into_ui(optical_path_manager); // Update Main UI   
				SetActiveTreeItem (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, index, VAL_TOGGLE_SELECTION_OF_ITEM);
			}

			optical_path_manager->_data_modified = 1;
			
			SetActiveCtrl (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE);
			break;
		}
	return 0;
}

int CVICALLBACK OnOpticalPathRightArrow (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	OpticalPathManager* optical_path_manager = (OpticalPathManager*)callbackData;
	int index, id;

	switch (event)
		{
		case EVENT_COMMIT:
			GetActiveTreeItem (optical_path_manager->_optical_path_table_panel, PATH_CONF_ALL_TREE, &index);
		
			if(index < 0)
				return 0;
		
			if(GetValueFromIndex (optical_path_manager->_optical_path_table_panel, PATH_CONF_ALL_TREE, index, &id) < 0)
				return -1;
				
			optical_path_manager_change_to_active(optical_path_manager, id); 

			break;
		}
	return 0;
}

int CVICALLBACK OnOpticalPathLeftArrow (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	OpticalPathManager* optical_path_manager = (OpticalPathManager*)callbackData;
	int index, id;

	switch (event)
		{
		case EVENT_COMMIT:
			GetActiveTreeItem (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, &index);
		
			if(index < 0)
				return 0;
		
			if(GetValueFromIndex (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, index, &id) < 0)
				return -1;
				
			optical_path_manager_remove_optical_path_at_active_position(optical_path_manager, id); 
			
			break;
		}
	return 0;
}

int CVICALLBACK OnOpticalPathDetailsAdd (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	OpticalPathManager* optical_path_manager = (OpticalPathManager*)callbackData;

	switch (event)
		{
		case EVENT_COMMIT:

			optical_path_manager_add_optical_path_ui(optical_path_manager); 
			break;
		}
	return 0;
}

int CVICALLBACK OnOpticalPathRemoveClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	OpticalPathManager* optical_path_manager = (OpticalPathManager*)callbackData;
	int index, id;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetActiveTreeItem (optical_path_manager->_optical_path_table_panel, PATH_CONF_ALL_TREE, &index);
		
			if(index < 0)
				return 0;
		
			if(GetValueFromIndex (optical_path_manager->_optical_path_table_panel, PATH_CONF_ALL_TREE, index, &id) < 0)
				return -1;
				
			optical_path_manager_remove_optical_path(optical_path_manager, id);
			
			optical_path_manager_load_all_possible_optical_paths_into_ui(optical_path_manager);
			break;
		}
	return 0;
}

int CVICALLBACK OnOpticalPathDetailsEdit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	OpticalPathManager* optical_path_manager = (OpticalPathManager*)callbackData;
	int index, id, count; 

	switch (event)
		{
		case EVENT_COMMIT:

			GetNumListItems (optical_path_manager->_optical_path_table_panel, PATH_CONF_ALL_TREE, &count);

			if(!count)
				return -1;

			GetActiveTreeItem (optical_path_manager->_optical_path_table_panel, PATH_CONF_ALL_TREE, &index);

			if(GetValueFromIndex (optical_path_manager->_optical_path_table_panel, PATH_CONF_ALL_TREE, index, &id) < 0)
				return -1;

			optical_path_manager_edit_optical_path_ui(optical_path_manager, id); 
			break;
		}
	return 0;
}

int CVICALLBACK OnOpticalPathTreeValueChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	//OpticalPathManager* optical_path_manager = (OpticalPathManager*)callbackData;

	switch (event)
		{
		case EVENT_COMMIT:

			//optical_path_manager->_active_tree_ctrl = control;

			break;
		}
	return 0;
}

int CVICALLBACK OnOpticalPathEditActive (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	OpticalPathManager* optical_path_manager = (OpticalPathManager*)callbackData;
	int index, id, count; 

	switch (event)
		{
		case EVENT_COMMIT:
			GetNumListItems (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, &count);
			if(!count)
				return -1;

			GetActiveTreeItem (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, &index);

			if(GetValueFromIndex (optical_path_manager->_optical_path_table_panel, PATH_CONF_ACTIVE_TREE, index, &id) < 0)
				return -1;

			optical_path_manager_edit_optical_path_ui(optical_path_manager, id); 
			break;
		}
	return 0;
}

int CVICALLBACK OnOpticalPathChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	OpticalPathManager* manager = (OpticalPathManager*)callbackData;
	int index, count; 
	
	switch (event)
		{
		case EVENT_COMMIT:

			GetNumListItems (manager->_main_ui_panel, OPTIC_PATH_TURRET_POS, &count);

			if(!count)
				return -1;

			GetCtrlAttribute (manager->_main_ui_panel, OPTIC_PATH_TURRET_POS, ATTR_CTRL_INDEX, &index);

			optical_path_manager_move_to_position(manager, index+1);
			
			break;
		}
	return 0;
}
