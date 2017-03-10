#ifndef __OPTICAL_CALIBRATION__
#define __OPTICAL_CALIBRATION__

#define OBJ_CALIBRATION_ERROR -1
#define OBJ_CALIBRATION_SUCCESS 0

#include "HardwareTypes.h" 
#include "gci_ui_module.h" 
#include "OpticalCalibrationDevice.h" 
#include "camera\gci_camera.h"
#include "objectives.h"

typedef struct _optical_calibration optical_calibration;

struct _optical_calibration
{
	UIModule parent;
	
	Microscope *ms;
	ObjectiveManager* objective_manager;

	IcsViewerWindow *window;      
	
	int _objective_manager_config_changed_signal_id;
	int panel_id;
	int changed;
	int selected_index;
	
	dictionary *calibrations[];
};

optical_calibration* optical_calibration_new(Microscope *ms);

int optical_calibration_initialise(optical_calibration* cal);

void optical_calibration_hide(optical_calibration* cal);
void optical_calibration_destroy(optical_calibration* cal);

int  optical_calibration_get_calibration_factor_for_objective_id(optical_calibration* cal, int objective_id, double *factor);
int  optical_calibration_get_calibration_factor_for_objective_pos(optical_calibration* cal, int bjective_pos, double *factor);
int  optical_calibration_get_calibration_factor_for_current_objective(optical_calibration* cal, double *factor);
int  optical_calibration_set_objective(optical_calibration* cal, int index, int obj_id, double factor);
int  optical_calibration_display_panel(optical_calibration* cal);
int  optical_calibration_read_data(optical_calibration* cal);
int  optical_calibration_write_data(optical_calibration* cal);      

ObjectiveManager*  optical_calibration_get_objective_manager(optical_calibration* cal); 
int  optical_calibration_get_selected_index(optical_calibration* cal); 

// Signals

typedef void (*OPTICAL_CALIBRATION_EVENT_HANDLER) (optical_calibration* cal, void *data);      

int optical_calibration_signal_calibration_changed_handler_connect (optical_calibration* cal,
	OPTICAL_CALIBRATION_EVENT_HANDLER handler, void *callback_data);

int  CVICALLBACK OnCalibrationSet(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCalTreeEvent(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCancelClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLoadClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnNumericChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOkClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSaveClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCalibrationDeviceChanged (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif
