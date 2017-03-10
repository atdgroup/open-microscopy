#ifndef __FLUORESCENCE_CUBE_MANAGER__
#define __FLUORESCENCE_CUBE_MANAGER__

#include "HardWareTypes.h"      
#include "HardWareDevice.h" 

#include "signals.h"
#include "gci_ui_module.h"
#include "device_list.h"
#include "toolbox.h" 

#define CUBE_MANAGER_SUCCESS 0
#define CUBE_MANAGER_ERROR -1

#define CUBE_MANAGER_VTABLE_PTR(ob, member) ((ob)->vtable.member)
#define CUBE_MANAGER_VTABLE(ob, member) (*((ob)->vtable.member))

#define CHECK_CUBE_MANAGER_VTABLE_PTR(ob, member) if(CUBE_MANAGER_VTABLE_PTR(ob, member) == NULL) { \
    send_fluocube_error_text(ob, "member not implemented"); \
    return CUBE_MANAGER_ERROR; \
}  

#define CALL_CUBE_MANAGER_VTABLE_PTR(ob, member) if(CUBE_MANAGER_VTABLE(ob, member)(ob) == CUBE_MANAGER_ERROR ) { \
	send_fluocube_error_text(ob, "member failed");  \
	return CUBE_MANAGER_ERROR; \
}

#define CUBE_MANAGER_NAME_SIZE 10

typedef struct _FluoCube FluoCube;

struct _FluoCube
{
	char	name[CUBE_MANAGER_NAME_SIZE];
	int		position;
	int		exc_min_nm;
	int		exc_max_nm;
	int		dichroic_nm;
	int		emm_min_nm;
	int		emm_max_nm;
};


typedef struct
{
	int	(*hardware_init) (FluoCubeManager* cube_manager); // Intended to be re-entrant (thread safe)
	int	(*initialise) (FluoCubeManager* cube_manager);
	int (*destroy) (FluoCubeManager* cube_manager);
	int (*move_to_cube_position) (FluoCubeManager* cube_manager, int position);
	int (*get_current_cube_position) (FluoCubeManager* cube_manager, int *position);

} FluoCubeManagerVtbl;


struct _FluoCubeManager
{
  HardwareDevice parent; 
  
  ModuleDeviceConfigurator* dc;
	
  FluoCubeManagerVtbl vtable;
  
  int 		 _hw_initialised;  
  int 		 _initialised;
  int     	 _timer;
  int	 	 _main_ui_panel;
  int		 _current_pos;
  int		 _requested_pos;
  int		 _lock;
  
  volatile int _prevent_timer_callback;
  volatile int _abort_cube_move_check;
  int		   _move_pos_thread_id;

  double    _timer_interval;
  
  // This panel is the add / edit panel
  int	 	 _details_ui_panel;
};


int CVICALLBACK OnCubeDetailsAdd (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK OnCubeDetailsEdit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

FluoCubeManager* cube_manager_new(const char *name, const char *description, const char *data_dir, const char *data_file, size_t size);

// Do NOT call any ui stuff here.
// It is meant for hardware initialisation and thus possibly called from
// another thread.
int cube_manager_hardware_initialise(FluoCubeManager* cube_manager); 

int cube_manager_initialise(FluoCubeManager* cube_manager, int move_to_default);

int cube_manager_is_initialised(FluoCubeManager* cube_manager); 

int  send_fluocube_error_text (FluoCubeManager* cube_manager, char fmt[], ...);

void cube_manager_set_error_handler(FluoCubeManager* cube_manager, UI_MODULE_ERROR_HANDLER handler, void *callback_data);

int  cube_manager_destroy(FluoCubeManager* cube_manager);
void cube_manager_stop_timer(FluoCubeManager* cube_manager);
void cube_manager_start_timer(FluoCubeManager* cube_manager);

int  cube_manager_goto_default_position(FluoCubeManager* cube_manager);
int  cube_manager_get_number_of_cubes(FluoCubeManager* cube_manager, int *number_of_cubes);
int  cube_manager_get_current_cube_position(FluoCubeManager* cube_manager, int *position);
int  cube_manager_get_current_cube(FluoCubeManager* cube_manager, FluoCube *cube);
int  cube_manager_get_cube_for_position(FluoCubeManager* cube_manager, int position, FluoCube* dst_cube);

double cube_manager_get_average_emmision(FluoCube cube);

int  cube_manager_load_active_cubes_into_list_control(FluoCubeManager* cube_manager, int panel, int ctrl);

int  cube_manager_move_to_position(FluoCubeManager* cube_manager, int position);
int  cube_manager_wait_for_stop_moving (FluoCubeManager* cube_manager, double timeout);

// Client must free the returned array
FluoCube* cube_manager_get_active_cubes(FluoCubeManager* cube_manager);


// Signals
typedef void (*CUBE_MANAGER_EVENT_HANDLER) (FluoCubeManager* cube_manager, void *data); 
typedef void (*CUBE_MANAGER_CUBE_EVENT_HANDLER) (FluoCubeManager* cube_manager, FluoCube cube, void *data);
typedef void (*CUBE_MANAGER_CHANGE_EVENT_HANDLER) (FluoCubeManager* cube_manager, int pos, void *data); 

int cube_manager_signal_close_handler_connect (FluoCubeManager* cube_manager,
	CUBE_MANAGER_EVENT_HANDLER handler, void *callback_data);

int cube_manager_signal_cube_changed_handler_connect(FluoCubeManager* cube_manager,
	CUBE_MANAGER_CHANGE_EVENT_HANDLER handler, void *callback_data);

int cube_manager_signal_cube_config_changed_handler_connect(FluoCubeManager* cube_manager,
	CUBE_MANAGER_EVENT_HANDLER handler, void *callback_data);

int  CVICALLBACK OnCubeAddEditOkClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeSetup(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnFluorCubeClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif

 
