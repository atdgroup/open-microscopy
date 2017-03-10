#include <cviauto.h>
#include "manual_shutter.h"
#include "shutter_ui.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

#define DONT_PROFILE
#include "profile.h"

////////////////////////////////////////////////////////////////////////////
//RJL November 2006
//Optical shutter control for manual microscope.
////////////////////////////////////////////////////////////////////////////

int Manual_shutter_destroy (Shutter* shutter)
{
	ShutterManual* shutter_manual = (ShutterManual *) shutter; 
	
	return SHUTTER_SUCCESS;
}


static int Manual_shutter_open(Shutter* shutter)
{
	ShutterManual* shutter_manual = (ShutterManual *) shutter; 
	
	if (shutter == NULL) return SHUTTER_ERROR;
	if (shutter->_mounted != 1) return SHUTTER_ERROR;

	//MessagePopup("Shutter", "Please open the shutter");
	shutter_changed(shutter, 1);

  	return SHUTTER_SUCCESS;
}

static int Manual_shutter_close(Shutter* shutter)
{
	ShutterManual* shutter_manual = (ShutterManual *) shutter; 
	
	if (shutter == NULL) return SHUTTER_ERROR;
	if (shutter->_mounted != 1) return SHUTTER_ERROR;

	//MessagePopup("Shutter", "Please close the shutter");
	shutter_changed(shutter, 0);

  	return SHUTTER_SUCCESS;
}

static int Manual_get_shutter_status (Shutter* shutter, int *status)
{
	ShutterManual* shutter_manual = (ShutterManual *) shutter; 

	//Read current shutter status, (1 = open).

	if (shutter == NULL) return SHUTTER_ERROR;
	if (shutter->_mounted != 1) return SHUTTER_ERROR;

	*status = shutter->_open;
	
	return SHUTTER_SUCCESS;
}

static int Manual_set_shutter_open_time (Shutter* shutter, int open_time)
{
	return SHUTTER_SUCCESS;
}

static int Manual_get_shutter_open_time (Shutter* shutter, int *open_time)
{
	return SHUTTER_SUCCESS;
}

static int Manual_shutter_trigger(Shutter *shutter)
{
	return Manual_shutter_close(shutter);
}


Shutter* manual_shutter_new()
{
	int mounted = 1;
	
	Shutter* shutter = shutter_new("manual shutter", "Shutter Control", sizeof(ShutterManual));
	
	ShutterManual* shutter_manual = (ShutterManual *) shutter;
	
	SHUTTER_VTABLE_PTR(shutter, destroy) = Manual_shutter_destroy; 
	SHUTTER_VTABLE_PTR(shutter, shutter_open) = Manual_shutter_open; 
	SHUTTER_VTABLE_PTR(shutter, shutter_close) = Manual_shutter_close; 
	SHUTTER_VTABLE_PTR(shutter, shutter_status) = Manual_get_shutter_status;
	SHUTTER_VTABLE_PTR(shutter, shutter_set_open_time) = Manual_set_shutter_open_time;
	SHUTTER_VTABLE_PTR(shutter, shutter_get_open_time) = Manual_get_shutter_open_time;
	SHUTTER_VTABLE_PTR(shutter, shutter_trigger) = Manual_shutter_trigger;

	shutter->_mounted = mounted;
	SetCtrlAttribute (shutter->_main_ui_panel, SHUTTER_OPEN, ATTR_DIMMED, !mounted);
	SetCtrlAttribute (shutter->_main_ui_panel, SHUTTER_CLOSE, ATTR_DIMMED, !mounted);

	shutter->_open = 1;
	
	//No trigger controls
	SetCtrlAttribute (shutter->_main_ui_panel, SHUTTER_TRIGGER, ATTR_VISIBLE, 0);
	SetCtrlAttribute (shutter->_main_ui_panel, SHUTTER_OPEN_TIME, ATTR_VISIBLE, 0);
	SetCtrlAttribute (shutter->_main_ui_panel, SHUTTER_DECORATION_10, ATTR_VISIBLE, 0);
	
	SetCtrlAttribute (shutter->_main_ui_panel, SHUTTER_EXIT, ATTR_TOP, 87);
	SetPanelAttribute (shutter->_main_ui_panel, ATTR_HEIGHT, 124);
	
	//Alternatively if trigger implemented
	//SetCtrlAttribute (shutter->_main_ui_panel, SHUTTER_EXIT, ATTR_TOP, 187);
	//SetPanelAttribute (shutter->_main_ui_panel, ATTR_HEIGHT, 223);
	
	//Hide the test button
	SetCtrlAttribute (shutter->_main_ui_panel, SHUTTER_TEST, ATTR_VISIBLE, 0);

	return shutter;
}
