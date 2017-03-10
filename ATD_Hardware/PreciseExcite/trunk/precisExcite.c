#include "precisExcite.h"
#include "precisExcite_ui.h"

#include "GCI_utils.h"
#include "GL_CVIRegistry.h"

#include <tcpsupp.h>
#include <utility.h>
#include "toolbox.h"
#include <ansi_c.h> 

static void OnprecisExciteChanged(precisExcite* precise_excite);

static void error_handler (char *error_string, precisExcite *precise_excite)
{
	LogError("precisExcite Error", error_string); 
}


static int PRECISEXCITE_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (precisExcite*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (precisExcite *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}

/*
void precise_excite_on_change(precisExcite* precise_excite)
{
	double val;
	static double previous_val = -1.0;
	
	CmtGetLock (precise_excite->_lock);
			
	if (precise_excite_get(precise_excite, &val) == PRECISEXCITE_SUCCESS) {
	
		if (FP_Compare (val, previous_val) != 0) {  //It's changed
			//SetCtrlVal (precise_excite->_main_ui_panel, ZOOM_ZOOM, val);
			OnprecisExciteChanged(precise_excite);
			previous_val = val;
		}
	}
			
	CmtReleaseLock(precise_excite->_lock);
}
*/

void preciseExcite_error_handler(precisExcite* precise_excite, int err)
{
	char err_info[50];
	
	if (err == 0) return;
	
	switch (err) {
		case -1:
			strcpy(err_info, "UnableToRegisterService");
			break;
		case -2:
			strcpy(err_info, "UnableToEstablishConnection");
			break;
		case -3:
			strcpy(err_info, "ExistingServer");
			break;
		case -4:
			strcpy(err_info, "ailedToConnect");
			break;
		case -5:
			strcpy(err_info, "ServerNotRegistered");
			break;
		case -6:
			strcpy(err_info, "TooManyConnections");
			break;
		case -7:
			strcpy(err_info, "ReadFailed");
			break;
		case -8:
			strcpy(err_info, "WriteFailed");
			break;
		case -9:
			strcpy(err_info, "InvalidParameter");
			break;
		case -10:
			strcpy(err_info, "OutOfMemory");
			break;
		case -11:
			strcpy(err_info, "TimeOutErr");
			break;
		case -12:
			strcpy(err_info, "NoConnectionEstablished");
			break;
		case -13:
			strcpy(err_info, "GeneralIOErr");
			break;
		case -14:
			strcpy(err_info, "ConnectionClosed");
			break;
		case -15:
			strcpy(err_info, "UnableToLoadWinsockDLL");
			break;
		case -16:
			strcpy(err_info, "IncorrectWinsockDLLVersion");
			break;
		case -17:
			strcpy(err_info, "ConnectionsStillOpen");
			break;
		case -18:
			strcpy(err_info, "DisconnectPending");
			break;
		case -19:
			strcpy(err_info, "InfoNotAvailable");
			break;
		case -20:
			strcpy(err_info, "HostAddressNotFound");
			break;
		case -21:
			strcpy(err_info, "NoConnectionEstablished");
			break;
		default:
			strcpy(err_info, "Unkown error");
			break;
	}

	logger_log(UIMODULE_LOGGER(precise_excite), LOGGER_ERROR, "%s", err_info);	
}

int CVICALLBACK preciseExciteCallback(unsigned handle, int event, int errCode, void *callbackData)
{
    char receiveBuf[256] = {0};
    int  dataSize         = sizeof (receiveBuf) - 1;
    int intensity, int_ctrl, status_ctrl;
    char chan, status;
	precisExcite* precise_excite = (precisExcite*) callbackData;
	
    switch (event) {
        case TCP_DATAREADY:
        
			//printf("got something. Handle %d. xType %d errCode %d\n", handle, event, errCode);
			
            if ((dataSize = ClientTCPRead (precise_excite->_conversation_handle, receiveBuf, dataSize, 1000)) < 0)
				SetCtrlVal(precise_excite->_setup_panel, SETUP_STATUS, "Receive Error");
            else {
                receiveBuf[dataSize] = '\0';
				SetCtrlVal(precise_excite->_setup_panel, SETUP_STATUS, "Read successful");

				//Interpret the data
				if (sscanf(receiveBuf, "C%c%d%c", &chan, &intensity, &status) < 3)
					break;
					
				if (chan == 'A') {
					int_ctrl = PE_MAIN_VIOLET_VAL;
					precise_excite->_active_channel = PE_VIOLET;
					status_ctrl = PE_MAIN_VIOLET_ON;
				}
				else if (chan == 'B') {
					int_ctrl = PE_MAIN_BLUE_VAL;
					precise_excite->_active_channel = PE_BLUE;
					status_ctrl = PE_MAIN_BLUE_ON;
				}
				else if (chan == 'C') {
					int_ctrl = PE_MAIN_GREEN_VAL;
					precise_excite->_active_channel = PE_GREEN;
					status_ctrl = PE_MAIN_GREEN_ON;
				}
				else {
					precise_excite->_active_channel = PE_NO_CHANNEL;
					break;
				}

				SetCtrlVal(precise_excite->_main_ui_panel, int_ctrl, intensity);
				if (status == 'N')
					SetCtrlVal(precise_excite->_main_ui_panel, status_ctrl, 1);
				else if (status == 'F')
					SetCtrlVal(precise_excite->_main_ui_panel, status_ctrl, 0);

            }
            break;
            
        case TCP_DISCONNECT:
        
            MessagePopup ("preciseExcite", "preciseExcite has closed TCP/IP connection");
			SetCtrlVal(precise_excite->_setup_panel, SETUP_STATUS, "Unconnected");
            
            precise_excite->_connected = 0;
            break;
    }
    return 0;
}

int preciseExcite_connect(precisExcite* precise_excite, char* ip_address, unsigned int port)
{
	int err=1, attempts = 0;
	
	if(precise_excite->_connected == 1)
		DisconnectFromTCPServer (precise_excite->_conversation_handle);

	while (err) {
	
		err = ConnectToTCPServer (&precise_excite->_conversation_handle, port, ip_address, preciseExciteCallback, precise_excite, 2000);
	
		if ((attempts > 0) && err) {
			if (GCI_ConfirmPopup("PrecisExcite Error", IDI_INFORMATION, "Cannot connect. Shall I give up?"))
				return PRECISEXCITE_ERROR;
		}
		
		attempts++;
	}
	
	precise_excite->_connected = 1;
	
	return PRECISEXCITE_SUCCESS;
}

int preciseExcite_write(precisExcite* precise_excite, char* data)
{
	int nbytes;
	
	strcat(data, "\n");
	nbytes = ClientTCPWrite (precise_excite->_conversation_handle, data, strlen(data), 5000);
	
	if (nbytes <= 0) 
		return -1;

	OnprecisExciteChanged(precise_excite);
	
	return 0;
}

int preciseExcite_read(precisExcite* precise_excite, char* data)
{
	int nbytes;
	
	nbytes = ClientTCPRead (precise_excite->_conversation_handle, data, 50, 5000);
	
	if (nbytes <= 0) 
		return -1;

	return 0;
}

int preciseExcite_enable_chanel_for_wavelength_range(precisExcite* precise_excite, int min_wavelength,
													 int max_wavelength)
{
	int i, peak, found=0, best=-1;
	double diff, best_diff;

	// Turn off channels
	preciseExcite_set_channel_on_off(precise_excite, PE_VIOLET, PE_OFF);
	preciseExcite_set_channel_on_off(precise_excite, PE_BLUE, PE_OFF);
	preciseExcite_set_channel_on_off(precise_excite, PE_GREEN, PE_OFF);

	for(i=1, found=0; i <= 3; i++) {
	
		peak = precise_excite->_channels_status[i].excitation_wavelength;

		if(peak >= min_wavelength && peak <= max_wavelength) {
			best = i;
			found++;
		}
	}

	if (found==1){
		preciseExcite_set_channel_on_off(precise_excite, (PRECITE_EXCITE_CHANNELS) best, PE_ON);
		return 0;
	}

	for(i=1, best_diff=1.0e10, found=0; i <= 3; i++) {
	
		peak = precise_excite->_channels_status[i].excitation_wavelength;

		diff = fabs(max_wavelength-peak);
		if(diff < best_diff && diff < 50.0) {
			best = i;
			best_diff = diff;
			found++;
		}

		diff = fabs(min_wavelength-peak);
		if(diff < best_diff && diff < 50.0) {
			best = i;
			best_diff = diff;
			found++;
		}
	}
	
	if (found) 
		preciseExcite_set_channel_on_off(precise_excite, (PRECITE_EXCITE_CHANNELS) best, PE_ON);

	return 0;
}

int preciseExcite_set_intensity(precisExcite* precise_excite, int channel, int val)
{
	char buf[50];
	char ch[4] = {0, 'A', 'B', 'C'};
	int ctrl_id = precise_excite->_channels_status[channel].intensity_ctrl_id, ret;
	
	precise_excite->_channels_status[channel].intensity = val;

	//Set intensity percent
	sprintf(buf, "C%cI%d", ch[channel], val);
	ret = preciseExcite_write(precise_excite, buf);

	if (ret == 0)
		SetCtrlVal(precise_excite->_main_ui_panel, ctrl_id, val);

	return ret;
}

int preciseExcite_get_intensity(precisExcite* precise_excite, int channel)
{
	return precise_excite->_channels_status[channel].intensity;
}

int preciseExcite_get_active_channel_intensity(precisExcite* precise_excite)
{
	return precise_excite->_channels_status[precise_excite->_active_channel].intensity;
}

int preciseExcite_get_active_channel_excitation_wavelength(precisExcite* precise_excite)
{
	return precise_excite->_channels_status[precise_excite->_active_channel].excitation_wavelength;
}

int preciseExcite_set_channel_on_off(precisExcite* precise_excite, PRECITE_EXCITE_CHANNELS channel, PRECITE_EXCITE_STATUS val)
{
	char buf[50];
	char ch[4] = {0, 'A', 'B', 'C'};
	char status[2] = {'F', 'N'};		//F = off, N = on
	int ctrl_id = precise_excite->_channels_status[channel].on_off_ctrl_id, ret;

	precise_excite->_channels_status[channel].status = val;

	//Set channel on or off
	sprintf(buf, "C%c%c", ch[channel], status[val]);

	precise_excite->_active_channel = channel;

	ret = preciseExcite_write(precise_excite, buf);

	if (ret == 0)
		SetCtrlVal(precise_excite->_main_ui_panel, ctrl_id, val);

	return ret;
}

int preciseExcite_set_active_channel_on_off(precisExcite* precise_excite, PRECITE_EXCITE_STATUS val)
{
	char buf[50];
	char ch[4] = {0, 'A', 'B', 'C'};
	char status[2] = {'F', 'N'};		//F = off, N = on
	int ctrl_id = precise_excite->_channels_status[precise_excite->_active_channel].on_off_ctrl_id;

	precise_excite->_channels_status[precise_excite->_active_channel].status = val;

	//Set channel on or off
	sprintf(buf, "C%c%c", ch[precise_excite->_active_channel], status[val]);

	SetCtrlVal(precise_excite->_main_ui_panel, ctrl_id, val);

	return preciseExcite_write(precise_excite, buf);
}

int preciseExcite_pulse_channel(precisExcite* precise_excite, PRECITE_EXCITE_CHANNELS channel, double secs)
{
	char buf[50];
	char ch[4] = {0, 'A', 'B', 'C'};
	
	//Set channel on or off
	sprintf(buf, "C%cPU%d", ch[channel], (int)(1000000.0 * secs + 0.5));  //send as microseconds (max 2147.483647 seconds)
	return preciseExcite_write(precise_excite, buf);
}

int preciseExcite_pulse_channel_fudge(precisExcite* precise_excite, int channel, double secs)
{
	preciseExcite_set_channel_on_off(precise_excite, channel, 1);
	Delay(secs);
	return preciseExcite_set_channel_on_off(precise_excite, channel, 0);
}

int preciseExcite_pulse_currently_on_channel(precisExcite* precise_excite, double secs)
{
	// Get the currently on channel
	//_active_channel
}

int preciseExcite_arm_channel(precisExcite* precise_excite, PRECITE_EXCITE_CHANNELS channel)
{
	char buf[50];
	char ch[4] = {0, 'A', 'B', 'C'};
	char edge[4] = {'+', '-', '*', '#'};		  //rising, falling, either, follow pulse
	
	//Arm channel 
	sprintf(buf, "A%c%c", ch[channel], edge[precise_excite->_trigger_edge]);
	return preciseExcite_write(precise_excite, buf);
}

int precise_excite_connect_to_default_port(precisExcite* precise_excite)
{
	char address[50];
	unsigned int port;
	int err;
	char device[UIMODULE_NAME_LEN];
	
	ui_module_get_name(UIMODULE_CAST(precise_excite), device);
	 
	get_device_string_param_from_ini_file   (device, "Address", address);  
	get_device_int_param_from_ini_file   (device, "Port", &port);  

	//GetCtrlVal(precise_excite->_setup_panel, SETUP_ADDRESS, address);
	//GetCtrlVal(precise_excite->_setup_panel, SETUP_PORT, &port);

	err = preciseExcite_connect(precise_excite, address, port);
	
	return err;
}

static int precise_excite_send_initial_settings(precisExcite* precise_excite)
{
	int err;
	
	// Turn off but leave intensity at 50 %
	
	err = preciseExcite_set_channel_on_off(precise_excite, PE_VIOLET, 0);
	if (err) return err;
	err = preciseExcite_set_channel_on_off(precise_excite, PE_BLUE, 0);	
	if (err) return err;
	err = preciseExcite_set_channel_on_off(precise_excite, PE_GREEN, 0);
	if (err) return err;
	
	err = preciseExcite_set_intensity(precise_excite, PE_VIOLET, 50);
	if (err) return err;
	err = preciseExcite_set_intensity(precise_excite, PE_BLUE, 50);
	if (err) return err;
	err = preciseExcite_set_intensity(precise_excite, PE_GREEN, 50);
	if (err) return err;

	return PRECISEXCITE_SUCCESS;
}

int precise_excite_get_current_value_text(HardwareDevice* device, char* info)
{
	int intensity;
	precisExcite* precise_excite = (precisExcite* ) device;

	if (info==NULL)
		return PRECISEXCITE_ERROR;
	
	intensity = precise_excite->_channels_status[precise_excite->_active_channel].intensity;

	if(precise_excite->_active_channel == PE_VIOLET)
		sprintf(info, "Violet LED: %d", intensity);
			
	if(precise_excite->_active_channel == PE_BLUE)
		sprintf(info, "Blue LED: %d", intensity);

	if(precise_excite->_active_channel == PE_GREEN)
		sprintf(info, "Green LED: %d", intensity);

	return PRECISEXCITE_SUCCESS;
}

int precise_excite_hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode)
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

	return PRECISEXCITE_SUCCESS;
}

int precise_excite_hardware_load_state_from_file (HardwareDevice* device, const char* filepath)
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

	return PRECISEXCITE_SUCCESS;
}

int precise_excite_initialise (precisExcite* precise_excite)
{
	if ( InstallCtrlCallback (precise_excite->_main_ui_panel, PE_MAIN_CLOSE, OnPrecisExciteClose, precise_excite) < 0)
		return PRECISEXCITE_ERROR;	
		
	if ( InstallCtrlCallback (precise_excite->_main_ui_panel, PE_MAIN_SETUP, OnPrecisExciteSetup, precise_excite) < 0)
		return PRECISEXCITE_ERROR;	
		
	if ( InstallCtrlCallback (precise_excite->_main_ui_panel, PE_MAIN_VIOLET_VAL, OnSetIntensityVal, precise_excite) < 0)
		return PRECISEXCITE_ERROR;	
		
	if ( InstallCtrlCallback (precise_excite->_main_ui_panel, PE_MAIN_BLUE_VAL, OnSetIntensityVal, precise_excite) < 0)
		return PRECISEXCITE_ERROR;	
		
	if ( InstallCtrlCallback (precise_excite->_main_ui_panel, PE_MAIN_GREEN_VAL, OnSetIntensityVal, precise_excite) < 0)
		return PRECISEXCITE_ERROR;	
		
	if ( InstallCtrlCallback (precise_excite->_main_ui_panel, PE_MAIN_VIOLET_ON, OnSetChannelOn, precise_excite) < 0)
		return PRECISEXCITE_ERROR;	
		
	if ( InstallCtrlCallback (precise_excite->_main_ui_panel, PE_MAIN_BLUE_ON, OnSetChannelOn, precise_excite) < 0)
		return PRECISEXCITE_ERROR;	
		
	if ( InstallCtrlCallback (precise_excite->_main_ui_panel, PE_MAIN_GREEN_ON, OnSetChannelOn, precise_excite) < 0)
		return PRECISEXCITE_ERROR;	
		
	if ( InstallCtrlCallback (precise_excite->_main_ui_panel, PE_MAIN_VIOLET_PULSE, OnPulse, precise_excite) < 0)
		return PRECISEXCITE_ERROR;	
		
	if ( InstallCtrlCallback (precise_excite->_main_ui_panel, PE_MAIN_BLUE_PULSE, OnPulse, precise_excite) < 0)
		return PRECISEXCITE_ERROR;	
		
	if ( InstallCtrlCallback (precise_excite->_main_ui_panel, PE_MAIN_GREEN_PULSE, OnPulse, precise_excite) < 0)
		return PRECISEXCITE_ERROR;	
		
	if ( InstallCtrlCallback (precise_excite->_main_ui_panel, PE_MAIN_VIOLET_ARM, OnArm, precise_excite) < 0)
		return PRECISEXCITE_ERROR;	
		
	if ( InstallCtrlCallback (precise_excite->_main_ui_panel, PE_MAIN_BLUE_ARM, OnArm, precise_excite) < 0)
		return PRECISEXCITE_ERROR;	
		
	if ( InstallCtrlCallback (precise_excite->_main_ui_panel, PE_MAIN_GREEN_ARM, OnArm, precise_excite) < 0)
		return PRECISEXCITE_ERROR;	
		
	if ( InstallCtrlCallback (precise_excite->_setup_panel, SETUP_CONNECT, OnConnect, precise_excite) < 0)
		return PRECISEXCITE_ERROR;	
		
	if ( InstallCtrlCallback (precise_excite->_setup_panel, SETUP_QUIT, OnSetupQuit, precise_excite) < 0)
		return PRECISEXCITE_ERROR;	
		
	SetPanelAttribute (precise_excite->_main_ui_panel, ATTR_TITLE, UIMODULE_GET_DESCRIPTION(precise_excite));
	 	
	if (precise_excite->_connected) {
		SetCtrlVal(precise_excite->_setup_panel, SETUP_STATUS, "Connected");
	}
	
	SetCtrlAttribute(precise_excite->_main_ui_panel, PE_MAIN_VIOLET_VAL, ATTR_DIMMED, !precise_excite->_connected);
	SetCtrlAttribute(precise_excite->_main_ui_panel, PE_MAIN_BLUE_VAL, ATTR_DIMMED, !precise_excite->_connected);
	SetCtrlAttribute(precise_excite->_main_ui_panel, PE_MAIN_GREEN_VAL, ATTR_DIMMED, !precise_excite->_connected);

	SetCtrlAttribute(precise_excite->_main_ui_panel, PE_MAIN_VIOLET_ON, ATTR_DIMMED, !precise_excite->_connected);
	SetCtrlAttribute(precise_excite->_main_ui_panel, PE_MAIN_BLUE_ON, ATTR_DIMMED, !precise_excite->_connected);
	SetCtrlAttribute(precise_excite->_main_ui_panel, PE_MAIN_GREEN_ON, ATTR_DIMMED, !precise_excite->_connected);

	SetCtrlAttribute(precise_excite->_main_ui_panel, PE_MAIN_VIOLET_PULSE, ATTR_DIMMED, !precise_excite->_connected);
	SetCtrlAttribute(precise_excite->_main_ui_panel, PE_MAIN_BLUE_PULSE, ATTR_DIMMED, !precise_excite->_connected);
	SetCtrlAttribute(precise_excite->_main_ui_panel, PE_MAIN_GREEN_PULSE, ATTR_DIMMED, !precise_excite->_connected);
	
	SetCtrlAttribute(precise_excite->_main_ui_panel, PE_MAIN_VIOLET_ARM, ATTR_DIMMED, !precise_excite->_connected);
	SetCtrlAttribute(precise_excite->_main_ui_panel, PE_MAIN_BLUE_ARM, ATTR_DIMMED, !precise_excite->_connected);
	SetCtrlAttribute(precise_excite->_main_ui_panel, PE_MAIN_GREEN_ARM, ATTR_DIMMED, !precise_excite->_connected);
	

  	return PRECISEXCITE_ERROR;
}

int precise_excite_hardware_initialise (precisExcite* precise_excite)
{
	if (precise_excite_connect_to_default_port(precise_excite) == PRECISEXCITE_SUCCESS) {
  		precise_excite_send_initial_settings(precise_excite);
		precise_excite->_initialised = 1;
  		return PRECISEXCITE_SUCCESS;
  	}
}

int precise_excite_hardware_is_initialised(precisExcite* precise_excite)
{
	return precise_excite->_initialised;	
}

int precise_excite_signal_hide_handler_connect (precisExcite* precise_excite, PRECISEXCITE_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(precise_excite), "Hide", handler, callback_data) == SIGNAL_ERROR) {
		ui_module_send_error(UIMODULE_CAST(precise_excite), "Precise Excite Error", "Can not connect signal handler for precisExcite Hide signal");
		return PRECISEXCITE_ERROR;
	}

	return PRECISEXCITE_SUCCESS;
}


int precise_excite_changed_handler(precisExcite* precise_excite, PRECISEXCITE_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(precise_excite), "precisExciteChanged", handler, data) == SIGNAL_ERROR) {
		ui_module_send_error(UIMODULE_CAST(precise_excite), "Precise Excite Error", "Can not connect signal handler for precisExcite Change signal");
		return PRECISEXCITE_ERROR;
	}

	return PRECISEXCITE_SUCCESS;
}

static void OnprecisExciteChanged(precisExcite* precise_excite)
{
	//Pass on the event to higher modules		
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(precise_excite), "precisExciteChanged", GCI_VOID_POINTER, precise_excite);  
}

////////////////////////////////////////////////////////////////////////////////////////////////////
precisExcite* precise_excite_new(char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	precisExcite* precise_excite = (precisExcite*) malloc(sizeof(precisExcite));

	memset(precise_excite, 0, sizeof(precisExcite));	

	hardware_device_hardware_constructor(HARDWARE_DEVICE_CAST(precise_excite), name);
	ui_module_set_description(UIMODULE_CAST(precise_excite), description);         
	ui_module_set_data_dir(UIMODULE_CAST(precise_excite), data_dir);

	ui_module_set_error_handler(UIMODULE_CAST(precise_excite), handler, precise_excite); 
	
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(precise_excite), hardware_get_current_value_text) = precise_excite_get_current_value_text; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(precise_excite), hardware_save_state_to_file) = precise_excite_hardware_save_state_to_file; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(precise_excite), hardware_load_state_from_file) = precise_excite_hardware_load_state_from_file; 

	precise_excite->_connected = 0;
	precise_excite->_conversation_handle = 0;
	precise_excite->_lock = -1;
	precise_excite->_trigger_edge = 2;	   //either edge

	CmtNewLock (NULL, 0, &precise_excite->_lock);
	
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(precise_excite), "Hide", PRECISEXCITE_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(precise_excite), "precisExciteChanged", PRECISEXCITE_PTR_MARSHALLER);

	precise_excite->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(precise_excite), "precisExcite_ui.uir", PE_MAIN, 1);  
    precise_excite->_setup_panel = ui_module_add_panel(UIMODULE_CAST(precise_excite), "precisExcite_ui.uir", SETUP, 0);  
 
	precise_excite->_channels_status[PE_VIOLET].on_off_ctrl_id = PE_MAIN_VIOLET_ON;
	precise_excite->_channels_status[PE_VIOLET].intensity_ctrl_id = PE_MAIN_VIOLET_VAL;
	precise_excite->_channels_status[PE_VIOLET].excitation_wavelength = 400;

	precise_excite->_channels_status[PE_BLUE].on_off_ctrl_id = PE_MAIN_BLUE_ON;
	precise_excite->_channels_status[PE_BLUE].intensity_ctrl_id = PE_MAIN_BLUE_VAL;
	precise_excite->_channels_status[PE_BLUE].excitation_wavelength = 470;

	precise_excite->_channels_status[PE_GREEN].on_off_ctrl_id = PE_MAIN_GREEN_ON;
	precise_excite->_channels_status[PE_GREEN].intensity_ctrl_id = PE_MAIN_GREEN_VAL;
	precise_excite->_channels_status[PE_GREEN].excitation_wavelength = 525;

	return precise_excite;
}


int precise_excite_destroy(precisExcite* precise_excite)
{
  	precise_excite_send_initial_settings(precise_excite);   //all channels off
	DisconnectFromTCPServer (precise_excite->_conversation_handle);

    CmtDiscardLock (precise_excite->_lock);

    ui_module_destroy(UIMODULE_CAST(precise_excite));

  	free(precise_excite);
  	
  	return PRECISEXCITE_SUCCESS;
}