#include "HardWareTypes.h"

#include "PmtSet.h"
#include "PmtSetUI.h"
#include "device_list.h"
#include "gci_utils.h"
#include "string_utils.h"

#include <utility.h>
#include <userint.h>
#include "asynctmr.h"

int CVICALLBACK OnPmtSetCalibrate (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			PmtSet* pmtset = (PmtSet*) callbackData;  
	
			pmtset_display_calib_ui(pmtset); 

			break;
		}
	}
	return 0;
}

int CVICALLBACK OnPmtSetChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			PmtSet* pmtset = (PmtSet*)callbackData;
			int pos = -1;
			CMDeviceNode *node = NULL;
	
			GetCtrlVal(panel, control, &pos);

			pmtset_move_to_position(pmtset, pos);
			
			}break;
		}
	
	return 0;
}


int CVICALLBACK OnPmtSetClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			PmtSet* pmtset = (PmtSet*) callbackData;
	
			if(device_conf_close_config_panel(pmtset->dc) < 0)
				return -1;

			ui_module_hide_all_panels(UIMODULE_CAST(pmtset));

			}break;
		}
	return 0;
}


int CVICALLBACK OnPmtSetSetup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			PmtSet* pmtset = (PmtSet*) callbackData;  

			ui_module_attach_panel_to_panel(UIMODULE_CAST(pmtset),
														  device_conf_get_panel_id(pmtset->dc),
														  UI_MODULE_REL_TOP_RIGHT,
														  5, 0);


			device_conf_display_panel(pmtset->dc);    

			break;
		}
	}

	return 0;
}

int CVICALLBACK OnPmtSetAddEditOkClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			PmtSet* pmtset = (PmtSet*) callbackData;  
			int add_or_edit, id;
			char name[50];
	
			
			GetCtrlVal(pmtset->_details_ui_panel, PATH_EDIT_ADDOREDIT, &add_or_edit);    
				
			// If 1 we edit - If 0 we are adding
			if(add_or_edit) {
				
				CMDeviceNode *node;       
				
				GetCtrlVal(pmtset->_details_ui_panel, PATH_EDIT_ID, &id);   
				
				if(device_conf_get_device_node_for_id(pmtset->dc, &node, id) == DEVICE_CONF_ERROR)
					return -1;

				GetCtrlVal(pmtset->_details_ui_panel, PATH_EDIT_NAME, node->name);
				
			}
			else {
				
				// Adding 
				GetCtrlVal(pmtset->_details_ui_panel, PATH_EDIT_NAME, name);
	
				device_conf_add_device(DEVICE_CONF_CAST(pmtset->dc), NULL, name, -1);    
			}
			
			HidePanel(pmtset->_details_ui_panel);
			
			device_conf_update_display(pmtset->dc);   
		
			}break;
		}
	return 0;
}


int CVICALLBACK OnPmtSetDetailsAdd (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			PmtSet* pmtset = (PmtSet*)callbackData;

			SetCtrlVal(pmtset->_details_ui_panel, PATH_EDIT_ADDOREDIT, 0);
			SetCtrlVal(pmtset->_details_ui_panel, PATH_EDIT_NAME, "");
			
//			DisplayPanel(pmtset->_details_ui_panel);
			device_conf_display_local_panel(pmtset->dc, pmtset->_details_ui_panel);
			
			device_conf_update_display(pmtset->dc);   
			
			}break;
		}
	return 0;
}


int CVICALLBACK OnPmtSetDetailsEdit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
		{
			PmtSet* pmtset = (PmtSet*)callbackData;
			CMDeviceNode *node;
	
			node =  device_conf_get_node_for_selected_item(pmtset->dc);

			SetCtrlVal(pmtset->_details_ui_panel, PATH_EDIT_ID, node->id); 
			SetCtrlVal(pmtset->_details_ui_panel, PATH_EDIT_ADDOREDIT, 1);  // We are editing so set to 1
			SetCtrlVal(pmtset->_details_ui_panel, PATH_EDIT_NAME, node->name);
		
//			DisplayPanel(pmtset->_details_ui_panel);
			device_conf_display_local_panel(pmtset->dc, pmtset->_details_ui_panel);
	
			device_conf_update_display(pmtset->dc);    
			
			}break;
		}
	}
		
	return 0;
}
/*
int CVICALLBACK OnPmtSetInit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
		{
			PmtSet* pmtset = (PmtSet*) callbackData;  
		
			pmtset_initialise(pmtset, 1);
			
			}break;
		}
	}
	return 0;
}
*/
