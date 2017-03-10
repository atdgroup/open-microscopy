#include "ATD_ZDrive_B.h"
#include "ATD_ZDriveAutoFocus_B.h"
#include "ZDriveUI.h" 
#include "gci_utils.h"
#include "ATD_UsbInterface_A.h" 

#include <userint.h>
#include <formatio.h>
#include <cviauto.h>  
#include "asynctmr.h"
#include <rs232.h>
#include <utility.h>

static int scale_factor_array[4] = {1, 8, 4, 2}; 

static const int PCF8574A = 0x70;

// Returns the address of the currently selected dac
int dac_read_address(ATD_ZDRIVE_B* atd_zdrive_b)
{
	return atd_zdrive_b->_dacs[atd_zdrive_b->_current_dac]._read_address;
}

int dac_write_address(ATD_ZDRIVE_B* atd_zdrive_b)
{
	return atd_zdrive_b->_dacs[atd_zdrive_b->_current_dac]._write_address;
}

void atd_zdrive_b_initialise(ATD_ZDRIVE_B* atd_zdrive_b, ATD_ZDRIVE_B_CONVERSION_MODE conversion, ATD_ZDRIVE_B_SAMPLING_VAL sampling)
{
	byte vals[20] = ""; 
	int config;

	config=0x00;
	config= ((conversion<<4) | (sampling<<2));  //Gain set to 1 (last 2 bits 0)
		
	atd_zdrive_b->_scale_factor = scale_factor_array[sampling];
	
	//Initalise first ADS1110
	vals[0]=0x90;	  //Address of ADS 1110 
   	vals[1]=config;	 //Configure byte
   			
   	GCI_writeI2C_multiPort(atd_zdrive_b->_com_port, 2, vals, atd_zdrive_b->_i2c_bus, __FUNCTION__);   //Write 2 bytes
	
	// Initalise second ADS1110 with same settings
	vals[0]=0x92;	  //Address of second ADS 1110 
   	vals[1]=config;	 //Configure byte
   			
   	GCI_writeI2C_multiPort(atd_zdrive_b->_com_port, 2, vals,  atd_zdrive_b->_i2c_bus, __FUNCTION__);   //Write 2 bytes
	
	atd_zdrive_b_set_output_mode(atd_zdrive_b, ATD_ZDRIVE_B_OUTPUT_MODE_DAC);
	atd_zdrive_b_set_dac(atd_zdrive_b, OBJ_DAC, 32767, 1);	// Set initial value to half of full range of 16 bit dac.
	atd_zdrive_b_set_dac(atd_zdrive_b, FOCUS_DAC, 32767, 1);          
}

int atd_zdrive_b_out_byte_max521_multiport (ATD_ZDRIVE_B* atd_zdrive_b, int port, int bus, unsigned char address,
											unsigned char DAC, unsigned char patt )
{
	byte val[3]; 									  
    int err;
    
    val[0] = atd_zdrive_b->_autofocus_atd_b_i2c_chip_type | (atd_zdrive_b->_autofocus_atd_b_i2c_chip_address <<1);
    val[1] = DAC;
    val[2] = patt;
    
    err = GCI_writeI2C_multiPort(port, 3, val, bus, NULL);
		
	return err;
}

int atd_zdrive_b_set_output_mode(ATD_ZDRIVE_B* atd_zdrive_b, ATD_ZDRIVE_B_OUTPUT_MODE mode)
{
	byte vals[2] = "";
	int ctrl_val=255;	 // Set control bits high
	
	ctrl_val = (ctrl_val & 0x0f);  //Clear control bits
	ctrl_val = (ctrl_val | mode<<4);//Set the control bits
			
	vals[0]=PCF8574A | (atd_zdrive_b->_pcf8574a_chip_address <<1);
   	vals[1]=ctrl_val;
   			
   	if(GCI_writeI2C_multiPort(atd_zdrive_b->_com_port, 2, vals, atd_zdrive_b->_i2c_bus, __FUNCTION__) < 0)   //Write 2 bytes
		return Z_DRIVE_ERROR;

	return Z_DRIVE_SUCCESS;
}


// dac_address is the address of DAC8571 with address pin low
// This should be 0x98 for dac1 and 0x9c for dac2
int  atd_zdrive_b_set_dac(ATD_ZDRIVE_B* atd_zdrive_b, int dac, int val, int update_ui_val)
{
	byte vals[4] = "";
	int msb, lsb;
	Dac *d = &(atd_zdrive_b->_dacs[dac]);

	lsb = val & 0xff;
	msb = val>>8;
	
	vals[0]=d->_write_address;	  //Address of DAC8571 with address pin low
   	vals[1]=0x10;			  //Control byte load DAC with data
   	vals[2]=msb;
   	vals[3]=lsb;
   			
   	if(GCI_writeI2C_multiPort(atd_zdrive_b->_com_port, 4, vals, atd_zdrive_b->_i2c_bus, __FUNCTION__) < 0)   //Write 4 bytes
		return Z_DRIVE_ERROR;

	d->_current_dac_val = val;

	if(update_ui_val)
		SetCtrlVal(atd_zdrive_b->_setup_panel_id, SETUP_PNL_DAC_VAL, (int) d->_current_dac_val);

	return Z_DRIVE_SUCCESS;
}

int  atd_zdrive_b_set_focus_dac(ATD_ZDRIVE_B* atd_zdrive_b, int val, int update_ui_val)
{
	return atd_zdrive_b_set_dac(atd_zdrive_b, FOCUS_DAC, val, update_ui_val);
}

int  atd_zdrive_b_set_obj_dac(ATD_ZDRIVE_B* atd_zdrive_b, int val, int update_ui_val)
{
	return atd_zdrive_b_set_dac(atd_zdrive_b, OBJ_DAC, val, update_ui_val);
}

// dac_address is the read address of ADS1110
// This should be 0x91 for dac1 and 0x93 for dac2
void atd_zdrive_b_read_dac(ATD_ZDRIVE_B* atd_zdrive_b, unsigned char dac_address, int *value)
{
	unsigned short msb, lsb;
	int configReg;
	byte vals[5] = "";
	Dac *dac = &(atd_zdrive_b->_dacs[atd_zdrive_b->_current_dac]);

	vals[0]=dac_address;
    		
    GCI_readI2C_multiPort(atd_zdrive_b->_com_port, 3, vals, atd_zdrive_b->_i2c_bus, __FUNCTION__);	 //Read 3 bytes
    		
    msb = vals[0] & 0xff;  
    lsb = vals[1] & 0xff;
    configReg = vals[2] & 0xff; 
    		
	// This scale factor depends on the sampling selected.
	*value = atd_zdrive_b->_scale_factor*(msb<<8 | lsb);

	if(*value > 65535){
		*value -= 131072; 	 //To stop roll-over			
	}

	//dac->_current_dac_val = *value;
}

void atd_zdrive_b_read_status_bits(ATD_ZDRIVE_B* atd_zdrive_b, int *obj_dac_selected, int *front_panel_enabled, int *focus_dac_selected, int *stage_removed)
{
	byte vals[6] = "";

	// Read PCF8574A	
	vals[0]=PCF8574A | (atd_zdrive_b->_pcf8574a_chip_address <<1) | 0x01;

	GCI_readI2C_multiPort(atd_zdrive_b->_com_port, 1, vals, atd_zdrive_b->_i2c_bus, __FUNCTION__);	 //Read 1 byte
			
	*obj_dac_selected = ((vals[0] & 0xff) & 0x01); 
	*front_panel_enabled = ((((vals[0] & 0xff) & 0x02)^0x02) >> 1); 	
	*focus_dac_selected = ((((vals[0] & 0xff) & 0x04)) >> 2); 	
	*stage_removed = ((((vals[0] & 0xff) & 0x08)) >> 3); 
}

static void start_atd_zdrive_timer(Z_Drive* zd)
{
	ATD_ZDRIVE_B* atd_zdrive_b = (ATD_ZDRIVE_B *) zd;  

	#ifdef SINGLE_THREADED_POLLING
		if(zd->_panel_id > 0) 
			SetCtrlAttribute(zd->_panel_id, atd_zdrive_b->_adc_timer, ATTR_ENABLED, 1); 
	#else
			SetAsyncTimerAttribute (atd_zdrive_b->_adc_timer, ASYNC_ATTR_ENABLED,  1);
	#endif	
}

static void stop_atd_zdrive_timer(Z_Drive* zd)
{
	ATD_ZDRIVE_B* atd_zdrive_b = (ATD_ZDRIVE_B *) zd;  

	#ifdef SINGLE_THREADED_POLLING
		if(zd->_panel_id > 0) 
			SetCtrlAttribute(zd->_panel_id, atd_zdrive_b->_adc_timer, ATTR_ENABLED, 0); 
	#else
			SetAsyncTimerAttribute (atd_zdrive_b->_adc_timer, ASYNC_ATTR_ENABLED,  0);
	#endif	
}				



static int atd_zdrive_b_piezo_focus_read_adc_position(Z_Drive* zd, double *position)
{
	ATD_ZDRIVE_B* atd_zdrive_b = (ATD_ZDRIVE_B *) zd;  
	

	return Z_DRIVE_SUCCESS;      
}


static int atd_zdrive_b_piezo_focus_get_position(Z_Drive* zd, double *focus_microns)
{
	ATD_ZDRIVE_B* atd_zdrive_b = (ATD_ZDRIVE_B *) zd;     

	*focus_microns = zd->_current_pos;
	
	return Z_DRIVE_SUCCESS;
}

static int CVICALLBACK busy_timer(void *callback)
{
	Z_Drive *zd = (Z_Drive *) callback;	
	
	zd->_busy = 1;
	
	Delay(zd->_busy_time);
	
	zd->_busy = 0;
	
	return Z_DRIVE_SUCCESS;
}

static int atd_zdrive_b_piezo_focus_set_position(Z_Drive* zd, double focus_microns)
{
	ATD_ZDRIVE_B* atd_zdrive_b = (ATD_ZDRIVE_B *) zd;     
	int focus; 
	double current_pos;
	const int DAC_max_value = 65535;

	Dac focus_dac = atd_zdrive_b->_dacs[FOCUS_DAC];

	atd_zdrive_b->_prevent_adc_update = 1;

	z_drive_get_position(zd, &current_pos);

	focus = (int) (((focus_microns - focus_dac._min_microns) * focus_dac._scale_factor + focus_dac._offset)  * zd->_steps_per_micron);
	
	if (focus<0) 
	{
		logger_log(UIMODULE_LOGGER(zd), LOGGER_WARNING, "%s requested pos outside travel %d", UIMODULE_GET_DESCRIPTION(zd), focus);
		focus=0;
	}

	if (focus>DAC_max_value) 
	{
		logger_log(UIMODULE_LOGGER(zd), LOGGER_WARNING, "%s requested pos outside travel %d", UIMODULE_GET_DESCRIPTION(zd), focus);
		focus=DAC_max_value;
	}
	
	// Only update focus dac from main panel
	atd_zdrive_b_set_focus_dac(atd_zdrive_b, focus, atd_zdrive_b->_current_dac == FOCUS_DAC);
	
	zd->_busy_time = (fabs(current_pos-focus_microns) * zd->_speed / 1000.0);	 // 1.0 ms/um
	
	CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE, busy_timer, zd, NULL);

	zd->_current_pos = focus_microns;
	
	Delay(0.2);

	atd_zdrive_b->_prevent_adc_update = 0;

	return Z_DRIVE_SUCCESS;
}

void atd_zdrive_b_set_dac1_use(ATD_ZDRIVE_B *atd_zdrive_b_zd)
{
	Z_Drive *zd = (Z_Drive *) atd_zdrive_b_zd;	  
	Dac *dac = &(atd_zdrive_b_zd->_dacs[OBJ_DAC]);

	atd_zdrive_b_zd->_current_dac = OBJ_DAC;
		
	SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_DAC_LABEL, "Objective Settings");
	SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_DAC_CAL_LABEL, "Objective Calibration");
	SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_DAC1SELECT, 1);
	SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_DAC2SELECT, 0);
	SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_DAC_VAL, (int) dac->_current_dac_val);
	
	SetCtrlAttribute(zd->_setup_panel_id, SETUP_PNL_DACMODE, ATTR_DIMMED, 1);
}

void atd_zdrive_b_set_dac2_use(ATD_ZDRIVE_B *atd_zdrive_b_zd)
{
	Z_Drive *zd = (Z_Drive *) atd_zdrive_b_zd;	  		
	Dac *dac = &(atd_zdrive_b_zd->_dacs[FOCUS_DAC]);

	atd_zdrive_b_zd->_current_dac = FOCUS_DAC;

	SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_DAC_LABEL, "Stage Settings");
	SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_DAC_CAL_LABEL, "Stage Calibration");
	SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_DAC1SELECT, 0);
	SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_DAC2SELECT, 1);
	SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_DAC_VAL, (int) dac->_current_dac_val);

	SetCtrlAttribute(zd->_setup_panel_id, SETUP_PNL_DACMODE, ATTR_DIMMED, 0);
}

static int CVICALLBACK OnAdcTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static int update_required=0;

	switch (event)
	{
		case EVENT_TIMER_TICK:
		{
			double microns, xdiff, ydiff;
			int value;
			Z_Drive *zd = (Z_Drive *) callbackData;	    
			ATD_ZDRIVE_B* atd_zdrive_b = (ATD_ZDRIVE_B *) zd;     
			Dac *dac = &(atd_zdrive_b->_dacs[atd_zdrive_b->_current_dac]);

			if(atd_zdrive_b->_prevent_adc_update == 1)
				goto Success;

			atd_zdrive_b_read_dac(atd_zdrive_b, dac_read_address(atd_zdrive_b), &value);

			ydiff = dac->_max_microns - dac->_min_microns;
			xdiff = dac->_adc_max - dac->_adc_min;
			
//			COMMENT this out to compile
			// This is a warning that there may be an error here, P Barber, March 2010
			// I think the next line should be:
//			microns = ((ydiff / xdiff) * (value - dac->_adc_min)) + dac->_min_microns;
			// try it and check the results with both z drives
	
			microns = ((ydiff / xdiff) * value) + dac->_min_microns;
		//	microns = microns * dac->_adc_scale_factor + dac->_adc_offset;
			microns = (microns + dac->_adc_offset) * dac->_adc_scale_factor ;
			
			//if(microns < zd->_min_microns)
			//	microns = zd->_min_microns;
			
			//if(microns > zd->_max_microns)
			//	microns = zd->_max_microns;		
				
			// Only update the main panel if we are getting focus dac values
			if(atd_zdrive_b->_current_dac == FOCUS_DAC) {

				// but we need to try and detect when someone is using the manual pot
				if (fabs(microns-zd->_current_pos) > 1.0) {
					if(zd->_panel_id > 0) {
						SetCtrlVal(zd->_panel_id, FOCUS_FOCUS, microns); 	
						SetCtrlVal(zd->_panel_id, FOCUS_FOCUS_2, microns); 	
					}
					update_required = 1;
				}
				else {
					if (update_required) {
						if(zd->_panel_id > 0) {
							SetCtrlVal(zd->_panel_id, FOCUS_FOCUS, microns); 	
							SetCtrlVal(zd->_panel_id, FOCUS_FOCUS_2, microns); 	
						}
						update_required = 0;
					}
				}
			
			
				//if(zd->_panel_id > 0) {
				//	SetCtrlVal(zd->_panel_id, FOCUS_FOCUS, microns); 	
				//	SetCtrlVal(zd->_panel_id, FOCUS_FOCUS_2, microns); 	
				//}

				//zd->_current_pos = microns;
			
				//GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveChanged", GCI_VOID_POINTER, zd); 
			}

			//SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_ADC_VAL, value);   
			//SetCtrlVal(zd->_setup_panel_id, SETUP_PNL_ACT_POS, microns); 
			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveChanged", GCI_VOID_POINTER, zd); 
			
			break;
		}
	}
	
Success:

	#ifdef TIMER_DEBUG
	printf("Exit Timer Tick -- %s\n", __FUNCTION__);
	#endif

	return 0;
}

// This timer callback monitors whether someone has chosen to changed the focus manually
// ie press the button on the control pad
static int CVICALLBACK OnMonitorManualMode (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
		{
			Z_Drive *zd = (Z_Drive *) callbackData;	    
			ATD_ZDRIVE_B* atd_zdrive_b = (ATD_ZDRIVE_B *) zd;  
			
			int dac1_selected = 0, front_panel_enabled = 0, dac2_selected = 0;

			atd_zdrive_b_read_status_bits(atd_zdrive_b, &dac1_selected, &front_panel_enabled, &dac2_selected, &(atd_zdrive_b->_stage_removed));

			SetCtrlVal(atd_zdrive_b->_setup_panel_id, SETUP_PNL_DACOBJLED, dac1_selected); 
			SetCtrlVal(atd_zdrive_b->_setup_panel_id, SETUP_PNL_FRONTPNLLED, front_panel_enabled); 
			SetCtrlVal(atd_zdrive_b->_setup_panel_id, SETUP_PNL_DACFOCSELLED, dac2_selected); 
			SetCtrlVal(atd_zdrive_b->_setup_panel_id, SETUP_PNL_STAGEREMLED, atd_zdrive_b->_stage_removed); 

			// do logic on the switches and give useful info to the user
			if (atd_zdrive_b->_stage_removed) 
			{
				// The main focus always control focus
				// If the focus is missing dimmed main z drive panel
				ui_module_disable_panel(zd->_panel_id, 1, FOCUS_SETUP);

				// If the stage gets removed the we dimmed the focus dac settings
				// If the stage gets removed, we can not happily proceed with objective control by use of
				// the objective dac. The hardware switches the objective to be controlled by the focus dac (crazy hardware) !

				atd_zdrive_b_set_dac1_use(atd_zdrive_b);
				SetCtrlAttribute(atd_zdrive_b->_setup_panel_id, SETUP_PNL_DAC2SELECT, ATTR_DIMMED, 1);

				if (dac2_selected)
					SetCtrlVal(zd->_panel_id, FOCUS_MESSAGE, "Stage removed, focus with objective via software"); 
				else
					SetCtrlVal(zd->_panel_id, FOCUS_MESSAGE, "Stage removed, focus with objective: MANUAL"); 
			}			
			else 
			{
				ui_module_enable_panel(zd->_panel_id, 0);

				SetCtrlAttribute(atd_zdrive_b->_setup_panel_id, SETUP_PNL_DAC2SELECT, ATTR_DIMMED, 0);

				if (dac2_selected)
					SetCtrlVal(zd->_panel_id, FOCUS_MESSAGE, "Focussing with Z stage via software"); 
				else
					SetCtrlVal(zd->_panel_id, FOCUS_MESSAGE, "Focussing with Z stage: MANUAL"); 
			}

			break;
		}
	}
	
	return 0;
}


static int atd_zdrive_b_piezo_focus_get_min_max_in_microns(Z_Drive* zd, int* min_microns, int* max_microns)
{
	ATD_ZDRIVE_B* atd_zdrive_b = (ATD_ZDRIVE_B *) zd;

	*min_microns = (int) atd_zdrive_b->_dacs[FOCUS_DAC]._min_microns;
	*max_microns = (int) atd_zdrive_b->_dacs[FOCUS_DAC]._max_microns; 
	
	return Z_DRIVE_SUCCESS; 	
}

static int atd_zdrive_b_piezo_focus_hardware_init(Z_Drive* zd)
{
	ATD_ZDRIVE_B* atd_zdrive_b = (ATD_ZDRIVE_B *) zd;
	char device[UIMODULE_NAME_LEN];
	int obj_dac_selected, front_panel_enabled, focus_dac_selected, stage_removed;
	Dac dac1, dac2, *dac = NULL;

	ui_module_get_name(UIMODULE_CAST(zd), device);
		
	get_device_int_param_from_ini_file   (device, "COM_Port", &(atd_zdrive_b->_com_port));  
	get_device_int_param_from_ini_file   (device, "i2c_Bus", &(atd_zdrive_b->_i2c_bus));
	get_device_int_param_from_ini_file   (device, "PCF8574A_Address", &(atd_zdrive_b->_pcf8574a_chip_address));  
	get_device_int_param_from_ini_file   (device, "Autofocus_i2c_ChipType", &(atd_zdrive_b->_autofocus_atd_b_i2c_chip_type));  
	get_device_int_param_from_ini_file   (device, "Autofocus_i2c_ChipAddress", &(atd_zdrive_b->_autofocus_atd_b_i2c_chip_address));  
	get_device_double_param_from_ini_file(device, "StepsPerMicron", &(zd->_steps_per_micron));

	// Setup Dac1
	dac1._read_address = 0x91;
	dac1._write_address = 0x98;

	if(get_device_double_param_from_ini_file(device, "DAC1 ADC Min", &(dac1._adc_min)) < 0)
		atd_zdrive_b->_has_adc = 0;
	
	if(get_device_double_param_from_ini_file(device, "DAC1 ADC Max", &(dac1._adc_max)) < 0)
		atd_zdrive_b->_has_adc = 0; 

	get_device_double_param_from_ini_file(device, "DAC1 Min Microns", &(dac1._min_microns));
	get_device_double_param_from_ini_file(device, "DAC1 Max Microns", &(dac1._max_microns));
	get_device_double_param_from_ini_file(device, "DAC1 Scale Factor", &(dac1._scale_factor));
	get_device_double_param_from_ini_file(device, "DAC1 Scale Offset", &(dac1._offset));
	get_device_double_param_from_ini_file(device, "DAC1 ADC Scale Factor", &(dac1._adc_scale_factor));
	get_device_double_param_from_ini_file(device, "DAC1 ADC Scale Offset", &(dac1._adc_offset));
		
	dac1._current_dac_val = 0;

	// Setup Dac2
	dac2._read_address = 0x93;
	dac2._write_address = 0x9C;

	if(get_device_double_param_from_ini_file(device, "DAC2 ADC Min", &(dac2._adc_min)) < 0)
		atd_zdrive_b->_has_adc = 0;
	
	if(get_device_double_param_from_ini_file(device, "DAC2 ADC Max", &(dac2._adc_max)) < 0)
		atd_zdrive_b->_has_adc = 0; 

	get_device_double_param_from_ini_file(device, "DAC2 Min Microns", &(dac2._min_microns));
	get_device_double_param_from_ini_file(device, "DAC2 Max Microns", &(dac2._max_microns));
	get_device_double_param_from_ini_file(device, "DAC2 Scale Factor", &(dac2._scale_factor));
	get_device_double_param_from_ini_file(device, "DAC2 Scale Offset", &(dac2._offset));
	get_device_double_param_from_ini_file(device, "DAC2 ADC Scale Factor", &(dac2._adc_scale_factor));
	get_device_double_param_from_ini_file(device, "DAC2 ADC Scale Offset", &(dac2._adc_offset));

	dac2._current_dac_val = 0;

	atd_zdrive_b->_dacs[OBJ_DAC] = dac1;
	atd_zdrive_b->_dacs[FOCUS_DAC] = dac2;

	// Set the default Dac
	atd_zdrive_b_set_dac2_use(atd_zdrive_b);

	dac = &(atd_zdrive_b->_dacs[atd_zdrive_b->_current_dac]);

	z_drive_set_min_max (zd, atd_zdrive_b->_dacs[FOCUS_DAC]._min_microns, atd_zdrive_b->_dacs[FOCUS_DAC]._max_microns);

	z_drive_set_centre_position(zd, 0.0);

	get_device_double_param_from_ini_file(device, "Speed", &(zd->_speed));

	if (initialise_comport(atd_zdrive_b->_com_port, 9600) == Z_DRIVE_ERROR)
		return Z_DRIVE_ERROR;
	
	atd_zdrive_b_initialise(atd_zdrive_b, ATD_ZDRIVE_B_CONVERSION_MODE_CONTINOUS, ATD_ZDRIVE_B_SAMPLING_VAL_15);

	// Check whether dac 2 (FOCUS_DAC) is present otherwise we need to default to dac 1
	atd_zdrive_b_read_status_bits(atd_zdrive_b, &obj_dac_selected, &front_panel_enabled, &focus_dac_selected, &stage_removed);

	// Set the default Dac
	atd_zdrive_b_set_dac2_use(atd_zdrive_b);

	if(stage_removed)
		atd_zdrive_b_set_dac1_use(atd_zdrive_b);

	return Z_DRIVE_SUCCESS;  
}


static int atd_zdrive_b_piezo_focus_init(Z_Drive* zd)
{
	ATD_ZDRIVE_B* atd_zdrive_b = (ATD_ZDRIVE_B *) zd; 
	
    if ( InstallCtrlCallback (atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_QUIT, cb_atd_b_quit, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR;  
    
    if ( InstallCtrlCallback (atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_INPUT_0, cb_atd_b_input_0, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR;  

 	if ( InstallCtrlCallback (atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_INPUT_1, cb_atd_b_input_1, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR;  	
	
	if ( InstallCtrlCallback (atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_LASERCURRENT, cb_atd_b_lasercurrent, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR;  		
	
	if ( InstallCtrlCallback (atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_ABATTN, cb_atd_b_abattn, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR;  
	
	if ( InstallCtrlCallback (atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_OFFSETCOARSE, cb_atd_b_offsetcoarse, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR;  
	
	if ( InstallCtrlCallback (atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_OFFSETFINE, cb_atd_b_offsetfine, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR;  
	
	if ( InstallCtrlCallback (atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_DIFFERENTIALGAIN, cb_atd_b_differentialgain, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR;  
	
	if ( InstallCtrlCallback (atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_LOWSIGNALLIMIT, cb_atd_b_lowsignalllimit, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR;  
	
	if ( InstallCtrlCallback (atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_SENDALL, cb_atd_b_sendall, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR;  
	
//	if ( InstallCtrlCallback (zd->_panel_id, FOCUS_AUTOFOCUS_SETUP, cb_atd_b_sendall_setup, atd_zdrive_b) < 0)
//		return Z_DRIVE_ERROR; 
	
	// Setup Panel
	if ( InstallCtrlCallback (atd_zdrive_b->_setup_panel_id, SETUP_PNL_CLOSE, OnAtdBZdriveSetup_Close, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR; 
	
	if ( InstallCtrlCallback (atd_zdrive_b->_setup_panel_id, SETUP_PNL_DACMODE, OnAtdBZdriveSetup_ModeChanged, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR; 

	if ( InstallCtrlCallback (atd_zdrive_b->_setup_panel_id, SETUP_PNL_CONVERSION_CONTROL,
		OnAtdBZdriveSetup_ConversionControlChanged, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR; 

	if ( InstallCtrlCallback (atd_zdrive_b->_setup_panel_id, SETUP_PNL_SAMPLINGRATE,
		OnAtdBZdriveSetup_ConversionControlChanged, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR; 

	if ( InstallCtrlCallback (atd_zdrive_b->_setup_panel_id, SETUP_PNL_DAC1SELECT,
		OnAtdBZdriveSetup_OnDac1Selected, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR; 

	if ( InstallCtrlCallback (atd_zdrive_b->_setup_panel_id, SETUP_PNL_DAC2SELECT,
		OnAtdBZdriveSetup_OnDac2Selected, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR; 

	if ( InstallCtrlCallback (atd_zdrive_b->_setup_panel_id, SETUP_PNL_DAC_VAL,
		OnAtdBZdriveSetup_OnDacValChanged, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR; 

	if ( InstallCtrlCallback (atd_zdrive_b->_setup_panel_id, SETUP_PNL_SET_POS_SCALE,
		OnAtdBZdriveSetup_OnDacSettingsChanged, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR; 

	if ( InstallCtrlCallback (atd_zdrive_b->_setup_panel_id, SETUP_PNL_SET_POS_OFFSET,
		OnAtdBZdriveSetup_OnDacSettingsChanged, atd_zdrive_b) < 0)
		return Z_DRIVE_ERROR; 

	if(atd_zdrive_b->_has_adc) {

		atd_zdrive_b->_adc_timer = NewAsyncTimer (0.2, -1, 0, OnAdcTimerTick, zd);
		SetAsyncTimerName(atd_zdrive_b->_adc_timer, "ZDrive ATD_A ADC");
		SetAsyncTimerAttribute (atd_zdrive_b->_adc_timer, ASYNC_ATTR_ENABLED,  0);
	
		atd_zdrive_b->_adc_monitor_timer = NewAsyncTimer (2.0, -1, 0, OnMonitorManualMode, zd);
		SetAsyncTimerName(atd_zdrive_b->_adc_monitor_timer, "ZDrive ATD_A Monitor");
		SetAsyncTimerAttribute (atd_zdrive_b->_adc_monitor_timer, ASYNC_ATTR_ENABLED,  1);
	}

	z_drive_reveal_message_controls(zd);

	// No autofocus yet
	SetCtrlAttribute(zd->_panel_id, FOCUS_AUTOFOCUS_SETUP, ATTR_DIMMED, 1);

	return Z_DRIVE_SUCCESS;   
}


int atd_zdrive_b_piezo_focus_destroy (Z_Drive* zd)
{
	ATD_ZDRIVE_B* atd_zdrive_b = (ATD_ZDRIVE_B *) zd; 
	int obj_dac_selected, front_panel_enabled, focus_dac_selected, stage_removed;

	atd_zdrive_b_read_status_bits(atd_zdrive_b, &obj_dac_selected, &front_panel_enabled, &focus_dac_selected, &stage_removed);

	if(focus_dac_selected == 0) {

		GCI_MessagePopup("Warning", "Please enable software focus control\nor manually set focus to minimum as\nthe Prior z drive may be turned off.");
	}

	// For stage input safety set the voltage to zero
	atd_zdrive_b_set_focus_dac(atd_zdrive_b, 0, 1);

    if(atd_zdrive_b->_has_adc) {

	#ifdef SINGLE_THREADED_POLLING
		if(zd->_panel_id > 0) 
			SetCtrlAttribute(zd->_panel_id, atd_zdrive_b->_adc_timer, ATTR_ENABLED, 0); 
	#else
			SetAsyncTimerAttribute (atd_zdrive_b->_adc_timer, ASYNC_ATTR_ENABLED,  0);
	#endif	
	
	    SetAsyncTimerAttribute (atd_zdrive_b->_adc_monitor_timer, ASYNC_ATTR_ENABLED,  0); 
    }
			
	return Z_DRIVE_SUCCESS;  
}

static int atd_b_z_drive_enable_disable_timers(Z_Drive* zd, int enable)
{
	ATD_ZDRIVE_B* atd_zdrive_b = (ATD_ZDRIVE_B *) zd;  

	SetAsyncTimerAttribute (atd_zdrive_b->_adc_timer, ASYNC_ATTR_ENABLED,  enable);
	SetAsyncTimerAttribute (atd_zdrive_b->_adc_monitor_timer, ASYNC_ATTR_ENABLED,  enable); 
	
	return Z_DRIVE_SUCCESS; 
}

static void OnZdriveSetupPanelDisplayed (Z_Drive* zd, void *data)
{
	start_atd_zdrive_timer(zd); 
}


Z_Drive* atd_zdrive_b_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	Z_Drive* zd = (Z_Drive*) malloc(sizeof(ATD_ZDRIVE_B));  
	ATD_ZDRIVE_B* atd_zdrive_b = (ATD_ZDRIVE_B *) zd;    

	atd_zdrive_b->_has_adc = 1;
	atd_zdrive_b->_stage_removed = 0;
	atd_zdrive_b->_prevent_adc_update = 0;

	z_drive_constructor(zd, name, description, data_dir);

	Z_DRIVE_VTABLE_PTR(zd, hw_initialise) = atd_zdrive_b_piezo_focus_hardware_init; 
	Z_DRIVE_VTABLE_PTR(zd, initialise) = atd_zdrive_b_piezo_focus_init; 
	Z_DRIVE_VTABLE_PTR(zd, destroy) = atd_zdrive_b_piezo_focus_destroy; 
	Z_DRIVE_VTABLE_PTR(zd, z_drive_set_position) = atd_zdrive_b_piezo_focus_set_position; 
	Z_DRIVE_VTABLE_PTR(zd, z_drive_get_position) = atd_zdrive_b_piezo_focus_get_position;  
	Z_DRIVE_VTABLE_PTR(zd, z_drive_enable_disable_timers) = atd_b_z_drive_enable_disable_timers;  

	atd_zdrive_b->_autofocus_atd_b_ui_pnl = ui_module_add_panel(UIMODULE_CAST(atd_zdrive_b), "ATD_ZDriveAutoFocus_B.uir", AUTOFOCUS, 0);
	atd_zdrive_b->_setup_panel_id = ui_module_add_panel(UIMODULE_CAST(atd_zdrive_b), "ATD_ZDriveAutoFocus_B.uir", SETUP_PNL, 0);	
	z_drive_set_setup_panel(zd, atd_zdrive_b->_setup_panel_id);

	z_drive_setup_displayed_handler_connect(zd, OnZdriveSetupPanelDisplayed, NULL);

	return zd;
}

// AUTOFOCUS COMMANDS

static int Set_Autofocus_Ctrl(ATD_ZDRIVE_B *atd_zdrive_b) 
{
	SetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_INPUT_0, atd_zdrive_b->_autofocus_atd_b_input0);  
    SetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_INPUT_1, atd_zdrive_b->_autofocus_atd_b_input1);  
	SetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_ABATTN, atd_zdrive_b->_autofocus_atd_b_errorRange); 
	SetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_LASERCURRENT,atd_zdrive_b->_autofocus_atd_b_laser_I); 
	SetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_OFFSETCOARSE, atd_zdrive_b->_autofocus_atd_b_offsetCoarse); 
	SetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_OFFSETFINE,atd_zdrive_b->_autofocus_atd_b_offsetFine); 
	SetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_LOWSIGNALLIMIT, atd_zdrive_b->_autofocus_atd_b_lowSignalLimit); 
	SetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_DIFFERENTIALGAIN,atd_zdrive_b->_autofocus_atd_b_differentialGain); 

	return Z_DRIVE_SUCCESS; 
}


int autofocus_atd_b_send_vals(ATD_ZDRIVE_B* atd_zdrive_b)
{
	/*
	//int input0,input1,laser_I,errorRange,offsetCourse,offsetFine,lowSignalLimit,differentialGain;
	//	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITHOLDCURRENT, &mirror_stepper->_init_holdcurrent);   
	GetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_INPUT_0, &atd_zdrive_b->_autofocus_atd_b_input0);
	GetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_INPUT_1, &atd_zdrive_b->_autofocus_atd_b_input1); 
	GetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_ABATTN, &atd_zdrive_b->_autofocus_atd_b_errorRange); 
	GetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_LASERCURRENT, &atd_zdrive_b->_autofocus_atd_b_laser_I); 
	GetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_OFFSETCOARSE, &atd_zdrive_b->_autofocus_atd_b_offsetCoarse); 
	GetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_OFFSETFINE, &atd_zdrive_b->_autofocus_atd_b_offsetFine); 
	GetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_LOWSIGNALLIMIT, &atd_zdrive_b->_autofocus_atd_b_lowSignalLimit); 
	GetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_DIFFERENTIALGAIN, &atd_zdrive_b->_autofocus_atd_b_differentialGain); 
	 	
	if(atd_zdrive_b->_autofocus_atd_b_input0<0){
	 	atd_zdrive_b->_autofocus_atd_b_input0=0;
	 	SetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_INPUT_0, atd_zdrive_b->_autofocus_atd_b_input0);  

	}
	
	if(atd_zdrive_b->_autofocus_atd_b_input0>255){
	 	atd_zdrive_b->_autofocus_atd_b_input0=255;
	 	SetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_INPUT_0, atd_zdrive_b->_autofocus_atd_b_input0);   
	}
	
	if(atd_zdrive_b->_autofocus_atd_b_input1<0 ){
	 	atd_zdrive_b->_autofocus_atd_b_input1=0;
	 	SetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_INPUT_1, atd_zdrive_b->_autofocus_atd_b_input1); 
	}
	
	if(atd_zdrive_b->_autofocus_atd_b_input1>255){
	 	atd_zdrive_b->_autofocus_atd_b_input1=255;
	 	SetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_INPUT_1, atd_zdrive_b->_autofocus_atd_b_input1); 
	}
	 
	if	(atd_zdrive_b_out_byte_max521_multiport (atd_zdrive_b, atd_zdrive_b->_com_port, atd_zdrive_b->_i2c_bus,atd_zdrive_b->_i2c_chip_address, DAC0, atd_zdrive_b->_autofocus_atd_b_input0 ))
		goto Error;

	if	(atd_zdrive_b_out_byte_max521_multiport (atd_zdrive_b, atd_zdrive_b->_com_port, atd_zdrive_b->_i2c_bus,atd_zdrive_b->_i2c_chip_address, DAC1, atd_zdrive_b->_autofocus_atd_b_input1 ))
		goto Error;

	if	(atd_zdrive_b_out_byte_max521_multiport (atd_zdrive_b, atd_zdrive_b->_com_port, atd_zdrive_b->_i2c_bus,atd_zdrive_b->_i2c_chip_address, DAC2, atd_zdrive_b->_autofocus_atd_b_errorRange ))
		goto Error;

	if	(atd_zdrive_b_out_byte_max521_multiport (atd_zdrive_b, atd_zdrive_b->_com_port, atd_zdrive_b->_i2c_bus,atd_zdrive_b->_i2c_chip_address, DAC7, atd_zdrive_b->_autofocus_atd_b_laser_I ))
		goto Error;

	if	(atd_zdrive_b_out_byte_max521_multiport (atd_zdrive_b, atd_zdrive_b->_com_port, atd_zdrive_b->_i2c_bus,atd_zdrive_b->_i2c_chip_address, DAC5, atd_zdrive_b->_autofocus_atd_b_offsetCoarse ))
		goto Error;

	if	(atd_zdrive_b_out_byte_max521_multiport (atd_zdrive_b, atd_zdrive_b->_com_port, atd_zdrive_b->_i2c_bus,atd_zdrive_b->_i2c_chip_address, DAC6, atd_zdrive_b->_autofocus_atd_b_offsetFine  ))
		goto Error;

	if	(atd_zdrive_b_out_byte_max521_multiport (atd_zdrive_b, atd_zdrive_b->_com_port, atd_zdrive_b->_i2c_bus,atd_zdrive_b->_i2c_chip_address, DAC4, atd_zdrive_b->_autofocus_atd_b_lowSignalLimit ))
		goto Error;

	if	(atd_zdrive_b_out_byte_max521_multiport (atd_zdrive_b, atd_zdrive_b->_com_port, atd_zdrive_b->_i2c_bus,atd_zdrive_b->_i2c_chip_address, DAC3, atd_zdrive_b->_autofocus_atd_b_differentialGain ))
		goto Error;
	
   	return Z_DRIVE_ERROR; 

Error:
	
	//send_autofocus_atd_b_error_text(atd_zdrive_b, "Failed to send initial settings");
	return  Z_DRIVE_ERROR ;

	*/

	return Z_DRIVE_SUCCESS; 
}

static int RestoreAutofocusSettings(ATD_ZDRIVE_B *atd_zdrive_b)
{
	char  dataPath[GCI_MAX_PATHNAME_LEN] ;  
	FILE *fp;
	Z_Drive *zd = (Z_Drive*) atd_zdrive_b;
	
	sprintf(dataPath, "%s\\autofocus_atd_b_%s.txt", zd->_data_dir, UIMODULE_GET_NAME(atd_zdrive_b));      
	
  	fp = fopen (dataPath, "r");
	
	if (fp != NULL) {
 
		SetWaitCursor (1);
		fscanf(fp,  "%i,\t", &atd_zdrive_b->_autofocus_atd_b_input0);
		fscanf(fp,  "%i,\t", &atd_zdrive_b->_autofocus_atd_b_input1);
		fscanf(fp,  "%i,\t", &atd_zdrive_b->_autofocus_atd_b_laser_I);
		fscanf(fp,  "%i,\t", &atd_zdrive_b->_autofocus_atd_b_errorRange);
		fscanf(fp,  "%i,\t", &atd_zdrive_b->_autofocus_atd_b_offsetCoarse);
		fscanf(fp,  "%i,\t", &atd_zdrive_b->_autofocus_atd_b_offsetFine);
		fscanf(fp,  "%i,\t", &atd_zdrive_b->_autofocus_atd_b_lowSignalLimit);
		fscanf(fp,  "%i,\t", &atd_zdrive_b->_autofocus_atd_b_differentialGain);
	
		fclose(fp);    
	
		Set_Autofocus_Ctrl(atd_zdrive_b);  			  //Set control values
	}
	
	if(autofocus_atd_b_send_vals(atd_zdrive_b) == Z_DRIVE_ERROR)
		return Z_DRIVE_ERROR;
	
	SetWaitCursor (0); 
  
	return Z_DRIVE_SUCCESS;
}	

static int GetAutofocusVals(ATD_ZDRIVE_B* atd_zdrive_b)		   //Running settings
{
	GetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_INPUT_0, &atd_zdrive_b->_autofocus_atd_b_input0);
	GetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_INPUT_1, &atd_zdrive_b->_autofocus_atd_b_input1); 
	GetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_ABATTN, &atd_zdrive_b->_autofocus_atd_b_errorRange); 
	GetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_LASERCURRENT, &atd_zdrive_b->_autofocus_atd_b_laser_I); 
	GetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_OFFSETCOARSE, &atd_zdrive_b->_autofocus_atd_b_offsetCoarse); 
	GetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_OFFSETFINE, &atd_zdrive_b->_autofocus_atd_b_offsetFine); 
	GetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_LOWSIGNALLIMIT, &atd_zdrive_b->_autofocus_atd_b_lowSignalLimit); 
	GetCtrlVal(atd_zdrive_b->_autofocus_atd_b_ui_pnl, AUTOFOCUS_DIFFERENTIALGAIN, &atd_zdrive_b->_autofocus_atd_b_differentialGain); 
		   
	return Z_DRIVE_ERROR;
}

int autofocus_atd_b_hide_main_ui(ATD_ZDRIVE_B* atd_zdrive_b)
{
	ui_module_hide_main_panel(UIMODULE_CAST(atd_zdrive_b));  

	return Z_DRIVE_SUCCESS;
}
