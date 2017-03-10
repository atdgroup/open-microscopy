#ifndef __PMTSET__
#define __PMTSET__

#include "HardWareTypes.h"    
#include "HardWareDevice.h" 

#include "signals.h"
#include "gci_ui_module.h"
#include "device_list.h"
#include "toolbox.h" 

#define PMTSET_SUCCESS 0
#define PMTSET_ERROR -1

#define PMTSET_VTABLE_PTR(ob, member) ((ob)->vtable.member)
#define PMTSET_VTABLE(ob, member) (*((ob)->vtable.member))

#define CHECK_PMTSET_VTABLE_PTR(ob, member) if(PMTSET_VTABLE_PTR(ob, member) == NULL) { \
    send_pmtset_error_text(ob, "member not implemented"); \
    return PMTSET_ERROR; \
}  

#define CALL_PMTSET_VTABLE_PTR(ob, member) if(PMTSET_VTABLE(ob, member)(ob) == PMTSET_ERROR ) { \
	send_pmtset_error_text(ob, "member failed");  \
	return PMTSET_ERROR; \
}

typedef struct
{
	int (*hw_init) (PmtSet* pmtset, int move_to_default);  
	int (*init) (PmtSet* pmtset); 
	int (*destroy) (PmtSet* pmtset);
	int (*move_to_pmtset_position) (PmtSet* pmtset, int position);
	int (*get_current_pmtset_position) (PmtSet* pmtset, int *position);
	int (*setup_pmtset) (PmtSet* pmtset);
	int (*hide_pmtset_calib) (PmtSet* pmtset);

} PmtSetVtbl;


struct _PmtSet {
 
  HardwareDevice parent;   
  
  ModuleDeviceConfigurator* dc;  
  
  PmtSetVtbl vtable;
  
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


int CVICALLBACK OnPmtSetDetailsAdd (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK OnPmtSetDetailsEdit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);


PmtSet* pmtset_new(const char *name, const char *description, const char* data_dir, const char *data_file, size_t size);

// Do NOT call any ui stuff here.
// It is meant for hardware initialisation and thus possibly called from
// another thread.
int pmtset_hardware_initialise(PmtSet* pmtset); 
int pmtset_initialise(PmtSet* pmtset); 
int pmtset_is_initialised(PmtSet* pmtset); 

int  send_pmtset_error_text (PmtSet* pmtset, char fmt[], ...);

void pmtset_set_error_handler(PmtSet* pmtset, UI_MODULE_ERROR_HANDLER handler, void *callback_data);
int  pmtset_destroy(PmtSet* pmtset);
void pmtset_stop_timer(PmtSet* pmtset);
void pmtset_start_timer(PmtSet* pmtset);
int  pmtset_goto_default_position(PmtSet* pmtset);
int  pmtset_get_number_of_positions(PmtSet* pmtset, int *number_of_cubes);
int  pmtset_get_current_position(PmtSet* pmtset, int *position);
int  pmtset_get_device_name_for_pos(PmtSet* pmtset, char *name, int pos);
int  pmtset_load_active_paths_into_list_control(PmtSet* pmtset, int panel, int ctrl);
int  pmtset_move_to_position(PmtSet* pmtset, int position);
int  pmtset_display_calib_ui(PmtSet* pmtset);
int  pmtset_hide_calib_ui(PmtSet* pmtset);  
int pmtset_set_list_control_to_pos(PmtSet* pmtset, int panel, int ctrl, int pos);

// Signals
typedef void (*PMTSET_EVENT_HANDLER) (PmtSet* pmtset, void *data); 
typedef void (*PMTSET_CHANGE_EVENT_HANDLER) (PmtSet* pmtset, int pos, void *data);

int pmtset_signal_close_handler_connect (PmtSet* pmtset,
	PMTSET_EVENT_HANDLER handler, void *callback_data);

int pmtset_signal_changed_handler_connect(PmtSet* pmtset,
	PMTSET_CHANGE_EVENT_HANDLER handler, void *callback_data);

int pmtset_signal_pre_change_handler_connect(PmtSet* pmtset,
	PMTSET_CHANGE_EVENT_HANDLER handler, void *callback_data);

int pmtset_signal_config_changed_handler_connect(PmtSet* pmtset,
	PMTSET_EVENT_HANDLER handler, void *callback_data);


int  CVICALLBACK OnPmtSetAddEditOkClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnPmtSetCalibrate(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnPmtSetChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnPmtSetClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnPmtSetSetup(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif

 
