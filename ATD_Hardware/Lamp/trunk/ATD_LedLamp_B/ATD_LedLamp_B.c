#include "ATD_LedLampUI_B.h"
#include "ATD_LedLamp_B.h" 
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

static int send_led_settings(ATD_LAMP_B *adt_b_led, int intensity, int min, int max, int on_off_mode)
{
	byte vals[10] = "";   
	int ledpower=0;	// Always enabled in my code !
	
	int range = MIN(255, max) - MAX(0, min);
	int setting = ((range*intensity)/100)+min;
	
	vals[0]=adt_b_led->_i2c_chip_type | (adt_b_led->_i2c_chip_address <<1);
   	vals[1]=(ledpower<<4 | on_off_mode<<3);	//instruction byte  including logic pins  
   	vals[2]=setting; 		//data byte
   	
   	if(GCI_writeI2C_multiPort(adt_b_led->_com_port, 3, vals, adt_b_led->_i2c_bus, "send_led_settings") < 0)
		return LAMP_ERROR;
	
	return LAMP_SUCCESS;  	
}

static char* construct_key(Lamp* lamp, char *buffer, const char* name)
{
	memset(buffer, 0, 1);
	
	sprintf(buffer, "%s:%s", UIMODULE_GET_NAME(lamp), name);
	
	return buffer;
}

int adt_b_led_lamp_destroy (Lamp* lamp)
{
    if(lamp == NULL)
		return LAMP_SUCCESS;   	
		
  	return LAMP_SUCCESS;
}

// Loads from arbitary file - for changing microscope modes.
int adt_b_led_lamp_load_settings (Lamp* lamp, const char *filepath)
{
	dictionary* d = NULL;
	int file_size;
	char *data_filepath = NULL, buffer[100];
	ATD_LAMP_B *adt_b_led = (ATD_LAMP_B *) lamp;    
	
	if(!FileExists(filepath, &file_size))
		return LAMP_SUCCESS;	 	
	
	d = iniparser_load(filepath);  

	if(d != NULL) {
		
		adt_b_led->_led_mode = dictionary_getint(d, construct_key(lamp, buffer, "Mode"), ATD_B_LED_OFF); 
		adt_b_led->_intensity = dictionary_getdouble(d, construct_key(lamp, buffer, "Intensity"), 0.0);
  		
        dictionary_del(d);
	}
	
	lamp_set_intensity (lamp, adt_b_led->_intensity);

	if(adt_b_led->_led_mode == ATD_B_LED_OFF)
		lamp_off (lamp);
	else
		lamp_on (lamp);


	return LAMP_SUCCESS;	      
}

// Saves to arbitary file - for changing microscope modes.
int adt_b_led_lamp_save_settings (Lamp* lamp, const char *filepath, const char *flags)
{
	FILE *fd;
	ATD_LAMP_B *adt_b_led = (ATD_LAMP_B *) lamp;
    dictionary *d = dictionary_new(20);
	
	fd = fopen(filepath, flags);
	
	dictionary_set(d, UIMODULE_GET_NAME(lamp), NULL);

	dictionary_setint(d, "Mode", adt_b_led->_led_mode);
    dictionary_setdouble(d, "Intensity", adt_b_led->_intensity);
 
	iniparser_save(d, fd); 
	
	fclose(fd);
	dictionary_del(d);
	
	return LAMP_SUCCESS;	      
}

int adt_b_led_lamp_off (Lamp* lamp)
{
	ATD_LAMP_B *adt_b_led = (ATD_LAMP_B *) lamp; 

	adt_b_led->_led_mode = ATD_B_LED_OFF;

	if ( send_led_settings(adt_b_led, (int) adt_b_led->_intensity, (int) adt_b_led->_min_intensity, 
		(int) adt_b_led->_max_intensity, ATD_B_LED_OFF) == LAMP_ERROR) return LAMP_ERROR;

	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_LEDSTATE, ATD_B_LED_OFF); 
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp); 

	return LAMP_SUCCESS;
}

int adt_b_led_lamp_on (Lamp* lamp)
{
	ATD_LAMP_B *adt_b_led = (ATD_LAMP_B *) lamp; 

	adt_b_led->_led_mode = ATD_B_LED_ON;

	if ( send_led_settings(adt_b_led, (int) adt_b_led->_intensity, (int)adt_b_led->_min_intensity, 
		(int)adt_b_led->_max_intensity, ATD_B_LED_ON) == LAMP_ERROR) return LAMP_ERROR;

	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_LEDSTATE, ATD_B_LED_ON); 
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp); 

	return LAMP_SUCCESS;
}

int adt_b_led_lamp_off_on_status (Lamp* lamp, LampStatus *status)
{
	ATD_LAMP_B *adt_b_led = (ATD_LAMP_B *) lamp;    
	
	if(adt_b_led->_led_mode != ATD_B_LED_OFF)
		*status = LAMP_ON;
	else
		*status = LAMP_OFF;    
		
	return LAMP_SUCCESS;         
}

int adt_b_led_lamp_set_intensity (Lamp* lamp, double intensity)
{
	ATD_LAMP_B *adt_b_led = (ATD_LAMP_B *) lamp;        
	
	adt_b_led->_intensity = intensity;   
	
    send_led_settings(adt_b_led, (int)adt_b_led->_intensity, (int)adt_b_led->_min_intensity, 
		(int)adt_b_led->_max_intensity, adt_b_led->_led_mode);

	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_INTENSITY, intensity); 
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp); 
	
	return LAMP_SUCCESS;         
}

int adt_b_led_lamp_get_intensity (Lamp* lamp, double *intensity)
{
	ATD_LAMP_B *adt_b_led = (ATD_LAMP_B *) lamp; 
	
    *intensity = adt_b_led->_intensity;
	
	return LAMP_SUCCESS;         
}

int adt_b_led_lamp_set_intensity_range (Lamp *lamp, double min, double max, double increment)
{
	ATD_LAMP_B *adt_b_led = (ATD_LAMP_B *) lamp;  
	
	adt_b_led->_min_intensity = MAX(0, min);
  	adt_b_led->_max_intensity = MIN(255, max);
  	adt_b_led->_intensity_increment = 1;

	return send_led_settings(adt_b_led, (int)adt_b_led->_intensity, (int) adt_b_led->_min_intensity, 
		(int) adt_b_led->_max_intensity, adt_b_led->_led_mode);       
}

static int CVICALLBACK OnLampIntensityChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
        {
            Lamp *lamp = (Lamp *) callbackData;
            double intensity;
            
            GetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_INTENSITY, &intensity);
			lamp_set_intensity (lamp, intensity);
            
            break;
		}
	}
	return 0;
}

static int CVICALLBACK OnLampOnOffChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;
            
            int mode;
    
			GetCtrlVal(panel, control, &mode);
	 
			if(mode == 0)
				lamp_off (lamp);
			else
				lamp_on (lamp);

            break;
		}
	}
    
	return 0;
}

static int CVICALLBACK OnLampCloseClicked (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;
			lamp_hide_main_ui(lamp);
            
            break;
		}
	}
    
	return 0;
}


int  adt_b_led_lamp_initialise(Lamp* lamp)
{
	ATD_LAMP_B *adt_b_led = (ATD_LAMP_B *) lamp;   
	lamp_set_main_panel (lamp, ui_module_add_panel(UIMODULE_CAST(adt_b_led), "ATD_LedLampUI_B.uir", LEDS_PNL, 1));

    //Load main panel and install callbacks such that adt_b_led is passed in the callback data	
    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_INTENSITY, OnLampIntensityChanged, adt_b_led) < 0)
		return LAMP_ERROR;
  	
    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_LEDSTATE, OnLampOnOffChanged, adt_b_led) < 0)
		return LAMP_ERROR;
  	
    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_QUIT, OnLampCloseClicked, adt_b_led) < 0)
		return LAMP_ERROR;

	return LAMP_SUCCESS;
}


static int adt_b_led_hardware_init (Lamp* lamp)
{
	int status = UIMODULE_ERROR_NONE; 
	ATD_LAMP_B *adt_b_led = (ATD_LAMP_B *) lamp;     
	
	get_device_param_from_ini_file(UIMODULE_GET_NAME(adt_b_led), "i2c_Bus", &(adt_b_led->_i2c_bus));
	get_device_param_from_ini_file(UIMODULE_GET_NAME(adt_b_led), "i2c_ChipAddress", &(adt_b_led->_i2c_chip_address));  
	get_device_param_from_ini_file(UIMODULE_GET_NAME(adt_b_led), "i2c_ChipType", &(adt_b_led->_i2c_chip_type));  
	get_device_param_from_ini_file(UIMODULE_GET_NAME(adt_b_led), "COM_Port", &(adt_b_led->_com_port));  
	get_device_double_param_from_ini_file(UIMODULE_GET_NAME(adt_b_led), "Min_Intensity", &(adt_b_led->_min_intensity)); 
	get_device_double_param_from_ini_file(UIMODULE_GET_NAME(adt_b_led), "Max_Intensity", &(adt_b_led->_max_intensity));
	
	initialise_comport(adt_b_led->_com_port, 9600);
	
	return LAMP_SUCCESS;
}

Lamp* adt_b_led_lamp_new(char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	ATD_LAMP_B* adt_b_led = (ATD_LAMP_B*) malloc(sizeof(ATD_LAMP_B));
	Lamp *lamp = (Lamp*) adt_b_led;
	
	memset(adt_b_led, 0, sizeof(ATD_LAMP_B));
	
	lamp_constructor(lamp, name, description, handler, data_dir);
	
	LAMP_VTABLE_PTR(lamp, init) = adt_b_led_lamp_initialise;   
	LAMP_VTABLE_PTR(lamp, hardware_init) = adt_b_led_hardware_init; 
	LAMP_VTABLE_PTR(lamp, destroy) = adt_b_led_lamp_destroy;   
	LAMP_VTABLE_PTR(lamp, lamp_off) = adt_b_led_lamp_off;   
	LAMP_VTABLE_PTR(lamp, lamp_on) = adt_b_led_lamp_on;   
	LAMP_VTABLE_PTR(lamp, lamp_off_on_status) = adt_b_led_lamp_off_on_status;
	LAMP_VTABLE_PTR(lamp, lamp_set_intensity) = adt_b_led_lamp_set_intensity;
	LAMP_VTABLE_PTR(lamp, lamp_get_intensity) = adt_b_led_lamp_get_intensity; 
	LAMP_VTABLE_PTR(lamp, lamp_set_intensity_range) = adt_b_led_lamp_set_intensity_range;
	LAMP_VTABLE_PTR(lamp, save_settings) = adt_b_led_lamp_save_settings;   
	LAMP_VTABLE_PTR(lamp, load_settings) = adt_b_led_lamp_load_settings;   

	return lamp;
}