#include "LaserPowerMonitor.h"
#include "ATD_LaserPowerMonitor_A.h"
#include "ATD_LaserPowerMonitor_A_UI.h"
#include "ATD_UsbInterface_A.h"

#include "asynctmr.h"
#include "dictionary.h"
#include "iniparser.h"

#include <cviauto.h>
#include <userint.h>
#include <formatio.h>
#include <utility.h>

#include "profile.h"
#include "password.h"

static int atd_laserpowermonitor_a_init_hardware(ATD_LaserPowerMonitor_A* atd_laserpower_mon_a, LaserPowerverMonitorBitModeType mode);

void laserpowermonitor_a_display_val(LaserPowerMonitor* laserpower_mon, double reading)
{
	SetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_OUTPUT, reading);

	// Limit at bottom of UI scale
	if(reading < 1)
		reading = 1;

	SetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_LOG2, log10(reading));
}

int atd_laserpowermonitor_a_destroy (LaserPowerMonitor *laserpower_mon)
{
	ATD_LaserPowerMonitor_A* atd_laserpower_mon_a = (ATD_LaserPowerMonitor_A*) laserpower_mon;

	#ifdef FTDI_NO_VIRTUAL_COMPORT
	ftdi_controller_close(atd_laserpower_mon_a->_controller);
	#else
	close_comport(atd_laserpower_mon_a->_com_port);
	#endif

	return HARDWARE_SUCCESS;
}

static int atd_laserpowermonitor_a_get (LaserPowerMonitor*laserpower_mon, double *val)
{
	unsigned int msb, lsb;
	int reading, gain;
	double holdoff, delay;
    byte vals[10] = "";
	
	ATD_LaserPowerMonitor_A* atd_laserpower_mon_a = (ATD_LaserPowerMonitor_A*) laserpower_mon;
   
	if (atd_laserpower_mon_a == NULL)
	  return HARDWARE_ERROR;
	
	GetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_GAIN, &gain);
	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_HOLDOFF, &holdoff);
	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_DELAY, &delay);

	Delay(holdoff);
	atd_laserpowermonitor_a_set_gain(atd_laserpower_mon_a, (LaserPowerverMonitorGainValue) gain);
	Delay(delay);

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_i2c_read_bytes(atd_laserpower_mon_a->_controller, atd_laserpower_mon_a->_i2c_chip_address, 3, vals) != FT_OK) {
		return HARDWARE_ERROR;
	}

	#else

	GetI2CPortLock(atd_laserpower_mon_a->_com_port, "atd_laserpowermonitor_a_get");  
	
	if(atd_laserpower_mon_a->_com_port < 0 || atd_laserpower_mon_a->_com_port > 20) {
		ReleaseI2CPortLock(atd_laserpower_mon_a->_com_port, "atd_laserpowermonitor_a_get");  
		return HARDWARE_ERROR;
	}

	vals[0] = atd_laserpower_mon_a->_i2c_chip_type | 0x01;
	
	if (GCI_readI2C_multiPort(atd_laserpower_mon_a->_com_port, 3, vals, atd_laserpower_mon_a->_i2c_bus, "atd_laserpowermonitor_a_get")) {
		ReleaseI2CPortLock(atd_laserpower_mon_a->_com_port, "atd_laserpowermonitor_a_get");  
	
		return HARDWARE_ERROR;
	}

	ReleaseI2CPortLock(atd_laserpower_mon_a->_com_port, "atd_laserpowermonitor_a_get");  

	#endif

	msb = vals[0] & 0xff; 
	lsb = vals[1] & 0xff; 
    		
    reading = (msb<<8 | lsb);
    
	
	/*
	if(reading > 0x8000) {		//If msb is set
		reading = reading ^ 0xffff;
		reading = -reading;
	}

	if(reading < 0)
		reading = 0;
	*/

	if(reading > 0x7fff) {
		reading=-(reading-0xffff);		
    }

	// Normalise value	
//	GetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_GAIN, &gain);
//	*val = (double) ((reading * LASER_POWER_GAIN_8V_V_FACTOR) / gain_factors[gain]); 

	// Convert from 0-32k to 0-10000
	*val = ((double)reading * 3.051850948); // 100000/32767
   	

	//*val = (double)reading; 

	return HARDWARE_SUCCESS; 
}

// This function does not do an acquisition by design should
// be a fast read from the ui.
int laser_power_monitor_a_hardware_get_current_value_text (HardwareDevice* device, char* info)
{
	int gain_index;
	double val;
	ATD_LaserPowerMonitor_A* atd_laserpower_mon_a = (ATD_LaserPowerMonitor_A*) device;
	LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) device;

	if (info==NULL)
		return HARDWARE_ERROR;

	GetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_OUTPUT, &val);
	GetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_GAIN, &gain_index);

	sprintf(info, "level %.0f gain %d", val, gain_factors[gain_index]);
	return HARDWARE_SUCCESS;

	//if (atd_laserpowermonitor_a_get (laserpower_mon, &val) == HARDWARE_SUCCESS){
	//	sprintf(info, "%f", val);
	//	return HARDWARE_SUCCESS;
	//}

	//return HARDWARE_ERROR;
}

static void ftdi_error_handler (FTDIController *controller, const char *title, const char *error_string, void *callback_data)    
{
	printf("%s: %s\n", title, error_string);
}

static int atd_laserpowermonitor_a_hardware_initialise (HardwareDevice *hw_device)
{
	char device_sn[FTDI_SERIAL_NUMBER_LINE_LENGTH] = "";
	char device[UIMODULE_NAME_LEN];

	LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) hw_device;
	ATD_LaserPowerMonitor_A* atd_laserpower_mon_a = (ATD_LaserPowerMonitor_A*) laserpower_mon;

	ui_module_get_name(UIMODULE_CAST(atd_laserpower_mon_a), device);
		
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	get_device_string_param_from_ini_file(device, "FTDI_SN", device_sn);  
	get_device_int_param_from_ini_file   (device, "I2C_DEVICE_BUS", &(atd_laserpower_mon_a->_i2c_bus));
	get_device_int_param_from_ini_file   (device, "I2C_DEVICE_ADDRESS", &(atd_laserpower_mon_a->_i2c_chip_address));  
	
	atd_laserpower_mon_a->_controller = ftdi_controller_new();

	//ftdi_controller_set_debugging(atd_laserpower_mon_a->controller, 1);
	ftdi_controller_set_error_handler(atd_laserpower_mon_a->_controller, ftdi_error_handler, NULL);
	ftdi_controller_open(atd_laserpower_mon_a->_controller, device_sn);

	#else

	get_device_int_param_from_ini_file   (device, "COM_Port", &(atd_laserpower_mon_a->_com_port));  
	get_device_int_param_from_ini_file   (device, "i2c_Bus", &(atd_laserpower_mon_a->_i2c_bus));
	get_device_int_param_from_ini_file   (device, "i2c_ChipAddress", &(atd_laserpower_mon_a->_i2c_chip_address));  
	get_device_int_param_from_ini_file   (device, "i2c_ChipType", &(atd_laserpower_mon_a->_i2c_chip_type));  

	if (initialise_comport(atd_laserpower_mon_a->_com_port, 9600) < 0)
		return HARDWARE_ERROR;
	
	#endif

	if (get_device_double_param_from_ini_file(UIMODULE_GET_NAME(atd_laserpower_mon_a), "TimerInterval", &(laserpower_mon->_timer_interval))<0) 
	{
		#ifdef HEAVY_I2C_TESTING 
		laserpower_mon->_timer_interval=0.001;
		#else
		laserpower_mon->_timer_interval=1.0;
		#endif
	}

	if(atd_laserpowermonitor_a_init_hardware(atd_laserpower_mon_a, LASER_POWER_MODE_16BIT_TRIGGERED) == HARDWARE_ERROR)
		return HARDWARE_ERROR;

	return HARDWARE_SUCCESS;  
}

static int atd_laserpowermonitor_a_init_hardware(ATD_LaserPowerMonitor_A* atd_laserpower_mon_a, LaserPowerverMonitorBitModeType mode)
{
	byte vals[10] = "";

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	vals[0]=mode;
   		
	if(ftdi_controller_i2c_write_bytes(atd_laserpower_mon_a->_controller, atd_laserpower_mon_a->_i2c_chip_address, 1, vals) != FT_OK) {
		return HARDWARE_ERROR;
	}

	#else

   	vals[0]=atd_laserpower_mon_a->_i2c_chip_type;		   //Set cofiguration
   	vals[1]=mode;	
   				
	if(GCI_writeI2C_multiPort(atd_laserpower_mon_a->_com_port, 2, vals, atd_laserpower_mon_a->_i2c_bus, "atd_laserpowermonitor_a_set_gain") < 0)
		return HARDWARE_ERROR;
	
	#endif

	return HARDWARE_SUCCESS;
}


static int CVICALLBACK OnLaserPowerMonitor_A_GainSet (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_LaserPowerMonitor_A* atd_laserpower_mon_a = (ATD_LaserPowerMonitor_A*) callbackData; 
			LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) atd_laserpower_mon_a;

			int gain;
			
			GetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_GAIN, &gain);

			atd_laserpowermonitor_a_set_gain(atd_laserpower_mon_a, (LaserPowerverMonitorGainValue) gain);

			break;
		}
	}

	return 0;
}

static int CVICALLBACK OnLaserPowerMonitor_A_ClosePressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_LaserPowerMonitor_A* atd_laserpower_mon_a = (ATD_LaserPowerMonitor_A*) callbackData; 

			ui_module_hide_main_panel(UIMODULE_CAST(atd_laserpower_mon_a));

			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnLaserPowerMonitor_A_AcquirePressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_LaserPowerMonitor_A* atd_laserpower_mon_a = (ATD_LaserPowerMonitor_A*) callbackData; 
			LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) atd_laserpower_mon_a;
			double reading;

			laserpowermonitor_disable_timer(laserpower_mon);

			if(laserpowermonitor_get (laserpower_mon, &reading) == HARDWARE_ERROR)
				return -1;

			laserpowermonitor_display_val(laserpower_mon, reading);

			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnLaserPowerMonitor_A_LivePressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_LaserPowerMonitor_A* atd_laserpower_mon_a = (ATD_LaserPowerMonitor_A*) callbackData; 
			LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) atd_laserpower_mon_a;

			laserpowermonitor_set_timer_interval(laserpower_mon, 1.0);
			laserpowermonitor_enable_timer(laserpower_mon);

			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnLaserPowerMonitor_A_MinShutter (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_LaserPowerMonitor_A* atd_laserpower_mon_a = (ATD_LaserPowerMonitor_A*) callbackData; 
			LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) atd_laserpower_mon_a;
			double val;

			GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_MIN_SHUTTER, &val);
			SetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_MIN_SHUTTER, val);

			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnLaserPowerMonitor_A_CloseSettings (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_LaserPowerMonitor_A* atd_laserpower_mon_a = (ATD_LaserPowerMonitor_A*) callbackData; 
			LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) atd_laserpower_mon_a;

			ui_module_hide_panel(UIMODULE_CAST(atd_laserpower_mon_a), laserpower_mon->_settings_panel_id);

			break; 
		}
	}
	
	return 0;
}

int CVICALLBACK OnLaserPowerMonitor_A_ShowSettings (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_LaserPowerMonitor_A* atd_laserpower_mon_a = (ATD_LaserPowerMonitor_A*) callbackData; 
			LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) atd_laserpower_mon_a;

			GCI_ShowPasswordProtectedPanel(laserpower_mon->_settings_panel_id, laserpower_mon->_main_panel_id);

			break; 
		}
	}
	
	return 0;
}

int CVICALLBACK OnLaserPowerMonitor_A_SaveSettings (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_LaserPowerMonitor_A* atd_laserpower_mon_a = (ATD_LaserPowerMonitor_A*) callbackData; 
			char path[GCI_MAX_PATHNAME_LEN];
			int ret;

			ui_module_get_data_dir(UIMODULE_CAST(atd_laserpower_mon_a), path);

			strcat(path, "\\");
			strcat(path, UIMODULE_GET_NAME(UIMODULE_CAST(atd_laserpower_mon_a)));
			strcat(path, ".ini");

			ret = hardware_save_state_to_file (HARDWARE_DEVICE_CAST(atd_laserpower_mon_a), path, "w");

			if (HARDWARE_SUCCESS==ret) {
				GCIDialog(0, "Configuration Saved", IDI_INFORMATION, GCI_OK_BUTTON, "Configuration saved successfully to:\n%s", path);
			}
			else {
				GCIDialog(0, "Configuration Save Error", IDI_EXCLAMATION, GCI_OK_BUTTON, "Configuration FAILED to save to:\n%s", path);
			}
			
			break; 
		}
	}
	
	return 0;
}

int atd_laserpowermonitor_a_initialise (LaserPowerMonitor* laserpower_mon)
{
	ATD_LaserPowerMonitor_A* atd_laserpower_mon_a = (ATD_LaserPowerMonitor_A *) laserpower_mon;  
	char path[GCI_MAX_PATHNAME_LEN] = "";

	laserpower_mon->_main_panel_id = ui_module_add_panel(UIMODULE_CAST(atd_laserpower_mon_a), "ATD_LaserPowerMonitor_A_UI.uir", LSR_PR_PNL, 1);

	laserpower_mon->_settings_panel_id = ui_module_add_panel(UIMODULE_CAST(atd_laserpower_mon_a), "ATD_LaserPowerMonitor_A_UI.uir", LSR_PR_SET, 0);

	if(InstallCtrlCallback (laserpower_mon->_main_panel_id, LSR_PR_PNL_GAIN, OnLaserPowerMonitor_A_GainSet, atd_laserpower_mon_a) < 0)
		return HARDWARE_ERROR;

	InstallCtrlCallback (laserpower_mon->_main_panel_id, LSR_PR_PNL_QUIT, OnLaserPowerMonitor_A_ClosePressed, atd_laserpower_mon_a);	
	InstallCtrlCallback (laserpower_mon->_main_panel_id, LSR_PR_PNL_SETTINGS, OnLaserPowerMonitor_A_ShowSettings, atd_laserpower_mon_a);

	InstallCtrlCallback (laserpower_mon->_settings_panel_id, LSR_PR_SET_LIVE, OnLaserPowerMonitor_A_LivePressed, atd_laserpower_mon_a);
	InstallCtrlCallback (laserpower_mon->_settings_panel_id, LSR_PR_SET_ACQUIRE, OnLaserPowerMonitor_A_AcquirePressed, atd_laserpower_mon_a);
	InstallCtrlCallback (laserpower_mon->_settings_panel_id, LSR_PR_SET_CLOSE_SETTINGS, OnLaserPowerMonitor_A_CloseSettings, atd_laserpower_mon_a);
	InstallCtrlCallback (laserpower_mon->_settings_panel_id, LSR_PR_SET_SAVE, OnLaserPowerMonitor_A_SaveSettings, atd_laserpower_mon_a);
	InstallCtrlCallback (laserpower_mon->_settings_panel_id, LSR_PR_SET_MIN_SHUTTER, OnLaserPowerMonitor_A_MinShutter, atd_laserpower_mon_a);

	sprintf(path, "%s\\%s.ini", UIMODULE_GET_DATA_DIR(atd_laserpower_mon_a), UIMODULE_GET_NAME(atd_laserpower_mon_a));

	hardware_load_state_from_file(HARDWARE_DEVICE_CAST(laserpower_mon), path); 

	return HARDWARE_SUCCESS;
}

int atd_laserpowermonitor_a_set_gain(ATD_LaserPowerMonitor_A* atd_laserpower_mon_a, LaserPowerverMonitorGainValue gain)
{	// NB takes the ctrl index as the gain value
	LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) atd_laserpower_mon_a;
	byte vals[10];
	int config = LASER_POWER_MODE_16BIT_TRIGGERED;
	config = (config & 0xFC);		//Clear gain bits
	config = (config | (int) gain);
				
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	vals[0]=config;
   		
	if(ftdi_controller_i2c_write_bytes(atd_laserpower_mon_a->_controller, atd_laserpower_mon_a->_i2c_chip_address, 2, vals) != FT_OK) {
		return HARDWARE_ERROR;
	}

	#else

   	vals[0]=atd_laserpower_mon_a->_i2c_chip_type;		   //Set cofiguration
   	vals[1]=config;	
   				
	GCI_writeI2C_multiPort(atd_laserpower_mon_a->_com_port, 2, vals, atd_laserpower_mon_a->_i2c_bus, "atd_laserpowermonitor_a_set_gain"); 

	#endif

	SetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_GAIN, gain);

	return HARDWARE_SUCCESS;
}

static int atd_laserpowermonitor_a_get_gain(LaserPowerMonitor* laserpower_mon, int *gain)
{	// returns the actual gain factor
	int gain_idx;
	GetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_GAIN, &gain_idx);
	*gain = gain_factors[gain_idx];

	return HARDWARE_SUCCESS;
}

static int atd_laserpowermonitor_a_get_min_shutter(LaserPowerMonitor* laserpower_mon, double *val)
{	// returns the actual gain factor

	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_MIN_SHUTTER, val);

	return HARDWARE_SUCCESS;
}

int atd_laserpowermonitor_a_hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode)
{
	int gain;
	double holdoff, delay, min_exposure;
	FILE *fp = NULL;
	LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) device;

	dictionary *d = dictionary_new(5);
	
	GetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_GAIN, &gain);
	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_HOLDOFF, &holdoff);
	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_DELAY, &delay);
	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_PNL_MIN_SHUTTER, &min_exposure);

	fp = fopen(filepath, mode);
	
	if(fp == NULL)
		return HARDWARE_ERROR;

	dictionary_set(d, UIMODULE_GET_NAME(laserpower_mon), NULL);
	dictionary_setint(d, "Gain", gain);  
	dictionary_setdouble(d, "Holdoff", holdoff);  
	dictionary_setdouble(d, "Delay", delay);  
	dictionary_setdouble(d, "MinShutter", min_exposure);  

	iniparser_save(d, fp); 
	
	fclose(fp);
	dictionary_del(d);

	return HARDWARE_SUCCESS;
}

int atd_laserpowermonitor_a_hardware_load_state_from_file (HardwareDevice* device, const char* filepath)
{
	LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) device;
	dictionary* d = NULL;
	int gain, file_size;   
	double holdoff, delay;
	char buffer[500] = "";

	if(!FileExists(filepath, &file_size))
		return HARDWARE_ERROR;	 	
	
	d = iniparser_load(filepath);  

	if(d != NULL) {

		gain = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(laserpower_mon), "gain"), -1); 

		if(gain >= 0) {
			
			atd_laserpowermonitor_a_set_gain((ATD_LaserPowerMonitor_A*) laserpower_mon, (LaserPowerverMonitorGainValue) gain);
		}

		holdoff = dictionary_getdouble(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(laserpower_mon), "holdoff"), -1.0); 

		if(holdoff >= 0) {
			
			SetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_HOLDOFF, holdoff);
		}

		delay = dictionary_getdouble(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(laserpower_mon), "delay"), -1.0); 

		if(delay >= 0) {
			
			SetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_DELAY, delay);
		}
	}


    dictionary_del(d);

	return HARDWARE_SUCCESS;
}

LaserPowerMonitor* atd_laserpowermonitor_a_new(const char *name, const char *description,
  UI_MODULE_ERROR_HANDLER error_handler, const char *data_dir, void *data)
{
	ATD_LaserPowerMonitor_A* atd_laserpower_mon_a = (ATD_LaserPowerMonitor_A*) malloc(sizeof(ATD_LaserPowerMonitor_A));
    LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) atd_laserpower_mon_a;

	memset(atd_laserpower_mon_a, 0, sizeof(ATD_LaserPowerMonitor_A));
	
	laserpowermonitor_constructor(laserpower_mon, name, description, error_handler, data_dir, data);

	LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, display_val) = laserpowermonitor_a_display_val; 
	LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, destroy) = atd_laserpowermonitor_a_destroy; 
	LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, get_laser_power) = atd_laserpowermonitor_a_get;
	LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, get_gain) = atd_laserpowermonitor_a_get_gain;
	LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, initialise) = atd_laserpowermonitor_a_initialise;
	LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, get_minimum_shutter_time) = atd_laserpowermonitor_a_get_min_shutter;

	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(atd_laserpower_mon_a), hardware_initialise) = atd_laserpowermonitor_a_hardware_initialise; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(atd_laserpower_mon_a), hardware_get_current_value_text) = laser_power_monitor_a_hardware_get_current_value_text; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(atd_laserpower_mon_a), hardware_save_state_to_file) = atd_laserpowermonitor_a_hardware_save_state_to_file; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(atd_laserpower_mon_a), hardware_load_state_from_file) = atd_laserpowermonitor_a_hardware_load_state_from_file; 

	return laserpower_mon;
}
