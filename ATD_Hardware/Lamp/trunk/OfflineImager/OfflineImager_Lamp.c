#include "OfflineImager_LampUI.h"
#include "OfflineImager_Lamp.h" 
#include <rs232.h>

#include "string_utils.h"
#include "gci_utils.h"
#include "signals.h"

#include "GL_CVIRegistry.h"
#include "toolbox.h"

#include <ansi_c.h> 
#include "ATD_UsbInterface_A.h"
#include <utility.h>

#define Round  RoundRealToNearestInteger 

#define MAX521		 0x50

static int GCI_Out_Byte_DAC_MAX521_multiPort (int port, int bus, byte address, byte DAC, byte patt )
{
	byte val[3]; 									  
    int err;
    
    val[0] = MAX521 | (address <<1);
    val[1] = DAC;
    val[2] = patt;
    
	err = GCI_writeI2C_multiPort(port, 3, val, bus, "GCI_Out_Byte_DAC_MAX521_multiPort");

	return err;
}

static char* construct_key(Lamp* lamp, char *buffer, const char* name)
{
	memset(buffer, 0, 1);
	
	sprintf(buffer, "%s:%s", UIMODULE_GET_NAME(lamp), name);
	
	return buffer;
}


// Save settings specific to  offline_imager_lamp
static int adt_ledlamp_a_save_settings(OfflineImager_Lamp *offline_imager_lamp)
{
	dictionary* d = NULL;
	FILE *fd;
	char data_filepath[GCI_MAX_PATHNAME_LEN];
	Lamp *lamp = (Lamp *) offline_imager_lamp;    
	
	sprintf(data_filepath, "%s\\%s_%s", UIMODULE_GET_DATA_DIR(lamp), UIMODULE_GET_NAME(lamp), DEFAULT_LAMP_FILENAME_SUFFIX);

	fd = fopen(data_filepath, "w");
	
	dictionary_set(d, UIMODULE_GET_NAME(lamp), NULL);

	dictionary_setint(d, "Mode", offline_imager_lamp->_led_mode);
    dictionary_setdouble(d, "Min Intensity", offline_imager_lamp->_min_intensity);
    dictionary_setdouble(d, "Max Intensity", offline_imager_lamp->_max_intensity);
    dictionary_setdouble(d, "Intensity Increment", offline_imager_lamp->_intensity_increment);
    dictionary_setdouble(d, "Intensity", offline_imager_lamp->_intensity);
 
	iniparser_save(d, fd); 
	
	fclose(fd);
	dictionary_del(d);
	
	return LAMP_SUCCESS;
}


// Saves to arbitary file - for changing microscope modes.
int offline_imager_lamp_load_settings (Lamp* lamp, const char *filepath)
{
	dictionary* d = NULL;
	int file_size;
	char *data_filepath = NULL, buffer[100];
	OfflineImager_Lamp *offline_imager_lamp = (OfflineImager_Lamp *) lamp;    
	
	if(!FileExists(filepath, &file_size))
		return LAMP_SUCCESS;	 	
	
	d = iniparser_load(filepath);  

	if(d != NULL) {
		
		offline_imager_lamp->_led_mode = dictionary_getint(d, construct_key(lamp, buffer, "Mode"), OFFLINE_IMAGER_LAMP_OFF); 
		offline_imager_lamp->_intensity = dictionary_getdouble(d, construct_key(lamp, buffer, "Intensity"), 0.0);
  		
        dictionary_del(d);

		if(offline_imager_lamp->_led_mode == OFFLINE_IMAGER_LAMP_ON)
			offline_imager_lamp_enable(lamp);
		else
			offline_imager_lamp_disable(lamp);

		offline_imager_lamp_set_intensity(lamp, offline_imager_lamp->_intensity);

	}
	
	return LAMP_SUCCESS;	      
}


int offline_imager_lamp_save_settings (Lamp* lamp, const char *filepath, const char *flags)
{
	FILE *fd;
	OfflineImager_Lamp *offline_imager_lamp = (OfflineImager_Lamp *) lamp;
    dictionary *d = dictionary_new(20);
	
	fd = fopen(filepath, flags);
	
	dictionary_set(d, UIMODULE_GET_NAME(lamp), NULL);

	dictionary_setint(d, "Mode", offline_imager_lamp->_led_mode);
    dictionary_setdouble(d, "Intensity", offline_imager_lamp->_intensity);
 
	iniparser_save(d, fd); 
	
	fclose(fd);
	dictionary_del(d);
	
	return LAMP_SUCCESS;	      
}


int offline_imager_lamp_destroy (Lamp* lamp)
{
    if(lamp == NULL)
		return LAMP_SUCCESS;   	
		
	adt_ledlamp_a_save_settings((OfflineImager_Lamp *)lamp);

  	return LAMP_SUCCESS;
}

static int GCI_In_Bit_multiPort(int port, int bus, byte chip_type, byte address, int bit, int *data)
{
	int i_val;
	byte val[3];
	
	GetI2CPortLock(port, "GCI_In_Bit_multiPort");

 	val[0] = chip_type | (address << 1) | 0x01;

    if (GCI_readI2C_multiPort(port, 2, val, bus, "GCI_In_Bit_multiPort")) {
		ReleaseI2CPortLock(port, "GCI_In_Bit_multiPort"); 
    	return -1;	//problem
    }

 	i_val = val[0] & ((int)pow (2.0, (double)bit));
 	i_val = (i_val >> bit);
    *data = i_val;
    
	ReleaseI2CPortLock(port, "GCI_In_Bit_multiPort"); 

    return 0;
}

static int IsLampEnabled(Lamp* lamp)
{
	OfflineImager_Lamp *offline_imager_lamp = (OfflineImager_Lamp *) lamp;

 //	if (GCI_In_Bit_multiPort(offline_imager_lamp->_com_port, offline_imager_lamp->_i2c_bus, offline_imager_lamp->_i2c_chip_address, 1, 5, &enabled)) {
//		ui_module_send_error (UIMODULE_CAST(lamp), UIMODULE_GET_DESCRIPTION(lamp),
//			"The lamp PC control does not seem to be enabled. "
//			"Is the lamp module control switch set to the PC position?");	
//		return LAMP_ERROR;
//	}

	return LAMP_SUCCESS;
}

int offline_imager_lamp_disable (Lamp* lamp)
{
    OfflineImager_Lamp *offline_imager_lamp = (OfflineImager_Lamp *) lamp;

	if(IsLampEnabled(lamp) == LAMP_ERROR)
		return LAMP_ERROR;

	//send 255 to DACs 3 and 4 of address 0
	GCI_Out_Byte_DAC_MAX521_multiPort (offline_imager_lamp->_com_port, offline_imager_lamp->_i2c_bus, 0, 3, 0);
	
	GCI_Out_Byte_DAC_MAX521_multiPort (offline_imager_lamp->_com_port, offline_imager_lamp->_i2c_bus, 0, 4, 0);

	offline_imager_lamp->_led_mode = OFFLINE_IMAGER_LAMP_OFF;
	
	SetCtrlAttribute(lamp->_main_ui_panel, OLLAMP_PNL_INTENSITY, ATTR_DIMMED, 1);
	SetCtrlVal(lamp->_main_ui_panel, OLLAMP_PNL_STATE, 1);

	Delay(1.0); //to allow new intensity level to settle

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp); 

    return LAMP_SUCCESS;
}

int offline_imager_lamp_enable (Lamp* lamp)
{
    OfflineImager_Lamp *offline_imager_lamp = (OfflineImager_Lamp *) lamp;

	if(IsLampEnabled(lamp) == LAMP_ERROR)
		return LAMP_ERROR;

	//send 255 to DAC 3 of address 0
	GCI_Out_Byte_DAC_MAX521_multiPort (offline_imager_lamp->_com_port, offline_imager_lamp->_i2c_bus, 0, 3, 255);
	
	//scope override
	GCI_Out_Byte_DAC_MAX521_multiPort (offline_imager_lamp->_com_port, offline_imager_lamp->_i2c_bus, 0, 4, 255);

	offline_imager_lamp->_led_mode = OFFLINE_IMAGER_LAMP_ON;
	
	SetCtrlAttribute(lamp->_main_ui_panel, OLLAMP_PNL_INTENSITY, ATTR_DIMMED, 0);
	SetCtrlVal(lamp->_main_ui_panel, OLLAMP_PNL_STATE, 2);

	Delay(1.0); //to allow new intensity level to settle

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp); 
	
    return LAMP_SUCCESS;
}


int offline_imager_lamp_off (Lamp* lamp)
{
    return offline_imager_lamp_disable (lamp);
}

int offline_imager_lamp_on (Lamp* lamp)
{
	return offline_imager_lamp_enable (lamp);
}

int offline_imager_lamp_off_on_status (Lamp* lamp, LampStatus *status)
{
	OfflineImager_Lamp *offline_imager_lamp = (OfflineImager_Lamp *) lamp;    
	
	if(offline_imager_lamp->_led_mode != OFFLINE_IMAGER_LAMP_OFF)
		*status = LAMP_ON;
	else
		*status = LAMP_OFF;    
	
	return LAMP_SUCCESS;         
}

int offline_imager_lamp_set_intensity (Lamp* lamp, double intensity)
{
	OfflineImager_Lamp *offline_imager_lamp = (OfflineImager_Lamp *) lamp;        
	
	offline_imager_lamp->_intensity = intensity;   
	
	GCI_Out_Byte_DAC_MAX521_multiPort (offline_imager_lamp->_com_port, offline_imager_lamp->_i2c_bus, 0, 7, (int) intensity);

	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), OLLAMP_PNL_INTENSITY, intensity); 
	
	Delay(1.0); //to allow new intensity level to settle

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp); 
	
	return LAMP_SUCCESS;         
}

int offline_imager_lamp_get_intensity (Lamp* lamp, double *intensity)
{
	OfflineImager_Lamp *offline_imager_lamp = (OfflineImager_Lamp *) lamp; 
	
    *intensity = offline_imager_lamp->_intensity;
	
	return LAMP_SUCCESS;         
}

int offline_imager_lamp_set_intensity_range (Lamp *lamp, double min, double max, double increment)
{
	OfflineImager_Lamp *offline_imager_lamp = (OfflineImager_Lamp *) lamp;  
	
	offline_imager_lamp->_min_intensity = min;
  	offline_imager_lamp->_max_intensity = max;
  	offline_imager_lamp->_intensity_increment = increment;

	return LAMP_SUCCESS;         
}

int offline_imager_lamp_hardware_initialise	(Lamp* lamp)
{
	OfflineImager_Lamp *offline_imager_lamp = (OfflineImager_Lamp *) lamp;     

	get_device_param_from_ini_file(UIMODULE_GET_NAME(offline_imager_lamp), "i2c_Bus", &(offline_imager_lamp->_i2c_bus));
	get_device_param_from_ini_file(UIMODULE_GET_NAME(offline_imager_lamp), "i2c_ChipAddress", &(offline_imager_lamp->_i2c_chip_address));  
	get_device_param_from_ini_file(UIMODULE_GET_NAME(offline_imager_lamp), "i2c_ChipType", &(offline_imager_lamp->_i2c_chip_type));  
	get_device_param_from_ini_file(UIMODULE_GET_NAME(offline_imager_lamp), "COM_Port", &(offline_imager_lamp->_com_port));  
	
	initialise_comport(offline_imager_lamp->_com_port, 9600);

	return LAMP_SUCCESS;
}

int  offline_imager_lamp_initialise(Lamp* lamp)
{
	int status = UIMODULE_ERROR_NONE; 
	OfflineImager_Lamp *offline_imager_lamp = (OfflineImager_Lamp *) lamp;     

	lamp_set_main_panel (lamp, ui_module_add_panel(UIMODULE_CAST(offline_imager_lamp), "OfflineImager_LampUI.uir", OLLAMP_PNL, 1));

    //Load main panel and install callbacks such that offline_imager_lamp is passed in the callback data
    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), OLLAMP_PNL_INTENSITY, offline_imager_lamp_intensity, offline_imager_lamp) < 0)
		return LAMP_ERROR;
  	
    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), OLLAMP_PNL_STATE, offline_imager_lamp_ledmode, offline_imager_lamp) < 0)
		return LAMP_ERROR;
  	
    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), OLLAMP_PNL_QUIT, offline_imager_lamp_quit, offline_imager_lamp) < 0)
		return LAMP_ERROR;
  	
	offline_imager_lamp_off (lamp);
	offline_imager_lamp_set_intensity_range (lamp, 0.0, 255.0, 1.0);
	
	return LAMP_SUCCESS;
}

Lamp* offline_imager_lamp_new(char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	OfflineImager_Lamp* offline_imager_lamp = (OfflineImager_Lamp*) malloc(sizeof(OfflineImager_Lamp));
	Lamp *lamp = (Lamp*) offline_imager_lamp;
	
	memset(offline_imager_lamp, 0, sizeof(OfflineImager_Lamp));
	
	lamp_constructor(lamp, name, description, handler, data_dir);
	
	LAMP_VTABLE_PTR(lamp, init) = offline_imager_lamp_initialise;  
	LAMP_VTABLE_PTR(lamp, hardware_init) = offline_imager_lamp_hardware_initialise;   
	LAMP_VTABLE_PTR(lamp, destroy) = offline_imager_lamp_destroy;   
	LAMP_VTABLE_PTR(lamp, lamp_off) = offline_imager_lamp_off;   
	LAMP_VTABLE_PTR(lamp, lamp_on) = offline_imager_lamp_on;   
	LAMP_VTABLE_PTR(lamp, lamp_off_on_status) = offline_imager_lamp_off_on_status;
	LAMP_VTABLE_PTR(lamp, lamp_set_intensity) = offline_imager_lamp_set_intensity;
	LAMP_VTABLE_PTR(lamp, lamp_get_intensity) = offline_imager_lamp_get_intensity; 
	LAMP_VTABLE_PTR(lamp, lamp_set_intensity_range) = offline_imager_lamp_set_intensity_range;
	LAMP_VTABLE_PTR(lamp, save_settings) = offline_imager_lamp_save_settings;   
	LAMP_VTABLE_PTR(lamp, load_settings) = offline_imager_lamp_load_settings;   

	return lamp;
}


int offline_imager_lamp_display_settings_ui(Lamp* lamp)
{
	OfflineImager_Lamp *offline_imager_lamp = (OfflineImager_Lamp *) lamp;  
	
	ui_module_display_panel(UIMODULE_CAST(offline_imager_lamp), UIMODULE_MAIN_PANEL_ID(lamp));  
	
	return LAMP_SUCCESS;
}


int offline_imager_lamp_hide_settings_ui(Lamp* lamp)
{
	OfflineImager_Lamp *offline_imager_lamp = (OfflineImager_Lamp *) lamp;  
	
	ui_module_hide_panel(UIMODULE_CAST(offline_imager_lamp), UIMODULE_MAIN_PANEL_ID(lamp));   

	return LAMP_SUCCESS;
}
