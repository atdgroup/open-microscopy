#include <cviauto.h>

#include "manual_lamp.h"
#include "lamp_ui.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#define DONT_PROFILE
#include "profile.h"

////////////////////////////////////////////////////////////////////////////
//RJL November 2006
//Brightness control for manual microscope.
////////////////////////////////////////////////////////////////////////////

int Manual_lamp_destroy (Lamp* lamp)
{
	LampManual *lamp_manual = (LampManual *) lamp; 
	
	return LAMP_SUCCESS;
}

static int Manual_lamp_off(Lamp* lamp)
{
	LampManual *lamp_manual = (LampManual *) lamp; 
	
	if (lamp == NULL) return LAMP_ERROR;
	if (lamp->_mounted != 1) return LAMP_ERROR;

	//MessagePopup("Lamp", "Please switch off lamp or block light to camera");
	
	lamp_manual->_status = 0;
	lamp_on_change(lamp); //update uir
	
  	return LAMP_SUCCESS;
}

static int Manual_lamp_on(Lamp* lamp)
{
	LampManual *lamp_manual = (LampManual *) lamp; 
	
	if (lamp == NULL) return LAMP_ERROR;
	if (lamp->_mounted != 1) return LAMP_ERROR;

	//MessagePopup("Lamp", "Please switch on lamp or restore light to camera");

	lamp_manual->_status = 1;
	lamp_on_change(lamp); //update uir
	
  	return LAMP_SUCCESS;
}

static int Manual_get_lamp_status (Lamp* lamp, int *status)
{
	LampManual *lamp_manual = (LampManual *) lamp; 
	
	//Read current lamp status, (1 = on).

	if (lamp == NULL) return LAMP_ERROR;
	if (lamp->_mounted != 1) return LAMP_ERROR;

	*status = lamp_manual->_status;
	
	return LAMP_SUCCESS;
}

static int Manual_set_lamp_intensity (Lamp* lamp, double intensity)
{
	LampManual *lamp_manual = (LampManual *) lamp; 
	
	//Set lamp intensity, (voltage = 1.0 to 12.0).

	if (lamp == NULL) return LAMP_ERROR;
	if (lamp->_mounted != 1) return LAMP_ERROR;

	lamp_manual->_intensity = intensity;
	
	return LAMP_SUCCESS;
}

static int Manual_get_lamp_intensity (Lamp* lamp, double *intensity)
{
	LampManual *lamp_manual = (LampManual *) lamp; 
	
	//Get lamp intensity, (voltage = 1.0 to 12.0).

	if (lamp == NULL) return LAMP_ERROR;
	if (lamp->_mounted != 1) return LAMP_ERROR;

	*intensity = lamp_manual->_intensity;
	
	return LAMP_SUCCESS;
}

static int Manual_set_intensity_range(Lamp *lamp, double min, double max, double increment)
{
	return LAMP_SUCCESS;
}

Lamp* Manual_lamp_new()
{
	int mounted = 1;
	
	Lamp* lamp = lamp_new("manual lamp", "Brightness Control", sizeof(LampManual));
	
	LampManual *lamp_manual = (LampManual *) lamp;
	
	lamp_manual->_status = 1;
	lamp_manual->_intensity = 1.0;
	
	LAMP_VTABLE_PTR(lamp, destroy) = Manual_lamp_destroy; 
	LAMP_VTABLE_PTR(lamp, lamp_off) = Manual_lamp_off; 
	LAMP_VTABLE_PTR(lamp, lamp_on) = Manual_lamp_on; 
	LAMP_VTABLE_PTR(lamp, lamp_status) = Manual_get_lamp_status;
	LAMP_VTABLE_PTR(lamp, lamp_set_intensity) = Manual_set_lamp_intensity;
	LAMP_VTABLE_PTR(lamp, lamp_get_intensity) = Manual_get_lamp_intensity;
	LAMP_VTABLE_PTR(lamp, lamp_set_intensity_range) = Manual_set_intensity_range;

	lamp->_mounted = mounted;
	SetCtrlAttribute (lamp->_main_ui_panel, LAMP_PNL_ONOFF, ATTR_DIMMED, !mounted);
	SetCtrlAttribute (lamp->_main_ui_panel, LAMP_PNL_INTENSITY, ATTR_DIMMED, !mounted);

	lamp_set_intensity_range(lamp, 1.0, 12.0, 1.0);

	lamp_on_change(lamp); //update uir
	
	return lamp;
}
