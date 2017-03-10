#ifndef __Z_DRIVE__
#define __Z_DRIVE__

#include "HardWareTypes.h"     
#include "HardWareDevice.h" 

#include "signals.h"
#include "gci_ui_module.h"   
#include "gci_utils.h" 

#define Z_DRIVE_SUCCESS 0
#define Z_DRIVE_ERROR -1

#define Z_DRIVE_VTABLE_PTR(ob, member) ((ob)->lpVtbl.member)
#define Z_DRIVE_VTABLE(ob, member) (*((ob)->lpVtbl.member))

#define CHECK_Z_DRIVE_VTABLE_PTR(ob, member) if(Z_DRIVE_VTABLE_PTR(ob, member) == NULL) { \
    send_z_drive_error_text(ob, "member not implemented"); \
    return Z_DRIVE_ERROR; \
}  

#define CALL_Z_DRIVE_VTABLE_PTR(ob, member) if( Z_DRIVE_VTABLE(ob, member)(ob) == Z_DRIVE_ERROR ) { \
	send_z_drive_error_text(ob, "member failed");  \
	return Z_DRIVE_ERROR; \
}


typedef struct
{
	int (*hw_initialise) (Z_Drive* zd); 
	int (*initialise) (Z_Drive* zd);    
	int (*destroy) (Z_Drive* zd);
	int (*z_drive_get_position) (Z_Drive* zd, double *focus_microns);
	int (*z_drive_set_position) (Z_Drive* zd, double focus_microns); 
	int (*z_drive_set_zero) (Z_Drive* zd);
	int (*z_drive_enable_autofocus) (Z_Drive* zd, int enable);
	int (*z_drive_wait_for_stop_moving) (Z_Drive* zd, double timeout);
	int (*z_drive_get_speed) (Z_Drive* zd, double *speed);
	int (*z_drive_get_accel) (Z_Drive* zd, double *accel);
	int (*z_drive_enable_disable_timers) (Z_Drive* zd, int enable);

} Z_DriveVtbl;


struct _Z_Drive
{
  HardwareDevice parent; 
  
  Z_DriveVtbl lpVtbl;
 
  int		 _panel_id, wait_panel_id;
  int		 _lock;
  int		 _setup_panel_id;
  int     	 _timer;
  int		 _initialised; 
  int		 _hw_initialised;  
  int		 _fast_reponse_ui;
  int		 _prevent_change_signal_emit;
  int		 _reverse;
  double	 _min_microns;
  double	 _max_microns;
  double 	 _steps_per_micron;
  double     _stored_pos;
  double     _z_step;
  double     _speed;
  double	 _centre_position;
  int		 _centre_position_set;
  int 		 _respond_to_event_val_changed;
  double 	 _current_pos;
  double	 _prev_focus;

  int		 _use_cached_position;
  int		 _is_part_of_stage;
  int        _busy;
  double     _busy_time;
  
  char		_data_dir[GCI_MAX_PATHNAME_LEN];
};


void z_drive_constructor(Z_Drive* zd, const char *name, const char *description, const char *data_dir);

void zdrive_get_lock(Z_Drive* zd, const char *name);
void zdrive_release_lock(Z_Drive* zd, const char *name);

void zdrive_dont_respond_to_event_val_changed(Z_Drive* zd);    

void zdrive_use_cached_data_for_read(Z_Drive* zd, int val);

int z_drive_hardware_initialise(Z_Drive* zd); 
int z_drive_initialise(Z_Drive* zd);    
int z_drive_is_initialised(Z_Drive* zd);
void z_drive_set_is_part_of_stage(Z_Drive* zd);
int z_drive_is_part_of_stage(Z_Drive* zd);
void z_drive_emit_change_signal(Z_Drive* zd, int update);
void z_drive_emit_move_begin_signal(Z_Drive* zd, int update);
void z_drive_prevent_changed_signal_emission(Z_Drive* zd);
void z_drive_allow_changed_signal_emission(Z_Drive* zd);
void z_drive_set_setup_panel(Z_Drive* zd, int panel_id);
void z_drive_display_setup_panel(Z_Drive* zd);
int z_drive_disable_timers(Z_Drive* zd);
int z_drive_enable_timers(Z_Drive* zd);

int  send_z_drive_error_text (Z_Drive* zd, char fmt[], ...);

void z_drive_set_error_handler(Z_Drive* zd, UI_MODULE_ERROR_HANDLER handler, void *callback_data);

int  z_drive_destroy(Z_Drive* zd);
int  z_drive_set_min_max (Z_Drive* zd, double min, double max);
int  z_drive_get_position(Z_Drive* zd, double *focus_microns);
int  z_drive_set_position(Z_Drive* zd, double focus_microns);
int  z_drive_update_current_position(Z_Drive* zd, double focus_microns);
int  z_drive_set_position_no_commit(Z_Drive* zd, double focus);
int  z_drive_set_min_position(Z_Drive* zd);
int  z_drive_set_max_position(Z_Drive* zd);
int	 z_drive_set_zero(Z_Drive* zd);

void z_drive_set_centre_position(Z_Drive* zd, double position);

int	 z_drive_get_speed(Z_Drive* zd, double *speed);
int  z_drive_get_accel(Z_Drive* zd, double *accel);

int  z_drive_save_settings_in_ini_fmt (Z_Drive* zd, const char *filepath);    
int  z_drive_load_settings_from_ini_fmt (Z_Drive* zd, const char *filepath); 

void z_drive_hide_autofocus_controls(Z_Drive* zd);
void z_drive_reveal_message_controls(Z_Drive* zd);
void z_drive_reveal_set_datum_control(Z_Drive* zd);
void z_drive_set_message(Z_Drive* zd, char *message);

int  z_drive_is_busy (Z_Drive* zd);
int  z_drive_wait_for_stop_moving (Z_Drive* zd, double timeout);

// Signals
typedef void (*Z_DRIVE_EVENT_HANDLER) (Z_Drive* zd, void *data); 
typedef void (*Z_DRIVE_INT_EVENT_HANDLER) (Z_Drive* zd, int commit, void *data); 

int z_drive_move_begin_handler_connect(Z_Drive* zd, Z_DRIVE_EVENT_HANDLER handler, void *data );
int z_drive_signal_hide_handler_connect (Z_Drive* zd, Z_DRIVE_EVENT_HANDLER handler, void *callback_data);
int z_drive_changed_handler_connect(Z_Drive* zd, Z_DRIVE_EVENT_HANDLER handler, void *data);
int z_drive_changed_by_user_handler_connect(Z_Drive* zd, Z_DRIVE_INT_EVENT_HANDLER handler, void *data );
int z_drive_setup_displayed_handler_connect(Z_Drive* zd, Z_DRIVE_EVENT_HANDLER handler, void *data);

#endif

 
