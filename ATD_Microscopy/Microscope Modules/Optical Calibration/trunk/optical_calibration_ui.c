#include "optical_calibration.h"
#include "optical_calibration_uir.h" 
#include <userint.h>

int CVICALLBACK OnSaveClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			optical_calibration* cal = (optical_calibration*) callbackData;   
			optical_calibration_write_data(cal);    

			break;
		}
	}
	
	return 0;
}


int CVICALLBACK OnCalibrationSet (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int id, index;
			unsigned int pixel_len;
			double um;
			char factor_str[500] = "";

			optical_calibration* cal = (optical_calibration*) callbackData;         
			
			GetActiveTreeItem(UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CAL_TREE, &index);
			
			if(index < 0)
				return 0;

			GetCtrlAttribute(UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_UM_NUMERIC, ATTR_CTRL_VAL, &um);

			if(GetValueFromIndex (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CAL_TREE, index, &id) < 0)
				return 0;
			
			if(um > 0.0) {
			
				GetCtrlAttribute(UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_PIXEL_NUMERIC, ATTR_CTRL_VAL, &pixel_len);
		
				sprintf(factor_str, "%f", um / pixel_len); 
			
				if(SetTreeCellAttribute (UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CAL_TREE, index, 1, ATTR_LABEL_TEXT, factor_str))
					return -1;  

				cal->changed = 1;
				GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(cal), "CalibrationChanged", GCI_VOID_POINTER, cal);  
			}

			break;
		}
	}
		
	return 0;
}


int CVICALLBACK OnOkClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			optical_calibration* cal = (optical_calibration*) callbackData;     
			
			//optical_calibration_destroy(cal);      
			optical_calibration_hide(cal);
			
			break;
		}
	}
		
	return 0;
}


// This function is called when someone clicks or double
// clicks on the table rows.
int CVICALLBACK OnCalTreeEvent (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_VAL_CHANGED:
		{
			int count, id;
			CMDeviceNode *node = NULL;     
	
			// This function changes the objective on the hardware
			// when the user selects a particular row.
			optical_calibration* cal = (optical_calibration*) callbackData;        
	
			GetNumListItems (panel, control, &count);
				
			if(count < 1)
				return -1;
			
			GetActiveTreeItem (panel, control, &cal->selected_index);
			
			if(GetValueFromIndex (UIMODULE_MAIN_PANEL_ID(cal), control, cal->selected_index, &id) < 0)
				return -1;
			
			if(device_conf_get_device_node_for_id(cal->objective_manager->dc, &node, id) == DEVICE_CONF_ERROR)   
				return OBJ_CALIBRATION_ERROR; 
			
			if(objective_manager_move_to_position(cal->objective_manager, node->position) == OBJECTIVE_MANAGER_ERROR)  
				return 0;
        
			break;
		}
	}
    			
    return 0;
}

// This function is called when someone clicks or double
// clicks on the table rows.
/*
int CVICALLBACK OnCalibrationDeviceChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_VAL_CHANGED:
		{
			int device_index;
			OpticalCalibrationDevice* device = NULL;

			optical_calibration* cal = (optical_calibration*) callbackData;        

			// Get the first calibration device by default.
			GetCtrlIndex(UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CALIBRATION_DEV_RING, &device_index);

			cal->device = optical_calibration_device_at_index(device_index);

			optical_calibration_display_panel(cal);

			break;
		}
	}
    			
    return 0;
}
*/

int CVICALLBACK OnCancelClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			optical_calibration* cal = (optical_calibration*) callbackData;          
			
			optical_calibration_hide(cal);     

			break;
		}
	}
	
	return 0;
}


int CVICALLBACK OnNumericChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			optical_calibration* cal = (optical_calibration*) callbackData;     
			
			SetActiveCtrl(UIMODULE_MAIN_PANEL_ID(cal), CAL_PANEL_CAL_TREE);     
			SetActiveTreeItem (panel, CAL_PANEL_CAL_TREE, cal->selected_index, VAL_REPLACE_SELECTION_WITH_ITEM);   
			
			break;
		}
	}
	return 0;
}
