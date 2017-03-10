#include "HardWareTypes.h"

#include "objectives.h"
#include "ObjectivesUI.h"
#include "device_list.h"
#include "gci_utils.h"
#include "string_utils.h"
#include <utility.h>
#include <ansi_c.h>
#include <userint.h>

	
int CVICALLBACK OnObjectivesCloseClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			ObjectiveManager* objective_manager = (ObjectiveManager*) callbackData;   
	
			if(device_conf_close_config_panel(objective_manager->dc) < 0)
				return -1;

			ui_module_hide_all_panels(UIMODULE_CAST(objective_manager));

			}break;
		}
	return 0;
}


int CVICALLBACK OnObjectivesConfig (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			
			ObjectiveManager* objective_manager = (ObjectiveManager*) callbackData;   
	
			ui_module_attach_panel_to_panel(UIMODULE_CAST(objective_manager),
														  device_conf_get_panel_id(objective_manager->dc),
														  UI_MODULE_REL_TOP_RIGHT,
														  5, 0);

			device_conf_display_panel(objective_manager->dc);    

			}break;
		}
	return 0;
}	


int CVICALLBACK OnObjectiveChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			ObjectiveManager* objective_manager = (ObjectiveManager*) callbackData;     
			int pos;
			CMDeviceNode *node = NULL;
	
			GetCtrlVal(panel, control, &pos);
			
			//device_conf_get_device_node_for_id(objective_manager->dc, &node, id);
			
			objective_manager_move_to_position(objective_manager, pos);
			
			}break;
		}
	
	return 0;
}


int CVICALLBACK OnObjectiveClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			ObjectiveManager* objective_manager = (ObjectiveManager*) callbackData;      
	

			ui_module_hide_all_panels(UIMODULE_CAST(objective_manager));
			
			}break;
		}
	return 0;
}


int CVICALLBACK OnObjectiveSetup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			
			ObjectiveManager* objective_manager = (ObjectiveManager*) callbackData;   
	
			device_conf_display_panel(objective_manager->dc);    

			}break;
		}
	return 0;
}

int CVICALLBACK OnObjectivesAddEditOkClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			
			ObjectiveManager* objective_manager = (ObjectiveManager*) callbackData;    
			int add_or_edit, id;
			Objective *objective;
			char name[50];
	
			GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_ADDOREDIT, &add_or_edit);    
			
			
			// If 1 we edit - If 0 we are adding
			if(add_or_edit) {
				
				CMDeviceNode *node;       
				
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_ID, &id);   
				
				if(device_conf_get_device_node_for_id(objective_manager->dc, &node, id) == DEVICE_CONF_ERROR)
					return -1;

				objective = (Objective *) node->device;
				
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_NAME, node->name);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_MEDIUM_RING, objective->_objective_medium);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_NUM_APERTURE, objective->_numerical_aperture);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_MAGNIFICATION, objective->_magnification_str);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_WORKING_DIST, objective->_working_distance);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_BACK_APERTURE, objective->_back_aperture);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_FOCUS_POSITION, objective->_focus_position);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_ILLUMINATION, objective->_illumination);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_APERTURE_STOP, objective->_aperture_stop);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_FIELD_STOP, objective->_field_stop);  
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_CONDENSER, objective->_condenser);  
			}
			else {
				
				// Adding 
				
				objective = malloc(sizeof(Objective));   
			
				objective->_calibrations = NULL;

				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_NAME, name);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_MEDIUM_RING, objective->_objective_medium);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_NUM_APERTURE, objective->_numerical_aperture);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_MAGNIFICATION, objective->_magnification_str);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_WORKING_DIST, objective->_working_distance);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_BACK_APERTURE, objective->_back_aperture);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_FOCUS_POSITION, objective->_focus_position);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_ILLUMINATION, objective->_illumination);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_APERTURE_STOP, objective->_aperture_stop);
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_FIELD_STOP, objective->_field_stop);  
				GetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_CONDENSER, objective->_condenser);  
			
				device_conf_add_device(DEVICE_CONF_CAST(objective_manager->dc), objective, name, -1);    
			}
			
			HidePanel(objective_manager->_details_ui_panel);
			
			device_conf_update_display(objective_manager->dc);   
			
			}break;
		}
	return 0;
}


int CVICALLBACK OnObjectivesDetailsAdd (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			ObjectiveManager* objective_manager = (ObjectiveManager*) callbackData;

			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_ADDOREDIT, 0);
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_NAME, "");
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_MEDIUM_RING, "Unknown Medium");
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_NUM_APERTURE, "0.0");
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_MAGNIFICATION, "Unknown Magnification");
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_WORKING_DIST, "0.0");
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_BACK_APERTURE, "0.0");
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_FOCUS_POSITION, "0.0");
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_ILLUMINATION, "0.0");
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_APERTURE_STOP, "0.0");
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_FIELD_STOP, "0.0");  
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_CONDENSER, "Unknown Condensor"); 
			
//			DisplayPanel(objective_manager->_details_ui_panel);
			device_conf_display_local_panel(objective_manager->dc, objective_manager->_details_ui_panel);
			
			device_conf_update_display(objective_manager->dc);   
			
			}break;
		}
	return 0;
}


int CVICALLBACK OnObjectivesDetailsEdit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
		{
			ObjectiveManager* objective_manager = (ObjectiveManager*) callbackData;       
			CMDeviceNode *node;
			Objective *objective;
	
			node =  device_conf_get_node_for_selected_item(objective_manager->dc);
			objective = (Objective *) node->device;
			
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_ID, node->id); 
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_ADDOREDIT, 1);  // We are editing so set to 1
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_NAME, node->name);
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_MEDIUM_RING, objective->_objective_medium);
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_NUM_APERTURE, objective->_numerical_aperture);
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_MAGNIFICATION, objective->_magnification_str);
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_WORKING_DIST, objective->_working_distance);
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_BACK_APERTURE, objective->_back_aperture);
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_FOCUS_POSITION, objective->_focus_position);
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_ILLUMINATION, objective->_illumination);
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_APERTURE_STOP, objective->_aperture_stop);
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_FIELD_STOP, objective->_field_stop);  
			SetCtrlVal(objective_manager->_details_ui_panel, EDIT_PANEL_CONDENSER, objective->_condenser);  
			
//			DisplayPanel(objective_manager->_details_ui_panel);
			device_conf_display_local_panel(objective_manager->dc, objective_manager->_details_ui_panel);
	
			device_conf_update_display(objective_manager->dc);   
			
			}break;
		}
	}
		
	return 0;
}
