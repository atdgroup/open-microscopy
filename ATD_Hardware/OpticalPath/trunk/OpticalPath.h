#ifndef __OPTICAL_PATH_MANAGER__
#define __OPTICAL_PATH_MANAGER__

#include "HardWareTypes.h"  
#include "HardWareDevice.h" 

#include "signals.h"
#include "gci_ui_module.h"
#include "device_list.h"
#include "toolbox.h" 

#define OPTICAL_PATH_MANAGER_SUCCESS 0
#define OPTICAL_PATH_MANAGER_ERROR -1

#define OPTICAL_PATH_MANAGER_VTABLE_PTR(ob, member) ((ob)->vtable.member)
#define OPTICAL_PATH_MANAGER_VTABLE(ob, member) (*((ob)->vtable.member))

#define CHECK_OPTICAL_PATH_MANAGER_VTABLE_PTR(ob, member) if(OPTICAL_PATH_MANAGER_VTABLE_PTR(ob, member) == NULL) { \
    send_optical_path_error_text(ob, "member not implemented"); \
    return OPTICAL_PATH_MANAGER_ERROR; \
}  

#define CALL_OPTICAL_PATH_MANAGER_VTABLE_PTR(ob, member) if(OPTICAL_PATH_MANAGER_VTABLE(ob, member)(ob) == OPTICAL_PATH_MANAGER_ERROR ) { \
	send_optical_path_error_text(ob, "member failed");  \
	return OPTICAL_PATH_MANAGER_ERROR; \
}

typedef struct
{
	int (*hw_init) (OpticalPathManager* optical_path_manager, int move_to_default);  
	int (*init) (OpticalPathManager* optical_path_manager); 
	int (*destroy) (OpticalPathManager* optical_path_manager);
	int (*move_to_optical_path_position) (OpticalPathManager* optical_path_manager, int position);
	int (*get_current_optical_path_position) (OpticalPathManager* optical_path_manager, int *position);
	int (*setup_optical_path) (OpticalPathManager* optical_path_manager);
	int (*hide_optical_path_calib) (OpticalPathManager* optical_path_manager);

} OpticalPathManagerVtbl;


struct _OpticalPathManager {
 
  HardwareDevice parent;  
  
  ModuleDeviceConfigurator* dc;  
  
  OpticalPathManagerVtbl vtable;
  
  int		 _requested_pos;
  int		 _moving;
  int		 _initialised;
  int		 _hw_initialised;    
  int     	 _timer;
  
  int		 _old_pos;
  int	 	 _main_ui_panel;
  int	 	 _details_ui_panel;

  double     _timer_interval;
};


int CVICALLBACK OnOpticalPathDetailsAdd (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK OnOpticalPathDetailsEdit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);


OpticalPathManager* optical_path_manager_new(const char *name, const char *description, const char* data_dir, const char *data_file, size_t size);

// Do NOT call any ui stuff here.
// It is meant for hardware initialisation and thus possibly called from
// another thread.
int optical_path_hardware_initialise(OpticalPathManager* optical_path); 
int optical_path_initialise(OpticalPathManager* optical_path, int move_to_default); 
int optical_path_is_initialised(OpticalPathManager* optical_path); 

int  send_optical_path_error_text (OpticalPathManager* optical_path, char fmt[], ...);

void optical_path_set_error_handler(OpticalPathManager* optical_path, UI_MODULE_ERROR_HANDLER handler, void *callback_data);
int  optical_path_destroy(OpticalPathManager* optical_path);
void optical_path_stop_timer(OpticalPathManager* optical_path);
void optical_path_start_timer(OpticalPathManager* optical_path);
int  optical_path_goto_default_position(OpticalPathManager* optical_path);
int  optical_path_get_number_of_positions(OpticalPathManager* optical_path, int *number_of_cubes);
int  optical_path_get_current_position(OpticalPathManager* optical_path, int *position);
int  optical_path_load_active_paths_into_list_control(OpticalPathManager* optical_path, int panel, int ctrl);
int  optical_path_move_to_position(OpticalPathManager* optical_path, int position);
int  optical_path_manager_display_calib_ui(OpticalPathManager* opm);
int  optical_path_manager_hide_calib_ui(OpticalPathManager* opm);  
int optical_path_set_list_control_to_pos(OpticalPathManager* opm, int panel, int ctrl, int pos);
int optical_path_get_current_path_name(OpticalPathManager* optical_path_manager, char *name);

// Signals
typedef void (*OPTICAL_PATH_MANAGER_EVENT_HANDLER) (OpticalPathManager* optical_path_manager, void *data); 
typedef void (*OPTICAL_PATH_MANAGER_CHANGE_EVENT_HANDLER) (OpticalPathManager* optical_path_manager, int pos, void *data);

int optical_path_signal_close_handler_connect (OpticalPathManager* optical_path,
	OPTICAL_PATH_MANAGER_EVENT_HANDLER handler, void *callback_data);

int optical_path_signal_changed_handler_connect(OpticalPathManager* optical_path,
	OPTICAL_PATH_MANAGER_CHANGE_EVENT_HANDLER handler, void *callback_data);

int optical_path_signal_pre_change_handler_connect(OpticalPathManager* opm,
	OPTICAL_PATH_MANAGER_CHANGE_EVENT_HANDLER handler, void *callback_data);

int optical_path_signal_config_changed_handler_connect(OpticalPathManager* optical_path,
	OPTICAL_PATH_MANAGER_EVENT_HANDLER handler, void *callback_data);


int  CVICALLBACK OnOpticalPathAddEditOkClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathCalibrate(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnOpticalPathSetup(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif

 
