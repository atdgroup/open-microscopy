#include "intensilight.h"
#include "intensilight_ui.h"

#include "asynctmr.h"
#include "GCI_utils.h"
#include "GL_CVIRegistry.h"

#include <tcpsupp.h>
#include <utility.h>
#include <ansi_c.h> 
#include <formatio.h>
#include <rs232.h>

static void error_handler (char *error_string, Intensilight* intensilight)
{
	LogError("Intensilight Error", error_string); 
}

static int INTENSILIGHT_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Intensilight*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Intensilight *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

static int INTENSILIGHT_PTR_INT_INT_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (Intensilight*, int, int, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (Intensilight *) args[0].void_ptr_data, (int) args[1].int_data, (int) args[2].int_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

static void intensilight_error_handler(Intensilight* intensilight, int err)
{
	char err_info[50];
	
	if (err == 0) return;
	
	switch (err) {
		case -1:
			strcpy(err_info, "Command error");
			break;
		case -2:
			strcpy(err_info, "Parameter error");
			break;
		case -3:
			strcpy(err_info, "Device control timeout");
			break;
		case -4:
			strcpy(err_info, "Receive buffer overflow");
			break;
		case -5:
			strcpy(err_info, "Unrecognised error");
			break;
		case -6:
			strcpy(err_info, "Command overflow");
			break;
		case -7:
			strcpy(err_info, "Processing disabled error");
			break;
		case -8:
			strcpy(err_info, "Initialisation error");
			break;
		case -9:
			strcpy(err_info, "Communications failure");
			break;
		default:
			strcpy(err_info, "Unknown error");
			break;
	}

	logger_log(UIMODULE_LOGGER(intensilight), LOGGER_ERROR, "Intensilight Error: %s", err_info);	
}

////////////////////////////////////////////////////////////
//Intensilight RX/TX functions.
//
//There are two types of TX command:-
//	c-type - command is sent - TE2000 responds with "o..." indicating successful completion
//	f-type - command is sent - 1. TE2000 responds with "q..." indicating successful receipt
//							   2. TE2000 responds with "o..." indicating successful completion
//
//All return 0 = success, 
//			-1 = command error.
//			-2 = parameter error.
//			-3 = device control timeout.
//			-4 = RX data buffer overflow.
//			-6 = drive command count overflow.
//			-7 = processing disabled error.
//			-8 = Initialisation failure.
//			-9 = Comms failure

static int RxCommand(Intensilight* intensilight, char *command, char *answer)
{
	char str[30];
	int error_code;
	size_t n;
	
	//Request data from Intensilight. 
	//Returned strings terminate with linefeed - ASCII 10
	//First character "a" indicates success. "n" indicates failure.

	memset (str, 0, 30);  //Clear reply buffer
	CmtGetLock (intensilight->_lock);

	FlushInQ(intensilight->_port);
	ComWrt (intensilight->_port, command, strlen(command));
	if (ComRdTerm (intensilight->_port, str, 30, 10) < 5) {
		intensilight_error_handler(intensilight, -9);
		CmtReleaseLock(intensilight->_lock);
		return INTENSILIGHT_ERROR;
	}

	//printf("Intensilight sending command %s recieved result %s\n", command, str);

	Delay(0.1);

	CmtReleaseLock(intensilight->_lock);

	
	if (strncmp (str, "a", 1)) {			//first character should be "a"
		n = strcspn (str, "0123456789");	//find first numeric character 
		sscanf(&str[n], "%d", &error_code);
		intensilight_error_handler(intensilight, error_code);
		return INTENSILIGHT_ERROR;
	}
	
	strcpy(answer, &str[4]);
	return INTENSILIGHT_SUCCESS;
}

static int RxCommandInt(Intensilight* intensilight, char *command, int *answer)
{
	char str[30];
	int n;
	
	if (RxCommand(intensilight, command, str) == INTENSILIGHT_ERROR)
		return INTENSILIGHT_ERROR;
		
	n = strcspn (str, "0123456789");	//find first numeric character 
	if (n<0) return INTENSILIGHT_ERROR;
	
	sscanf(&str[n], "%d", answer);
	return INTENSILIGHT_SUCCESS;
}

static int TxCommandF(Intensilight* intensilight, char *command, double timeout)
{
	char str[30];
	int error_code;
	
	//Send f_type command to Intensilight. 
	//Returned strings terminate with linefeed - ASCII 10
	//There are two responses. 
	//The first indicates receipt of the command. First character "q" indicates sucess.
	//The second indicates completion of the command. First character "o" indicates sucess.
	
	CmtGetLock (intensilight->_lock);

	FlushInQ(intensilight->_port);
	ComWrt (intensilight->_port, command, strlen(command));

	SetComTime (intensilight->_port, timeout); //allow time for device to move
	if (ComRdTerm (intensilight->_port, str, 30, 10) < 5) {
		intensilight_error_handler(intensilight, -9);
		CmtReleaseLock(intensilight->_lock);
		return INTENSILIGHT_ERROR;
	}
	if (strncmp (str, "q", 1)) {
		sscanf(&str[4], "%d", &error_code);
		intensilight_error_handler(intensilight, error_code);
		CmtReleaseLock(intensilight->_lock);
		return INTENSILIGHT_ERROR;
	}

	if (ComRdTerm (intensilight->_port, str, 10, 10) < 5) {
		intensilight_error_handler(intensilight, -9);
		CmtReleaseLock(intensilight->_lock);
		return INTENSILIGHT_ERROR;
	}
	if (strncmp (str, "o", 1)) {
		sscanf(&str[4], "%d", &error_code);
		intensilight_error_handler(intensilight, error_code);
		CmtReleaseLock(intensilight->_lock);
		return INTENSILIGHT_ERROR;
	}

	SetComTime (intensilight->_port, 5.0); //normal communications timeout
	CmtReleaseLock(intensilight->_lock);
	return INTENSILIGHT_SUCCESS;
}

static int TxCommandC(Intensilight* intensilight, char *command)
{
	char str[30];
	int error_code;
	
	//Send c_type command to TE2000. 
	//Returned strings terminate with linefeed - ASCII 10
	//First character "o" indicates sucess.
	
	CmtGetLock (intensilight->_lock);

	FlushInQ(intensilight->_port);
	ComWrt (intensilight->_port, command, strlen(command));
	Delay(0.002);
	if (ComRdTerm (intensilight->_port, str, 30, 10) < 5) {
		intensilight_error_handler(intensilight, -9);
		CmtReleaseLock(intensilight->_lock);
		return INTENSILIGHT_ERROR;
	}
	CmtReleaseLock(intensilight->_lock);

	if (strncmp (str, "o", 1)) {
		sscanf(&str[4], "%d", &error_code);
		intensilight_error_handler(intensilight, error_code);
		return INTENSILIGHT_ERROR;
	}

	return INTENSILIGHT_SUCCESS;
}

////////////////////////////////////////////////////////////

int intensilight_connect(Intensilight* intensilight)
{	
	char version[200] = "";

	if(initialise_comport(intensilight->_port, 9600) < 0)
		return INTENSILIGHT_ERROR; 	

	//Get the version number from the intensilight
	if (!RxCommand(intensilight, "rVEN\r", version) == INTENSILIGHT_SUCCESS)
		return INTENSILIGHT_ERROR; 	
			
	SetCtrlVal (intensilight->_main_ui_panel, I_MAIN_VERSION, version);
	SetComTime (intensilight->_port, 5.0); 
	intensilight->_connected = 1;
	
	return INTENSILIGHT_SUCCESS;
}

////////////////////////////////////////////////////////////
//ND filters

int intensilight_set_nd_filter(Intensilight* intensilight, int nd_filter)
{
	char command[10];
	
	//Set ND filter
	if (nd_filter < 1 || nd_filter > 6) return INTENSILIGHT_ERROR;
	
	sprintf(command, "cNDM%d\r", nd_filter);
	return TxCommandC(intensilight, command);
}

int intensilight_get_nd_filter(Intensilight* intensilight, int *nd_filter)
{
	//Get ND filter
	if (RxCommandInt(intensilight, "rNAR\r", nd_filter) == INTENSILIGHT_ERROR)
		return INTENSILIGHT_ERROR;
		
	intensilight->_nd_filter = *nd_filter;
	return INTENSILIGHT_SUCCESS;
}

////////////////////////////////////////////////////////////
//shutter

int intensilight_open_shutter(Intensilight* intensilight)
{
	return TxCommandC(intensilight, "cSXC1\r");
}

int intensilight_close_shutter(Intensilight* intensilight)
{
	return TxCommandC(intensilight, "cSXC2\r");
}

int intensilight_set_shutter_open_time(Intensilight* intensilight, int ms)
{
	//1 = 100ms, so round up to nearest 100 ms
	intensilight->_shutter_open_time = (ms+99)/100;
	return INTENSILIGHT_SUCCESS;
}

int intensilight_trigger_shutter(Intensilight* intensilight)
{
	char command[20]="", pstr[5]="";

    //Open shutter for a set length of time.
    //Send four chars with leading zeros. 1 == 100ms
    
    Fmt(pstr, "%s<%i[w4p0]", intensilight->_shutter_open_time); 
	sprintf(command, "cSXT1:%s\r", pstr);
	return TxCommandC(intensilight, command);
}

int intensilight_read_shutter(Intensilight* intensilight, int *shutter_status)
{
	//1 = open, 2 = closed
	if (RxCommandInt(intensilight, "rSXR\r", shutter_status) == INTENSILIGHT_ERROR)
		return INTENSILIGHT_ERROR;
		
	if (*shutter_status == 2)
		*shutter_status = 0;
		
	intensilight->_shutter_status = *shutter_status;
	return INTENSILIGHT_SUCCESS;
}

////////////////////////////////////////////////////////////
//Remote

int intensilight_set_remote_enable(Intensilight* intensilight, int enable)
{
	char command[10];

	if (enable < 0 || enable > 1) return INTENSILIGHT_ERROR;

	sprintf(command, "cREM%d\r", enable);
	return TxCommandC(intensilight, command);
}

int intensilight_get_remote_enable(Intensilight* intensilight, int *enabled)
{
	//1 = enabled
	if (RxCommandInt(intensilight, "rREN\r", enabled) == INTENSILIGHT_ERROR) 
		return INTENSILIGHT_ERROR;
		
	intensilight->_remote_enabled = *enabled;
	return INTENSILIGHT_SUCCESS;
}

int intensilight_set_remote_led(Intensilight* intensilight, int led)
{
	char command[10];

	//0 = off, 1=dark, 2=bright
	if (led < 0 || led > 2) return INTENSILIGHT_ERROR;
	
	sprintf(command, "cRIL%d\r", led);
	return TxCommandC(intensilight, command);
}

int intensilight_get_remote_led(Intensilight* intensilight, int *led)
{
	//0 = off, 1=dark, 2=bright
	if (RxCommandInt(intensilight, "rRIR\r", led) == INTENSILIGHT_ERROR)
		return INTENSILIGHT_ERROR;
		
	intensilight->_remote_led = *led;
	return INTENSILIGHT_SUCCESS;
}


int CVICALLBACK OnIntensilightTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	Intensilight* intensilight = (Intensilight*) callbackData;
	int nd_filter, shutter_status, changed = 0;
	static int prev_nd_filter=0, prev_shutter_status=0;
	double t1;
	
    switch (event)
    {
        case EVENT_TIMER_TICK:
		{
        	t1 = Timer();
			if (intensilight_get_nd_filter(intensilight, &nd_filter) == INTENSILIGHT_SUCCESS) {
			
				if (nd_filter != prev_nd_filter) {  //It's changed
			
					SetCtrlVal(intensilight->_main_ui_panel, I_MAIN_ND, intensilight->_nd_filter);
					changed = 1;
					prev_nd_filter = nd_filter;
				}
			}
			
			if (intensilight_read_shutter(intensilight, &shutter_status) == INTENSILIGHT_SUCCESS) {
			
				if (shutter_status != prev_shutter_status) {  //It's changed
				
					SetCtrlVal(intensilight->_main_ui_panel, I_MAIN_OPEN_LED, intensilight->_shutter_status);
					changed = 1;
					prev_shutter_status = shutter_status;
				}
			}
			
			if(changed)
				GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(intensilight), "intensilightChanged",
						GCI_VOID_POINTER, intensilight, GCI_INT, nd_filter, GCI_INT, shutter_status); 
			
            break;
		}
    }
    
    return 0;
}

static int intensilight_send_initial_settings(Intensilight* intensilight)
{
	int err;
	
	err = intensilight_set_nd_filter(intensilight, 6);  	//ND32
	if (err) return err;
	err = intensilight_close_shutter(intensilight);
	if (err) return err;
	err = intensilight_set_remote_enable(intensilight, 1);  //enabled
	if (err) return err;
	err = intensilight_set_remote_led(intensilight, 2);	  	//bright
	return err;
}

static void OnPanelsClosedOrHidden (UIModule *module, void *data)
{
	Intensilight* intensilight = (Intensilight*) data; 

	intensilight_stop_timer(intensilight);
}

static void OnPanelsDisplayed (UIModule *module, int panel_id, void *data)
{
	Intensilight* intensilight = (Intensilight*) data; 

	intensilight_start_timer(intensilight);
}

int intensilight_is_connected(Intensilight* intensilight)
{
	return intensilight->_connected;
}

int intensilight_hardware_initialise (Intensilight* intensilight)
{
	char device[UIMODULE_NAME_LEN];

	intensilight->_connected = 0;
	intensilight->_port = 0;

	ui_module_get_name(UIMODULE_CAST(intensilight), device);
	 
	get_device_int_param_from_ini_file  (device, "COM_Port", &(intensilight->_port));  
	
	intensilight_send_initial_settings(intensilight);
 
	intensilight->_initialised = 1;

	return INTENSILIGHT_SUCCESS;
}

int intensilight_initialise (Intensilight* intensilight)
{
	char device[UIMODULE_NAME_LEN];

	intensilight->_lock = -1;
	intensilight->_timer = -1;
	intensilight->_main_ui_panel = -1;
	intensilight->_nd_filter = 32;
	intensilight->_shutter_status = 0;
	intensilight->_shutter_open_time = 1;
	intensilight->_remote_enabled = 1;
	intensilight->_remote_led = 2;
	intensilight->_signal_delay = 20;
	intensilight->_bounce_delay = 60;

	CmtNewLock (NULL, 0, &intensilight->_lock);
	
	ui_module_get_name(UIMODULE_CAST(intensilight), device);
	 
	get_device_int_param_from_ini_file  (device, "COM_Port", &(intensilight->_port));  
	
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(intensilight), "intensilightChanged", INTENSILIGHT_PTR_INT_INT_MARSHALLER);

	intensilight->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(intensilight),
		"intensilight_ui.uir", I_MAIN, 1);   

	if ( InstallCtrlCallback (intensilight->_main_ui_panel, I_MAIN_CLOSE, cbIntensilightClose, intensilight) < 0)
		return INTENSILIGHT_ERROR;	
		
	if ( InstallCtrlCallback (intensilight->_main_ui_panel, I_MAIN_ND, cbIntensilightSetNdfilter, intensilight) < 0)
		return INTENSILIGHT_ERROR;	
		
	if ( InstallCtrlCallback (intensilight->_main_ui_panel, I_MAIN_OPEN_SHUTTER, cbIntensilightShutterOpen, intensilight) < 0)
		return INTENSILIGHT_ERROR;	
		
	if ( InstallCtrlCallback (intensilight->_main_ui_panel, I_MAIN_CLOSE_SHUTTER, cbIntensilightShutterClose, intensilight) < 0)
		return INTENSILIGHT_ERROR;	
		
	if ( InstallCtrlCallback (intensilight->_main_ui_panel, I_MAIN_TRIGGER_SHUTTER, cbIntensilightShutterTrigger, intensilight) < 0)
		return INTENSILIGHT_ERROR;	
		
	if ( InstallCtrlCallback (intensilight->_main_ui_panel, I_MAIN_SHUTTER_TIME, cbIntensilightShutterOpenTime, intensilight) < 0)
		return INTENSILIGHT_ERROR;	
		
	if ( InstallCtrlCallback (intensilight->_main_ui_panel, I_MAIN_TEST_SHUTTER, cbIntensilightShutterTest, intensilight) < 0)
		return INTENSILIGHT_ERROR;	
		
	if ( InstallCtrlCallback (intensilight->_main_ui_panel, I_MAIN_REMOTE_ENABLE, cbIntensilightRemoteEnable, intensilight) < 0)
		return INTENSILIGHT_ERROR;	
		
	if ( InstallCtrlCallback (intensilight->_main_ui_panel, I_MAIN_BRIGHTNESS, cbIntensilightRemotebrightness, intensilight) < 0)
		return INTENSILIGHT_ERROR;	
		
	if ( InstallCtrlCallback (intensilight->_main_ui_panel, I_MAIN_SIGAL_DELAY, cb_IntensilightSignalDelay, intensilight) < 0)
		return INTENSILIGHT_ERROR;	
		
	if ( InstallCtrlCallback (intensilight->_main_ui_panel, I_MAIN_BOUNCE_DELAY, cb_IntensilightBounceDelay, intensilight) < 0)
		return INTENSILIGHT_ERROR;	
		
	if ( InstallCtrlCallback (intensilight->_main_ui_panel, I_MAIN_EXPOSURE, cb_IntensilightExposure, intensilight) < 0)
		return INTENSILIGHT_ERROR;	
		
	if ( InstallCtrlCallback (intensilight->_main_ui_panel, I_MAIN_TEST_EXP, cbIntensilightExpTest, intensilight) < 0)
		return INTENSILIGHT_ERROR;	
			
  	if (intensilight_connect(intensilight) == INTENSILIGHT_ERROR)
  		return INTENSILIGHT_ERROR;
  		
	intensilight->_timer = NewAsyncTimer (1.0, -1, 1, OnIntensilightTimerTick, intensilight);
	SetAsyncTimerAttribute (intensilight->_timer, ASYNC_ATTR_ENABLED,  0);

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(intensilight), OnPanelsClosedOrHidden, intensilight);
	ui_module_panel_show_handler_connect (UIMODULE_CAST(intensilight), OnPanelsDisplayed, intensilight);
	
  	return INTENSILIGHT_SUCCESS;
}

int intensilight_hardware_is_initialised(Intensilight* intensilight)
{
	return intensilight->_initialised;	
}

int intensilight_changed_handler_connect(Intensilight* intensilight, INTENSILIGHT_CHANGE_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(intensilight), "intensilightChanged", handler, data) == SIGNAL_ERROR) {
		ui_module_send_error(UIMODULE_CAST(intensilight), "Intensilight Error", "Can not connect signal handler for intensilight Change signal");
		
		return INTENSILIGHT_ERROR;
	}

	return INTENSILIGHT_SUCCESS;
}

int intensilight_get_current_value_text(HardwareDevice* device, char* info)
{
	int nd_filter;
	Intensilight* intensilight = (Intensilight* ) device;

	if (info==NULL)
		return INTENSILIGHT_ERROR;

	if(intensilight_get_nd_filter(intensilight, &nd_filter) == INTENSILIGHT_ERROR)
		return INTENSILIGHT_ERROR;

	sprintf(info, "ND %d", nd_filter);
	
	return INTENSILIGHT_SUCCESS;
}

int intensilight_hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode)
{
	//TODO
	/*
	FluoCubeManager* cube_manager = (FluoCubeManager*)device;
	int pos;
	FILE *fp = NULL;

	dictionary *d = dictionary_new(5);
	
	cube_manager_get_current_cube_position(cube_manager, &pos);

	fp = fopen(filepath, mode);
	
	if(fp == NULL)
		return CUBE_MANAGER_ERROR;

	dictionary_set(d, UIMODULE_GET_NAME(cube_manager), NULL);
	dictionary_setint(d, "Position", pos);      

	iniparser_save(d, fp); 
	
	fclose(fp);
	dictionary_del(d);
*/

	return INTENSILIGHT_SUCCESS;
}

int intensilight_hardware_load_state_from_file (HardwareDevice* device, const char* filepath)
{
	/*
	FluoCubeManager* cube_manager = (FluoCubeManager*)device;
	dictionary* d = NULL;
	int pos, file_size, num_devices;    
	char buffer[500] = "";

	if(!FileExists(filepath, &file_size))
		return CUBE_MANAGER_ERROR;	 	
	
	d = iniparser_load(filepath);  

	num_devices = device_conf_get_num_active_devices(cube_manager->dc);

	if(d != NULL) {

		pos = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(cube_manager), "position"), -1); 

		if(pos >= 0 && pos <= num_devices)
			cube_manager_move_to_position(cube_manager, pos);
	}

    dictionary_del(d);
*/

	return INTENSILIGHT_SUCCESS;
}

Intensilight* intensilight_new(char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	Intensilight* intensilight = (Intensilight*) malloc(sizeof(Intensilight));
	
	memset(intensilight, 0, sizeof(Intensilight));	
	
	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(intensilight), name);
	ui_module_set_description(UIMODULE_CAST(intensilight), description);         
	ui_module_set_data_dir(UIMODULE_CAST(intensilight), data_dir);

	ui_module_set_error_handler(UIMODULE_CAST(intensilight), handler, intensilight); 
	
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(intensilight), hardware_get_current_value_text) = intensilight_get_current_value_text; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(intensilight), hardware_save_state_to_file) = intensilight_hardware_save_state_to_file; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(intensilight), hardware_load_state_from_file) = intensilight_hardware_load_state_from_file; 

	return intensilight;
}


int intensilight_destroy(Intensilight* intensilight)
{
	SetAsyncTimerAttribute (intensilight->_timer, ASYNC_ATTR_ENABLED,  0);
	DiscardAsyncTimer(intensilight->_timer);

  	intensilight_send_initial_settings(intensilight); 
  	close_comport(intensilight->_port);

	ui_module_destroy(intensilight);
  	
  	CmtDiscardLock (intensilight->_lock);
  	
  	free(intensilight);
  	
  	return INTENSILIGHT_SUCCESS;
}

void intensilight_stop_timer(Intensilight* intensilight)
{
	SetAsyncTimerAttribute (intensilight->_timer, ASYNC_ATTR_ENABLED,  0);
}

void intensilight_start_timer(Intensilight* intensilight)
{
	SetAsyncTimerAttribute (intensilight->_timer, ASYNC_ATTR_ENABLED,  1);
}

int intensilight_is_main_ui_visible(Intensilight* intensilight)
{
	int visible;
	
	GetPanelAttribute(intensilight->_main_ui_panel, ATTR_VISIBLE, &visible);
	
	return visible;
}

