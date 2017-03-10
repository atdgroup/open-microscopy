#include "LaserPowerMonitor.h"
#include "ATD_LaserPowerMonitor_LOGP.h"
#include "ATD_LaserPowerMonitor_LOGP_UI.h"
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

double detector_response_values[13]      = {0.07,0.09,0.13,0.19,0.25,0.285,0.31,0.33,0.35,0.365,0.38,0.39,0.4};
double detector_response_wavelengths[13] = {350.0,375.0,400.0,425.0,450.0,475.0,500.0,525.0,550.0,575.0,600.0,625.0,650.0};
const int detector_response_nValues = 13;

double detector_response_integral (double wl, int left_integral, int *left_index)
{  // Uses the detector_response_values global array
   // Finds the integral from the point at wl to the nearest defined pt lower (left_integral)
   // or higher (right integral)
   // returns the left index if needed

	int i=0, n = detector_response_nValues, i1, i2;
	double *wls = detector_response_wavelengths;
	double *vals = detector_response_values;
	double val, integral;
	
	// find values that are neighbours to wl
	for (i=0; i<n; i++){
		if (wls[i] > wl)
			break;
	}
	
	i2 = i;  
	i1 = i-1;

	// find the linearly interpolated value
	val	= vals[i1] + (vals[i2]-vals[i1]) * (wl-wls[i1]) / (wls[i2]-wls[i1]);

	if (left_integral)
		integral = (wl-wls[i1]) * (val+vals[i1]) / 2.0;
	else  // do right integral
		integral = (wls[i2]-wl) * (val+vals[i2]) / 2.0;

	if (left_index!=NULL)
		*left_index = i1;

	return integral;
}

double get_detector_response (double min, double max)
{	// This function will return a number for the average detector response or responsivity over the wavelength range given
	// it uses the global arrays detector_response_values and detector_response_wavelengths

	int i=0, n = detector_response_nValues, i1, i2;
	double *wls = detector_response_wavelengths;
	double *vals = detector_response_values;
	double area=0.0, response=1.0;

	if (min<wls[0] || max<wls[0] || min>wls[n-1] || max>wls[n-1]) 
		return 1.0;  // cannot do it

	// get area from min wl to the nearest higher wl point
	area = detector_response_integral (min, 0, &i1);

	// add area from max wl to its nearest lower wl point
	area += detector_response_integral (max, 1, &i2);

	// add area of intervening points
	for (i=0; i<n; i++) {
		if (i>=i1 && (i+1)<i2) {  // whole region is between min and max
			area += (wls[i+1]-wls[i]) * (vals[i]+vals[i+1]) / 2.0;
		}
	}

	area /= (max-min);

	return area;
}

void laserpowermonitor_LOGP_set_wavelength_range (LaserPowerMonitor* laserpower_mon, double min_nm, double max_nm)
{
	ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP *) laserpower_mon;  

	if (min_nm > 0.0 && 
		min_nm < max_nm &&
		max_nm < 2000.0) {
		
			atd_laserpower_mon_LOGP->_wavelength_factor = 1.0 / get_detector_response (min_nm, max_nm);
			sprintf(atd_laserpower_mon_LOGP->_wavelength_message, "%.0f to %.0f nm", min_nm, max_nm);
//			sprintf(atd_laserpower_mon_LOGP->_wavelength_message, "wl x %.3f", atd_laserpower_mon_LOGP->_wavelength_factor);
	}
	else {
			atd_laserpower_mon_LOGP->_wavelength_factor = 1.0;
			sprintf(atd_laserpower_mon_LOGP->_wavelength_message, "No cube correction.");
	}
}

void laserpowermonitor_LOGP_set_aperture (LaserPowerMonitor* laserpower_mon, double aperture)
{
	ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP *) laserpower_mon; 

	if (aperture > 0.0 && 
		aperture < 100.0) {
		
			atd_laserpower_mon_LOGP->_aperture_factor = pow(aperture, 2.0) / pow(atd_laserpower_mon_LOGP->_detector_aperture, 2.0);
			sprintf(atd_laserpower_mon_LOGP->_aperture_message, "%.1f mm obj aperture", aperture);
//			sprintf(atd_laserpower_mon_LOGP->_aperture_message, "obj x %.3f", atd_laserpower_mon_LOGP->_aperture_factor);
	}
	else {
			atd_laserpower_mon_LOGP->_aperture_factor = 1.0;
			sprintf(atd_laserpower_mon_LOGP->_aperture_message, "No obj correction.");
	}

}

void laserpowermonitor_LOGP_display_val(LaserPowerMonitor* laserpower_mon, double PD_current)
{
	ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP *) laserpower_mon; 
	double scaled_PD_current;

	// store the level
	atd_laserpower_mon_LOGP->level = PD_current/1000000000.0; // Store a value in mA

	if(PD_current<=1000.0 ){
		scaled_PD_current = PD_current;
		SetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_UNIT,"pW"); 
	}

	else if(PD_current>=1000.0 && PD_current<1000000.0 ){
	   scaled_PD_current = PD_current/1000.0;
	   SetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_UNIT,"nW"); 
	   
	}
	 else if(PD_current>=1000000.0 && PD_current<1000000000.0 ){ 
	     scaled_PD_current = PD_current/1000000.0;
	   SetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_UNIT,"uW"); 
	   
	}
	  else if(PD_current>=1000000000.0 ){ 
	     scaled_PD_current = PD_current/1000000000.0;
	   SetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_UNIT,"mW"); 
	}

	if(scaled_PD_current<10.0){
		SetCtrlAttribute (laserpower_mon->_main_panel_id, LSR_PR_PNL_OUTPUT , ATTR_PRECISION, 3);
	}
	else if(scaled_PD_current<100.0){
	    SetCtrlAttribute (laserpower_mon->_main_panel_id, LSR_PR_PNL_OUTPUT , ATTR_PRECISION, 2);
	}
	else if(scaled_PD_current<1000.0){
	    SetCtrlAttribute (laserpower_mon->_main_panel_id, LSR_PR_PNL_OUTPUT , ATTR_PRECISION, 1);
	}
	else {
	    SetCtrlAttribute (laserpower_mon->_main_panel_id, LSR_PR_PNL_OUTPUT , ATTR_PRECISION, 0);
	}

	SetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_OUTPUT, scaled_PD_current);   

	// Limit at bottom of UI scale, at 1 pW
	if(PD_current < 1)
		PD_current = 1;

	SetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_LOG, log10(PD_current));

	SetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_CUBE_INFO, atd_laserpower_mon_LOGP->_wavelength_message);
	SetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_OBJ_INFO, atd_laserpower_mon_LOGP->_aperture_message);
}

int atd_laserpowermonitor_LOGP_destroy (LaserPowerMonitor *laserpower_mon)
{
	ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP*) laserpower_mon;

	#ifdef FTDI_NO_VIRTUAL_COMPORT
	ftdi_controller_close(atd_laserpower_mon_LOGP->_controller);
	#else
	close_comport(atd_laserpower_mon_LOGP->_com_port);
	#endif

	return HARDWARE_SUCCESS;
}

static int atd_laserpowermonitor_LOGP_get (LaserPowerMonitor*laserpower_mon, double *val)
{
	ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP*) laserpower_mon;
	unsigned int msb, lsb;
	int reading, gain;
	double holdoff, delay, bias, PD_current;
    byte vals[10] = "";
   
	if (atd_laserpower_mon_LOGP == NULL)
	  return HARDWARE_ERROR;
	
	// Get configuration values from the UI
	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_GAIN, &gain);
	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_HOLDOFF, &holdoff);
	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_DELAY, &delay);
	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_BIAS, &bias);

	// Apropriate delays so that reading occurs at correct point in the excitation cycle (e.g. when the FL shutter is open)
	Delay(holdoff/1000.0);		//Delay in seconds
	atd_laserpowermonitor_LOGP_set_gain(atd_laserpower_mon_LOGP, (LaserPowerverMonitorGainValue) gain);
	Delay(delay/1000.0);		//Delay in seconds

	// Read a value from the device
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_i2c_read_bytes(atd_laserpower_mon_LOGP->_controller, atd_laserpower_mon_LOGP->_i2c_chip_address, 3, vals) != FT_OK) {
		return HARDWARE_ERROR;
	}

	#else

	GetI2CPortLock(atd_laserpower_mon_LOGP->_com_port, "atd_laserpowermonitor_LOGP_get");  
	
	if(atd_laserpower_mon_LOGP->_com_port < 0 || atd_laserpower_mon_LOGP->_com_port > 20) {
		ReleaseI2CPortLock(atd_laserpower_mon_LOGP->_com_port, "atd_laserpowermonitor_LOGP_get");  
		return HARDWARE_ERROR;
	}

	vals[0] = atd_laserpower_mon_LOGP->_i2c_chip_type | 0x01;
	
	if (GCI_readI2C_multiPort(atd_laserpower_mon_LOGP->_com_port, 3, vals, atd_laserpower_mon_LOGP->_i2c_bus, "atd_laserpowermonitor_LOGP_get")) {
		ReleaseI2CPortLock(atd_laserpower_mon_LOGP->_com_port, "atd_laserpowermonitor_LOGP_get");  
	
		return HARDWARE_ERROR;
	}

	ReleaseI2CPortLock(atd_laserpower_mon_LOGP->_com_port, "atd_laserpowermonitor_LOGP_get");  

	#endif

	// decode the value from the least and most signifacant bytes
	msb = vals[0] & 0xff; 
	lsb = vals[1] & 0xff; 
    		
    reading = (msb<<8 | lsb);
    
	// interpret as 16-bit signed int, in case value falls below zero
	if(reading>32767)
	   reading=(reading-65536);

	// Calculate the photodiode current
	PD_current = 10 * pow (10,((reading/3200.0)+(5*bias)));   //Value in pA

	// correct for device specific calibration
	PD_current *= atd_laserpower_mon_LOGP->_calibration;

	// correct for wavelength range
	PD_current *= atd_laserpower_mon_LOGP->_wavelength_factor;

	// correct for objective back aperture
	PD_current *= atd_laserpower_mon_LOGP->_aperture_factor;

	*val = PD_current;

	return HARDWARE_SUCCESS; 
}

// This function does not do an acquisition by design should
// be a fast read from the ui.
int laser_power_monitor_LOGP_hardware_get_current_value_text (HardwareDevice* device, char* info)
{
	ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP*) device;
	LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) device;

	if (info==NULL)
		return HARDWARE_ERROR;

//	GetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_OUTPUT, &val);
//	GetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_UNIT, units);
//	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_GAIN, &gain_index);
//	sprintf(info, "%.3f %s", val, units);

	sprintf(info, "%.3g", atd_laserpower_mon_LOGP->level);

	return HARDWARE_SUCCESS;

	//if (atd_laserpowermonitor_LOGP_get (laserpower_mon, &val) == HARDWARE_SUCCESS){
	//	sprintf(info, "%f", val);
	//	return HARDWARE_SUCCESS;
	//}

	//return HARDWARE_ERROR;
}

static void ftdi_error_handler (FTDIController *controller, const char *title, const char *error_string, void *callback_data)    
{
	printf("%s: %s\n", title, error_string);
}

static int atd_laserpowermonitor_LOGP_init_hardware(ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP, LaserPowerverMonitorBitModeType mode)
{
	byte vals[10] = "";

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	vals[0]=mode;
   		
	if(ftdi_controller_i2c_write_bytes(atd_laserpower_mon_LOGP->_controller, atd_laserpower_mon_LOGP->_i2c_chip_address, 1, vals) != FT_OK) {
		return HARDWARE_ERROR;
	}

	#else

   	vals[0]=atd_laserpower_mon_LOGP->_i2c_chip_type;		   //Set cofiguration
   	vals[1]=mode;	
   				
	if(GCI_writeI2C_multiPort(atd_laserpower_mon_LOGP->_com_port, 2, vals, atd_laserpower_mon_LOGP->_i2c_bus, "atd_laserpowermonitor_LOGP_set_gain") < 0)
		return HARDWARE_ERROR;
	
	#endif

	return HARDWARE_SUCCESS;
}

static int atd_laserpowermonitor_LOGP_hardware_initialise (HardwareDevice *hw_device)
{
	char device_sn[FTDI_SERIAL_NUMBER_LINE_LENGTH] = "";
	char device[UIMODULE_NAME_LEN];

	LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) hw_device;
	ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP*) laserpower_mon;

	ui_module_get_name(UIMODULE_CAST(atd_laserpower_mon_LOGP), device);

	get_device_double_param_from_ini_file(device, "aperture", &(atd_laserpower_mon_LOGP->_detector_aperture));
	get_device_double_param_from_ini_file(device, "calibration", &(atd_laserpower_mon_LOGP->_calibration));
		
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	get_device_string_param_from_ini_file(device, "FTDI_SN", device_sn);  
	get_device_int_param_from_ini_file   (device, "I2C_DEVICE_BUS", &(atd_laserpower_mon_LOGP->_i2c_bus));
	get_device_int_param_from_ini_file   (device, "I2C_DEVICE_ADDRESS", &(atd_laserpower_mon_LOGP->_i2c_chip_address));  
	
	atd_laserpower_mon_LOGP->_controller = ftdi_controller_new();

	//ftdi_controller_set_debugging(atd_laserpower_mon_LOGP->controller, 1);
	ftdi_controller_set_error_handler(atd_laserpower_mon_LOGP->_controller, ftdi_error_handler, NULL);
	ftdi_controller_open(atd_laserpower_mon_LOGP->_controller, device_sn);

	#else

	get_device_int_param_from_ini_file   (device, "COM_Port", &(atd_laserpower_mon_LOGP->_com_port));  
	get_device_int_param_from_ini_file   (device, "i2c_Bus", &(atd_laserpower_mon_LOGP->_i2c_bus));
	get_device_int_param_from_ini_file   (device, "i2c_ChipAddress", &(atd_laserpower_mon_LOGP->_i2c_chip_address));  
	get_device_int_param_from_ini_file   (device, "i2c_ChipType", &(atd_laserpower_mon_LOGP->_i2c_chip_type));  

	if (initialise_comport(atd_laserpower_mon_LOGP->_com_port, 9600) < 0)
		return HARDWARE_ERROR;
	
	#endif

	if (get_device_double_param_from_ini_file(UIMODULE_GET_NAME(atd_laserpower_mon_LOGP), "TimerInterval", &(laserpower_mon->_timer_interval))<0) 
	{
		#ifdef HEAVY_I2C_TESTING 
		laserpower_mon->_timer_interval=0.001;
		#else
		laserpower_mon->_timer_interval=1.0;
		#endif
	}

	if(atd_laserpowermonitor_LOGP_init_hardware(atd_laserpower_mon_LOGP, LASER_POWER_MODE_16BIT_TRIGGERED) == HARDWARE_ERROR)
		return HARDWARE_ERROR;

	return HARDWARE_SUCCESS;  
}


static int CVICALLBACK OnLaserPowerMonitor_LOGP_GainSet (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP*) callbackData; 
			LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) atd_laserpower_mon_LOGP;

			int gain;
			
			GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_GAIN, &gain);

			atd_laserpowermonitor_LOGP_set_gain(atd_laserpower_mon_LOGP, (LaserPowerverMonitorGainValue) gain);

			break;
		}
	}

	return 0;
}

static int CVICALLBACK OnLaserPowerMonitor_LOGP_ClosePressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP*) callbackData; 

			ui_module_hide_main_panel(UIMODULE_CAST(atd_laserpower_mon_LOGP));

			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnLaserPowerMonitor_LOGP_AcquirePressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP*) callbackData; 
			LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) atd_laserpower_mon_LOGP;
			double reading;

			laserpowermonitor_disable_timer(laserpower_mon);
			SetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_LIVE, 0);

			if(laserpowermonitor_get (laserpower_mon, &reading) == HARDWARE_ERROR)
				return -1;

			laserpowermonitor_display_val(laserpower_mon, reading);

			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnLaserPowerMonitor_LOGP_LivePressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int liveOn=0;

			ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP*) callbackData; 
			LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) atd_laserpower_mon_LOGP;

			GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_LIVE, &liveOn);

			if (liveOn){
				laserpowermonitor_set_timer_interval(laserpower_mon, 1.0);
				laserpowermonitor_enable_timer(laserpower_mon);
			}
			else {
				laserpowermonitor_disable_timer(laserpower_mon);
			}

			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnLaserPowerMonitor_LOGP_MinShutter (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP*) callbackData; 
			LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) atd_laserpower_mon_LOGP;
			double val;

			GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_MIN_SHUTTER, &val);
			SetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_MIN_SHUTTER, val);

			break;
		}
	}
	
	return 0;
}

int CVICALLBACK OnLaserPowerMonitor_LOGP_CloseSettings (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP*) callbackData; 
			LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) atd_laserpower_mon_LOGP;

			ui_module_hide_panel(UIMODULE_CAST(atd_laserpower_mon_LOGP), laserpower_mon->_settings_panel_id);

			break; 
		}
	}
	
	return 0;
}

int CVICALLBACK OnLaserPowerMonitor_LOGP_ShowSettings (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP*) callbackData; 
			LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) atd_laserpower_mon_LOGP;

			GCI_ShowPasswordProtectedPanel(laserpower_mon->_settings_panel_id, laserpower_mon->_main_panel_id);

			break; 
		}
	}
	
	return 0;
}

int CVICALLBACK OnLaserPowerMonitor_LOGP_SaveSettings (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP*) callbackData; 
			char path[GCI_MAX_PATHNAME_LEN];
			int ret;

			ui_module_get_data_dir(UIMODULE_CAST(atd_laserpower_mon_LOGP), path);

			strcat(path, "\\");
			strcat(path, UIMODULE_GET_NAME(UIMODULE_CAST(atd_laserpower_mon_LOGP)));
			strcat(path, ".ini");

			ret = hardware_save_state_to_file (HARDWARE_DEVICE_CAST(atd_laserpower_mon_LOGP), path, "w");

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

int atd_laserpowermonitor_LOGP_initialise (LaserPowerMonitor* laserpower_mon)
{
	ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP *) laserpower_mon;  
	char path[GCI_MAX_PATHNAME_LEN] = "";

	laserpower_mon->_main_panel_id = ui_module_add_panel(UIMODULE_CAST(atd_laserpower_mon_LOGP), "ATD_LaserPowerMonitor_LOGP_UI.uir", LSR_PR_PNL, 1);

	laserpower_mon->_settings_panel_id = ui_module_add_panel(UIMODULE_CAST(atd_laserpower_mon_LOGP), "ATD_LaserPowerMonitor_LOGP_UI.uir", LSR_PR_SET, 0);

	InstallCtrlCallback (laserpower_mon->_settings_panel_id, LSR_PR_SET_GAIN, OnLaserPowerMonitor_LOGP_GainSet, atd_laserpower_mon_LOGP);

	InstallCtrlCallback (laserpower_mon->_main_panel_id, LSR_PR_PNL_QUIT, OnLaserPowerMonitor_LOGP_ClosePressed, atd_laserpower_mon_LOGP);	
	InstallCtrlCallback (laserpower_mon->_main_panel_id, LSR_PR_PNL_SETTINGS, OnLaserPowerMonitor_LOGP_ShowSettings, atd_laserpower_mon_LOGP);
	InstallCtrlCallback (laserpower_mon->_settings_panel_id, LSR_PR_SET_MIN_SHUTTER, OnLaserPowerMonitor_LOGP_MinShutter, atd_laserpower_mon_LOGP);

	InstallCtrlCallback (laserpower_mon->_settings_panel_id, LSR_PR_SET_LIVE, OnLaserPowerMonitor_LOGP_LivePressed, atd_laserpower_mon_LOGP);
	InstallCtrlCallback (laserpower_mon->_settings_panel_id, LSR_PR_SET_ACQUIRE, OnLaserPowerMonitor_LOGP_AcquirePressed, atd_laserpower_mon_LOGP);
	InstallCtrlCallback (laserpower_mon->_settings_panel_id, LSR_PR_SET_CLOSE_SETTINGS, OnLaserPowerMonitor_LOGP_CloseSettings, atd_laserpower_mon_LOGP);
	InstallCtrlCallback (laserpower_mon->_settings_panel_id, LSR_PR_SET_SAVE, OnLaserPowerMonitor_LOGP_SaveSettings, atd_laserpower_mon_LOGP);

	sprintf(path, "%s\\%s.ini", UIMODULE_GET_DATA_DIR(atd_laserpower_mon_LOGP), UIMODULE_GET_NAME(atd_laserpower_mon_LOGP));

	hardware_load_state_from_file(HARDWARE_DEVICE_CAST(laserpower_mon), path); 

	return HARDWARE_SUCCESS;
}

int atd_laserpowermonitor_LOGP_set_gain(ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP, LaserPowerverMonitorGainValue gain)
{	// NB takes the ctrl index as the gain value
	LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) atd_laserpower_mon_LOGP;
	byte vals[10];
	int config = LASER_POWER_MODE_16BIT_TRIGGERED;
	config = (config & 0xFC);		//Clear gain bits
	config = (config | (int) gain);
				
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	vals[0]=config;
   		
	if(ftdi_controller_i2c_write_bytes(atd_laserpower_mon_LOGP->_controller, atd_laserpower_mon_LOGP->_i2c_chip_address, 2, vals) != FT_OK) {
		return HARDWARE_ERROR;
	}

	#else

   	vals[0]=atd_laserpower_mon_LOGP->_i2c_chip_type;		   //Set cofiguration
   	vals[1]=config;	
   				
	GCI_writeI2C_multiPort(atd_laserpower_mon_LOGP->_com_port, 2, vals, atd_laserpower_mon_LOGP->_i2c_bus, "atd_laserpowermonitor_LOGP_set_gain"); 

	#endif

	SetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_GAIN, gain);

	return HARDWARE_SUCCESS;
}

static int atd_laserpowermonitor_LOGP_get_gain(LaserPowerMonitor* laserpower_mon, int *gain)
{	// returns the actual gain factor
	int gain_idx;

	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_GAIN, &gain_idx);
	*gain = gain_factors[gain_idx];

	return HARDWARE_SUCCESS;
}

static int atd_laserpowermonitor_LOGP_get_min_shutter(LaserPowerMonitor* laserpower_mon, double *val)
{	// returns the actual gain factor

	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_MIN_SHUTTER, val);

	return HARDWARE_SUCCESS;
}

int atd_laserpowermonitor_hardware_save_state_to_file (HardwareDevice* device, const char* filepath, const char *mode)
{
	int gain;
	double holdoff, delay, bias, min_exposure;
	FILE *fp = NULL;
	LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) device;

	dictionary *d = dictionary_new(5);
	
	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_GAIN, &gain);
	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_HOLDOFF, &holdoff);
	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_DELAY, &delay);
	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_BIAS, &bias);
	GetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_MIN_SHUTTER, &min_exposure);

	fp = fopen(filepath, mode);
	
	if(fp == NULL)
		return HARDWARE_ERROR;

	dictionary_set(d, UIMODULE_GET_NAME(laserpower_mon), NULL);
	dictionary_setint(d, "Gain", gain);  
	dictionary_setdouble(d, "Holdoff", holdoff);  
	dictionary_setdouble(d, "Delay", delay);  
	dictionary_setdouble(d, "Bias", bias);  
	dictionary_setdouble(d, "MinShutter", min_exposure);  

	iniparser_save(d, fp); 
	
	fclose(fp);
	dictionary_del(d);

	return HARDWARE_SUCCESS;
}

int atd_laserpowermonitor_LOGP_hardware_load_state_from_file (HardwareDevice* device, const char* filepath)
{
	LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) device;

	dictionary* d = NULL;
	int gain, file_size;   
	double holdoff, delay, bias, min_exposure;
	char buffer[500] = "";

	if(!FileExists(filepath, &file_size))
		return HARDWARE_ERROR;	 	
	
	d = iniparser_load(filepath);  

	if(d != NULL) {

		gain = dictionary_getint(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(laserpower_mon), "gain"), -1); 

		if(gain >= 0) {
			
			atd_laserpowermonitor_LOGP_set_gain((ATD_LaserPowerMonitor_LOGP*) laserpower_mon, (LaserPowerverMonitorGainValue) gain);
		}

		holdoff = dictionary_getdouble(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(laserpower_mon), "holdoff"), -1.0); 

		if(holdoff >= 0) {
			
			SetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_HOLDOFF, holdoff);
		}

		delay = dictionary_getdouble(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(laserpower_mon), "delay"), -1.0); 

		if(delay >= 0) {
			
			SetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_DELAY, delay);
		}

		bias = dictionary_getdouble(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(laserpower_mon), "bias"), -1.0); 

		if(bias >= 0) {
			
			SetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_BIAS, bias);
		}

		// set default 100.0 as is a new parameter and may not be in file.
		SetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_MIN_SHUTTER, 100.0);
		SetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_MIN_SHUTTER, 100.0);
		min_exposure = dictionary_getdouble(d,  dictionary_get_section_key(buffer, UIMODULE_GET_NAME(laserpower_mon), "MinShutter"), 100.0); 

		if(min_exposure >= 0) {
			SetCtrlVal(laserpower_mon->_main_panel_id, LSR_PR_PNL_MIN_SHUTTER, min_exposure);
			SetCtrlVal(laserpower_mon->_settings_panel_id, LSR_PR_SET_MIN_SHUTTER, min_exposure);
		}
	}

    dictionary_del(d);

	return HARDWARE_SUCCESS;
}

LaserPowerMonitor* atd_laserpowermonitor_LOGP_new(const char *name, const char *description,
  UI_MODULE_ERROR_HANDLER error_handler, const char *data_dir, void *data)
{
	ATD_LaserPowerMonitor_LOGP* atd_laserpower_mon_LOGP = (ATD_LaserPowerMonitor_LOGP*) malloc(sizeof(ATD_LaserPowerMonitor_LOGP));
    LaserPowerMonitor* laserpower_mon = (LaserPowerMonitor*) atd_laserpower_mon_LOGP;

	memset(atd_laserpower_mon_LOGP, 0, sizeof(ATD_LaserPowerMonitor_LOGP));
	
	laserpowermonitor_constructor(laserpower_mon, name, description, error_handler, data_dir, data);

	// Default to no correction for cube or obj
	atd_laserpower_mon_LOGP->_detector_aperture = 0.0;
	atd_laserpower_mon_LOGP->_calibration = 1.0;
	atd_laserpower_mon_LOGP->_aperture_factor = 1.0;
	atd_laserpower_mon_LOGP->_wavelength_factor = 1.0;

	LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, display_val) = laserpowermonitor_LOGP_display_val; 
	LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, set_wavelength_range) = laserpowermonitor_LOGP_set_wavelength_range; 
	LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, set_aperture) = laserpowermonitor_LOGP_set_aperture; 
	LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, destroy) = atd_laserpowermonitor_LOGP_destroy; 
	LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, get_laser_power) = atd_laserpowermonitor_LOGP_get;
	LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, get_gain) = atd_laserpowermonitor_LOGP_get_gain;
	LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, initialise) = atd_laserpowermonitor_LOGP_initialise;
	LASER_PWR_MONITOR_VTABLE_PTR(laserpower_mon, get_minimum_shutter_time) = atd_laserpowermonitor_LOGP_get_min_shutter;

	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(atd_laserpower_mon_LOGP), hardware_initialise) = atd_laserpowermonitor_LOGP_hardware_initialise; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(atd_laserpower_mon_LOGP), hardware_get_current_value_text) = laser_power_monitor_LOGP_hardware_get_current_value_text; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(atd_laserpower_mon_LOGP), hardware_save_state_to_file) = atd_laserpowermonitor_hardware_save_state_to_file; 
	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(atd_laserpower_mon_LOGP), hardware_load_state_from_file) = atd_laserpowermonitor_LOGP_hardware_load_state_from_file; 

	return laserpower_mon;
}