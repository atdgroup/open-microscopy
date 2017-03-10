#include "ATD_LedLightingUI_A.h"
#include "ATD_LedLighting_A.h" 
#include <rs232.h>

#include "string_utils.h"
#include "gci_utils.h"
#include "signals.h"

#include "GL_CVIRegistry.h"
#include "toolbox.h"

#include <ansi_c.h> 
#include "ATD_UsbInterface_A.h"
#include <utility.h>

////////////////////////////////////////////////////////////////////////////
//SE Microscope system. 
//IR LED Lighting
////////////////////////////////////////////////////////////////////////////

#define Round  RoundRealToNearestInteger 

static int atd_led_lighting_a_set_mode(Lamp *lamp, int value)
{
    ATD_LIGHTING_A *atd_a_led = (ATD_LIGHTING_A *) lamp;
	unsigned char val[6] = "";

	val[0]=atd_a_led->_i2c_chip_type;
   	val[1]=3;

    if(value == 1)
		SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LIGHT_PNL_LED, 1); 
    else
		SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LIGHT_PNL_LED, 0); 
   	
	atd_a_led->_led_mode = val[2] = value;

	// PIC expects 6 bytes so even though we fill in only 3 bytes will send 6.
	if(GCI_writeI2C_multiPort(atd_a_led->_com_port, 6, val, atd_a_led->_i2c_bus, "atd_led_lighting_a_set_mode"))
		return LAMP_ERROR; 

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp);
	
	return LAMP_SUCCESS;       
}

int atd_led_lighting_a_destroy (Lamp* lamp)
{
//	ATD_LIGHTING_A *atd_a_led = (ATD_LIGHTING_A *) lamp;

//	free(atd_a_led);

  	return LAMP_SUCCESS;
}

static char* construct_key(char *buffer, const char *section, const char* name)
{
	memset(buffer, 0, 1);
	
	sprintf(buffer, "%s:%s", section, name);
	
	return buffer;
}

// Loads from arbitary file - for changing microscope modes.
int adt_led_lighting_a_load_settings (Lamp* lamp, const char *filepath)
{
	dictionary* d = NULL;
	int file_size;
	char *data_filepath = NULL, buffer[100];
	ATD_LIGHTING_A *adt_led = (ATD_LIGHTING_A *) lamp;    
	
	if(!FileExists(filepath, &file_size))
		return LAMP_SUCCESS;	 	
	
	d = iniparser_load(filepath);  

	if(d != NULL) {
		
		adt_led->_led_mode = dictionary_getint(d, construct_key(UIMODULE_GET_NAME(lamp), buffer, "Mode"), 0); 
  		
        dictionary_del(d);
	}
	
	if(adt_led->_led_mode == 0)
		lamp_off (lamp);
	else
		lamp_on (lamp);


	return LAMP_SUCCESS;	      
}

// Saves to arbitary file - for changing microscope modes.
int adt_led_lighting_a_save_settings (Lamp* lamp, const char *filepath, const char *flags)
{
	FILE *fd;
	ATD_LIGHTING_A *adt_led = (ATD_LIGHTING_A *) lamp;
    dictionary *d = dictionary_new(20);
	
	fd = fopen(filepath, flags);
	
	dictionary_set(d, UIMODULE_GET_NAME(lamp), NULL);

	dictionary_setint(d, "Mode", adt_led->_led_mode);
 
	iniparser_save(d, fd); 
	
	fclose(fd);
	dictionary_del(d);
	
	return LAMP_SUCCESS;	      
}

int atd_led_lighting_a_off (Lamp* lamp)
{
    return atd_led_lighting_a_set_mode (lamp, 0);
}

int atd_led_lighting_a_on (Lamp* lamp)
{
	return atd_led_lighting_a_set_mode (lamp, 1);
}

int atd_led_lighting_a_off_on_status (Lamp* lamp, LampStatus *status)
{
	ATD_LIGHTING_A *atd_a_led = (ATD_LIGHTING_A *) lamp;    
	
	if(atd_a_led->_led_mode != 0)
		*status = LAMP_ON;
	else
		*status = LAMP_OFF;    
		
	return LAMP_SUCCESS;         
}

static int atd_led_lighting_a_log_led_mode (Lamp *lamp, int value)
{
	char state_text[100] = "";
	ATD_LIGHTING_A *atd_a_led = (ATD_LIGHTING_A *) lamp;   
	
    switch (value)
	{
		case 0:
			strcpy(state_text, "Off");
			break;
		case 1:
			strcpy(state_text, "On");
			break;
	}
	
	logger_log(UIMODULE_LOGGER(atd_a_led), LOGGER_INFORMATIONAL, "%s changed: power %s",
		UIMODULE_GET_DESCRIPTION(atd_a_led), state_text);
	
    return LAMP_SUCCESS;
}


static int atd_led_lighting_a_hardware_initialise (Lamp* lamp)
{
	int status = UIMODULE_ERROR_NONE; 
	ATD_LIGHTING_A *atd_a_led = (ATD_LIGHTING_A *) lamp;     
	
	get_device_param_from_ini_file(UIMODULE_GET_NAME(atd_a_led), "i2c_Bus", &(atd_a_led->_i2c_bus));
	get_device_param_from_ini_file(UIMODULE_GET_NAME(atd_a_led), "i2c_ChipAddress", &(atd_a_led->_i2c_chip_address));  
	get_device_param_from_ini_file(UIMODULE_GET_NAME(atd_a_led), "AD5241_i2c_ChipAddress", &(atd_a_led->_i2c_ADaddress)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(atd_a_led), "i2c_ChipType", &(atd_a_led->_i2c_chip_type));  
	get_device_param_from_ini_file(UIMODULE_GET_NAME(atd_a_led), "COM_Port", &(atd_a_led->_com_port));  
	
	initialise_comport(atd_a_led->_com_port, 9600);

	return LAMP_SUCCESS;
}

int  atd_led_lighting_a_initialise(Lamp* lamp)
{
	ATD_LIGHTING_A *atd_a_led = (ATD_LIGHTING_A *) lamp;  

	ui_module_add_panel(UIMODULE_CAST(atd_a_led), "ATD_LedLightingUI_A.uir", LIGHT_PNL, 1);

    //Load main panel and install callbacks such that atd_a_led is passed in the callback data
  	
    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LIGHT_PNL_LEDSTATE, OnLedLightingStateChanged, atd_a_led) < 0)
		return LAMP_ERROR;
  	
    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LIGHT_PNL_QUIT, OnLedLightingQuit, atd_a_led) < 0)
		return LAMP_ERROR;
	
	return LAMP_SUCCESS;
}

Lamp* atd_led_lighting_a_new(char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	ATD_LIGHTING_A* atd_a_led = (ATD_LIGHTING_A*) malloc(sizeof(ATD_LIGHTING_A));
	Lamp *lamp = (Lamp*) atd_a_led;
	
	memset(atd_a_led, 0, sizeof(ATD_LIGHTING_A));
	
	lamp_constructor(lamp, name, description, handler, data_dir);
	
	LAMP_VTABLE_PTR(lamp, init) = atd_led_lighting_a_initialise;    
	LAMP_VTABLE_PTR(lamp, hardware_init) = atd_led_lighting_a_hardware_initialise;   
	LAMP_VTABLE_PTR(lamp, destroy) = atd_led_lighting_a_destroy;   
	LAMP_VTABLE_PTR(lamp, lamp_off) = atd_led_lighting_a_off;   
	LAMP_VTABLE_PTR(lamp, lamp_on) = atd_led_lighting_a_on;   
	LAMP_VTABLE_PTR(lamp, lamp_off_on_status) = atd_led_lighting_a_off_on_status; 
	LAMP_VTABLE_PTR(lamp, save_settings) = adt_led_lighting_a_save_settings;   
	LAMP_VTABLE_PTR(lamp, load_settings) = adt_led_lighting_a_load_settings;   

	return lamp;
}


int atd_led_lighting_a_display_settings_ui(Lamp* lamp)
{
	ATD_LIGHTING_A *atd_a_led = (ATD_LIGHTING_A *) lamp;  
	
	ui_module_display_panel(UIMODULE_CAST(atd_a_led), UIMODULE_MAIN_PANEL_ID(lamp));  
	
	return LAMP_SUCCESS;
}


int atd_led_lighting_a_hide_settings_ui(Lamp* lamp)
{
	ATD_LIGHTING_A *atd_a_led = (ATD_LIGHTING_A *) lamp;  
	
	ui_module_hide_panel(UIMODULE_CAST(atd_a_led), UIMODULE_MAIN_PANEL_ID(lamp));   

	return LAMP_SUCCESS;
}
