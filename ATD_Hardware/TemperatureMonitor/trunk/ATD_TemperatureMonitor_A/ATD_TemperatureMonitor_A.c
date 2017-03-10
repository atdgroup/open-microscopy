#include "ATD_TemperatureMonitor_A.h"
#include "ATD_TemperatureMonitor_A_UI.h"
#include "TemperatureMonitorUI.h" 
 
#include "gci_utils.h"
#include "ATD_UsbInterface_A.h" 

#include <userint.h>
#include <formatio.h>
#include <cviauto.h>  
#include "asynctmr.h"
#include <rs232.h>
#include <utility.h>

static int atd_temperature_monitor_read_temperatures (TemperatureMonitor* tm, double *value1, double *value2, double *value3, double *value4)
{
	ATD_TEMPERATURE_MONITOR_A *tm_a = (ATD_TEMPERATURE_MONITOR_A*) tm;

	byte vals[20] = "";
	int chan0_h,chan0_l,chan1_h,chan1_l,chan2_h,chan2_l,chan3_h,chan3_l;
	double temp_scale, temp_offset;
	double temp0, temp1, temp2, temp3;

	*value1 = *value2 = *value3 = *value4 = 0.0;

   	vals[0]=tm_a->_i2c_type;
   	vals[1]=0;  // Set mode for reading A/D inputs  

	GetI2CPortLock(tm_a->_com_port, "atd_temperature_monitor_read_temperatures");   

   	if(GCI_writeI2C_multiPort(tm_a->_com_port, 6, vals, tm_a->_i2c_bus, "atd_temperature_monitor_read_temperature") < 0){ 	 
		send_temperature_monitor_error_text(tm, "Failed to read temperature inputs");
		ReleaseI2CPortLock(tm_a->_com_port, "atd_temperature_monitor_read_temperatures");   
		return  TEMPERATURE_MONITOR_ERROR ; 
		
	}

	Delay(0.1);		//Allow time to read A/D inputs (25ms)
   			    
	vals[0]=tm_a->_i2c_type;
   	vals[1]=1;  // Set to the read input
   	vals[2]=1;  // Set to read A/D  readings

   	if(GCI_writeI2C_multiPort(tm_a->_com_port, 6, vals, tm_a->_i2c_bus, "atd_temperature_monitor_read_temperature") < 0){ 	 
		send_temperature_monitor_error_text(tm, "Failed to read temperature");
		ReleaseI2CPortLock(tm_a->_com_port, "atd_temperature_monitor_read_temperatures");   
		return  TEMPERATURE_MONITOR_ERROR; 
   	}

	// Clear the array we use form sending and reading.
	memset(vals, 0, 20);

   	vals[0] = tm_a->_i2c_type  | 0x01;

    if(GCI_readI2C_multiPort(tm_a->_com_port, 8, vals, tm_a->_i2c_bus, "atd_temperature_monitor_read_temperature") < 0){ 	 
		send_temperature_monitor_error_text(tm, "Failed to read temperature");
		ReleaseI2CPortLock(tm_a->_com_port, "atd_temperature_monitor_read_temperatures");   
		return  TEMPERATURE_MONITOR_ERROR ; 	
    }	
    			
	ReleaseI2CPortLock(tm_a->_com_port, "atd_temperature_monitor_read_temperatures");   

	chan0_h = vals[0] & 0xff; 
    chan0_l = vals[1] & 0xff;
    chan1_h = vals[2] & 0xff; 
    chan1_l = vals[3] & 0xff;
    chan2_h = vals[4] & 0xff; 
    chan2_l = vals[5] & 0xff;
    chan3_h = vals[6] & 0xff; 
   	chan3_l = vals[7] & 0xff;  
   			    
	SetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_AN0  ,(((chan0_h<<8) | chan0_l)));  
    SetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_AN1  ,(((chan1_h<<8) | chan1_l)));  
    SetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_AN2  ,(((chan2_h<<8) | chan2_l))); 
    SetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_AN3  ,(((chan3_h<<8) | chan3_l))); 

	GetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_CALTEMPSCALE1, &temp_scale);
	GetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_CALTEMPOFFSET1, &temp_offset);

   	temp0=((chan0_h<<8) | chan0_l);
   	temp0=(temp0/65472);
   	temp0=((temp0 * temp_scale) + temp_offset);
   			   
	GetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_CALTEMPSCALE2, &temp_scale);
	GetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_CALTEMPOFFSET2, &temp_offset);

   	temp1=((chan1_h<<8) | chan1_l);
   	temp1=(temp1/65472);
   	temp1=((temp1 * temp_scale) + temp_offset);
   			   
	GetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_CALTEMPSCALE3, &temp_scale);
	GetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_CALTEMPOFFSET3, &temp_offset);

   	temp2=((chan2_h<<8) | chan2_l);
   	temp2=(temp2/65472);
   	temp2=((temp2 * temp_scale) + temp_offset);
   			  			
	GetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_CALTEMPSCALE4, &temp_scale);
	GetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_CALTEMPOFFSET4, &temp_offset);

   	temp3=((chan3_h<<8) | chan3_l);
   	temp3=(temp3/65472);
   	temp3=((temp3 * temp_scale) + temp_offset);
   			 		
	*value1 = temp0;
	*value2 = temp1;
	*value3 = temp2;
	*value4 = temp3;		

	return TEMPERATURE_MONITOR_SUCCESS;  
}

static int atd_temperature_get_status_flags(TemperatureMonitor* tm,  int *thermostat_error,
											int *heater_over_temp, int *enclosure_state)
{
	ATD_TEMPERATURE_MONITOR_A *tm_a = (ATD_TEMPERATURE_MONITOR_A*) tm;
	byte vals[20] = "";

	temperature_monitor_clear_errors (tm);
	
	GetI2CPortLock(tm_a->_com_port, "atd_temperature_set_errors");    

	vals[0]=tm_a->_i2c_type; 
   	vals[1]=1;	// Set to the read input   
   	vals[2]=2;  // Set to read digital inputs
   			
	if(GCI_writeI2C_multiPort(tm_a->_com_port, 6, vals, tm_a->_i2c_bus, "atd_temperature_set_errors") < 0){ 	 
		send_temperature_monitor_error_text(tm, "Failed to read digital inputs");
		ReleaseI2CPortLock(tm_a->_com_port, "atd_temperature_set_errors");    
		return  TEMPERATURE_MONITOR_ERROR; 
   	}

	// Clear the array we use form sending and reading.
	memset(vals, 0, 6);
   	vals[0]=tm_a->_i2c_type | 0x01;
   				
    if(GCI_readI2C_multiPort(tm_a->_com_port, 1, vals, tm_a->_i2c_bus, "atd_temperature_set_errors") < 0){ 	 
		send_temperature_monitor_error_text(tm, "Failed to read temperature");
		ReleaseI2CPortLock(tm_a->_com_port, "atd_temperature_set_errors");   
		return TEMPERATURE_MONITOR_ERROR; 
    }

	ReleaseI2CPortLock(tm_a->_com_port, "atd_temperature_set_errors");   

    *thermostat_error = !(vals[0] & 0x01);  
    *heater_over_temp = (vals[0] & 0x02)>>1;
    *enclosure_state  = (vals[0] & 0x04)>>2; 
	return TEMPERATURE_MONITOR_SUCCESS; 
}

static int atd_temperature_set_errors (TemperatureMonitor* tm)
{
	ATD_TEMPERATURE_MONITOR_A *tm_a = (ATD_TEMPERATURE_MONITOR_A*) tm;
	int thermostat_error = 0, heater_over_temp = 0, enclosure_state = 0;

	atd_temperature_get_status_flags(tm, &thermostat_error, &heater_over_temp, &enclosure_state);

	if(thermostat_error) {
		if (tm_a->_last_thermostat_error != thermostat_error) {
			temperature_monitor_add_error (tm, 1, TEMP_ERROR_CRITICAL, "Thermostat Error          ");
			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(tm), "TemperatureStateChanged", GCI_VOID_POINTER, tm);
		}
		else
			temperature_monitor_add_error (tm, 0, TEMP_ERROR_CRITICAL, "Thermostat Error          ");
	}

	if(heater_over_temp) {
		if(tm_a->_last_heater_over_temp != heater_over_temp) {
			temperature_monitor_add_error (tm, 1, TEMP_ERROR_CRITICAL, "Heater over temperature     ");		
			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(tm), "TemperatureStateChanged",
				GCI_VOID_POINTER, tm);
		}
		else
			temperature_monitor_add_error (tm, 0, TEMP_ERROR_CRITICAL, "Heater over temperature     ");	
	}

	if(enclosure_state) {
		if(tm_a->_last_enclosure_state != enclosure_state) {
			temperature_monitor_add_error (tm, 1, TEMP_ERROR_INFO, "Enclosure Open        ");	
			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(tm), "TemperatureStateChanged", GCI_VOID_POINTER, tm, GCI_STRING, "Enclosure", GCI_STRING, "Enclosure Open");
		}
		else {
			temperature_monitor_add_error (tm, 0, TEMP_ERROR_INFO, "Enclosure Open        ");
		}
	}
	else {
		if(tm_a->_last_enclosure_state != enclosure_state) {
			temperature_monitor_add_error (tm, 1, TEMP_ERROR_INFO, "Enclosure Closed        ");	
			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(tm), "TemperatureStateChanged", GCI_VOID_POINTER, tm);
		}
		else {
			temperature_monitor_add_error (tm, 0, TEMP_ERROR_INFO, "Enclosure Closed        ");
		}
	}
	

	tm_a->_last_thermostat_error = thermostat_error;  
    tm_a->_last_heater_over_temp = heater_over_temp;
    tm_a->_last_enclosure_state = enclosure_state;

	return TEMPERATURE_MONITOR_SUCCESS;  
}


static int atd_temperature_monitor_hardware_init(TemperatureMonitor *tm)
{
	ATD_TEMPERATURE_MONITOR_A *tm_a = (ATD_TEMPERATURE_MONITOR_A*) tm;

	get_device_param_from_ini_file(UIMODULE_GET_NAME(tm), "COM_Port", &(tm_a->_com_port)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(tm), "i2c_ChipAddress", &(tm_a->_i2c_address));      
	get_device_param_from_ini_file(UIMODULE_GET_NAME(tm), "i2c_Bus", &(tm_a->_i2c_bus));      
	get_device_param_from_ini_file(UIMODULE_GET_NAME(tm), "i2c_ChipType", &(tm_a->_i2c_type));      

	get_device_double_param_from_ini_file(UIMODULE_GET_NAME(tm), "TempScale1", &(tm_a->_temp_cal_scale0));
	get_device_double_param_from_ini_file(UIMODULE_GET_NAME(tm), "TempScale2", &(tm_a->_temp_cal_scale1));
	get_device_double_param_from_ini_file(UIMODULE_GET_NAME(tm), "TempScale3", &(tm_a->_temp_cal_scale2));
	get_device_double_param_from_ini_file(UIMODULE_GET_NAME(tm), "TempScale4", &(tm_a->_temp_cal_scale3));
	get_device_double_param_from_ini_file(UIMODULE_GET_NAME(tm), "TempOffset1", &(tm_a->_temp_cal_offset0));
	get_device_double_param_from_ini_file(UIMODULE_GET_NAME(tm), "TempOffset2", &(tm_a->_temp_cal_offset1));
	get_device_double_param_from_ini_file(UIMODULE_GET_NAME(tm), "TempOffset3", &(tm_a->_temp_cal_offset2));
	get_device_double_param_from_ini_file(UIMODULE_GET_NAME(tm), "TempOffset4", &(tm_a->_temp_cal_offset3));
	
	initialise_comport(tm_a->_com_port, 9600);

	return TEMPERATURE_MONITOR_SUCCESS;  
}

static int CVICALLBACK OnTemperatureMonitor_SetupClosePressed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			TemperatureMonitor *tm = (TemperatureMonitor*) callbackData;  

			ui_module_hide_panel(UIMODULE_CAST(tm), panel);

			break;
		}
	}
	
	return 0;
}

static int atd_temperature_monitor_init(TemperatureMonitor *tm)
{
	ATD_TEMPERATURE_MONITOR_A *tm_a = (ATD_TEMPERATURE_MONITOR_A*) tm;

	tm_a->_setup_panel_id = ui_module_add_panel(UIMODULE_CAST(tm), "ATD_TemperatureMonitor_A_UI.uir", SETUP_PNL, 0);
	temperature_monitor_set_setup_panel(tm, tm_a->_setup_panel_id);

	SetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_CALTEMPSCALE1, tm_a->_temp_cal_scale0);
	SetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_CALTEMPOFFSET1, tm_a->_temp_cal_offset0);

	SetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_CALTEMPSCALE2, tm_a->_temp_cal_scale1);
	SetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_CALTEMPOFFSET2, tm_a->_temp_cal_offset1);

	SetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_CALTEMPSCALE3, tm_a->_temp_cal_scale2);
	SetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_CALTEMPOFFSET3, tm_a->_temp_cal_offset2);

	SetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_CALTEMPSCALE4, tm_a->_temp_cal_scale3);
	SetCtrlVal(tm_a->_setup_panel_id, SETUP_PNL_CALTEMPOFFSET4, tm_a->_temp_cal_offset3);

	InstallCtrlCallback (tm_a->_setup_panel_id, SETUP_PNL_CLOSE, OnTemperatureMonitor_SetupClosePressed, tm);

	return TEMPERATURE_MONITOR_SUCCESS;   
}

static int atd_temperature_monitor_destroy (TemperatureMonitor *tm)
{			
	temperature_monitor_disable_timer(tm);

	return TEMPERATURE_MONITOR_SUCCESS;  
}

void OnTempMonitorPanelDisplayed (UIModule *module, void *data)
{
	TemperatureMonitor* tm = (TemperatureMonitor*) data;

	temperature_monitor_enable_timer(tm);
}

TemperatureMonitor* atd_temperature_monitor_a_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	TemperatureMonitor* tm = (TemperatureMonitor*) malloc(sizeof(ATD_TEMPERATURE_MONITOR_A));  
	ATD_TEMPERATURE_MONITOR_A* atd_temp_a = (ATD_TEMPERATURE_MONITOR_A *) tm;    

	memset(atd_temp_a, 0, sizeof(ATD_TEMPERATURE_MONITOR_A));  
	
	temperature_monitor_constructor(tm, name, description, data_dir);

	atd_temp_a->_last_thermostat_error = -1;  
    atd_temp_a->_last_heater_over_temp = -1;
    atd_temp_a->_last_enclosure_state = -1;

	TEMPERATURE_MONITOR_VTABLE_PTR(tm, hw_initialise) = atd_temperature_monitor_hardware_init; 
	TEMPERATURE_MONITOR_VTABLE_PTR(tm, initialise) = atd_temperature_monitor_init; 
	TEMPERATURE_MONITOR_VTABLE_PTR(tm, destroy) = atd_temperature_monitor_destroy; 
	TEMPERATURE_MONITOR_VTABLE_PTR(tm, read_temperatures) = atd_temperature_monitor_read_temperatures; 
	TEMPERATURE_MONITOR_VTABLE_PTR(tm, set_errors) = atd_temperature_set_errors;
	TEMPERATURE_MONITOR_VTABLE_PTR(tm, get_status_flags) = atd_temperature_get_status_flags;

	ui_module_main_panel_show_mainpanel_handler_connect(UIMODULE_CAST(tm), OnTempMonitorPanelDisplayed, tm);

	return tm;
}
