#ifndef __PRECISEXCITE__
#define __PRECISEXCITE__

#include "HardwareTypes.h"
#include "HardWareDevice.h" 

#include "gci_ui_module.h"
#include "iniparser.h"
#include "signals.h"

#define PRECISEXCITE_SUCCESS 0
#define PRECISEXCITE_ERROR -1

typedef enum {PE_NO_CHANNEL, PE_VIOLET, PE_BLUE, PE_GREEN} PRECITE_EXCITE_CHANNELS;
typedef enum {PE_OFF, PE_ON} PRECITE_EXCITE_STATUS;

typedef struct
{
	int intensity;
	int status;
	int on_off_ctrl_id;
	int intensity_ctrl_id;
	int excitation_wavelength;

} ChannelInfo;

struct _precisExcite
{
  HardwareDevice parent;

  unsigned int 	_conversation_handle;
  int	 	 	_connected;
  int 	 	 	_lock;
  int     	 	_timer;
  int	 	 	_main_ui_panel;
  int	 	 	_setup_panel;
 				
  int			_active_channel;
  ChannelInfo	_channels_status[4];
  int			_trigger_edge; 

  int			_initialised;
};


precisExcite* precise_excite_new(char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir);

int precise_excite_hardware_initialise (precisExcite* precise_excite);
int precise_excite_initialise (precisExcite* precise_excite);
int precise_excite_hardware_is_initialised(precisExcite* precise_excite);

int  send_precise_excite_error_text (precisExcite* precise_excite, char fmt[], ...);
void precise_excite_set_error_handler(precisExcite* precise_excite, void (*handler) (char *error_string, precisExcite* precise_excite));

int  precise_excite_destroy(precisExcite* precise_excite);

void precise_excite_on_change(precisExcite* precise_excite);

int  precise_excite_display_main_ui (precisExcite* precise_excite);
int  precise_excite_hide_main_ui (precisExcite* precise_excite);
int  precise_excite_is_main_ui_visible (precisExcite* precise_excite);

int precise_excite_display_setup_ui(precisExcite* precise_excite);
int precise_excite_hide_setup_ui(precisExcite* precise_excite);
int precise_excite_is_setup_ui_visible(precisExcite* precise_excite);

int preciseExcite_connect(precisExcite* precise_excite, char* ip_address, unsigned int port);
int precise_excite_connect_to_default_port(precisExcite* precise_excite);
int preciseExcite_write(precisExcite* precise_excite, char* data);
int preciseExcite_read(precisExcite* precise_excite, char* data);

int preciseExcite_set_intensity(precisExcite* precise_excite, PRECITE_EXCITE_CHANNELS channel, int val);
int preciseExcite_set_channel_on_off(precisExcite* precise_excite, PRECITE_EXCITE_CHANNELS channel, PRECITE_EXCITE_STATUS val);
int preciseExcite_set_active_channel_on_off(precisExcite* precise_excite, PRECITE_EXCITE_STATUS val);
int preciseExcite_pulse_channel(precisExcite* precise_excite, PRECITE_EXCITE_CHANNELS channel, double secs);
int preciseExcite_pulse_channel_fudge(precisExcite* precise_excite, PRECITE_EXCITE_CHANNELS channel, double secs);
int preciseExcite_arm_channel(precisExcite* precise_excite, PRECITE_EXCITE_CHANNELS channel);
int preciseExcite_get_intensity(precisExcite* precise_excite, int channel);
int preciseExcite_get_active_channel_intensity(precisExcite* precise_excite);
int preciseExcite_get_active_channel_excitation_wavelength(precisExcite* precise_excite);
int preciseExcite_enable_chanel_for_wavelength_range(precisExcite* precise_excite, int min_wavelength,
													 int max_wavelength);

// Signals
typedef void (*PRECISEXCITE_EVENT_HANDLER) (precisExcite* precise_excite, void *data); 

int precise_excite_signal_hide_handler_connect (precisExcite* precise_excite, PRECISEXCITE_EVENT_HANDLER handler, void *callback_data);
int precise_excite_changed_handler(precisExcite* precise_excite, PRECISEXCITE_EVENT_HANDLER handler, void *data );

int CVICALLBACK OnPrecisExciteClose (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);  
int CVICALLBACK OnPrecisExciteSetup (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnSetIntensityVal (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnSetChannelOn (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnPulse (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnArm (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnConnect (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int CVICALLBACK OnSetupQuit (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
                                                                                                                                                                                                                                       
#endif

 
