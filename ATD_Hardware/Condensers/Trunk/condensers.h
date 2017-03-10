#ifndef __CONDENSER_MANAGER__
#define __CONDENSER_MANAGER__

#include "HardWareTypes.h"      
#include "HardWareDevice.h" 

#include "signals.h"
#include "gci_ui_module.h"
#include "device_list.h"
#include "toolbox.h" 

#define CONDENSER_MANAGER_SUCCESS 0
#define CONDENSER_MANAGER_ERROR -1

#define MAX_NUMBER_OF_CONDENSERS 15
#define CONDENSER_TURRET_SIZE 6

#define CONDENSER_MANAGER_VTABLE_PTR(ob, member) ((ob)->vtable.member)
#define CONDENSER_MANAGER_VTABLE(ob, member) (*((ob)->vtable.member))

#define CHECK_CONDENSER_MANAGER_VTABLE_PTR(ob, member) if(CONDENSER_MANAGER_VTABLE_PTR(ob, member) == NULL) { \
    send_condenser_error_text(ob, "member not implemented"); \
    return CONDENSER_MANAGER_ERROR; \
}  

#define CALL_CONDENSER_MANAGER_VTABLE_PTR(ob, member) if(CONDENSER_MANAGER_VTABLE(ob, member)(ob) == CONDENSER_MANAGER_ERROR ) { \
	send_condenser_error_text(ob, "member failed");  \
	return CONDENSER_MANAGER_ERROR; \
}


typedef struct {

	char	name[15];
//	int		id;
	int		position;

} Condenser;


typedef struct _CondenserManager CondenserManager;


typedef struct
{
	int	(*hardware_init) (CondenserManager* condenser_manager); // Intended to be re-entrant (thread safe)
	int	(*initialise) (CondenserManager* condenser_manager);
	int (*destroy) (CondenserManager* condenser_manager);
	int (*move_to_condenser_position) (CondenserManager* condenser_manager, int position);
	int (*get_current_condenser_position) (CondenserManager* condenser_manager, int *position);

} CondenserManagerVtbl;


struct _CondenserManager
{
	HardwareDevice parent; 

	ModuleDeviceConfigurator* dc;

	CondenserManagerVtbl vtable;
 
	int		 _hw_initialised;
	int		 _initialised;
	int      _timer;
	int	 	 _main_ui_panel;
	int	 	 _details_ui_panel; 
	int		 _current_pos;
	int		 _mounted;

	volatile int _prevent_timer_callback;

	double   _timer_interval;
};


CondenserManager* condenser_manager_new(char *name, char *description, const char *data_dir, const char *data_file, size_t size);

int condenser_manager_initialise (CondenserManager* condenser_manager);
int condenser_manager_is_initialised(CondenserManager* condenser_manager);
int condenser_manager_hardware_initialise(CondenserManager* condenser_manager);
int condenser_manager_hardware_is_initialised(CondenserManager* condenser_manager);

int  send_condenser_error_text (CondenserManager* condenser_manager, char fmt[], ...);

void condenser_manager_set_error_handler(CondenserManager* condenser_manager, void (*handler) (char *error_string, CondenserManager *condenser_manager));

int  condenser_manager_set_i2c_port(CondenserManager* condenser_manager, int port);
int  condenser_manager_set_description(CondenserManager* condenser_manager, const char* description);
int  condenser_manager_get_description(CondenserManager* condenser_manager, char *description);
int  condenser_manager_set_name(CondenserManager* condenser_manager, char* name);
int  condenser_manager_get_name(CondenserManager* condenser_manager, char *name);
int  condenser_manager_set_datafile(CondenserManager* condenser_manager, const char* data_file);
int  condenser_manager_destroy(CondenserManager* condenser_manager);
void condenser_manager_stop_timer(CondenserManager* condenser_manager);
void condenser_manager_start_timer(CondenserManager* condenser_manager);

int  condenser_manager_set_number_of_condensers(CondenserManager* condenser_manager, int number_of_condensers);
int  condenser_manager_get_number_of_condensers(CondenserManager* condenser_manager, int *number_of_condensers);
int  condenser_manager_get_current_condenser_position(CondenserManager* condenser_manager, int *position);
int  condenser_manager_get_current_condenser(CondenserManager* condenser_manager, Condenser *condenser);
int  condenser_manager_get_condenser(CondenserManager* condenser_manager, int condenser_number, Condenser *condenser);
Condenser* condenser_manager_get_condenser_ptr_for_position(CondenserManager* condenser_manager, int position);
Condenser* condenser_manager_get_condenser_ptr_for_id(CondenserManager* condenser_manager, int id);
Condenser* condenser_manager_get_condenser_with_name(CondenserManager* condenser_manager, const char* name);   
int  condenser_manager_move_to_position(CondenserManager* condenser_manager, int position);
//int  condenser_manager_get_condenser_pos_for_name(CondenserManager* condenser_manager, char* name, int *turret_pos);

int  condenser_manager_hardware_load_state_from_file (HardwareDevice* device, const char* filepath);
int  condenser_manager_hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode);
int  condenser_manager_get_current_value_text(HardwareDevice* device, char* info);

int  condenser_manager_display_main_ui (CondenserManager* condenser_manager);
int  condenser_manager_hide_main_ui (CondenserManager* condenser_manager);
int  condenser_manager_is_main_ui_visible (CondenserManager* condenser_manager);
int  condenser_manager_display_config_ui(CondenserManager* condenser_manager);
int  condenser_manager_hide_config_ui(CondenserManager* condenser_manager);
int  condenser_manager_is_config_ui_visible(CondenserManager* condenser_manager);

int  condenser_manager_save_condenser_data(CondenserManager* condenser_manager, const char *filepath);
int  condenser_manager_load_condenser_file(CondenserManager* condenser_manager, const char *filepath);
int  condenser_manager_load_all_possible_condensers_into_ui(CondenserManager* condenser_manager);

int  condenser_manager_add_condenser_ui(CondenserManager* condenser_manager);
int  condenser_manager_edit_condenser_ui(CondenserManager* condenser_manager, int index);

int  condenser_manager_switch_active_position(CondenserManager* condenser_manager, int id1, int id2);
int  condenser_manager_change_to_active(CondenserManager* condenser_manager, int id);
int  condenser_manager_remove_condenser_at_active_position(CondenserManager* condenser_manager, int id);

int  condenser_manager_add_condenser(CondenserManager* condenser_manager);
int  condenser_manager_edit_condenser(CondenserManager* condenser_manager, int index);
int  condenser_manager_remove_condenser(CondenserManager* condenser_manager, int id);

int  condenser_manager_load_active_condensers_into_list_control(CondenserManager* condenser_manager, int panel, int ctrl);

// Signals
typedef void (*CONDENSER_MANAGER_EVENT_HANDLER) (CondenserManager* condenser_manager, void *data); 
typedef void (*CONDENSER_MANAGER_CHANGE_EVENT_HANDLER) (CondenserManager* condenser_manager, int turret_pos, void *data); 
typedef void (*CONDENSER_MANAGER_CONDENSER_EVENT_HANDLER) (CondenserManager* condenser_manager, Condenser *condenser, void *data);

int condenser_manager_signal_close_handler_connect (CondenserManager* condenser_manager,
	CONDENSER_MANAGER_EVENT_HANDLER handler, void *callback_data);

int condenser_manager_signal_condenser_changed_handler_connect(CondenserManager* condenser_manager,
	CONDENSER_MANAGER_CHANGE_EVENT_HANDLER handler, void *callback_data);

int  CVICALLBACK OnCondenserAddEditOkClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCondenserChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCondenserSetup(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCondenserClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCondenserDetailsAdd (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCondenserDetailsEdit (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif

 
