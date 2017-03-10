#include "HardWareTypes.h"

#include "OpticalPath.h"
#include "OpticalPathUI.h"
#include "device_list.h"
#include "gci_utils.h"
#include "string_utils.h"

#include <utility.h>
#include <userint.h>
#include "asynctmr.h"

int CVICALLBACK OnOpticalPathCalibrate (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			OpticalPathManager* optical_path_manager = (OpticalPathManager*) callbackData;  
	
			optical_path_manager_display_calib_ui(optical_path_manager); 

			break;
		}
	}
	return 0;
}

int CVICALLBACK OnOpticalPathChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			OpticalPathManager* optical_path_manager = (OpticalPathManager*)callbackData;
			int pos = -1;
			CMDeviceNode *node = NULL;
	
			GetCtrlVal(panel, control, &pos);

			optical_path_move_to_position(optical_path_manager, pos);
			
			}break;
		}
	
	return 0;
}


int CVICALLBACK OnOpticalPathClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			OpticalPathManager* optical_path_manager = (OpticalPathManager*) callbackData;
	
			if(device_conf_close_config_panel(optical_path_manager->dc) < 0)
				return -1;

			ui_module_hide_all_panels(UIMODULE_CAST(optical_path_manager));

			}break;
		}
	return 0;
}


int CVICALLBACK OnOpticalPathSetup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int device_config_panel_id;

			OpticalPathManager* optical_path_manager = (OpticalPathManager*) callbackData;  

			device_config_panel_id = device_conf_get_panel_id(optical_path_manager->dc);

			ui_module_attach_panel_to_panel(UIMODULE_CAST(optical_path_manager),
														  device_config_panel_id,
														  UI_MODULE_REL_TOP_RIGHT,
														  5, 0);

			device_conf_display_panel(optical_path_manager->dc);      

			break;
		}
	}

	return 0;
}

int CVICALLBACK OnOpticalPathAddEditOkClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			OpticalPathManager* optical_path_manager = (OpticalPathManager*) callbackData;  
			int add_or_edit, id;
			char name[50];
	
			
			GetCtrlVal(optical_path_manager->_details_ui_panel, PATH_EDIT_ADDOREDIT, &add_or_edit);    
				
			// If 1 we edit - If 0 we are adding
			if(add_or_edit) {
				
				CMDeviceNode *node;       
				
				GetCtrlVal(optical_path_manager->_details_ui_panel, PATH_EDIT_ID, &id);   
				
				if(device_conf_get_device_node_for_id(optical_path_manager->dc, &node, id) == DEVICE_CONF_ERROR)
					return -1;

				GetCtrlVal(optical_path_manager->_details_ui_panel, PATH_EDIT_NAME, node->name);
				
			}
			else {
				
				// Adding 
				GetCtrlVal(optical_path_manager->_details_ui_panel, PATH_EDIT_NAME, name);
	
				device_conf_add_device(DEVICE_CONF_CAST(optical_path_manager->dc), NULL, name, -1);    
			}
			
			HidePanel(optical_path_manager->_details_ui_panel);
			
			device_conf_update_display(optical_path_manager->dc);   
		
			}break;
		}
	return 0;
}


int CVICALLBACK OnOpticalPathDetailsAdd (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			OpticalPathManager* optical_path_manager = (OpticalPathManager*)callbackData;

			SetCtrlVal(optical_path_manager->_details_ui_panel, PATH_EDIT_ADDOREDIT, 0);
			SetCtrlVal(optical_path_manager->_details_ui_panel, PATH_EDIT_NAME, "");
			
//			DisplayPanel(optical_path_manager->_details_ui_panel);
			device_conf_display_local_panel(optical_path_manager->dc, optical_path_manager->_details_ui_panel);
			
			device_conf_update_display(optical_path_manager->dc);   
			
			}break;
		}
	return 0;
}


int CVICALLBACK OnOpticalPathDetailsEdit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
		{
			OpticalPathManager* optical_path_manager = (OpticalPathManager*)callbackData;
			CMDeviceNode *node;
	
			node =  device_conf_get_node_for_selected_item(optical_path_manager->dc);

			SetCtrlVal(optical_path_manager->_details_ui_panel, PATH_EDIT_ID, node->id); 
			SetCtrlVal(optical_path_manager->_details_ui_panel, PATH_EDIT_ADDOREDIT, 1);  // We are editing so set to 1
			SetCtrlVal(optical_path_manager->_details_ui_panel, PATH_EDIT_NAME, node->name);
		
//			DisplayPanel(optical_path_manager->_details_ui_panel);
			device_conf_display_local_panel(optical_path_manager->dc, optical_path_manager->_details_ui_panel);
	
			device_conf_update_display(optical_path_manager->dc);    
			
			}break;
		}
	}
		
	return 0;
}
/*
int CVICALLBACK OnOpticalPathInit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
		{
			OpticalPathManager* optical_path_manager = (OpticalPathManager*) callbackData;  
		
			optical_path_initialise(optical_path_manager, 1);
			
			}break;
		}
	}
	return 0;
}
*/
