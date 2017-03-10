#include "HardWareTypes.h"

#include "CubeSlider.h"
#include "CubeSliderUI.h"
#include "device_list.h"
#include "gci_utils.h"
#include "string_utils.h"

#include <utility.h>
#include <userint.h>
#include "asynctmr.h"

int CVICALLBACK OnCubeChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			FluoCubeManager* cube_manager = (FluoCubeManager*)callbackData;
			int pos = -1;
			CMDeviceNode *node = NULL;
	
			GetCtrlVal(panel, control, &pos);
			
			cube_manager_move_to_position(cube_manager, pos);
			
			}break;
		}
	
	return 0;
}


int CVICALLBACK OnFluorCubeClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			FluoCubeManager* cube_manager = (FluoCubeManager*) callbackData;

			if(device_conf_close_config_panel(cube_manager->dc) < 0)
				return -1;

			ui_module_hide_all_panels(UIMODULE_CAST(cube_manager));

			}break;
		}
	return 0;
}


int CVICALLBACK OnCubeSetup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			FluoCubeManager* cube_manager = (FluoCubeManager*) callbackData;  
	
			ui_module_attach_panel_to_panel(UIMODULE_CAST(cube_manager),
														  device_conf_get_panel_id(cube_manager->dc),
														  UI_MODULE_REL_TOP_RIGHT,
														  5, 0);

			device_conf_display_panel(cube_manager->dc);    

			}break;
		}
	return 0;
}

int CVICALLBACK OnCubeAddEditOkClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			
			FluoCubeManager* cube_manager = (FluoCubeManager*) callbackData;  
			int add_or_edit, id;
			FluoCube *cube;
			char name[50];
	
			GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_ADDOREDIT, &add_or_edit);    
			
			
			// If 1 we edit - If 0 we are adding
			if(add_or_edit) {
				
				CMDeviceNode *node;       
				
				GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_ID, &id);   
				
				if(device_conf_get_device_node_for_id(cube_manager->dc, &node, id) == DEVICE_CONF_ERROR)
					return -1;

				cube = (FluoCube *) node->device;
				
				GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_NAME, node->name);
				GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EXCMIN, &(cube->exc_min_nm)); 
				GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EXCMAX, &(cube->exc_max_nm)); 
				GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_DICHROICNM, &(cube->dichroic_nm)); 
				GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EMMIN, &(cube->emm_min_nm)); 
				GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EMMAX, &(cube->emm_max_nm)); 
				
			}
			else {
				
				// Adding 
				
				cube = malloc(sizeof(FluoCube));   
			
				GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_NAME, name);
				GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EXCMIN, &(cube->exc_min_nm)); 
				GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EXCMAX, &(cube->exc_max_nm)); 
				GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_DICHROICNM, &(cube->dichroic_nm)); 
				GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EMMIN, &(cube->emm_min_nm)); 
				GetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EMMAX, &(cube->emm_max_nm)); 
				
				device_conf_add_device(DEVICE_CONF_CAST(cube_manager->dc), cube, name, -1);    
			}
			
			HidePanel(cube_manager->_details_ui_panel);
			
			device_conf_update_display(cube_manager->dc);   
			
			}break;
		}
	return 0;
}

int CVICALLBACK OnCubeDetailsAdd (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			FluoCubeManager* cube_manager = (FluoCubeManager*)callbackData;

			SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_ADDOREDIT, 0);
			SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_NAME, "");
			SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EXCMIN, 0 );
			SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EXCMAX, 0 );
			SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_DICHROICNM, 0);
			SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EMMIN, 0);
			SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EMMAX, 0);
			
//			DisplayPanel(cube_manager->_details_ui_panel);
			device_conf_display_local_panel(cube_manager->dc, cube_manager->_details_ui_panel);
				
			device_conf_update_display(cube_manager->dc);   
			
			}break;
		}
	return 0;
}


int CVICALLBACK OnCubeDetailsEdit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
		{
			FluoCubeManager* cube_manager = (FluoCubeManager*)callbackData;
			CMDeviceNode *node;
			FluoCube *cube;
	
			node =  device_conf_get_node_for_selected_item(cube_manager->dc);
			cube = (FluoCube *) node->device;
			
			SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_ID, node->id); 
			SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_ADDOREDIT, 1);  // We are editing so set to 1
			SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_NAME, node->name);
			SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EXCMIN, cube->exc_min_nm );
			SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EXCMAX, cube->exc_max_nm );
			SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_DICHROICNM, cube->dichroic_nm);
			SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EMMIN, cube->emm_min_nm);
			SetCtrlVal(cube_manager->_details_ui_panel, EDIT_PANEL_EMMAX, cube->emm_max_nm);

//			DisplayPanel(cube_manager->_details_ui_panel);
			device_conf_display_local_panel(cube_manager->dc, cube_manager->_details_ui_panel);
	
			device_conf_update_display(cube_manager->dc);   
			
			}break;
		}
	}
		
	return 0;
}
