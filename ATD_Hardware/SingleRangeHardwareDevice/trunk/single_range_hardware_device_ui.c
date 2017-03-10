#include "single_range_hardware_device.h"
#include "single_range_hardware_device_ui.h"

#include <userint.h>

int CVICALLBACK OnSingleRangeHardwareDevice (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	SingleRangeHardwareDevice *field_stop = (SingleRangeHardwareDevice *) callbackData;
	double val;
	
	switch (event)
		{
		case EVENT_COMMIT:
		
			GetCtrlVal (field_stop->_main_ui_panel, SR_HD_PNL_RANGE_VAL, &val);
			single_range_hardware_device_set(field_stop, val);
			break;
		}
	return 0;
}

int CVICALLBACK OnSingleRangeHardwareDeviceClose (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	SingleRangeHardwareDevice *field_stop = (SingleRangeHardwareDevice*) callbackData;
	
	switch (event)
	{
		case EVENT_COMMIT:

			ui_module_hide_all_panels(UIMODULE_CAST(field_stop));  

			break;
	}
	
	return 0;
}