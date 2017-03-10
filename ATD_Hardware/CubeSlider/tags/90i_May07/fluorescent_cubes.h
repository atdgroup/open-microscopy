#ifndef __FLUORESCENCE_CUBE_MANAGER__
#define __FLUORESCENCE_CUBE_MANAGER__

#include "signals.h"
#include "toolbox.h" 

////////////////////////////////////////////////////////////////////////////
//RJL/GP April 2006
//GCI 90i Microscope system. 
//Fluorescent cube control.
////////////////////////////////////////////////////////////////////////////

//#define ENABLE_CUBE_STATUS_POLLING 

#define CUBE_MANAGER_SUCCESS 0
#define CUBE_MANAGER_ERROR -1

#define MAX_NUMBER_OF_CUBES 15
#define CUBE_TURRET_SIZE 6

#define CUBE_MANAGER_VTABLE_PTR(ob, member) ((ob)->lpVtbl->member)
#define CUBE_MANAGER_VTABLE(ob, member) (*((ob)->lpVtbl->member))

#define CHECK_CUBE_MANAGER_VTABLE_PTR(ob, member) if(CUBE_MANAGER_VTABLE_PTR(ob, member) == NULL) { \
    send_fluocube_error_text(ob, "member not implemented"); \
    return CUBE_MANAGER_ERROR; \
}  

#define CALL_CUBE_MANAGER_VTABLE_PTR(ob, member) if(CUBE_MANAGER_VTABLE(ob, member)(ob) == CUBE_MANAGER_ERROR ) { \
	send_fluocube_error_text(ob, "member failed");  \
	return CUBE_MANAGER_ERROR; \
}


typedef struct {

	char	name[10];
	int		id;
	int		position;
	int		exc_nm;
	int		dichroic_nm;
	int		emm_min_nm;
	int		emm_max_nm;

} FluoCube;


typedef struct _FluoCubeManager FluoCubeManager;


typedef struct
{
	int (*destroy) (FluoCubeManager* cube_manager);
	int (*move_to_cube_position) (FluoCubeManager* cube_manager, int position);
	int (*get_current_cube_position) (FluoCubeManager* cube_manager, int *position);

} FluoCubeManagerVtbl;


struct _FluoCubeManager {
 
  CAObjHandle hCube;
  FluoCubeManagerVtbl *lpVtbl;
 
  char  	*_name;
  char  	*_description;
  char 		*_data_file;
 
  ListType	 _list;
  
  int		 number_of_present_cubes;
  int		 _current_position;
  int		 _default_pos;
  int	 	 _i2c_port;
  int 	 	 _lock;
  int     	 _timer;
  int	 	 _main_ui_panel;
  int	 	 _cube_table_panel;
  int	 	 _details_ui_panel;
 
  int		 _mounted;
  int		 _data_modified;
  
  signal_table signal_table;
  
  void (*_error_handler) (char *error_string, FluoCubeManager* cube_manager);  
};


FluoCubeManager* cube_manager_new(char *name, char *description, const char *data_file, size_t size);

int  send_fluocube_error_text (FluoCubeManager* cube_manager, char fmt[], ...);

void cube_manager_set_error_handler(FluoCubeManager* cube_manager, void (*handler) (char *error_string, FluoCubeManager *cube_manager));

int  cube_manager_set_i2c_port(FluoCubeManager* cube_manager, int port);
int  cube_manager_set_description(FluoCubeManager* cube_manager, const char* description);
int  cube_manager_get_description(FluoCubeManager* cube_manager, char *description);
int  cube_manager_set_name(FluoCubeManager* cube_manager, char* name);
int  cube_manager_get_name(FluoCubeManager* cube_manager, char *name);
int  cube_manager_set_datafile(FluoCubeManager* cube_manager, const char* data_file);
int  cube_manager_destroy(FluoCubeManager* cube_manager);
void cube_manager_stop_timer(FluoCubeManager* cube_manager);
void cube_manager_start_timer(FluoCubeManager* cube_manager);
void cube_manager_on_change(FluoCubeManager* cube_manager);

int  cube_manager_goto_default_position(FluoCubeManager* cube_manager);
int  cube_manager_set_number_of_cubes(FluoCubeManager* cube_manager, int number_of_cubes);
int  cube_manager_get_number_of_cubes(FluoCubeManager* cube_manager, int *number_of_cubes);
int  cube_manager_get_current_cube_position(FluoCubeManager* cube_manager, int *position);
int  cube_manager_current_cube_position(FluoCubeManager* cube_manager, int *position);
int  cube_manager_get_current_cube(FluoCubeManager* cube_manager, FluoCube *cube);
int  cube_manager_get_cube(FluoCubeManager* cube_manager, int cube_number, FluoCube *cube);
int  cube_manager_get_cube_for_position(FluoCubeManager* cube_manager, int position, FluoCube* dst_cube);
FluoCube* cube_manager_get_cube_ptr_for_position(FluoCubeManager* cube_manager, int position);
FluoCube* cube_manager_get_cube_ptr_for_id(FluoCubeManager* cube_manager, int id);
int  cube_manager_move_to_position(FluoCubeManager* cube_manager, int position);
int  cube_manager_move_to_empty_position(FluoCubeManager* cube_manager);

int  cube_manager_display_main_ui (FluoCubeManager* cube_manager);
int  cube_manager_hide_main_ui (FluoCubeManager* cube_manager);
int  cube_manager_is_main_ui_visible (FluoCubeManager* cube_manager);
int  cube_manager_display_config_ui(FluoCubeManager* cube_manager);
int  cube_manager_hide_config_ui(FluoCubeManager* cube_manager);
int  cube_manager_is_config_ui_visible(FluoCubeManager* cube_manager);

int  cube_manager_save_cube_data(FluoCubeManager* cube_manager, const char *filepath);
int  cube_manager_load_cube_file(FluoCubeManager* cube_manager, const char *filepath);
int  cube_manager_load_all_possible_cubes_into_ui(FluoCubeManager* cube_manager);

int  cube_manager_add_cube_ui(FluoCubeManager* cube_manager);
int  cube_manager_edit_cube_ui(FluoCubeManager* cube_manager, int index);

int  cube_manager_switch_active_position(FluoCubeManager* cube_manager, int id1, int id2);
int  cube_manager_change_to_active(FluoCubeManager* cube_manager, int id);
int  cube_manager_remove_obj_at_active_position(FluoCubeManager* cube_manager, int id);

int  cube_manager_add_cube(FluoCubeManager* cube_manager);
int  cube_manager_edit_cube(FluoCubeManager* cube_manager, int index);
int  cube_manager_remove_cube(FluoCubeManager* cube_manager, int id);

// Signals
typedef void (*CUBE_MANAGER_EVENT_HANDLER) (FluoCubeManager* cube_manager, void *data); 
typedef void (*CUBE_MANAGER_CUBE_EVENT_HANDLER) (FluoCubeManager* cube_manager, FluoCube *cube, void *data);

int cube_manager_signal_close_handler_connect (FluoCubeManager* cube_manager,
	CUBE_MANAGER_EVENT_HANDLER handler, void *callback_data);

int cube_manager_signal_cube_changed_handler_connect(FluoCubeManager* cube_manager,
	CUBE_MANAGER_EVENT_HANDLER handler, void *callback_data);

#endif

 
