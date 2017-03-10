#include <cviauto.h>

#include "FilterSet_Dummy.h"
#include "FilterSetUI.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#define DONT_PROFILE
#include "profile.h"

#define MANUAL_FILTERSET_POSITIONS 5

int manual_filterset_hardware_init (FilterSetCollection* filterset)
{
	return FILTERSET_SUCCESS;
}

int manual_filterset_destroy (FilterSetCollection* filterset)
{
	FilterSetCollectionManual * filterset_manual = (FilterSetCollectionManual *) filterset;
	
	return FILTERSET_SUCCESS;
}

static int manual_filterset_move_to_filter_position(FilterSetCollection* filterset, int position)
{
	FilterSetCollectionManual * filterset_manual = (FilterSetCollectionManual *) filterset;
	char info[UIMODULE_NAME_LEN];
	
	//Move filter cassette to specified position, (1 to MANUAL_FILTERSET_POSITIONS).
	if ((position < 1) || ( position > MANUAL_FILTERSET_POSITIONS))
		return FILTERSET_ERROR;

	if (filterset_manual->stored_position == position)  // already at the position
		return FILTERSET_SUCCESS;

	filterset_manual->stored_position = position;

	hardware_device_get_current_value_text((HardwareDevice *)filterset, info);
	GCI_MessagePopup("Manual Filter Set", "Please change the filter set on the microscope to %s.", info);

	return FILTERSET_SUCCESS;
}

static int manual_filterset_get_current_filter_position (FilterSetCollection* filterset, int *position)
{
	FilterSetCollectionManual * filterset_manual = (FilterSetCollectionManual *) filterset;
	
	//Read current cube position, (1 to MANUAL_FILTERSET_POSITIONS).
	*position = filterset_manual->stored_position;
	
	if ((*position < 1) || ( *position > MANUAL_FILTERSET_POSITIONS))
		return FILTERSET_ERROR;
	
	return FILTERSET_SUCCESS;
}


FilterSetCollection* manual_filterset_new(const char *name, const char *description,
								const char *data_dir, const char *filepath,
								UI_MODULE_ERROR_HANDLER error_handler, void *data)
{
	int mounted = 1;
	
	FilterSetCollection* filterset = filterset_new(name, description, data_dir, filepath, sizeof(FilterSetCollectionManual));
	
	FilterSetCollectionManual * filterset_manual = (FilterSetCollectionManual *) filterset; 
	
	filterset_manual->stored_position = 1;
	
	ui_module_set_error_handler(UIMODULE_CAST(filterset_manual), error_handler, data); 
	
	FILTERSET_VTABLE_PTR(filterset, hardware_init) = manual_filterset_hardware_init;  
	FILTERSET_VTABLE_PTR(filterset, destroy) = manual_filterset_destroy;
	FILTERSET_VTABLE_PTR(filterset, move_to_filter_position) = manual_filterset_move_to_filter_position; 
	FILTERSET_VTABLE_PTR(filterset, get_current_filter_position) = manual_filterset_get_current_filter_position; 

	hardware_device_set_as_manual(HARDWARE_DEVICE_CAST(filterset));

	return filterset;
}
