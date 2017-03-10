#include "HardWareTypes.h"

#include "condensers.h"
#include "condensers_ui.h"
#include "device_list.h"
#include "gci_utils.h"
#include "string_utils.h"

#include <utility.h>
#include <userint.h>
#include "asynctmr.h"

int CVICALLBACK OnCondenserChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			CondenserManager* condensor_manager = (CondenserManager*)callbackData;
			int pos = -1;
			CMDeviceNode *node = NULL;
	
			GetCtrlVal(panel, control, &pos);
			
			condenser_manager_move_to_position(condensor_manager, pos);
			
			}break;
		}
	
	return 0;
}


int CVICALLBACK OnCondenserClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			CondenserManager* condensor_manager = (CondenserManager*) callbackData;
	
			ui_module_hide_all_panels(UIMODULE_CAST(condensor_manager));
			
			}break;
		}
	return 0;
}


int CVICALLBACK OnCondenserSetup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			CondenserManager* condensor_manager = (CondenserManager*) callbackData;  
	
			
			device_conf_display_panel(condensor_manager->dc);    

			}break;
		}
	return 0;
}

int CVICALLBACK OnCondenserAddEditOkClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			
			CondenserManager* condensor_manager = (CondenserManager*) callbackData;  
			int add_or_edit, id;
			Condenser *condenser;
			char name[50];
	
			GetCtrlVal(condensor_manager->_details_ui_panel, EDIT_PANEL_ADDOREDIT, &add_or_edit);    
			
			// If 1 we edit - If 0 we are adding
			if(add_or_edit) {
				
				CMDeviceNode *node;       
				
				GetCtrlVal(condensor_manager->_details_ui_panel, EDIT_PANEL_ID, &id);   
				
				if(device_conf_get_device_node_for_id(condensor_manager->dc, &node, id) == DEVICE_CONF_ERROR)
					return -1;

				condenser = (Condenser *) node->device;
				
				GetCtrlVal(condensor_manager->_details_ui_panel, EDIT_PANEL_NAME, node->name);
			}
			else {
				
				// Adding 
				
				condenser = malloc(sizeof(Condenser));   
			
				GetCtrlVal(condensor_manager->_details_ui_panel, EDIT_PANEL_NAME, name);
			
				device_conf_add_device(DEVICE_CONF_CAST(condensor_manager->dc), condenser, name, -1);    
			}
			
			HidePanel(condensor_manager->_details_ui_panel);
			
			device_conf_update_display(condensor_manager->dc);   
			
			}break;
		}
	return 0;
}

int CVICALLBACK OnCondenserDetailsAdd (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			CondenserManager* condensor_manager = (CondenserManager*)callbackData;

			SetCtrlVal(condensor_manager->_details_ui_panel, EDIT_PANEL_ADDOREDIT, 0);
			SetCtrlVal(condensor_manager->_details_ui_panel, EDIT_PANEL_NAME, "");
			
			device_conf_display_local_panel(condensor_manager->dc, condensor_manager->_details_ui_panel);
				
			device_conf_update_display(condensor_manager->dc);   
			
			}break;
		}
	return 0;
}


int CVICALLBACK OnCondenserDetailsEdit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
		{
			CondenserManager* condensor_manager = (CondenserManager*)callbackData;
			CMDeviceNode *node;
			Condenser *condenser;
	
			node =  device_conf_get_node_for_selected_item(condensor_manager->dc);
			condenser = (Condenser *) node->device;
			
			SetCtrlVal(condensor_manager->_details_ui_panel, EDIT_PANEL_ID, node->id); 
			SetCtrlVal(condensor_manager->_details_ui_panel, EDIT_PANEL_ADDOREDIT, 1);  // We are editing so set to 1
			SetCtrlVal(condensor_manager->_details_ui_panel, EDIT_PANEL_NAME, node->name);
			
			device_conf_display_local_panel(condensor_manager->dc, condensor_manager->_details_ui_panel);
	
			device_conf_update_display(condensor_manager->dc);   
			
			}break;
		}
	}
		
	return 0;
}
