#include "HardWareTypes.h"

#include "StagePlate.h"
#include "StagePlateUI.h"
#include "device_list.h"
#include "gci_utils.h"
#include "string_utils.h"

#include <utility.h>
#include <userint.h>
#include "asynctmr.h"

int CVICALLBACK OnStagePlateChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			StagePlateModule* stage_plate_module = (StagePlateModule*)callbackData;
			int pos = -1;
			CMDeviceNode *node = NULL;
	
			GetCtrlVal(panel, control, &pos);
			
			stage_plate_move_to_position(stage_plate_module, pos);
			
			}break;
		}
	
	return 0;
}


int CVICALLBACK OnStagePlateClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			StagePlateModule* stage_plate_module = (StagePlateModule*) callbackData;
			
			if(device_conf_close_config_panel(stage_plate_module->dc) < 0)
				return -1;

			ui_module_hide_all_panels(UIMODULE_CAST(stage_plate_module));
			
			}break;
		}
	return 0;
}


int CVICALLBACK OnStagePlateSetup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			StagePlateModule* stage_plate_module = (StagePlateModule*) callbackData;  
	
			ui_module_attach_panel_to_panel(UIMODULE_CAST(stage_plate_module),
														  device_conf_get_panel_id(stage_plate_module->dc),
														  UI_MODULE_REL_TOP_RIGHT,
														  5, 0);

			device_conf_display_panel(stage_plate_module->dc);    

			}break;
		}
	return 0;
}

int CVICALLBACK OnStagePlateAddEditOkClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			
			StagePlateModule* stage_plate_module = (StagePlateModule*) callbackData;  
			int add_or_edit, id;
			StagePlate *plate;
			char name[50];
	
			GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_ADDOREDIT, &add_or_edit);    
		
			// If 1 we edit - If 0 we are adding
			if(add_or_edit) {
				
				CMDeviceNode *node;       
				
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_ID, &id);   
				
				if(device_conf_get_device_node_for_id(stage_plate_module->dc, &node, id) == DEVICE_CONF_ERROR)
					return -1;

				plate = (StagePlate *) node->device;
				
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_NAME, node->name);
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_TYPE, &(plate->type));
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_ROWS, &(plate->rows)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_COLS, &(plate->cols)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_XSIZE, &(plate->x_size)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_YSIZE, &(plate->y_size)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_XOFFSET, &(plate->x_offset)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_YOFFSET, &(plate->y_offset)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_XSPACING, &(plate->x_spacing)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_YSPACING, &(plate->y_spacing)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_L, &(plate->safe_left_top.x)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_T, &(plate->safe_left_top.y)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_R, &(plate->safe_right_bottom.x)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_B, &(plate->safe_right_bottom.y)); 
			}
			else {
				
				// Adding 
				
				plate = malloc(sizeof(StagePlate));   
			
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_NAME, name);
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_TYPE, &(plate->type));
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_ROWS, &(plate->rows)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_COLS, &(plate->cols)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_XSIZE, &(plate->x_size)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_YSIZE, &(plate->y_size)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_XOFFSET, &(plate->x_offset)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_YOFFSET, &(plate->y_offset)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_XSPACING, &(plate->x_spacing)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_YSPACING, &(plate->y_spacing)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_L, &(plate->safe_left_top.x)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_T, &(plate->safe_left_top.y)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_R, &(plate->safe_right_bottom.x)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_B, &(plate->safe_right_bottom.y)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_SHAPE, &(plate->safe_region_shape)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_CX, &(plate->safe_center.x)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_CY, &(plate->safe_center.y)); 
				GetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_RADIUS, &(plate->safe_radius)); 

				device_conf_add_device(DEVICE_CONF_CAST(stage_plate_module->dc), plate, name, -1);    
			}
			
			HidePanel(stage_plate_module->_details_ui_panel);
			
			device_conf_update_display(stage_plate_module->dc);   
					
			stage_plate_setup_stage_safe_region(stage_plate_module, plate);

			}break;
		}
	return 0;
}

int CVICALLBACK OnStagePlateDetailsAdd (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			StagePlateModule* stage_plate_module = (StagePlateModule*)callbackData;

			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_ADDOREDIT, 0);
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_NAME, "");

			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_TYPE, PLATE_WELLPLATE);
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_ROWS, 1); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_COLS, 1); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_XSIZE, 0.0); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_YSIZE, 0.0); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_XOFFSET, 0.0); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_YOFFSET, 0.0); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_XSPACING, 0.0); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_YSPACING, 0.0); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_L, -20000.0); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_T, -20000.0); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_R, 20000.0); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_B, 20000.0); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_SHAPE, STAGE_SHAPE_RECTANGLE); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_CX, 0.0); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_CY, 0.0); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_RADIUS, 40000.0); 

			device_conf_display_local_panel(stage_plate_module->dc, stage_plate_module->_details_ui_panel);
				
			device_conf_update_display(stage_plate_module->dc);   
			
			}break;
		}
	return 0;
}

int CVICALLBACK OnStagePlateDetailsEdit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
		{
			StagePlateModule* stage_plate_module = (StagePlateModule*)callbackData;
			CMDeviceNode *node;
			StagePlate *plate;
	
			node =  device_conf_get_node_for_selected_item(stage_plate_module->dc);
			plate = (StagePlate *) node->device;
			
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_ID, node->id); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_ADDOREDIT, 1);  // We are editing so set to 1
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_NAME, node->name);
	
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_TYPE, plate->type);
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_ROWS, plate->rows); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_COLS, plate->cols); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_XSIZE, plate->x_size); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_YSIZE, plate->y_size); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_XOFFSET, plate->x_offset); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_YOFFSET, plate->y_offset); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_XSPACING, plate->x_spacing); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_YSPACING, plate->y_spacing); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_L, plate->safe_left_top.x); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_T, plate->safe_left_top.y); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_R, plate->safe_right_bottom.x); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_B, plate->safe_right_bottom.y); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_SHAPE, plate->safe_region_shape); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_CX, plate->safe_center.x); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_CY, plate->safe_center.y); 
			SetCtrlVal(stage_plate_module->_details_ui_panel, EDIT_PANEL_SAFE_RADIUS, plate->safe_radius); 

			device_conf_display_local_panel(stage_plate_module->dc, stage_plate_module->_details_ui_panel);
	
			device_conf_update_display(stage_plate_module->dc);   
			
			}break;
		}
	}
		
	return 0;
}

int CVICALLBACK OnStagePlateItemChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			StagePlateModule* stage_plate_module = (StagePlateModule*)callbackData;
			stage_plate_draw_plate_for_plate_ui_values(stage_plate_module);

			break;
		}	
	}

	return 0;
}


int CVICALLBACK OnStagePlateTestClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			StagePlateModule* stage_plate_module = (StagePlateModule*)callbackData;

			PlateDialogResult result = stage_plate_display_region_selection_dialog(stage_plate_module);

			break;
		}	
	}
		
	return 0;
}