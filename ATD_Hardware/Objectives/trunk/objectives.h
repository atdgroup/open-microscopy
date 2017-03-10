#ifndef __OBJECTIVE_MANAGER__
#define __OBJECTIVE_MANAGER__

#include "HardWareTypes.h"   
#include "HardWareDevice.h" 

#include "gci_ui_module.h"
#include "device_list.h"  
#include "toolbox.h" 

#define OBJECTIVE_MANAGER_SUCCESS 0
#define OBJECTIVE_MANAGER_ERROR -1

#define INVALID_FOCUS_POS		3000.0
#define INVALID_ILLUMINATION	0.0

#define OBJECTIVE_MANAGER_VTABLE_PTR(ob, member) ((ob)->vtable.member)
#define OBJECTIVE_MANAGER_VTABLE(ob, member) (*((ob)->vtable.member))

#define CHECK_OBJECTIVE_MANAGER_VTABLE_PTR(ob, member) if(OBJECTIVE_MANAGER_VTABLE_PTR(ob, member) == NULL) { \
    send_objectives_error_text(ob, "member not implemented"); \
    exit(2); \
}  

#define CALL_OBJECTIVE_MANAGER_VTABLE_PTR(ob, member) if( OBJECTIVE_MANAGER_VTABLE(ob, member)(ob) == OBJECTIVE_MANAGER_ERROR ) { \
	send_objectives_error_text(ob, "member failed");  \
	exit(2); \
}

#define OBJ_STR_LEN 50

typedef struct
{
  int		 _active;
  int		 _turret_position;
  char 		 _objective_name[OBJ_STR_LEN];
  char 	 	 _objective_medium[20]; 
  char 		 _magnification_str[20]; 
  char	 	 _numerical_aperture[20];			
  char 	 	 _working_distance[20];								   
  char	 	 _back_aperture[20]; 
  char 	 	 _focus_position[20];								   
  char 	 	 _illumination[20];								   
  char 		 _condenser[20]; 
  char 	 	 _aperture_stop[20];								   
  char 	 	 _field_stop[20];
 
  // This dictionary holds calibrations for various dectectors.
  // The dictionary will used the module name for the key.
  // Ie camera name / spc module name ...
  dictionary *_calibrations;

} Objective;


typedef struct _ObjectiveManagerVtbl
{
	int (*init) (ObjectiveManager* objective_manager);  
	int (*hw_init) (ObjectiveManager* objective_manager); 
	int (*destroy) (ObjectiveManager* objective_manager);
	int (*move_to_turret_position) (ObjectiveManager* objective_manager, int position);
	int (*get_current_turret_position) (ObjectiveManager* objective_manager, int *position);
	
} ObjectiveManagerVtbl;


struct _ObjectiveManager
{
  HardwareDevice parent; 
  
  ModuleDeviceConfigurator* dc;  
  
  ObjectiveManagerVtbl vtable;
 
  int 		 _initialised;
  int 		 _hw_initialised;
  int     	 _timer;
  int	 	 _main_ui_panel;
  int		 _details_ui_panel;

  double     _timer_interval;
};


int CVICALLBACK OnObjectivesDetailsAdd (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK OnObjectivesDetailsEdit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

ObjectiveManager* objective_manager_new(const char *name, const char *description, const char *data_dir, const char *data_file, size_t size);

int 		objective_manager_initialise(ObjectiveManager* objective_manager, int move_to_default);
int 		objective_manager_hardware_initialise(ObjectiveManager* objective_manager);

int  		send_objectives_error_text (ObjectiveManager* objective_manager, char fmt[], ...);

void 		objective_manager_set_error_handler(ObjectiveManager* objective_manager, UI_MODULE_ERROR_HANDLER handler);


//int 		objective_manager_get_turret_size(ObjectiveManager* objective_manager);
int  		objective_manager_destroy(ObjectiveManager* objective_manager);
int 		objective_manager_goto_default_position(ObjectiveManager* objective_manager);
int 		objective_manager_display_main_ui (ObjectiveManager* objective_manager);
int 		objective_manager_hide_main_ui (ObjectiveManager* objective_manager);
int  		objective_manager_is_main_ui_visible (ObjectiveManager* objective_manager);

int 		objective_manager_get_current_objective_position(ObjectiveManager* objective_manager, int *position);
int 		objective_manager_get_current_objective_name(ObjectiveManager* objective_manager, char *name);  
int  		objective_manager_get_objective_for_position(ObjectiveManager* objective_manager, int position, Objective* obj);
int  		objective_manager_get_current_position(ObjectiveManager* objective_manager, int *position);
Objective*  objective_manager_get_objective_with_name(ObjectiveManager* objective_manager, const char* name);   
int 		objective_manager_get_current_objective(ObjectiveManager* objective_manager, Objective* objective);
int			objective_manager_get_number_of_defined_objectives(ObjectiveManager* objective_manager);
int  		objective_manager_get_number_of_active_objectives(ObjectiveManager* objective_manager);
int 		objective_manager_get_max_no_objectives(ObjectiveManager* objective_manager, int *nObjectives);
int 		objective_manager_move_to_position(ObjectiveManager* objective_manager, int position);
int			objective_manager_enable_dynamic_objective_options(ObjectiveManager* objective_manager);

void 		objective_manager_start_timer(ObjectiveManager* objective_manager);
void 		objective_manager_stop_timer(ObjectiveManager* objective_manager);

int 		objective_manager_load_active_objectives_into_list_control(ObjectiveManager* objective_manager, int panel, int ctrl)  ;

int 		objective_manager_load_objectives_file(ObjectiveManager* objective_manager, const char *filepath);

int			objective_set_calibration(Objective* objective, const char* module_name, double calibration);
int			objective_get_calibration(Objective* objective, const char* module_name, double *calibration);
int			objective_set_calibration_for_objective_id(ObjectiveManager* objective_manager, int id, const char* module_name, double calibration);
int			objective_get_calibration_for_objective_id(ObjectiveManager* objective_manager, int id, const char* module_name, double *calibration);

// Signals
typedef void (*OBJECTIVE_EVENT_HANDLER) (ObjectiveManager* objective_manager, void *data); 
typedef void (*OBJECTIVE_CHANGE_EVENT_HANDLER) (ObjectiveManager* objective_manager, int pos, void *data);

int objective_manager_signal_close_handler_connect (ObjectiveManager* objective_manager, OBJECTIVE_EVENT_HANDLER handler, void *callback_data);
int objective_manager_signal_start_change_handler_connect (ObjectiveManager* objective_manager, OBJECTIVE_CHANGE_EVENT_HANDLER handler, void *callback_data);
int objective_manager_signal_end_change_handler_connect (ObjectiveManager* objective_manager, OBJECTIVE_CHANGE_EVENT_HANDLER handler, void *callback_data);

int objective_manager_signal_end_change_handler_connect (ObjectiveManager* objective_manager, OBJECTIVE_CHANGE_EVENT_HANDLER handler, void *callback_data);

int objective_manager_signal_config_changed_handler_connect(ObjectiveManager* objective_manager,
	OBJECTIVE_EVENT_HANDLER handler, void *callback_data);
int objective_manager_signal_config_changed_handler_disconnect(ObjectiveManager* objective_manager, int id);

int  CVICALLBACK OnObjectiveChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnObjectivesAddEditOkClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnObjectivesCloseClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnObjectivesConfig(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif

 
