#include "gci_camera.h" 
#include "gci_orca_camera.h" 
#include "gci_C9100_camera.h"
#include "gci_C9100_camera_ui.h"
#include "string_utils.h"
#include "gci_utils.h"
#include "drawTools.h"
#include "FreeImageAlgorithms_IO.h"

#include <utility.h>
#include "toolbox.h"
#include <formatio.h>

//////////////////////////////////////////////////////////////////////////////////
// Camera module for the Hamatsu C9100 EM camera
// GP/RJL May 2006
//////////////////////////////////////////////////////////////////////////////////

#define IM_MAX_WIDTH 512
#define IM_MAX_HEIGHT 512
#define IM_MIN_WIDTH 32
#define IM_MIN_HEIGHT 32

//static int gci_C9100_camera_set_default_settings (GciCamera* camera)
//{
//	return gci_orca_camera_set_default_settings (camera);
//}


//static int  gci_C9100_camera_set_gain(GciCamera* camera, double gain)
//{
//	GciC9100Camera *C9100_camera = (GciC9100Camera *) camera;
//	
//	if ((gain < 0) || (gain > 255))
//		return CAMERA_ERROR;
//		
//	return C9100SetFeature (C9100_camera, DCAM_IDFEATURE_SENSITIVITY, (double)Round(gain));
//}

static int gci_C9100_camera_get_min_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = IM_MIN_WIDTH;
	*height = IM_MIN_HEIGHT;

	return CAMERA_SUCCESS;
}


static int  gci_C9100_camera_get_max_size(GciCamera* camera, unsigned int *width, unsigned int *height)
{
	*width = IM_MAX_WIDTH;
	*height = IM_MAX_HEIGHT;

	return CAMERA_SUCCESS;
}

/*
static int gci_C9100_camera_set_blacklevel(GciCamera* camera, double black_level)
{
	GciC9100Camera *C9100_camera = (GciC9100Camera *) camera;
		
	if ((black_level < -255) || (black_level > 0))
		return CAMERA_ERROR; 

	C9100_camera->_black_level = black_level;

	if(camera->_extra_ui_panel != -1)
		SetCtrlVal(camera->_extra_ui_panel, EXTRA_PNL_BLACKLEVEL, black_level);

	return C9100SetFeature(C9100_camera, DCAM_IDFEATURE_BRIGHTNESS, (double)Round(black_level));
}
*/


static int gci_C9100_camera_initialise (GciCamera* camera)
{
	GciC9100Camera* C9100_camera = (GciC9100Camera*) camera;
	GciOrcaCamera* orca_camera = (GciOrcaCamera*) camera;    
	
	if(camera == NULL)
		return CAMERA_ERROR;

	gci_camera_set_description(camera, "Hamamatsu C9100 Camera");
	gci_camera_set_name(camera, "C9100 Camera");
	camera->_bits = 14;
	orca_camera->_binning = NO_BINNING;
	orca_camera->_trigger_mode = CAMERA_INTERNAL_TRIG;
	orca_camera->_black_level = 255.0;
	orca_camera->_light_mode = 1;
	
	gci_camera_set_extra_panel_uir(camera, "gci_C9100_camera_ui.uir", EXTRA_PNL);    

	gci_camera_set_exposure_range(camera, 1, 5000, "ms", VAL_INTEGER);    
	
  	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_DATAMODE, C9100_Camera_onDataMode, C9100_camera) < 0)
		return CAMERA_ERROR;
  	
  	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_LIGHTMODE, C9100_Camera_onLightMode, C9100_camera) < 0)
		return CAMERA_ERROR;
  	
  	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_QUIT, C9100_Camera_onExtrasQuit, C9100_camera) < 0)
		return CAMERA_ERROR;
  	
  	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_BLACKLEVEL, C9100_Camera_onBlackLevel, C9100_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_BINNING, C9100_Camera_onBinning, C9100_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_WIDTH, C9100_Camera_onSetSizePosition, C9100_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_HEIGHT, C9100_Camera_onSetSizePosition, C9100_camera) < 0)
		return CAMERA_ERROR;
 
    if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_LEFT, C9100_Camera_onSetSizePosition, C9100_camera) < 0)
		return CAMERA_ERROR;
		
	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_TOP, C9100_Camera_onSetSizePosition, C9100_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_AUTOCENTRE, C9100_Camera_onSetSizePosition, C9100_camera) < 0)
		return CAMERA_ERROR;

	if ( InstallCtrlCallback (camera->_extra_ui_panel, EXTRA_PNL_SUBWINDOW_PRSETWINDOW, C9100_Camera_onPresetSubWindow, C9100_camera) < 0)
		return CAMERA_ERROR;

	return CAMERA_SUCCESS;
}


int gci_C9100_camera_destroy(GciCamera* camera)
{
	GciC9100Camera* C9100_camera = (GciC9100Camera*) camera;

	gci_orca_camera_deconstructor(camera);
  	
  	free(C9100_camera);
  	
  	return CAMERA_SUCCESS;
}

int gci_C9100_camera_constructor(GciCamera* camera, HINSTANCE hInstance, const char *name, const char *description)
{
    GciOrcaCamera* orca_camera = (GciOrcaCamera*) camera;
    
    gci_orca_constructor(camera, hInstance, name, description);
    
    strcpy(orca_camera->default_settings_file_path, "C9100DefaultSettings.cam");

    //CAMERA_VTABLE_PTR(camera).set_gain = gci_C9100_camera_set_gain; 
    CAMERA_VTABLE_PTR(camera).initialise = gci_C9100_camera_initialise;
    //CAMERA_VTABLE_PTR(camera).set_exposure_time = gci_C9100_camera_set_exposure_time;
    CAMERA_VTABLE_PTR(camera).get_min_size = gci_C9100_camera_get_min_size;
    CAMERA_VTABLE_PTR(camera).get_max_size = gci_C9100_camera_get_max_size;
    CAMERA_VTABLE_PTR(camera).destroy = gci_C9100_camera_destroy;
    //CAMERA_VTABLE_PTR(camera).set_default_settings = gci_C9100_camera_set_default_settings;
    
    return CAMERA_SUCCESS;
}

GciCamera* gci_C9100_camera_new(HINSTANCE hInstance, const char *name, const char *description)
{
    GciC9100Camera* C9100_camera = (GciC9100Camera*) malloc(sizeof(GciC9100Camera));
    GciCamera *camera = (GciCamera*) C9100_camera;
    
    gci_C9100_camera_constructor(camera, hInstance, name, description);
	
	return (GciCamera*) C9100_camera;
}


