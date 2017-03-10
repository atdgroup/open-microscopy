#include "ATD_ZDrive_A.h"
#include "ATD_ZDriveAutoFocus_A.h"
#include "ZDriveUI.h" 
#include "gci_utils.h"
#include "ATD_UsbInterface_A.h" 

#include <userint.h>
#include <formatio.h>
#include <cviauto.h>  
#include "asynctmr.h"
#include <rs232.h>
#include <utility.h>

#define MAX521		 0x50 

static int atd_zdrive_a_piezo_focus_init_adc(Z_Drive* zd, int continuous, int sample_rate)
{
	unsigned char val[20];  
	ATD_ZDRIVE_A* atd_zdrive_a_zd = (ATD_ZDRIVE_A *) zd;  
	
	int config=0x00;
	continuous = !continuous;
	
	config= ((continuous<<4) | (sample_rate<<2));  //Gain set to 1 (last 2 bits 0)
		
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	val[0]=config; //Configure byte
   		
	if(ftdi_controller_i2c_write_bytes(atd_zdrive_a_zd->_controller, atd_zdrive_a_zd->_i2c_chip_address, 3, val) != FT_OK) {
		return Z_DRIVE_ERROR;
	}

	#else

	val[0]=0x90;	  //Address
   	val[1]=config;	  //Configure byte
   			
   	GCI_writeI2C_multiPort(atd_zdrive_a_zd->_com_port, 3, val, atd_zdrive_a_zd->_i2c_bus, "write adc init");   // Write 2 bytes
	
	#endif

	return Z_DRIVE_SUCCESS;
}

static int atd_zdrive_a_piezo_focus_read_adc_position(Z_Drive* zd, double *position)
{
	unsigned char val1[20];
	int msb,lsb,configReg;
	ATD_ZDRIVE_A* atd_zdrive_a_zd = (ATD_ZDRIVE_A *) zd;  
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_i2c_read_bytes(atd_zdrive_a_zd->_controller, 0x48, 3, val1) != FT_OK) {
		return Z_DRIVE_ERROR;
	}

	#else

	val1[0]=0x91;	  //Read ADC
    		
    if(GCI_readI2C_multiPort(atd_zdrive_a_zd->_com_port, 3, val1, atd_zdrive_a_zd->_i2c_bus, "read adc position") < 0)	 //Read 3 bytes
    	return Z_DRIVE_ERROR;	

	#endif

    msb = val1[0] & 0xff;  
    lsb = val1[1] & 0xff;
    configReg = val1[2] & 0xff; 
    
	*position = atd_zdrive_a_zd->_sampling_scale_factor * (msb<<8 | lsb);
	
	// The ADC can underflow and return -ve values as just below 2*65535=131070, or overflow over 65535
	// Let's use a 1.5*65535=98302 threhold and values greater than that get converted to -ve.
	if (*position > 98302)
		*position -= 131070;
	
	return Z_DRIVE_SUCCESS;      
}


static int atd_zdrive_a_piezo_focus_get_position(Z_Drive* zd, double *focus_microns)
{
	ATD_ZDRIVE_A* atd_zdrive_a_zd = (ATD_ZDRIVE_A *) zd;     
//	unsigned char val1[20];
//	int val,msb,lsb;

	*focus_microns = zd->_current_pos;
	
	/*
	// Read what we sent to the registers
	
	lsb = focus & 0xff;
	msb = focus >> 8;
		   	
	//val1[0]=DAC8571| (address <<2);
	val1[0] = 0x9c | 0x01;
   			
   	GCI_readI2C_multiPort(atd_zdrive_a_zd->_com_port, 3, val1, atd_zdrive_a_zd->_i2c_bus, "Z_Drive");   //Write 4 bytes

	msb = val1[0];
	lsb = val1[1];
	command = val1[2];
	*/
	
	return Z_DRIVE_SUCCESS;
}

int CVICALLBACK busy_timer(void *callback)
{
	static int count = 0;
	
	Z_Drive *zd = (Z_Drive *) callback;	
	
	zd->_busy = 1;

	Delay(zd->_busy_time);

	printf("Waiting for z stage, busy_time: %f, %d\n", zd->_busy_time, count++);
	
	zd->_busy = 0;

	return Z_DRIVE_SUCCESS;
}

static int atd_zdrive_a_piezo_focus_set_position(Z_Drive* zd, double focus_microns)
{
	ATD_ZDRIVE_A* atd_zdrive_a_zd = (ATD_ZDRIVE_A *) zd;     
	unsigned char val1[20] = "";
	int thread_id, msb,lsb, focus; //val,
	double current_pos;
	const int DAC_max_value = 65535;
//	char time_str[256];

	//zdrive_get_lock(zd, __FUNCTION__);
	atd_zdrive_a_zd->_prevent_adc_update = 1;

	z_drive_get_position(zd, &current_pos);

	if (zd->_reverse) // inverts the scale so +ve is toward sample, up on an inverted
		focus_microns = -focus_microns;

//		focus = DAC_max_value - ((focus_microns - zd->_min_microns) * zd->_steps_per_micron);  // inverts the scale so +ve is toward sample, up on an inverted
	focus = (int) ((focus_microns - zd->_min_microns) * zd->_steps_per_micron);  // this is the traditional condition, prior to Sept 2013
	
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
	
	lsb = focus & 0xff;
	msb = focus >> 8;
		   	
	//val1[0]=DAC8571| (address <<2);
	//val1[0] = 0x9c;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	val1[0] = 0x10;	 //Control byte load DAC with data
   	val1[1] = msb;
   	val1[2] = lsb;
   		
	if(ftdi_controller_i2c_write_bytes(atd_zdrive_a_zd->_controller, atd_zdrive_a_zd->_i2c_chip_address, 4, val1) != FT_OK) {
		return Z_DRIVE_ERROR;
	}

	#else

	val1[0] = atd_zdrive_a_zd->_i2c_chip_type | (atd_zdrive_a_zd->_i2c_chip_address <<2);
	val1[1] = 0x10;	 //Control byte load DAC with data
   	val1[2] = msb;
   	val1[3] = lsb;
   			
   	if(GCI_writeI2C_multiPort(atd_zdrive_a_zd->_com_port, 4, val1, atd_zdrive_a_zd->_i2c_bus, "Z_Drive") < 0)   //Write 4 bytes
	{
		send_z_drive_error_text(zd, "ZDrive I2C Error");
		return Z_DRIVE_ERROR;
	}

	#endif

	zd->_busy_time = (fabs(current_pos-focus_microns) * zd->_speed / 1000.0);	 // 1.0 ms/um
	
#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: Start z drive busy_timer\n", time_str);
#endif

	CmtScheduleThreadPoolFunction (gci_thread_pool(), busy_timer, zd, &thread_id);

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: Wait for z drive busy_timer\n", time_str);
#endif

	if(zd->_busy == 1)
		CmtWaitForThreadPoolFunctionCompletion(gci_thread_pool(), thread_id, 0);

#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf("%s: Stopping z drive busy_timer 1", time_str);
#endif
	
	CmtReleaseThreadPoolFunctionID(gci_thread_pool(), thread_id);

#ifdef VERBOSE_DEBUG
	printf(" 2");
#endif
	// delay for the ADC to read - but we assume the set position below, this could be improved
	Delay(atd_zdrive_a_zd->_adc_read_wait_time);

#ifdef VERBOSE_DEBUG
	printf(" 3");
#endif

	//zdrive_release_lock(zd, __FUNCTION__);
	atd_zdrive_a_zd->_prevent_adc_update = 0;

	zd->_current_pos = focus_microns;
	
#ifdef VERBOSE_DEBUG
	get_time_string(time_str);
	printf(" 4: %s\n", time_str);
#endif

	return Z_DRIVE_SUCCESS;
}


int CVICALLBACK OnAdcTimerTick (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	static int update_required=0;
	
	switch (event)
	{
		case EVENT_TIMER_TICK:
		{
			double position, microns, xdiff, ydiff;
			Z_Drive *zd = (Z_Drive *) callbackData;	    
			ATD_ZDRIVE_A* atd_zdrive_a_zd = (ATD_ZDRIVE_A *) zd;     
			
			#ifdef TIMER_DEBUG
			printf("Enter Timer Tick -- %s\n", __FUNCTION__);
			#endif

			//zdrive_get_lock(zd, __FUNCTION__);
			if(atd_zdrive_a_zd->_prevent_adc_update == 1)
				goto Success;

			if(atd_zdrive_a_piezo_focus_read_adc_position(zd, &position) == Z_DRIVE_ERROR)
				goto Error;
				
			ydiff = zd->_max_microns - zd->_min_microns;
			xdiff = atd_zdrive_a_zd->_adc_closedloop_max - atd_zdrive_a_zd->_adc_closedloop_min;
			
			microns = ((ydiff / xdiff) * (position - atd_zdrive_a_zd->_adc_closedloop_min)) + zd->_min_microns;

			if (zd->_reverse)
				microns = -microns;

			if(microns < zd->_min_microns)
				microns = zd->_min_microns;
			
			if(microns > zd->_max_microns)
				microns = zd->_max_microns;			
		
			// We no longer read back the position through the adc 
			// We decided there is no point as the zdrive should do the best it can going to the 
			// location we tell it.

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
			
			GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(zd), "Z_DriveChanged", GCI_VOID_POINTER, zd); 

			//zdrive_release_lock(zd, __FUNCTION__);

			break;
		}
	}
	
Success:

	#ifdef TIMER_DEBUG
	printf("Exit Timer Tick -- %s\n", __FUNCTION__);
	#endif

	return 0;

Error:

	#ifdef TIMER_DEBUG
	printf("Exit Timer Tick -- %s\n", __FUNCTION__);
	#endif

	return -1;
}

// This timer callback monitors whether someone has chosen to changed the focus manually
// ie press the button on the control pad
int CVICALLBACK OnMonitorManualMode (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_TIMER_TICK:
		{
			Z_Drive *zd = (Z_Drive *) callbackData;	    
			ATD_ZDRIVE_A* atd_zdrive_a_zd = (ATD_ZDRIVE_A *) zd;  
			
			static int last_mode = -1;
			
			int manual = 1;
			
			if(last_mode == manual)
				break;
			
			if(manual) {
				
				// We must turn the continous readout on
				atd_zdrive_a_piezo_focus_init_adc(zd, 1, atd_zdrive_a_zd->_sampling_rate);  
			}
			else {
				
				atd_zdrive_a_piezo_focus_init_adc(zd, 0, atd_zdrive_a_zd->_sampling_rate);  
			}
	
			SetAsyncTimerAttribute (atd_zdrive_a_zd->_adc_timer, ASYNC_ATTR_ENABLED,  1);

			last_mode = manual;   

			break;
		}
	}
	
	return 0;
}


static int atd_zdrive_a_piezo_focus_get_min_max_in_microns(Z_Drive* zd, int* min_microns, int* max_microns)
{
	*min_microns = (int) zd->_min_microns;
	*max_microns = (int) zd->_max_microns; 
	
	return Z_DRIVE_SUCCESS; 	
}

static void ftdi_error_handler (FTDIController *controller, const char *title, const char *error_string, void *callback_data)    
{
	printf("%s: %s\n", title, error_string);
}

static int atd_zdrive_a_piezo_focus_hardware_init(Z_Drive* zd)
{
	ATD_ZDRIVE_A* atd_zdrive_a_zd = (ATD_ZDRIVE_A *) zd;
	char device[UIMODULE_NAME_LEN];
	char device_sn[FTDI_SERIAL_NUMBER_LINE_LENGTH] = "";

	ui_module_get_name(UIMODULE_CAST(zd), device);
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	get_device_string_param_from_ini_file(device, "FTDI_SN", device_sn);  
	get_device_int_param_from_ini_file   (device, "I2C_DEVICE_BUS", &(atd_zdrive_a_zd->_i2c_bus));
	get_device_int_param_from_ini_file   (device, "I2C_DEVICE_ADDRESS", &(atd_zdrive_a_zd->_i2c_chip_address));  
	
	atd_zdrive_a_zd->_controller = ftdi_controller_new();

//	ftdi_controller_set_debugging(atd_zdrive_a_zd->_controller, 1);
	ftdi_controller_set_error_handler(atd_zdrive_a_zd->_controller, ftdi_error_handler, NULL);
	ftdi_controller_open(atd_zdrive_a_zd->_controller, device_sn);

	#else

	get_device_int_param_from_ini_file   (device, "COM_Port", &(atd_zdrive_a_zd->_com_port));  
	get_device_int_param_from_ini_file   (device, "i2c_Bus", &(atd_zdrive_a_zd->_i2c_bus));
	get_device_int_param_from_ini_file   (device, "i2c_ChipAddress", &(atd_zdrive_a_zd->_i2c_chip_address));  
	get_device_int_param_from_ini_file   (device, "i2c_ChipType", &(atd_zdrive_a_zd->_i2c_chip_type));

	if (initialise_comport(atd_zdrive_a_zd->_com_port, 9600) == Z_DRIVE_ERROR)
		return Z_DRIVE_ERROR;
	
	#endif

	get_device_int_param_from_ini_file   (device, "Autofocus_i2c_ChipType", &(atd_zdrive_a_zd->_autofocus_i2c_chip_type));  
	get_device_double_param_from_ini_file(device, "StepsPerMicron", &(zd->_steps_per_micron));
	get_device_double_param_from_ini_file(device, "Min Microns", &(zd->_min_microns));
	get_device_double_param_from_ini_file(device, "Max Microns", &(zd->_max_microns));
	get_device_int_param_from_ini_file   (device, "Reverse", &(zd->_reverse));  

	if(get_device_double_param_from_ini_file(device, "ADC ClosedLoop Min", &(atd_zdrive_a_zd->_adc_closedloop_min)) < 0)
		atd_zdrive_a_zd->_has_adc = 0;
	
	if(get_device_double_param_from_ini_file(device, "ADC ClosedLoop Max", &(atd_zdrive_a_zd->_adc_closedloop_max)) < 0)
		atd_zdrive_a_zd->_has_adc = 0; 
	
	get_device_double_param_from_ini_file(device, "Speed", &(zd->_speed));

	z_drive_set_centre_position(zd, 0.0);

	return Z_DRIVE_SUCCESS;  
}


static int atd_zdrive_a_piezo_focus_init(Z_Drive* zd)
{
	ATD_ZDRIVE_A* atd_zdrive_a_zd = (ATD_ZDRIVE_A *) zd; 
	
	atd_zdrive_a_zd->_autofocus_ui_pnl = ui_module_add_panel(UIMODULE_CAST(atd_zdrive_a_zd), "ATD_ZDriveAutoFocus_A.uir", AUTOFOCUS, 0);
 
    if ( InstallCtrlCallback (atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_QUIT, cb_autofocus_quit, atd_zdrive_a_zd) < 0)
		return Z_DRIVE_ERROR;  
    
    if ( InstallCtrlCallback (atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_INPUT_0, cb_autofocus_input_0, atd_zdrive_a_zd) < 0)
		return Z_DRIVE_ERROR;  

 	if ( InstallCtrlCallback (atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_INPUT_1, cb_autofocus_input_1, atd_zdrive_a_zd) < 0)
		return Z_DRIVE_ERROR;  	
	
	if ( InstallCtrlCallback (atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_LASERCURRENT, cb_autofocus_lasercurrent, atd_zdrive_a_zd) < 0)
		return Z_DRIVE_ERROR;  		
	
	if ( InstallCtrlCallback (atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_ABATTN, cb_autofocus_abattn, atd_zdrive_a_zd) < 0)
		return Z_DRIVE_ERROR;  
	
	if ( InstallCtrlCallback (atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_OFFSETCOARSE, cb_autofocus_offsetcoarse, atd_zdrive_a_zd) < 0)
		return Z_DRIVE_ERROR;  
	
	if ( InstallCtrlCallback (atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_OFFSETFINE, cb_autofocus_offsetfine, atd_zdrive_a_zd) < 0)
		return Z_DRIVE_ERROR;  
	
	if ( InstallCtrlCallback (atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_DIFFERENTIALGAIN, cb_autofocus_differentialgain, atd_zdrive_a_zd) < 0)
		return Z_DRIVE_ERROR;  
	
	if ( InstallCtrlCallback (atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_LOWSIGNALLIMIT, cb_autofocus_lowsignalllimit, atd_zdrive_a_zd) < 0)
		return Z_DRIVE_ERROR;  
	
	if ( InstallCtrlCallback (atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_SENDALL, cb_autofocus_sendall, atd_zdrive_a_zd) < 0)
		return Z_DRIVE_ERROR;  
	
	if ( InstallCtrlCallback (zd->_panel_id, FOCUS_AUTOFOCUS_SETUP, cb_autofocus_sendall_setup, atd_zdrive_a_zd) < 0)
		return Z_DRIVE_ERROR; 
	
	if(atd_zdrive_a_zd->_has_adc) {

		atd_zdrive_a_zd->_adc_timer = NewAsyncTimer (0.2, -1, 0, OnAdcTimerTick, zd);
		SetAsyncTimerName(atd_zdrive_a_zd->_adc_timer, "ZDrive ATD_A ADC");
		SetAsyncTimerAttribute (atd_zdrive_a_zd->_adc_timer, ASYNC_ATTR_ENABLED,  0);
	
		atd_zdrive_a_zd->_adc_monitor_timer = NewAsyncTimer (2.0, -1, 0, OnMonitorManualMode, zd);
		SetAsyncTimerName(atd_zdrive_a_zd->_adc_monitor_timer, "ZDrive ATD_A Monitor");
		SetAsyncTimerAttribute (atd_zdrive_a_zd->_adc_monitor_timer, ASYNC_ATTR_ENABLED,  1);
	}
	
	return Z_DRIVE_SUCCESS;   
}


int atd_zdrive_a_piezo_focus_destroy (Z_Drive* zd)
{
	ATD_ZDRIVE_A* atd_zdrive_a_zd = (ATD_ZDRIVE_A *) zd; 
			
	DiscardAsyncTimer(atd_zdrive_a_zd->_adc_timer);
	DiscardAsyncTimer(atd_zdrive_a_zd->_adc_monitor_timer);

	// To extend life of piezo drive, return it to it's rest position.
	if (zd->_reverse)
		atd_zdrive_a_piezo_focus_set_position(zd, zd->_max_microns);
	else
		atd_zdrive_a_piezo_focus_set_position(zd, zd->_min_microns);

	Delay(0.5); // Make sure z drive has stopped before detroying it all

	#ifdef FTDI_NO_VIRTUAL_COMPORT
	ftdi_controller_close(atd_zdrive_a_zd->_controller);
	#else
	close_comport(atd_zdrive_a_zd->_com_port);
	#endif

	return Z_DRIVE_SUCCESS;  
}

static int atd_a_z_drive_enable_disable_timers(Z_Drive* zd, int enable)
{
	ATD_ZDRIVE_A* atd_zdrive_a_zd = (ATD_ZDRIVE_A *) zd;

	SetAsyncTimerAttribute (atd_zdrive_a_zd->_adc_timer, ASYNC_ATTR_ENABLED,  enable);
	SetAsyncTimerAttribute (atd_zdrive_a_zd->_adc_monitor_timer, ASYNC_ATTR_ENABLED,  enable); 

	return Z_DRIVE_SUCCESS; 
}

void atd_zdrive_a_set_alternate_adc_sampling (Z_Drive* zd)
{
	ATD_ZDRIVE_A* atd_zdrive_a_zd = (ATD_ZDRIVE_A *) zd;

	if (zd == NULL)
		return;

	atd_zdrive_a_zd->_sampling_rate = 1; // 60 samples/sec
	atd_zdrive_a_zd->_sampling_scale_factor = 8; // depends on samples/sec (see Rob's test code)
	atd_zdrive_a_zd->_adc_read_wait_time = 0.02f;
}

Z_Drive* atd_zdrive_a_new(const char *name, const char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	Z_Drive* zd = (Z_Drive*) malloc(sizeof(ATD_ZDRIVE_A));  
	ATD_ZDRIVE_A* atd_zdrive_a_zd = (ATD_ZDRIVE_A *) zd;    

	
// #define SAMPLING_RATE  3    // 15 samples/sec (see Rob's test code)
// #define SAMPLING_SCALE_FACTOR 2    // depends on samples/sec (see Rob's test code)
	// The linac required these to be changed, so the became variables
	atd_zdrive_a_zd->_sampling_rate = 3;
	atd_zdrive_a_zd->_sampling_scale_factor = 2;
	atd_zdrive_a_zd->_adc_read_wait_time = 0.2f;

	atd_zdrive_a_zd->_has_adc = 1;
	atd_zdrive_a_zd->_prevent_adc_update = 0;
	zd->_reverse = 0;

	z_drive_constructor(zd, name, description, data_dir);

	Z_DRIVE_VTABLE_PTR(zd, hw_initialise) = atd_zdrive_a_piezo_focus_hardware_init; 
	Z_DRIVE_VTABLE_PTR(zd, initialise) = atd_zdrive_a_piezo_focus_init; 
	Z_DRIVE_VTABLE_PTR(zd, destroy) = atd_zdrive_a_piezo_focus_destroy; 
	Z_DRIVE_VTABLE_PTR(zd, z_drive_set_position) = atd_zdrive_a_piezo_focus_set_position; 
	Z_DRIVE_VTABLE_PTR(zd, z_drive_get_position) = atd_zdrive_a_piezo_focus_get_position;  
	Z_DRIVE_VTABLE_PTR(zd, z_drive_enable_disable_timers) = atd_a_z_drive_enable_disable_timers;  

	return zd;
}

// AUTOFOCUS COMMANDS

static int Set_Autofocus_Ctrl(ATD_ZDRIVE_A *atd_zdrive_a_zd) 
{
	SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_INPUT_0, atd_zdrive_a_zd->_autofocus_input0);  
    SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_INPUT_1, atd_zdrive_a_zd->_autofocus_input1);  
	SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_ABATTN, atd_zdrive_a_zd->_autofocus_errorRange); 
	SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_LASERCURRENT,atd_zdrive_a_zd->_autofocus_laser_I); 
	SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_OFFSETCOARSE, atd_zdrive_a_zd->_autofocus_offsetCoarse); 
	SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_OFFSETFINE,atd_zdrive_a_zd->_autofocus_offsetFine); 
	SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_LOWSIGNALLIMIT, atd_zdrive_a_zd->_autofocus_lowSignalLimit); 
	SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_DIFFERENTIALGAIN,atd_zdrive_a_zd->_autofocus_differentialGain); 

	 return Z_DRIVE_SUCCESS; 
}


int atd_zdrive_a_out_byte_max521_multiport (int port, int bus, unsigned char address, unsigned char DAC, unsigned char patt )
{
	byte val[3]; 									  
    int err;
    
    val[0] = MAX521 | (address <<1);
    val[1] = DAC;
    val[2] = patt;
    
    err = GCI_writeI2C_multiPort(port, 3, val, bus, NULL);
		
	return err;
}

int autofocus_send_vals(ATD_ZDRIVE_A* atd_zdrive_a_zd)
{
	//int input0,input1,laser_I,errorRange,offsetCourse,offsetFine,lowSignalLimit,differentialGain;
	//	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITHOLDCURRENT, &mirror_stepper->_init_holdcurrent);   
	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_INPUT_0, &atd_zdrive_a_zd->_autofocus_input0);
	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_INPUT_1, &atd_zdrive_a_zd->_autofocus_input1); 
	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_ABATTN, &atd_zdrive_a_zd->_autofocus_errorRange); 
	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_LASERCURRENT, &atd_zdrive_a_zd->_autofocus_laser_I); 
	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_OFFSETCOARSE, &atd_zdrive_a_zd->_autofocus_offsetCoarse); 
	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_OFFSETFINE, &atd_zdrive_a_zd->_autofocus_offsetFine); 
	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_LOWSIGNALLIMIT, &atd_zdrive_a_zd->_autofocus_lowSignalLimit); 
	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_DIFFERENTIALGAIN, &atd_zdrive_a_zd->_autofocus_differentialGain); 
	 	
	if(atd_zdrive_a_zd->_autofocus_input0<0){
	 	atd_zdrive_a_zd->_autofocus_input0=0;
	 	SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_INPUT_0, atd_zdrive_a_zd->_autofocus_input0);  

	}
	
	if(atd_zdrive_a_zd->_autofocus_input0>255){
	 	atd_zdrive_a_zd->_autofocus_input0=255;
	 	SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_INPUT_0, atd_zdrive_a_zd->_autofocus_input0);   
	}
	
	if(atd_zdrive_a_zd->_autofocus_input1<0 ){
	 	atd_zdrive_a_zd->_autofocus_input1=0;
	 	SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_INPUT_1, atd_zdrive_a_zd->_autofocus_input1); 
	}
	
	if(atd_zdrive_a_zd->_autofocus_input1>255){
	 	atd_zdrive_a_zd->_autofocus_input1=255;
	 	SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_INPUT_1, atd_zdrive_a_zd->_autofocus_input1); 
	}
	 
	if	(atd_zdrive_a_out_byte_max521_multiport ( atd_zdrive_a_zd->_com_port, atd_zdrive_a_zd->_i2c_bus,atd_zdrive_a_zd->_i2c_chip_address, DAC0, atd_zdrive_a_zd->_autofocus_input0 ))
		goto Error;

	if	(atd_zdrive_a_out_byte_max521_multiport ( atd_zdrive_a_zd->_com_port, atd_zdrive_a_zd->_i2c_bus,atd_zdrive_a_zd->_i2c_chip_address, DAC1, atd_zdrive_a_zd->_autofocus_input1 ))
		goto Error;

	if	(atd_zdrive_a_out_byte_max521_multiport ( atd_zdrive_a_zd->_com_port, atd_zdrive_a_zd->_i2c_bus,atd_zdrive_a_zd->_i2c_chip_address, DAC2, atd_zdrive_a_zd->_autofocus_errorRange ))
		goto Error;

	if	(atd_zdrive_a_out_byte_max521_multiport ( atd_zdrive_a_zd->_com_port, atd_zdrive_a_zd->_i2c_bus,atd_zdrive_a_zd->_i2c_chip_address, DAC7, atd_zdrive_a_zd->_autofocus_laser_I ))
		goto Error;

	if	(atd_zdrive_a_out_byte_max521_multiport ( atd_zdrive_a_zd->_com_port, atd_zdrive_a_zd->_i2c_bus,atd_zdrive_a_zd->_i2c_chip_address, DAC5, atd_zdrive_a_zd->_autofocus_offsetCoarse ))
		goto Error;

	if	(atd_zdrive_a_out_byte_max521_multiport ( atd_zdrive_a_zd->_com_port, atd_zdrive_a_zd->_i2c_bus,atd_zdrive_a_zd->_i2c_chip_address, DAC6, atd_zdrive_a_zd->_autofocus_offsetFine  ))
		goto Error;

	if	(atd_zdrive_a_out_byte_max521_multiport ( atd_zdrive_a_zd->_com_port, atd_zdrive_a_zd->_i2c_bus,atd_zdrive_a_zd->_i2c_chip_address, DAC4, atd_zdrive_a_zd->_autofocus_lowSignalLimit ))
		goto Error;

	if	(atd_zdrive_a_out_byte_max521_multiport ( atd_zdrive_a_zd->_com_port, atd_zdrive_a_zd->_i2c_bus,atd_zdrive_a_zd->_i2c_chip_address, DAC3, atd_zdrive_a_zd->_autofocus_differentialGain ))
		goto Error;
	
   	return Z_DRIVE_ERROR; 

Error:
	
	//send_autofocus_error_text(atd_zdrive_a_zd, "Failed to send initial settings");
	return  Z_DRIVE_ERROR ;
}

static int RestoreAutofocusSettings(ATD_ZDRIVE_A *atd_zdrive_a_zd)
{
	char  dataPath[GCI_MAX_PATHNAME_LEN] ;  
	FILE *fp;
	Z_Drive *zd = (Z_Drive*) atd_zdrive_a_zd;
	
	sprintf(dataPath, "%s\\autofocus_%s.txt", zd->_data_dir, UIMODULE_GET_NAME(atd_zdrive_a_zd));      
	
  	fp = fopen (dataPath, "r");
	
	if (fp != NULL) {
 
		SetWaitCursor (1);
		fscanf(fp,  "%i,\t", &atd_zdrive_a_zd->_autofocus_input0);
		fscanf(fp,  "%i,\t", &atd_zdrive_a_zd->_autofocus_input1);
		fscanf(fp,  "%i,\t", &atd_zdrive_a_zd->_autofocus_laser_I);
		fscanf(fp,  "%i,\t", &atd_zdrive_a_zd->_autofocus_errorRange);
		fscanf(fp,  "%i,\t", &atd_zdrive_a_zd->_autofocus_offsetCoarse);
		fscanf(fp,  "%i,\t", &atd_zdrive_a_zd->_autofocus_offsetFine);
		fscanf(fp,  "%i,\t", &atd_zdrive_a_zd->_autofocus_lowSignalLimit);
		fscanf(fp,  "%i,\t", &atd_zdrive_a_zd->_autofocus_differentialGain);
	
		fclose(fp);    
	
		Set_Autofocus_Ctrl(atd_zdrive_a_zd);  			  //Set control values
	}
	
	if(autofocus_send_vals(atd_zdrive_a_zd) == Z_DRIVE_ERROR)
		return Z_DRIVE_ERROR;
	
	SetWaitCursor (0); 
  
	return Z_DRIVE_SUCCESS;
}	

static int GetAutofocusVals(ATD_ZDRIVE_A* atd_zdrive_a_zd)		   //Running settings
{
	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_INPUT_0, &atd_zdrive_a_zd->_autofocus_input0);
	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_INPUT_1, &atd_zdrive_a_zd->_autofocus_input1); 
	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_ABATTN, &atd_zdrive_a_zd->_autofocus_errorRange); 
	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_LASERCURRENT, &atd_zdrive_a_zd->_autofocus_laser_I); 
	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_OFFSETCOARSE, &atd_zdrive_a_zd->_autofocus_offsetCoarse); 
	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_OFFSETFINE, &atd_zdrive_a_zd->_autofocus_offsetFine); 
	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_LOWSIGNALLIMIT, &atd_zdrive_a_zd->_autofocus_lowSignalLimit); 
	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_DIFFERENTIALGAIN, &atd_zdrive_a_zd->_autofocus_differentialGain); 
		   
	return Z_DRIVE_ERROR;
}

int autofocus_hide_main_ui(ATD_ZDRIVE_A* atd_zdrive_a_zd)
{
	ui_module_hide_main_panel(UIMODULE_CAST(atd_zdrive_a_zd));  

	return Z_DRIVE_SUCCESS;
}
