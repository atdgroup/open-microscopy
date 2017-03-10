#include "HardWareTypes.h"

#include "FilterSet.h"
#include "FilterSetUI.h"
#include "device_list.h"
#include "gci_utils.h"
#include "string_utils.h"

#include <utility.h>
#include <userint.h>
#include "asynctmr.h"

int CVICALLBACK OnFilterChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			FilterSetCollection* filterset = (FilterSetCollection*)callbackData;
			int pos = -1;
			CMDeviceNode *node = NULL;
	
			GetCtrlVal(panel, control, &pos);
			
			filterset_move_to_position(filterset, pos);
			
			}break;
		}
	
	return 0;
}


int CVICALLBACK OnFilterClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			FilterSetCollection* filterset = (FilterSetCollection*) callbackData;
	
			if(device_conf_close_config_panel(filterset->dc) < 0)
				return -1;

			ui_module_hide_all_panels(UIMODULE_CAST(filterset));

			}break;
		}
	return 0;
}


int CVICALLBACK OnFilterSetup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			FilterSetCollection* filterset = (FilterSetCollection*) callbackData;  
	
			ui_module_attach_panel_to_panel(UIMODULE_CAST(filterset),
														  device_conf_get_panel_id(filterset->dc),
														  UI_MODULE_REL_TOP_RIGHT,
														  5, 0);

			device_conf_display_panel(filterset->dc);    

			}break;
		}
	return 0;
}

int CVICALLBACK OnFilterAddEditOkClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			
			FilterSetCollection* filterset = (FilterSetCollection*) callbackData;  
			int add_or_edit, id;
			FilterSet *filter;
			char name[FILTER_NAME_LEN];
	
			GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_ADDOREDIT, &add_or_edit);    
			
			
			// If 1 we edit - If 0 we are adding
			if(add_or_edit) {
				
				CMDeviceNode *node;       
				
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_ID, &id);   
				
				if(device_conf_get_device_node_for_id(filterset->dc, &node, id) == DEVICE_CONF_ERROR)
					return -1;

				filter = (FilterSet *) node->device;
				
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_NAME, node->name);
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EXC_NAME, filter->exc_name);
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EXCMIN, &(filter->exc_min_nm)); 
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EXCMAX, &(filter->exc_max_nm));
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_DIC_NAME, filter->dic_name);
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_DICHROICNM, &(filter->dichroic_nm)); 
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EMM_NAME, filter->emm_name);
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EMMIN, &(filter->emm_min_nm)); 
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EMMAX, &(filter->emm_max_nm)); 
				
			}
			else {
				
				// Adding 
				
				filter = malloc(sizeof(FilterSet));   
			
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_NAME, name);
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EXC_NAME, filter->exc_name);
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EXCMIN, &(filter->exc_min_nm)); 
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EXCMAX, &(filter->exc_max_nm)); 
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_DIC_NAME, filter->dic_name);
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_DICHROICNM, &(filter->dichroic_nm)); 
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EMM_NAME, filter->emm_name);
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EMMIN, &(filter->emm_min_nm)); 
				GetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EMMAX, &(filter->emm_max_nm)); 
				
				device_conf_add_device(DEVICE_CONF_CAST(filterset->dc), filter, name, -1);    
			}
			
			HidePanel(filterset->_details_ui_panel);
			
			device_conf_update_display(filterset->dc);   
			
			}break;
		}
	return 0;
}

int CVICALLBACK OnFilterDetailsAdd (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{

			FilterSetCollection* filterset = (FilterSetCollection*)callbackData;

			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_ADDOREDIT, 0);
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_NAME, "");
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EXC_NAME, "");
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EXCMIN, 0 );
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EXCMAX, 0 );
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_DIC_NAME, "");
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_DICHROICNM, 0);
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EMM_NAME, "");
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EMMIN, 0);
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EMMAX, 0);
			
//			DisplayPanel(filterset->_details_ui_panel);
			device_conf_display_local_panel(filterset->dc, filterset->_details_ui_panel);
				
			device_conf_update_display(filterset->dc);   
			
			}break;
		}
	return 0;
}


int CVICALLBACK OnFilterDetailsEdit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
		{
			FilterSetCollection* filterset = (FilterSetCollection*)callbackData;
			CMDeviceNode *node;
			FilterSet *filter;
	
			node =  device_conf_get_node_for_selected_item(filterset->dc);
			filter = (FilterSet *) node->device;
			
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_ID, node->id); 
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_ADDOREDIT, 1);  // We are editing so set to 1
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_NAME, node->name);
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EXC_NAME, filter->exc_name);
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EXCMIN, filter->exc_min_nm );
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EXCMAX, filter->exc_max_nm );
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_DIC_NAME, filter->dic_name);
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_DICHROICNM, filter->dichroic_nm);
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EMM_NAME, filter->emm_name);
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EMMIN, filter->emm_min_nm);
			SetCtrlVal(filterset->_details_ui_panel, EDIT_PANEL_EMMAX, filter->emm_max_nm);

//			DisplayPanel(filterset->_details_ui_panel);
			device_conf_display_local_panel(filterset->dc, filterset->_details_ui_panel);
	
			device_conf_update_display(filterset->dc);   
			
			}break;
		}
	}
		
	return 0;
}
