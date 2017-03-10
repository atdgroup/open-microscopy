#include "white_light_laser.h"
#include "white_light_laser_ui.h"
#include "gci_utils.h"

#include <userint.h>
#include <formatio.h>
#include <cviauto.h>  
#include "asynctmr.h"
#include <rs232.h>
#include <utility.h>
#include "string_utils.h"
#include "ThreadDebug.h"

#include "fortify.h"

#define RS232_ARRAY_SIZE 500
#define KEY_ON_STRING "Mode cannot be changed while laser is enabled"
#define OSCILLATOR_STRING "Laser Temperature ready"
#define OSCILLATOR_ADMIN_STRING "Supervisor mode is on. Laser Temperature check has been disabled"
#define USER_TEMP_WARNING "The oscillator is not yet at the required temperature."

static const char* const LaserTypeStrings[] = 
{
        "Fianium SC450-4",
        "Fianium SC450-M",
};

const static char* Modes[][2] = 
{
	{"Pot controlled open loop mode", "Manual Control"},
	{"USB open loop mode", "Computer controlled"},
	{"Voltage controlled open loop mode", "Externally Controlled"}
};

static void disable_enable_panel_for_state(Laser* laser)
{
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;

	if(!whitelight_laser_is_oscillator_ready(laser)) {
		ui_module_disable_panel(wl_laser->_panel_id, 2, WL_PNL_CLOSE, WL_PNL_MORE);
		ui_module_disable_panel(wl_laser->_extra_panel, 1, WL_PNL_CLOSE);
	}
	else {

		WHITE_LIGHT_LASER_MODE mode;

		whitelight_laser_get_mode(laser, &mode);

		if(mode == WL_COMPUTER_MODE) {

			ui_module_enable_panel(wl_laser->_panel_id, 0);
			ui_module_enable_panel(wl_laser->_extra_panel, 0);
		}
		else {

			ui_module_disable_panel(wl_laser->_panel_id, 2, WL_PNL_CLOSE, WL_PNL_MORE);
			ui_module_disable_panel(wl_laser->_extra_panel, 2, WL_PNL_CLOSE, WL_MORE_MODE);
		}
	}
}

#ifdef FTDI_NO_VIRTUAL_COMPORT

FT_STATUS ftdi_controller_laser_write_string(FTDIController* controller, const char* fmt, ...)
{
	FT_STATUS ftStatus;
	char buffer[200] = "";
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buffer, fmt, ap);
	va_end(ap); 

	ftStatus = ftdi_controller_write_bytes(controller, strlen(buffer), buffer);

	// Something odd about the laser.
	// This is not the time for RS232 that has been dealt with above. 
	// It must been some overhead in the laser firmware or control ?
	Delay(0.2);

	if(ftStatus != FT_OK)
		return  ftStatus;

	return ftStatus;
}

#else

static int GCI_ComWrt(int COMPort, char buffer[], int count)
{
	int ret;
	char read_data[RS232_ARRAY_SIZE]="";

	ret = ComWrt(COMPort, buffer, count);    
	
	if (ret < 0) {
		return ret;
	}
	
	while(GetOutQLen(COMPort)>0) {
        ProcessSystemEvents();
		Delay(0.001);
		continue;
	}
    
	// Something odd about the laser.
	// This is not the time for RS232 that has been dealt with above. 
	// It must been some overhead in the laser firmware or control ?
	Delay(0.2);

	return ret;
}

static int RS232_SendString(Laser* laser, char* fmt, ...) 
{
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
    char buffer[RS232_ARRAY_SIZE] = "";
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buffer, fmt, ap);
	va_end(ap); 

	if(wl_laser->_destroying == 1)
		return -1;

    GciCmtGetLock (wl_laser->_lock);	// For multi-threading
	
	// Flush the In Queue
	FlushInQ(wl_laser->_com_port);
	FlushOutQ(wl_laser->_com_port);

	// send string 
	if (GCI_ComWrt(wl_laser->_com_port, buffer, strlen(buffer)) < 0) {
		GciCmtReleaseLock (wl_laser->_lock);
		logger_log(UIMODULE_LOGGER(laser), LOGGER_ERROR, "White Light Laser Controller Error"); 
		wl_laser->_nErrors++;

		if (wl_laser->_nErrors>=3){ 
			wl_laser->_initialised = 0;
			logger_log(UIMODULE_LOGGER(laser), LOGGER_ERROR, "White Light Laser: Comms stopped"); 
		}
		
		return -1;
	}

	GciCmtReleaseLock (wl_laser->_lock);

	return 0;
}	

#endif

static int RS233_ReadString(Laser* laser, int number_of_characters, char *retval)
{
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
	int no_bytes_read, queue_length;
	char read_data[RS232_ARRAY_SIZE]="";

	// Reads the Stage Port and returns a string
	// Returned character string is always terminated with LF, (ASCII 10)
	// will return 0 if read is successful, -1 otherwise
	GciCmtGetLock (wl_laser->_lock);					

	retval[0] = '\0';
	read_data[0] = '\0';

	queue_length = GetInQLen(wl_laser->_com_port);

	if(queue_length > 0) {

		no_bytes_read = ComRd(wl_laser->_com_port, read_data, queue_length);
	
		if(rs232err == -99) {
			FlushInQ(wl_laser->_com_port);
			FlushOutQ(wl_laser->_com_port);
			GciCmtReleaseLock (wl_laser->_lock);
			printf("RS233 Error\n");
			return  WL_ERROR;
		}

		if(no_bytes_read) {

			strncpy(retval, read_data, RS232_ARRAY_SIZE - 1);
		}
	}

	Delay(0.2);

	GciCmtReleaseLock (wl_laser->_lock);
	
	return 0;
}

static void ftdi_error_handler (FTDIController *controller, const char *title, char *error_string, void *callback_data)    
{
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) callback_data;

	logger_log(UIMODULE_LOGGER(wl_laser), LOGGER_ERROR, error_string); 
}

int initFianiumLaserRS232Port(Laser* laser)
{
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
	int attempts=0, baud=19200;
	char retval[RS232_ARRAY_SIZE] = "";
	char version_str[RS232_ARRAY_SIZE] = "";
	DWORD amount_in_rx_queue = 0;

	GciCmtGetLock (wl_laser->_lock);   
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	//ftdi_controller_set_debugging(zd->controller, 1);
	ftdi_controller_set_error_handler(wl_laser->controller, ftdi_error_handler, wl_laser);
	
	if(ftdi_controller_open(wl_laser->controller, wl_laser->device_sn) != FT_OK)
		goto NotOpenedError;

	if(ftdi_controller_set_baudrate(wl_laser->controller, baud) != FT_OK)
		goto Error;

	if(ftdi_controller_set_data_characteristics(wl_laser->controller, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE) != FT_OK)
		goto Error;

	if(ftdi_controller_laser_write_string(wl_laser->controller, "V?\n") != FT_OK)
		goto Error;

	if(ftdi_controller_read_bytes_availiable_in_rx_queue(wl_laser->controller, retval, &amount_in_rx_queue) != FT_OK) {
		logger_log(UIMODULE_LOGGER(laser), LOGGER_ERROR, "Failed to read White Light Laser Version"); 
		goto Error;
	}

	strcpy(version_str, retval);

	#else

		if(initialise_comport(wl_laser->_com_port, baud) < 0) {
			wl_laser->_com_port = -1;
			GciCmtReleaseLock (wl_laser->_lock); 
			return WL_ERROR;
		}

		// Get the version number from the controller
		if (RS232_SendString(laser, "V?\n") == 0) {
				
			RS233_ReadString(laser, 30, retval);   
					
    		if (strlen(retval) == 0)
				goto Error;
		}
		else 
			goto Error;

		strcpy(version_str, retval);

	#endif

	if(wl_laser->type == FIANIUM_SC450_4) {
		sscanf(version_str, "Supercontinuum Controller Issue %s", wl_laser->_version);
	}
	else {	
		sscanf(version_str, "Issue %s", wl_laser->_version);
	}

	// New laser firmware version 5.09 seems to have changed response
	// for getting the dac value
	//if((fabs(wl_laser->_version - 5.09) < 0.001)) {
	//	wl_laser->get_dac_func_ptr = whitelight_laser_get_dac_version_509;
	//}

	#ifdef FTDI_NO_VIRTUAL_COMPORT
	if(ftdi_controller_set_deadman_timout(wl_laser->controller, 1000) != FT_OK)
		goto Error;
	#else
	SetComTime(wl_laser->_com_port, 1.0);
	#endif

	GciCmtReleaseLock (wl_laser->_lock); 

	return WL_SUCCESS;

Error:

	#ifdef FTDI_NO_VIRTUAL_COMPORT
	ftdi_controller_close(wl_laser->controller);
	#else
	close_comport(wl_laser->_com_port);
	#endif

NotOpenedError:

	GciCmtReleaseLock (wl_laser->_lock); 

	return WL_ERROR;
}


int whitelight_laser_display_panel(Laser* laser)
{ 
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;

	ui_module_display_main_panel(UIMODULE_CAST(laser)); 

	if(!whitelight_laser_is_oscillator_ready(laser)) {
		GCI_MessagePopup("White Light Laser", USER_TEMP_WARNING);
		disable_enable_panel_for_state(laser);
		return  WL_ERROR;  
	}

	whitelight_laser_start_display_timer(laser);
	
	return  WL_SUCCESS;
}

int whitelight_laser_hide_panel(Laser* laser)
{ 
	ui_module_hide_main_panel(UIMODULE_CAST(laser)); 

	whitelight_laser_stop_display_timer(laser);
	
	return  WL_SUCCESS;
}

int whitelight_laser_set_mode(Laser* laser, WHITE_LIGHT_LASER_MODE mode, int wait)
{    
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
	char retval[RS232_ARRAY_SIZE] = "";   
	int err;
	DWORD amount_read = 0;

	if(!wl_laser->_initialised)
		return WL_ERROR;

	if(!whitelight_laser_is_oscillator_ready(laser)) {
		GCI_MessagePopup("White Light Laser", USER_TEMP_WARNING);
		return  WL_ERROR;  
	}

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_laser_write_string(wl_laser->controller, "M=%d\n") != FT_OK)
	{
        return  WL_ERROR;
	}

	GciCmtGetLock (wl_laser->_lock);   

	if(ftdi_controller_laser_write_string(wl_laser->controller, "M?\n") != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}

	if(ftdi_controller_read_bytes_availiable_in_rx_queue(wl_laser->controller, retval, &amount_read) != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}

	if(wait) {
	
		while(strncmp(retval, KEY_ON_STRING, strlen(KEY_ON_STRING)) == 0) {

			GCI_MessagePopup("Error", "Unable to set laser mode. Please turn off the key on the laser.");
		    
			if(ftdi_controller_read_bytes_availiable_in_rx_queue(wl_laser->controller, retval, &amount_read) != FT_OK)
			{
				GciCmtReleaseLock (wl_laser->_lock);
				return  WL_ERROR;
			}
		}
	}
	else {

		if(strncmp(retval, KEY_ON_STRING, strlen(KEY_ON_STRING)) == 0) {

			GCI_MessagePopup("Error", "Unable to set laser mode. Please turn off the key on the laser.");
		    
			return  WL_ERROR;
		}
	}

	GciCmtReleaseLock (wl_laser->_lock);

	#else

	GciCmtGetLock (wl_laser->_lock);   

	err =  RS232_SendString(laser, "M=%d\n", (int) mode);

	if (err) {
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}

	err =  RS232_SendString(laser, "M?\n", (int) mode);

	if (err) {
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}

	RS233_ReadString(laser, 50, retval);   

	if(wait) {
	
		while(strncmp(retval, KEY_ON_STRING, strlen(KEY_ON_STRING)) == 0) {

			GCI_MessagePopup("Error", "Unable to set laser mode. Please turn off the key on the laser.");
		    
			RS233_ReadString(laser, 30, retval); 
		}
	}
	else {

		if(strncmp(retval, KEY_ON_STRING, strlen(KEY_ON_STRING)) == 0) {

			GCI_MessagePopup("Error", "Unable to set laser mode. Please turn off the key on the laser.");
		    
			return  WL_ERROR;
		}
	}

	GciCmtReleaseLock (wl_laser->_lock); 

	#endif

	//disable_enable_panel_for_state(laser);

    return  WL_SUCCESS;
}

int whitelight_laser_get_mode(Laser* laser, WHITE_LIGHT_LASER_MODE *mode)
{
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
	char retval[RS232_ARRAY_SIZE] = "";   
	int i, err;
	DWORD amount_read = 0;

	if(!wl_laser->_initialised)
		return WL_ERROR;

	GciCmtGetLock (wl_laser->_lock);   

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_laser_write_string(wl_laser->controller, "M?\n") != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}

	if(ftdi_controller_read_bytes_availiable_in_rx_queue(wl_laser->controller, retval, &amount_read) != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}

	#else

	err =  RS232_SendString(laser, "M?\n");

	if (err) {
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}
		
	RS233_ReadString(laser, 30, retval);       
	
	#endif

	GciCmtReleaseLock (wl_laser->_lock);

	for(i=0; i<3; i++) {
		if(strncmp(retval, Modes[i][0], 3) == 0) {
			*mode = i;
			SetCtrlVal(wl_laser->_extra_panel, WL_MORE_MODE, i); 
			break;
		}
	}

    return  WL_SUCCESS;
}

int whitelight_laser_turn_on(Laser* laser)
{ 
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
	char retval[RS232_ARRAY_SIZE] = "";   
	int err;
	
	if(!wl_laser->_initialised)
		return WL_ERROR;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_laser_write_string(wl_laser->controller, "Q=%d\n", wl_laser->_last_amp_value) != FT_OK)
		return  WL_ERROR;

	#else

	err =  RS232_SendString(laser, "Q=%d\n", wl_laser->_last_amp_value);

	if (err)
        return  WL_ERROR;
		
	#endif

    return  WL_SUCCESS;
}

int whitelight_laser_turn_off(Laser* laser)
{  
	return whitelight_laser_set_dac(laser, 0);
}

int whitelight_laser_set_dac(Laser* laser, int value)
{   
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
	char retval[RS232_ARRAY_SIZE] = "";   
	int err;
	
	if(!wl_laser->_initialised)
		return WL_ERROR;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_laser_write_string(wl_laser->controller, "Q=%d\n", value) != FT_OK)
	{
		return  WL_ERROR;
	}

	#else

	err =  RS232_SendString(laser, "Q=%d\n", value);

	if (err) {
        return  WL_ERROR;
	}
	
	#endif

    wl_laser->_last_amp_value = value;

    return  WL_SUCCESS;
}


int whitelight_laser_get_dac(Laser* laser, int *value)
{  
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;

	return wl_laser->get_dac_func_ptr(laser, value);
}

int whitelight_laser_get_dac_default(Laser* laser, int *value)
{  
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
	char retval[RS232_ARRAY_SIZE] = "", *dac_str;   
	int err;
	DWORD amount_read = 0;

	if(!wl_laser->_initialised)
		return WL_ERROR;

	*value = 0;

	GciCmtGetLock (wl_laser->_lock);   

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_laser_write_string(wl_laser->controller, "Q?\n") != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
		return  WL_ERROR;
	}

	if(ftdi_controller_read_bytes_availiable_in_rx_queue(wl_laser->controller, retval, &amount_read) != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}

	#else

	err =  RS232_SendString(laser, "Q?\n");

	if (err) {
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}
		
	RS233_ReadString(laser, 30, retval);       

	#endif

	GciCmtReleaseLock (wl_laser->_lock);

	str_change_char(retval, 10, '-');
	str_change_char(retval, 13, '-');

	dac_str = strtok(retval, "--");
	dac_str = strtok(NULL, "--");

	// Value has not been read correctly ?
	if(dac_str == NULL) {
		return 0;
	}

	sscanf(dac_str, "DAC Level = %d", value);

    return  WL_SUCCESS;
}

int whitelight_laser_get_dac_version_509(Laser* laser, int *value)
{  
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
	char retval[RS232_ARRAY_SIZE] = "";   
	int err;
	DWORD amount_read = 0;

	if(!wl_laser->_initialised)
		return WL_ERROR;

	*value = 0;

	GciCmtGetLock (wl_laser->_lock);   

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_laser_write_string(wl_laser->controller, "Q?\n") != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
		return  WL_ERROR;
	}

	if(ftdi_controller_read_bytes_availiable_in_rx_queue(wl_laser->controller, retval, &amount_read) != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}

	#else

	err =  RS232_SendString(laser, "Q?\n");

	if (err) {
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}
		
	RS233_ReadString(laser, 30, retval);       

	#endif

	GciCmtReleaseLock (wl_laser->_lock);

	sscanf(retval, "q%d,%d\r\n", value, value);

    return  WL_SUCCESS;
}

int whitelight_laser_is_oscillator_ready(Laser* laser)
{  
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
	char retval[RS232_ARRAY_SIZE] = "";   
	int err;
	DWORD amount_read = 0;

	GciCmtGetLock (wl_laser->_lock);   

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_laser_write_string(wl_laser->controller, "O?\n") != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
		return  WL_ERROR;
	}

	if(ftdi_controller_read_bytes_availiable_in_rx_queue(wl_laser->controller, retval, &amount_read) != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}

	#else

	err =  RS232_SendString(laser, "O?\n");

	if (err) {
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}

	RS233_ReadString(laser, 50, retval);       
	
	#endif

	GciCmtReleaseLock (wl_laser->_lock);

	if(strncmp(retval, OSCILLATOR_STRING, strlen(OSCILLATOR_STRING)) == 0)
		return  1;

	if(strncmp(retval, OSCILLATOR_ADMIN_STRING, strlen(OSCILLATOR_ADMIN_STRING)) == 0)
		return 1;

    return 0;
}

int whitelight_laser_display_alarms (Laser* laser)
{  
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
	char retval[RS232_ARRAY_SIZE] = "", *str;   
	int err;
	DWORD amount_read = 0;

	if(!wl_laser->_initialised)
		return WL_ERROR;

	GciCmtGetLock (wl_laser->_lock);   

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_laser_write_string(wl_laser->controller, "A?\n") != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
		return  WL_ERROR;
	}

	if(ftdi_controller_read_bytes_availiable_in_rx_queue(wl_laser->controller, retval, &amount_read) != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}

	#else

	err =  RS232_SendString(laser, "A?\n");

	if (err) {
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}

	RS233_ReadString(laser, 50, retval);       
	
	#endif

	GciCmtReleaseLock (wl_laser->_lock);

	str = strtok(retval, "C");

	GCI_MessagePopup("Fianium Laser Alarms", str);

    return  WL_SUCCESS;
}

int whitelight_laser_clear_alarms(Laser* laser)
{   
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
	int err;
	
	if(!wl_laser->_initialised)
		return WL_ERROR;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_laser_write_string(wl_laser->controller, "A=0\n") != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
		return  WL_ERROR;
	}

	#else

	err =  RS232_SendString(laser, "A=0\n");

	if (err) {
        return  WL_ERROR;
	}

	#endif

    return  WL_SUCCESS;
}

int whitelight_laser_get_back_reflection(Laser* laser, int *value)
{  
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
	char retval[RS232_ARRAY_SIZE] = "";   
	int err;
	DWORD amount_read = 0;

	if(!wl_laser->_initialised)
		return WL_ERROR;

	GciCmtGetLock (wl_laser->_lock);   

	*value = 0;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_laser_write_string(wl_laser->controller, "B?\n") != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
		return  WL_ERROR;
	}

	if(ftdi_controller_read_bytes_availiable_in_rx_queue(wl_laser->controller, retval, &amount_read) != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}

	#else

	err =  RS232_SendString(laser, "B?\n");

	if (err) {
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}

	RS233_ReadString(laser, 50, retval);       
	
	#endif

	GciCmtReleaseLock (wl_laser->_lock);

 	sscanf(retval, "Back reflection photodiode = %d", value);

    return  WL_SUCCESS;
}

int whitelight_laser_get_pre_amp_diode_val(Laser* laser, int *value)
{  
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
	char retval[RS232_ARRAY_SIZE] = "";   
	int err;
	DWORD amount_read = 0;

	if(!wl_laser->_initialised)
		return WL_ERROR;

	GciCmtGetLock (wl_laser->_lock); 

	*value = 0;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_laser_write_string(wl_laser->controller, "P?\n") != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
		return  WL_ERROR;
	}

	if(ftdi_controller_read_bytes_availiable_in_rx_queue(wl_laser->controller, retval, &amount_read) != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}

	#else

	err =  RS232_SendString(laser, "P?\n");

	if (err) {
		GciCmtReleaseLock (wl_laser->_lock);
        return  WL_ERROR;
	}

	RS233_ReadString(laser, 10, retval);       
	
	#endif

	GciCmtReleaseLock (wl_laser->_lock);

	sscanf(retval, "Preamplifier photodiode = %d", value);

    return  WL_SUCCESS;
}

int whitelight_laser_get_running_time_string(Laser* laser, char *value)
{  
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
	int err = 0;
	static int running = 0;
	char retval[RS232_ARRAY_SIZE] = "";
	char *time_str = NULL;
	DWORD amount_read = 0;

	if(running == 1)
		return WL_SUCCESS;

	running = 1;

	if(!wl_laser->_initialised) {
		running = 0;
		return WL_ERROR;
	}

	GciCmtGetLock (wl_laser->_lock);   

	value[0] = 0;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_laser_write_string(wl_laser->controller, "W?\n") != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
		running = 0;
		return  WL_ERROR;
	}

	if(ftdi_controller_read_bytes_availiable_in_rx_queue(wl_laser->controller, retval, &amount_read) != FT_OK)
	{
		GciCmtReleaseLock (wl_laser->_lock);
		running = 0;
        return  WL_ERROR;
	}

	#else

	err =  RS232_SendString(laser, "W?\n");

	if (err) {
		GciCmtReleaseLock (wl_laser->_lock);
		running = 0;
        return  WL_ERROR;
	}

	RS233_ReadString(laser, 50, retval);       

	#endif

	GciCmtReleaseLock (wl_laser->_lock);

	time_str = strtok(retval, "\r\n");

	if(strcmp(retval, "") != 0 && strcmp(retval, "Command error") != 0 && strcmp(retval, "Command>") != 0) {

		time_str = strtok(retval, "=");

		if(time_str != NULL) {
			time_str = strtok(NULL, "=") + 1; // First char is a space so we don't copy it
			strncpy(value, time_str, 50);
		}
	}
	
	running = 0;

    return  WL_SUCCESS;
}

int whitelight_laser_is_initialised(Laser* laser)
{
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;

	return wl_laser->_initialised;
}

int whitelight_laser_hardware_init(Laser* laser)
{
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
	char device[UIMODULE_NAME_LEN];

	ui_module_get_name(UIMODULE_CAST(laser), device);
	 
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	get_device_string_param_from_ini_file(device, "FTDI_SN", wl_laser->device_sn);  
 	
	#else

	get_device_int_param_from_ini_file   (device, "COM_Port", &(wl_laser->_com_port));  

	#endif

	if(initFianiumLaserRS232Port(laser) == WL_ERROR) {
		ui_module_disable_panel(wl_laser->_panel_id, 2, WL_PNL_CLOSE, WL_PNL_MORE);
//		ui_module_disable_panel(wl_laser->_extra_panel, 1, WL_PNL_CLOSE);  PUT THIS BACK BUT WITH EXCEPTION FOR A REINIT BUTTON
		return WL_ERROR;
	}

	wl_laser->_initialised = 1;
	wl_laser->_nErrors = 0;

	return  WL_SUCCESS;  
}

int whitelight_laser_init(Laser* laser)
{
	WHITE_LIGHT_LASER_MODE mode;

	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;

	whitelight_laser_get_mode(laser, &mode);

	SetAsyncTimerAttribute (wl_laser->_log_timer, ASYNC_ATTR_ENABLED,  1);

	ui_module_enable_panel(wl_laser->_panel_id, 0);
	ui_module_enable_panel(wl_laser->_extra_panel, 0);

	return  WL_SUCCESS;  
}

int whitelight_laser_destroy (Laser* laser)
{
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
	wl_laser->_destroying = 1;

    whitelight_laser_stop_display_timer(laser);
	SetAsyncTimerAttribute (wl_laser->_log_timer, ASYNC_ATTR_ENABLED,  0);
	SetAsyncTimerAttribute(wl_laser->_timer, ASYNC_ATTR_ENABLED, 0);

//	SetCtrlAttribute(wl_laser->_panel_id, wl_laser->_timer, ATTR_ENABLED, 0);

	// If the laser is not initialised it may have locked up
	// don't bother trying to close the comport as it will probably fail and hang the app.
	if(wl_laser->_initialised) {

		FlushInQ(wl_laser->_com_port);
		FlushOutQ(wl_laser->_com_port);

		while(GetOutQLen(wl_laser->_com_port)>0) {
			ProcessSystemEvents();
			Delay(0.001);
			continue;
		}

		#ifdef FTDI_NO_VIRTUAL_COMPORT
			ftdi_controller_close(wl_laser->controller);	
		#else
			close_comport(wl_laser->_com_port);
		#endif
		
	}

	fclose(wl_laser->_log_fp);

	CmtDiscardLock(wl_laser->_lock);

	wl_laser->_com_port = 0;

	DiscardAsyncTimer(wl_laser->_log_timer);
	DiscardAsyncTimer(wl_laser->_timer);

	return  WL_SUCCESS;  
}

void whitelight_laser_start_display_timer(Laser* laser)
{
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;

	if(wl_laser->_initialised) {
		SetAsyncTimerAttribute (wl_laser->_timer, ASYNC_ATTR_ENABLED,  1);
		//SetCtrlAttribute(wl_laser->_panel_id, wl_laser->_timer, ATTR_ENABLED, 1);
	}
}

void whitelight_laser_stop_display_timer(Laser* laser)
{
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;

	SetAsyncTimerAttribute (wl_laser->_timer, ASYNC_ATTR_ENABLED,  0);
	//SetCtrlAttribute(wl_laser->_panel_id, wl_laser->_timer, ATTR_ENABLED, 0);
}

static int CVICALLBACK OnLaserTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
		{
			Laser* laser = (Laser*) callbackData;
			WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
			int value;
			char val_str[RS232_ARRAY_SIZE] = "";
			
			if(wl_laser->_destroying)
				return 0;

			GciCmtGetLock (wl_laser->_lock);   

			if(whitelight_laser_get_dac(laser, &value) != WL_ERROR) {  	
				SetCtrlVal(wl_laser->_panel_id, WL_PNL_DAC_SLIDE, value);    
			}	
			
			val_str[0] = 0;

			if(whitelight_laser_get_running_time_string(laser, val_str) != WL_ERROR)
				SetCtrlVal(wl_laser->_extra_panel, WL_MORE_TXT_RUNTIME, val_str);
			
			if(whitelight_laser_get_back_reflection(laser, &value) != WL_ERROR) {
				sprintf(val_str, "%d", value);
				SetCtrlVal(wl_laser->_panel_id, WL_PNL_BR_TXT, val_str); 
			}

			whitelight_laser_get_pre_amp_diode_val(laser, &value);
			sprintf(val_str, "%d", value);
			SetCtrlVal(wl_laser->_extra_panel, WL_MORE_PREAMP_TXT, val_str);

			GciCmtReleaseLock (wl_laser->_lock);

            break;
		}
    }
    
    return 0;
}

static int CVICALLBACK OnLaserLogTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
    switch (event)
    {
        case EVENT_TIMER_TICK:
		{
			Laser* laser = (Laser*) callbackData;
			WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
			int dac_value, br_val, pre_amp_val;
			char running_time_str[RS232_ARRAY_SIZE] = "", time_str[200];
	
			if(wl_laser->_destroying)
				return 0;

			if(wl_laser->_log_fp == NULL)
				return 0;

			if(!wl_laser->_initialised)
				return 0;

			get_time_string(time_str);

			whitelight_laser_get_dac(laser, &dac_value);    
			whitelight_laser_get_back_reflection(laser, &br_val);
			whitelight_laser_get_pre_amp_diode_val(laser, &pre_amp_val);
			whitelight_laser_get_running_time_string(laser, running_time_str);
		
			fprintf(wl_laser->_log_fp, "%s,%d,%d,%d,%s\n", time_str, dac_value, br_val, pre_amp_val, running_time_str);
			fflush(wl_laser->_log_fp);

            break;
		}
    }
    
    return 0;
}

static int CVICALLBACK OnLaserMoreClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Laser* laser = (Laser*) callbackData; 
			WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;

			ui_module_display_panel(UIMODULE_CAST(laser), wl_laser->_extra_panel);
				
			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnLaserMoreCloseClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Laser* laser = (Laser*) callbackData; 
			WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;

			ui_module_hide_panel(UIMODULE_CAST(laser), wl_laser->_extra_panel);
				
			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnLaserCloseClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Laser* laser = (Laser*) callbackData; 
			WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;

			whitelight_laser_stop_display_timer(laser);

			whitelight_laser_hide_panel(laser);
				
			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnLaserModeChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int val = 0;
			WHITE_LIGHT_LASER_MODE mode;
			
			Laser* laser = (Laser*) callbackData; 
			WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;

			GetCtrlVal(panel, control, &val);
			
			whitelight_laser_set_mode(laser, (WHITE_LIGHT_LASER_MODE) val, 0);   
			
			whitelight_laser_get_mode(laser, &mode);      
				
			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnLaserCheckAlarms (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Laser* laser = (Laser*) callbackData; 
			WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;

			whitelight_laser_display_alarms(laser);      
				
			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnLaserClearAlarms (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Laser* laser = (Laser*) callbackData; 
			WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;

			whitelight_laser_clear_alarms(laser);      

			break;
		}
	}
	
	return 0;
}

static int CVICALLBACK OnLaserReInitialise (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			Laser* laser = (Laser*) callbackData; 
			WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;
			
			whitelight_laser_stop_display_timer(laser);
			
			#ifdef FTDI_NO_VIRTUAL_COMPORT
			ftdi_controller_close(wl_laser->controller);	
			#else
			CloseComPortWithTimeOut(wl_laser->_com_port, 5.0);
			#endif

			GCI_MessagePopup("Re-initialise Laser", "Please remove and replace the USB cable from the laser.");

			if (whitelight_laser_hardware_init(laser) == WL_SUCCESS) {
				GCI_MessagePopup("Re-initialise Laser", "Succeeded!");
				logger_log(UIMODULE_LOGGER(laser), LOGGER_INFORMATIONAL, "White Light Laser Re-Initialised"); 
				whitelight_laser_start_display_timer(laser);
			}
			else {
				GCI_MessagePopup("Re-initialise Laser", "Failed.");
			}

			break;
		}
	}
	
	return 0;
}


static int CVICALLBACK OnLaserDacChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
		{
			int val = 0;
			
			Laser* laser = (Laser*) callbackData; 
			WhiteLightLaser* wl_laser = (WhiteLightLaser*) laser;

			GetCtrlVal(panel, control, &val);
			
			whitelight_laser_set_dac(laser, val);

			break;
		}
	}
	
	return 0;
}


static void OnLaserPanelsClosedOrHidden (UIModule *module, void *data)
{
	Laser* laser = (Laser*) data; 

	whitelight_laser_stop_display_timer(laser);
}

static void OnLaserPanelsDisplayed (UIModule *module, int panel_id, void *data)
{
	Laser* laser = (Laser*) data; 

	whitelight_laser_start_display_timer(laser);
}

static int white_light_laser_get_info (HardwareDevice* device, char* info)
{
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) device;

	if(info != NULL) {
//		sprintf(info, "%s %s", LaserTypeStrings[wl_laser->type], wl_laser->_version);
		sprintf(info, "%s", wl_laser->_version);
	}

	return WL_SUCCESS;
}


static int whitelight_laser_get_power(Laser* laser, float *power)
{
	int val = 0;

	whitelight_laser_get_dac(laser, &val);

	*power = (float) val;

	return WL_SUCCESS;
}

Laser* whitelight_laser_new(FianiumLaserType type, const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	WhiteLightLaser* wl_laser = (WhiteLightLaser*) malloc(sizeof(WhiteLightLaser));  
	Laser *laser = (Laser*) wl_laser;

	char log_filename[GCI_MAX_PATHNAME_LEN] = "";
	int file_size = 0;

	memset(laser, 0, sizeof(WhiteLightLaser));
    
	laser_constructor(laser, name, description, data_dir);

	HARDWARE_VTABLE_PTR(HARDWARE_DEVICE_CAST(laser), hardware_getinfo) = white_light_laser_get_info; 
	LASER_VTABLE_PTR(laser, destroy) = whitelight_laser_destroy; 
	LASER_VTABLE_PTR(laser, get_power) = whitelight_laser_get_power; 

	#ifdef FTDI_NO_VIRTUAL_COMPORT
	wl_laser->controller = ftdi_controller_new();
	#endif

	wl_laser->type = type;

	if(type == FIANIUM_SC450_4)
		wl_laser->get_dac_func_ptr = whitelight_laser_get_dac_default;
	else
		wl_laser->get_dac_func_ptr = whitelight_laser_get_dac_version_509;

	GciCmtNewLock("WhiteLightLaser", 0, &(wl_laser->_lock));

	sprintf(log_filename, "%s\\%s", data_dir, "LaserLog.csv");

	if(!FileExists(log_filename, &file_size)){
		wl_laser->_log_fp = fopen(log_filename, "w");
		fprintf(wl_laser->_log_fp, "Date, DAC Value, Back reflection diode value, Preamplifier diode value, Total running time\n");
		fflush(wl_laser->_log_fp);
	}
	else {
		wl_laser->_log_fp = fopen(log_filename, "a");
	}

	wl_laser->_panel_id = ui_module_add_panel(UIMODULE_CAST(laser), "white_light_laser_ui.uir", WL_PNL, 1); 
	wl_laser->_extra_panel = ui_module_add_panel(UIMODULE_CAST(laser), "white_light_laser_ui.uir", WL_MORE, 0); 

	wl_laser->_timer = NewAsyncTimer (2.0, -1, 0, OnLaserTimerTick, laser);
	SetAsyncTimerName(wl_laser->_timer, "Laser");
	SetAsyncTimerAttribute (wl_laser->_timer, ASYNC_ATTR_ENABLED,  0);

	//wl_laser->_timer = NewCtrl (wl_laser->_panel_id, CTRL_TIMER, "", 0, 0);
	//InstallCtrlCallback(wl_laser->_panel_id, wl_laser->_timer, OnLaserTimerTick, laser);
	//SetCtrlAttribute(wl_laser->_panel_id, wl_laser->_timer, ATTR_INTERVAL, 2.0);
	//SetCtrlAttribute(wl_laser->_panel_id, wl_laser->_timer, ATTR_ENABLED, 0);

	wl_laser->_log_timer = NewAsyncTimer (300.0, -1, 0, OnLaserLogTimerTick, laser);
	SetAsyncTimerName(wl_laser->_log_timer, "LaserLog");
	SetAsyncTimerAttribute (wl_laser->_log_timer, ASYNC_ATTR_ENABLED,  0);

	InstallCtrlCallback (wl_laser->_panel_id, WL_PNL_CLOSE, OnLaserCloseClicked, laser); 
	InstallCtrlCallback (wl_laser->_panel_id, WL_PNL_DAC, OnLaserDacChanged, laser); 
	InstallCtrlCallback (wl_laser->_panel_id, WL_PNL_MORE, OnLaserMoreClicked, laser); 
	InstallCtrlCallback (wl_laser->_extra_panel, WL_MORE_CLOSE, OnLaserMoreCloseClicked, laser);

	InstallCtrlCallback (wl_laser->_extra_panel, WL_MORE_MODE, OnLaserModeChanged, laser); 

	InstallCtrlCallback (wl_laser->_extra_panel, WL_MORE_ALARMS, OnLaserCheckAlarms, laser); 
	InstallCtrlCallback (wl_laser->_extra_panel, WL_MORE_CLEAR_ALARMS, OnLaserClearAlarms, laser); 
	InstallCtrlCallback (wl_laser->_extra_panel, WL_MORE_REINIT, OnLaserReInitialise, laser); 

	ui_module_main_panel_hide_or_close_handler_connect (UIMODULE_CAST(laser), OnLaserPanelsClosedOrHidden, laser);
	ui_module_panel_show_handler_connect (UIMODULE_CAST(laser), OnLaserPanelsDisplayed, laser);

	wl_laser->_nErrors = 0;

	return laser;
}


Laser* whitelight_laser_sc450_4_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	return whitelight_laser_new(FIANIUM_SC450_4, name, description, handler, data_dir);
}

Laser* whitelight_laser_sc450_m_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	return whitelight_laser_new(FIANIUM_SC450_M, name, description, handler, data_dir);
}
