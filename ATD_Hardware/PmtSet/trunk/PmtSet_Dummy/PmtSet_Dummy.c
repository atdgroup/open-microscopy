#include <cviauto.h>

#include "PmtSet_Dummy.h"
#include "PmtSetUI.h"
#include "gci_utils.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

int manual_pmtset_hw_init (PmtSet* pmtset, int move_to_default)
{
	return PMTSET_SUCCESS;
}

int manual_pmtset_destroy (PmtSet* pmtset)
{
	return PMTSET_SUCCESS;
}


static int manual_move_to_pmtset_position(PmtSet* pmtset, int position)
{
	ManualPmtSet *pmtset_manual =  (ManualPmtSet *) pmtset;   
	char info[UIMODULE_NAME_LEN];
	
	if (position < 1)
		return PMTSET_ERROR;

	if (pmtset_manual->manual_position == position)  // already at the position
		return PMTSET_SUCCESS;

	pmtset_manual->manual_position = position;

	hardware_device_get_current_value_text((HardwareDevice *)pmtset, info);
	GCI_MessagePopup("Manual PMT", "Please change the PMT to %s.", info);

	return PMTSET_SUCCESS;
}

static int manual_get_current_pmtset_position (PmtSet* pmtset, int *position)
{
	ManualPmtSet *pmtset_manual =  (ManualPmtSet *) pmtset;   
    
	//Get current optical path position, (1 to OPTICAL_PATH_TURRET_SIZE).
	*position = pmtset_manual->manual_position;

	return PMTSET_SUCCESS;
}

static int manual_pmtset_setup (PmtSet* pmtset)
{
	return PMTSET_SUCCESS;
}

static int manual_hide_pmtset_calib (PmtSet* pmtset)
{
	return PMTSET_SUCCESS;
}

PmtSet* manual_pmtset_new(const char *name, const char *description, const char* data_dir, const char *filepath)
{
	PmtSet* pmtset = pmtset_new(name, description, data_dir, filepath, sizeof(ManualPmtSet));
	
	ManualPmtSet *pmtset_manual =  (ManualPmtSet *) pmtset; 
	
	pmtset_manual->manual_position = 1;

	PMTSET_VTABLE_PTR(pmtset, hw_init) = manual_pmtset_hw_init; 
	PMTSET_VTABLE_PTR(pmtset, destroy) = manual_pmtset_destroy;
	PMTSET_VTABLE_PTR(pmtset, move_to_pmtset_position) = manual_move_to_pmtset_position; 
	PMTSET_VTABLE_PTR(pmtset, get_current_pmtset_position) = manual_get_current_pmtset_position; 
	PMTSET_VTABLE_PTR(pmtset, setup_pmtset) = manual_pmtset_setup; 
	PMTSET_VTABLE_PTR(pmtset, hide_pmtset_calib) = manual_hide_pmtset_calib; 

	hardware_device_set_as_manual(HARDWARE_DEVICE_CAST(pmtset));

	return pmtset;
}
