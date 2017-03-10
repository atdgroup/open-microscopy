#ifndef __FILTERSET__
#define __FILTERSET__

#include "HardWareTypes.h"      
#include "HardWareDevice.h" 

#include "signals.h"
#include "gci_ui_module.h"
#include "device_list.h"
#include "toolbox.h" 

#define FILTERSET_SUCCESS 0
#define FILTERSET_ERROR -1

#define FILTERSET_VTABLE_PTR(ob, member) ((ob)->vtable.member)
#define FILTERSET_VTABLE(ob, member) (*((ob)->vtable.member))

#define CHECK_FILTERSET_VTABLE_PTR(ob, member) if(FILTERSET_VTABLE_PTR(ob, member) == NULL) { \
    send_fluofilter_error_text(ob, "member not implemented"); \
    return FILTERSET_ERROR; \
}  

#define CALL_FILTERSET_VTABLE_PTR(ob, member) if(FILTERSET_VTABLE(ob, member)(ob) == FILTERSET_ERROR ) { \
	send_fluofilter_error_text(ob, "member failed");  \
	return FILTERSET_ERROR; \
}

typedef struct _FilterSet FilterSet;

#define FILTER_NAME_LEN 50

struct _FilterSet
{
	int		position;
	int		exc_min_nm;
	int		exc_max_nm;
	int		dichroic_nm;
	int		emm_min_nm;
	int		emm_max_nm;

	char	name[FILTER_NAME_LEN];
	char	exc_name[FILTER_NAME_LEN];
	char	emm_name[FILTER_NAME_LEN];
	char	dic_name[FILTER_NAME_LEN];
};


typedef struct
{
	int	(*hardware_init) (FilterSetCollection* filterset); // Intended to be re-entrant (thread safe)
	int	(*initialise) (FilterSetCollection* filterset);
	int (*destroy) (FilterSetCollection* filterset);
	int (*move_to_filter_position) (FilterSetCollection* filterset, int position);
	int (*get_current_filter_position) (FilterSetCollection* filterset, int *position);

} FilterSetCollectionVtbl;


struct _FilterSetCollection
{
  HardwareDevice parent; 
  
  ModuleDeviceConfigurator* dc;
	
  FilterSetCollectionVtbl vtable;
  
  int 		 _hw_initialised;  
  int 		 _initialised;
  int     	 _timer;
  int	 	 _main_ui_panel;
  int		 _current_pos;
  int		 _requested_pos;
  
  volatile int _prevent_timer_callback;
  volatile int _abort_filter_move_check;
  int		   _move_pos_thread_id;

  double    _timer_interval;
  
  // This panel is the add / edit panel
  int	 	 _details_ui_panel;
};


int CVICALLBACK OnFilterDetailsAdd (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

int CVICALLBACK OnFilterDetailsEdit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2);

FilterSetCollection* filterset_new(const char *name, const char *description, const char *data_dir, const char *data_file, size_t size);

// Do NOT call any ui stuff here.
// It is meant for hardware initialisation and thus possibly called from
// another thread.
int filterset_hardware_initialise(FilterSetCollection* filterset); 

int filterset_initialise(FilterSetCollection* filterset);

int filterset_is_initialised(FilterSetCollection* filterset); 

int  send_fluofilter_error_text (FilterSetCollection* filterset, char fmt[], ...);

void filterset_set_error_handler(FilterSetCollection* filterset, UI_MODULE_ERROR_HANDLER handler, void *callback_data);

int  filterset_destroy(FilterSetCollection* filterset);
void filterset_stop_timer(FilterSetCollection* filterset);
void filterset_start_timer(FilterSetCollection* filterset);

int  filterset_goto_default_position(FilterSetCollection* filterset);
int  filterset_get_number_of_filters(FilterSetCollection* filterset, int *number_of_filters);
int  filterset_get_current_filter_position(FilterSetCollection* filterset, int *position);
int  filterset_get_current_filterset(FilterSetCollection* filterset, FilterSet *filter);
int  filterset_get_filter_for_position(FilterSetCollection* filterset, int position, FilterSet* dst_filter);

double filterset_get_average_emmision(FilterSet filter);

int  filterset_load_active_filters_into_list_control(FilterSetCollection* filterset, int panel, int ctrl);

int  filterset_move_to_position(FilterSetCollection* filterset, int position);
int  filterset_wait_for_stop_moving (FilterSetCollection* filterset, double timeout);

// Client must free the returned array
FilterSet* filterset_get_active_filters(FilterSetCollection* filterset);


// Signals
typedef void (*FILTERSET_EVENT_HANDLER) (FilterSetCollection* filterset, void *data); 
typedef void (*FILTERSET_CUBE_EVENT_HANDLER) (FilterSetCollection* filterset, FilterSet filter, void *data);
typedef void (*FILTERSET_CHANGE_EVENT_HANDLER) (FilterSetCollection* filterset, int pos, void *data); 

int filterset_signal_close_handler_connect (FilterSetCollection* filterset,
	FILTERSET_EVENT_HANDLER handler, void *callback_data);

int filterset_signal_filter_changed_handler_connect(FilterSetCollection* filterset,
	FILTERSET_CHANGE_EVENT_HANDLER handler, void *callback_data);

int filterset_signal_filter_config_changed_handler_connect(FilterSetCollection* filterset,
	FILTERSET_EVENT_HANDLER handler, void *callback_data);

int  CVICALLBACK OnFilterAddEditOkClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnFilterChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnFilterSetup(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnFilterClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);

#endif

 
