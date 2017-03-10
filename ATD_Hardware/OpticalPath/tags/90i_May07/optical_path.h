#ifndef __OPTICAL_PATH_MANAGER__
#define __OPTICAL_PATH_MANAGER__

#include "signals.h"
#include "toolbox.h" 

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Optical Path control.
////////////////////////////////////////////////////////////////////////////

//#define ENABLE_OPTICAL_PATH_POLLING

#define OPTICAL_PATH_MANAGER_SUCCESS 0
#define OPTICAL_PATH_MANAGER_ERROR -1

#define MAX_NUMBER_OF_OPTICAL_PATHS 15
#define OPTICAL_PATH_TURRET_SIZE 3

#define OPTICAL_PATH_MANAGER_VTABLE_PTR(ob, member) ((ob)->lpVtbl->member)
#define OPTICAL_PATH_MANAGER_VTABLE(ob, member) (*((ob)->lpVtbl->member))

#define CHECK_OPTICAL_PATH_MANAGER_VTABLE_PTR(ob, member) if(OPTICAL_PATH_MANAGER_VTABLE_PTR(ob, member) == NULL) { \
    send_optical_path_error_text(ob, "member not implemented"); \
    return OPTICAL_PATH_MANAGER_ERROR; \
}  

#define CALL_OPTICAL_PATH_MANAGER_VTABLE_PTR(ob, member) if(OPTICAL_PATH_MANAGER_VTABLE(ob, member)(ob) == OPTICAL_PATH_MANAGER_ERROR ) { \
	send_optical_path_error_text(ob, "member failed");  \
	return OPTICAL_PATH_MANAGER_ERROR; \
}


typedef struct {

	char	name[10];
	int		id;
	int		position;

} OpticalPath;


typedef struct _OpticalPathManager OpticalPathManager;


typedef struct
{
	int (*destroy) (OpticalPathManager* optical_path_manager);
	int (*move_to_optical_path_position) (OpticalPathManager* optical_path_manager, int position);
	int (*get_current_optical_path_position) (OpticalPathManager* optical_path_manager, int *position);

} OpticalPathManagerVtbl;


struct _OpticalPathManager {
 
  OpticalPathManagerVtbl *lpVtbl;
 
  char  	*_name;
  char  	*_description;
  char 		*_data_file;
 
  ListType	 _list;
  
  int		number_of_present_optical_paths;
  
  int	 	 _i2c_port;
  int 	 	 _lock;
  int     	 _timer;
  int	 	 _main_ui_panel;
  int	 	 _optical_path_table_panel;
  int	 	 _details_ui_panel;
 
  int		 _mounted;
  int		 _data_modified;
  
  signal_table signal_table;
  
  void (*_error_handler) (char *error_string, OpticalPathManager* optical_path_manager);  
};


OpticalPathManager* optical_path_manager_new(char *name, char *description, const char *data_file, size_t size);

int  send_optical_path_error_text (OpticalPathManager* optical_path_manager, char fmt[], ...);

void optical_path_manager_set_error_handler(OpticalPathManager* optical_path_manager, void (*handler) (char *error_string, OpticalPathManager *optical_path_manager));

int  optical_path_manager_set_i2c_port(OpticalPathManager* optical_path_manager, int port);
int  optical_path_manager_set_description(OpticalPathManager* optical_path_manager, const char* description);
int  optical_path_manager_get_description(OpticalPathManager* optical_path_manager, char *description);
int  optical_path_manager_set_name(OpticalPathManager* optical_path_manager, char* name);
int  optical_path_manager_get_name(OpticalPathManager* optical_path_manager, char *name);
int  optical_path_manager_set_datafile(OpticalPathManager* optical_path_manager, const char* data_file);
int  optical_path_manager_destroy(OpticalPathManager* optical_path_manager);

int  optical_path_manager_set_number_of_optical_paths(OpticalPathManager* optical_path_manager, int number_of_optical_paths);
int  optical_path_manager_get_number_of_optical_paths(OpticalPathManager* optical_path_manager, int *number_of_optical_paths);
int  optical_path_manager_get_current_optical_path_position(OpticalPathManager* optical_path_manager, int *position);
int  optical_path_manager_get_current_optical_path(OpticalPathManager* optical_path_manager, OpticalPath *optical_path);
int  optical_path_manager_get_optical_path(OpticalPathManager* optical_path_manager, int optical_path_number, OpticalPath *optical_path);
int optical_path_manager_get_optical_path_for_position(OpticalPathManager* optical_path_manager, int position, OpticalPath* optical_path);
OpticalPath* optical_path_manager_get_optical_path_ptr_for_position(OpticalPathManager* optical_path_manager, int position);
OpticalPath* optical_path_manager_get_optical_path_ptr_for_id(OpticalPathManager* optical_path_manager, int id);
int  optical_path_manager_move_to_position(OpticalPathManager* optical_path_manager, int position);

int  optical_path_manager_display_main_ui (OpticalPathManager* optical_path_manager);
int  optical_path_manager_hide_main_ui (OpticalPathManager* optical_path_manager);
int  optical_path_manager_is_main_ui_visible (OpticalPathManager* optical_path_manager);
int  optical_path_manager_display_config_ui(OpticalPathManager* optical_path_manager);
int  optical_path_manager_hide_config_ui(OpticalPathManager* optical_path_manager);
int  optical_path_manager_is_config_ui_visible(OpticalPathManager* optical_path_manager);

int  optical_path_manager_save_optical_path_data(OpticalPathManager* optical_path_manager, const char *filepath);
int  optical_path_manager_load_optical_path_file(OpticalPathManager* optical_path_manager, const char *filepath);
int  optical_path_manager_load_all_possible_optical_paths_into_ui(OpticalPathManager* optical_path_manager);

int  optical_path_manager_add_optical_path_ui(OpticalPathManager* optical_path_manager);
int  optical_path_manager_edit_optical_path_ui(OpticalPathManager* optical_path_manager, int index);

int  optical_path_manager_switch_active_position(OpticalPathManager* optical_path_manager, int id1, int id2);
int  optical_path_manager_change_to_active(OpticalPathManager* optical_path_manager, int id);
int  optical_path_manager_remove_optical_path_at_active_position(OpticalPathManager* optical_path_manager, int id);

int  optical_path_manager_add_optical_path(OpticalPathManager* optical_path_manager);
int  optical_path_manager_edit_optical_path(OpticalPathManager* optical_path_manager, int index);
int  optical_path_manager_remove_optical_path(OpticalPathManager* optical_path_manager, int id);

void optical_path_manager_start_timer(OpticalPathManager* optical_path_manager);
void optical_path_manager_stop_timer(OpticalPathManager* optical_path_manager);
void optical_path_manager_on_change(OpticalPathManager* optical_path_manager);

// Signals
typedef void (*OPTICAL_PATH_MANAGER_EVENT_HANDLER) (OpticalPathManager* optical_path_manager, void *data); 
typedef void (*OPTICAL_PATH_MANAGER_OPTICAL_PATH_EVENT_HANDLER) (OpticalPathManager* optical_path_manager, OpticalPath *optical_path, void *data);

int optical_path_manager_signal_close_handler_connect (OpticalPathManager* optical_path_manager,
	OPTICAL_PATH_MANAGER_EVENT_HANDLER handler, void *callback_data);

int optical_path_manager_signal_optical_path_changed_handler_connect(OpticalPathManager* optical_path_manager,
	OPTICAL_PATH_MANAGER_EVENT_HANDLER handler, void *callback_data);

#endif

 
