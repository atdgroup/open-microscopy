#include "ATD_StepperMotor_A.h"
#include "ATD_StepperMotorUI_A.h"
#include <rs232.h>

#include "string_utils.h"
#include "gci_utils.h"
#include "signals.h"

#include "GL_CVIRegistry.h"
#include "toolbox.h"
#include "password.h"

#include <ansi_c.h> 
#include "ATD_UsbInterface_A.h"
#include <utility.h>

////////////////////////////////////////////////////////////////////////////
//RJL/RGN April 2007
//GCI HTP Microscope system. 
//Mirror stepper control.
////////////////////////////////////////////////////////////////////////////

//#define DEBUG

static int GetFullStatus(MirrorStepper* mirror_stepper);

int send_mirror_stepper_error_text (MirrorStepper* mirror_stepper, char fmt[], ...)
{
	char message[512];
	va_list ap;
	va_start(ap, fmt);     
	
	vsprintf(message, fmt, ap);
	logger_log(UIMODULE_LOGGER(mirror_stepper), LOGGER_ERROR, "%s Error: %s", UIMODULE_GET_DESCRIPTION(mirror_stepper), message);  
	
	ui_module_send_valist_error(UIMODULE_CAST(mirror_stepper), "Mirror Stepper Error", fmt, ap);
	
	va_end(ap);  
	
	return MIRROR_STEPPER_SUCCESS;
}


static int default_error_handler (UIModule *module, const char *title, const char *error_string, void *callback_data)     
{
	GCI_MessagePopup("MirrorStepper Error", error_string);  
	
	return UIMODULE_ERROR_NONE;    
}


static void mirror_stepper_read_or_write_main_panel_registry_settings(MirrorStepper *mirror_stepper, int panel, int write)
{
	ui_module_read_or_write_registry_settings(UIMODULE_CAST(mirror_stepper), write);   
}


static int MIRROR_STEPPER_PTR_MARSHALLER (void *handler, void *callback_data, GCI_Signal_Arg* args)
{
	typedef void (*HANDLER) (MirrorStepper*, void *);
	HANDLER func;

	assert(handler != NULL);
	
	func = (HANDLER) handler;
	
	func ( (MirrorStepper *) args[0].void_ptr_data, callback_data);
	
	return SIGNAL_SUCCESS;	
}


void mirror_stepper_on_change(MirrorStepper* mirror_stepper)
{
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(mirror_stepper), "MirrorStepperChanged", GCI_VOID_POINTER, mirror_stepper);  
}


static int Set_Cal_Factors(MirrorStepper* mirror_stepper)
{
	int RB_default_controls[5] = {0, MS_CALPNL_DEFAULT_1, MS_CALPNL_DEFAULT_2, MS_CALPNL_DEFAULT_3, MS_CALPNL_DEFAULT_4};

	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITHOLDCURRENT, mirror_stepper->_init_holdcurrent); 
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITRUNCURRENT, mirror_stepper->_init_runcurrent);
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITMAXVELOCITY, mirror_stepper->_init_maxvelocity); 
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITMINVELOCITY, mirror_stepper->_init_minvelocity);
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITACC, mirror_stepper->_init_acc);
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITSHAFT, mirror_stepper->_init_shaft);
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITRESOLUTION, mirror_stepper->_init_resolution);  
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITACCSHAPE, mirror_stepper->_init_accshape);
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_TARGET_1, mirror_stepper->_target_1); 
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_TARGET_2, mirror_stepper->_target_2); 
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_TARGET_3, mirror_stepper->_target_3); 
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_TARGET_4, mirror_stepper->_target_4);
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET, mirror_stepper->_posoffsets[0]);
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET_1, mirror_stepper->_posoffsets[1]); 
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET_2, mirror_stepper->_posoffsets[2]); 
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET_3, mirror_stepper->_posoffsets[3]); 
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET_4, mirror_stepper->_posoffsets[4]); 
	
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_DEFAULT_1 ,0);
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_DEFAULT_2 ,0); 
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_DEFAULT_3 ,0); 
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_DEFAULT_4 ,0);
	SetCtrlVal(mirror_stepper->_calib_ui_panel, RB_default_controls[mirror_stepper->_default_pos], 1); 
   
   	return MIRROR_STEPPER_SUCCESS;
}


int  mirror_stepper_set_data_dir(MirrorStepper* mirror_stepper, const char *dir)
{
	ui_module_set_data_dir(UIMODULE_CAST(mirror_stepper), dir);

	return MIRROR_STEPPER_SUCCESS;  
}

static int RestoreCalData(MirrorStepper* mirror_stepper)
{
	char dataPath[GCI_MAX_PATHNAME_LEN] = "", data_dir[GCI_MAX_PATHNAME_LEN] = "";  
	char name[UIMODULE_NAME_LEN] = "";
	FILE *fp;
	
	ui_module_get_data_dir(UIMODULE_CAST(mirror_stepper), data_dir);
	ui_module_get_name(UIMODULE_CAST(mirror_stepper), name);
	sprintf(dataPath, "%s\\%s.txt", data_dir, name);
	
  	fp = fopen (dataPath, "r");

	if (fp == NULL)
		return MIRROR_STEPPER_ERROR;
 
	SetWaitCursor (1);
	fscanf(fp,  "%i,\t", &mirror_stepper->_init_holdcurrent);
	fscanf(fp,  "%i,\t", &mirror_stepper->_init_runcurrent);
	fscanf(fp,  "%i,\t", &mirror_stepper->_init_maxvelocity);
	fscanf(fp,  "%i,\t", &mirror_stepper->_init_minvelocity);
	fscanf(fp,  "%i,\t", &mirror_stepper->_init_acc);
	fscanf(fp,  "%i,\t", &mirror_stepper->_init_accshape);
	fscanf(fp,  "%i,\t", &mirror_stepper->_init_shaft);
	fscanf(fp,  "%i,\t", &mirror_stepper->_init_resolution);
	fscanf(fp,  "%i,\t", &mirror_stepper->_target_1);
	fscanf(fp,  "%i,\t", &mirror_stepper->_target_2);
	fscanf(fp,  "%i,\t", &mirror_stepper->_target_3);
	fscanf(fp,  "%i,\t", &mirror_stepper->_target_4);
	fscanf(fp,  "%i,\t", &mirror_stepper->_default_pos);
	fscanf(fp,  "%i,\t", &mirror_stepper->_posoffsets[0]);
	fscanf(fp,  "%i,\t", &mirror_stepper->_posoffsets[1]);  
	fscanf(fp,  "%i,\t", &mirror_stepper->_posoffsets[2]);  
	fscanf(fp,  "%i,\t", &mirror_stepper->_posoffsets[3]);  
	fscanf(fp,  "%i,\t", &mirror_stepper->_posoffsets[4]);  
	
	fclose(fp);    
	
	Set_Cal_Factors(mirror_stepper);  			  //Display calibration factors
	SetWaitCursor (0); 
  
	return MIRROR_STEPPER_SUCCESS;
}	

int DisableSwitches(MirrorStepper* mirror_stepper, int state)
{
	SetCtrlAttribute (mirror_stepper->_main_ui_panel, MS_PANEL_MIRRORPOS ,ATTR_DIMMED, state);
	SetCtrlAttribute (mirror_stepper->_main_ui_panel, MS_PANEL_INIT ,ATTR_DIMMED, state); 
	
	return MIRROR_STEPPER_SUCCESS;
}

int GetInitParamVals(MirrorStepper* mirror_stepper)		//Initalisation settings
{
	int i, selected;
	int RB_default_controls[5] = {0, MS_CALPNL_DEFAULT_1, MS_CALPNL_DEFAULT_2, MS_CALPNL_DEFAULT_3, MS_CALPNL_DEFAULT_4};
	
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITHOLDCURRENT, &mirror_stepper->_init_holdcurrent); 
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITRUNCURRENT, &mirror_stepper->_init_runcurrent);
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITMAXVELOCITY, &mirror_stepper->_init_maxvelocity); 
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITMINVELOCITY, &mirror_stepper->_init_minvelocity);
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITACC, &mirror_stepper->_init_acc);
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITSHAFT, &mirror_stepper->_init_shaft);
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITRESOLUTION, &mirror_stepper->_init_resolution);  
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INITACCSHAPE, &mirror_stepper->_init_accshape);
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_TARGET_1, &mirror_stepper->_target_1); 
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_TARGET_2, &mirror_stepper->_target_2); 
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_TARGET_3, &mirror_stepper->_target_3); 
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_TARGET_4, &mirror_stepper->_target_4);
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET, &mirror_stepper->_posoffsets[0]); 
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET_1, &mirror_stepper->_posoffsets[1]); 
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET_2, &mirror_stepper->_posoffsets[2]); 
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET_3, &mirror_stepper->_posoffsets[3]); 
	GetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET_4, &mirror_stepper->_posoffsets[4]); 
	
	for (i = 1; i < 5; i++) {
		GetCtrlVal(mirror_stepper->_calib_ui_panel, RB_default_controls[i], &selected);
		if (selected) {
			mirror_stepper->_default_pos = i;
			break;
		}
	}
		
	return MIRROR_STEPPER_SUCCESS;
}


static int SetInitMotorParam(MirrorStepper* mirror_stepper)
{
	unsigned char data[8];

   	data[0]=0x89;										//Command for SetMotorParam
   	data[1]=0xFF;	       			 
   	data[2]=0xFF;
   	data[3]=((mirror_stepper->_init_runcurrent<<4) | mirror_stepper->_init_holdcurrent);	//Value ofIrun and Ihold
   	data[4]=((mirror_stepper->_init_maxvelocity<<4) | mirror_stepper->_init_minvelocity);	//Value of Velocity_max and Velocity_min
   	data[5]=(0<<5|(mirror_stepper->_init_shaft<<4)|mirror_stepper->_init_acc);				//Acceleration shape /shaft rotation/ secue position top 3 bits
   	data[6]=0;				   																//Secure position bits 0 to 7
   	data[7]=mirror_stepper->_init_accshape<<4 | mirror_stepper->_init_resolution<<2;		//Acceleration shape /stepmode
   		
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_i2c_write_bytes(mirror_stepper->_controller, mirror_stepper->_i2c_chip_address, 8, data) != FT_OK) {
		return MIRROR_STEPPER_ERROR;
	}

	#else

	if(GCI_Out_PIC_multiPort (mirror_stepper->_com_port, 
		mirror_stepper->_i2c_bus, mirror_stepper->_i2c_chip_type, mirror_stepper->_i2c_chip_address, 8, data) < 0) {

		return MIRROR_STEPPER_ERROR;
	}

	#endif

	return MIRROR_STEPPER_SUCCESS;
}

static int wait_move_complete(MirrorStepper* mirror_stepper)
{
	double start_time = Timer(), time_taken;

	#ifdef FTDI_NO_VIRTUAL_COMPORT
	ftdi_controller_get_lock(mirror_stepper->_controller);
	#else
	GetI2CPortLock(mirror_stepper->_com_port, "Mirror wait_move_complete");    
	#endif

	mirror_stepper->_motion = 1;
	
	time_taken = 0.0;

   	// Allow time to move:wait until motion = 0  
	while (mirror_stepper->_motion != 0 && (time_taken <= 5.0)){	
		
		if(mirror_stepper->_move_abort > 0) {

			#ifdef FTDI_NO_VIRTUAL_COMPORT
			ftdi_controller_release_lock(mirror_stepper->_controller);
			#else
			ReleaseI2CPortLock(mirror_stepper->_com_port, "Mirror wait_move_complete"); 
			#endif

			return MIRROR_STEPPER_ERROR;
		}

		if (GetFullStatus(mirror_stepper) == MIRROR_STEPPER_ERROR) {
			
			#ifdef FTDI_NO_VIRTUAL_COMPORT
			ftdi_controller_release_lock(mirror_stepper->_controller);
			#else
			ReleaseI2CPortLock(mirror_stepper->_com_port, "Mirror wait_move_complete"); 
			#endif

			return MIRROR_STEPPER_ERROR;
		}
		
		time_taken = Timer() - start_time;
		ProcessSystemEvents ();  
	} 

	if(time_taken > 5.0) {
		logger_log(UIMODULE_LOGGER(mirror_stepper), LOGGER_ERROR, "The mirror stepper failed complete wait_move_complete before timeout");
	}
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT
	ftdi_controller_release_lock(mirror_stepper->_controller);
	#else
	ReleaseI2CPortLock(mirror_stepper->_com_port, "Mirror wait_move_complete"); 
	#endif
	
	return MIRROR_STEPPER_SUCCESS;
}


static int SetPosition(MirrorStepper* mirror_stepper, int position)
{
	char data[10];

	if (position < 0)
		position = (position | 0x80);   //To get +/- 32767  
   				
   	data[0]=0x8B;		   			//SetPosition command 
   	data[1]=0xFF;	       			// 
   	data[2]=0xFF;					//
   	data[3]=position>>8;			//MSB position
   	data[4]=position & 0xFF;		//LSB position
   		
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_i2c_write_bytes(mirror_stepper->_controller, mirror_stepper->_i2c_chip_address, 5, data) != FT_OK) {
		return MIRROR_STEPPER_ERROR;
	}

	#else

	if (GCI_Out_PIC_multiPort (mirror_stepper->_com_port, mirror_stepper->_i2c_bus,
		mirror_stepper->_i2c_chip_type, mirror_stepper->_i2c_chip_address, 5, data))
		return MIRROR_STEPPER_ERROR;

	#endif

	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INIT, 1);
	
	if (wait_move_complete(mirror_stepper))
		return MIRROR_STEPPER_ERROR;
	
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INIT, 0);

	return MIRROR_STEPPER_SUCCESS;
}

static int GetActualPosition(MirrorStepper* mirror_stepper, unsigned short *position)
{
	unsigned char val1[10];
	int address1;
	int position1, position2;

	memset(val1, 0, 8);
		
	#ifdef FTDI_NO_VIRTUAL_COMPORT

   	val1[0] = 0xFC;		   			//GetFullStatus2 command

	ftdi_controller_get_lock(mirror_stepper->_controller);
	
	if(ftdi_controller_i2c_write_bytes(mirror_stepper->_controller, mirror_stepper->_i2c_chip_address, 1, val1) != FT_OK) {
		ftdi_controller_release_lock(mirror_stepper->_controller);
		return MIRROR_STEPPER_ERROR;
	}

	if(ftdi_controller_i2c_read_bytes(mirror_stepper->_controller, mirror_stepper->_i2c_chip_address, 8, val1) != FT_OK) {
		ftdi_controller_release_lock(mirror_stepper->_controller);
		return MIRROR_STEPPER_ERROR;
	}

	ftdi_controller_release_lock(mirror_stepper->_controller);
	
	#else

	val1[0] = mirror_stepper->_i2c_chip_type;	//Address
   	val1[1] = 0xFC;		   			//GetFullStatus2 command

	GetI2CPortLock(mirror_stepper->_com_port, "Mirror GetActualPosition");    
	
	if (GCI_writeI2C_multiPort(mirror_stepper->_com_port, 2, val1, mirror_stepper->_i2c_bus, "Mirror GetActualPosition")) {
		ReleaseI2CPortLock(mirror_stepper->_com_port, "Mirror GetActualPosition");  
   		return MIRROR_STEPPER_ERROR;
	}
   			
   	val1[0] = mirror_stepper->_i2c_chip_type | 0x01;
   				
	if (GCI_readI2C_multiPort(mirror_stepper->_com_port, 8, val1, mirror_stepper->_i2c_bus, "Mirror GetActualPosition")) {	//Read response
		ReleaseI2CPortLock(mirror_stepper->_com_port, "Mirror GetActualPosition");  
    	return MIRROR_STEPPER_ERROR; 
	}

	ReleaseI2CPortLock(mirror_stepper->_com_port, "Mirror GetActualPosition");               
	
	#endif

    address1 = val1[0] & 0xff; 
	position1 = val1[1] & 0xff;   
	position2 = val1[2] & 0xff;
	
	*position = ((unsigned short) position1) << 8;
	*position |= position2;
	
    return MIRROR_STEPPER_SUCCESS;	
}


static int HardStop(MirrorStepper* mirror_stepper)
{
	unsigned char val1[2];
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

   	val1[0] = 0x84;		   			//HardStop command

	if(ftdi_controller_i2c_write_bytes(mirror_stepper->_controller, mirror_stepper->_i2c_chip_address, 1, val1) != FT_OK) {
		return MIRROR_STEPPER_ERROR;
	}

	#else

	val1[0] = mirror_stepper->_i2c_chip_type;	//Address
   	val1[1] = 0x84;		   			//HardStop command

   	if (GCI_writeI2C_multiPort(mirror_stepper->_com_port, 2, val1, mirror_stepper->_i2c_bus, "Mirror HardStop"))
   		return MIRROR_STEPPER_ERROR; 
   	
	#endif

	return MIRROR_STEPPER_SUCCESS; 
}

static int RecoverFromDeadLock(MirrorStepper* mirror_stepper)
{
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	ftdi_controller_get_lock(mirror_stepper->_controller);

	if(HardStop(mirror_stepper) == MIRROR_STEPPER_ERROR) {
		ftdi_controller_release_lock(mirror_stepper->_controller);
		return MIRROR_STEPPER_ERROR;
	}
	
	if(GetFullStatus(mirror_stepper) == MIRROR_STEPPER_ERROR) {
		ftdi_controller_release_lock(mirror_stepper->_controller);
		return MIRROR_STEPPER_ERROR;
	}

	ftdi_controller_release_lock(mirror_stepper->_controller);

	#else

	GetI2CPortLock(mirror_stepper->_com_port, "Mirror RecoverFromDeadLock");   
	
	if(HardStop(mirror_stepper) == MIRROR_STEPPER_ERROR) {
		ReleaseI2CPortLock(mirror_stepper->_com_port, "Mirror RecoverFromDeadLock");  
		return MIRROR_STEPPER_ERROR;
	}
	
	if(GetFullStatus(mirror_stepper) == MIRROR_STEPPER_ERROR) {
		ReleaseI2CPortLock(mirror_stepper->_com_port, "Mirror RecoverFromDeadLock");  
		return MIRROR_STEPPER_ERROR;
	}
	
	ReleaseI2CPortLock(mirror_stepper->_com_port, "Mirror RecoverFromDeadLock");
	
	#endif

	return MIRROR_STEPPER_SUCCESS; 
}

static int runInit(MirrorStepper* mirror_stepper, int target_A,int target_B)
{
	unsigned char data[20];
	unsigned short position;
	int msb_target_A,lsb_target_A,msb_target_B,lsb_target_B;
		  
	memset(data, 0, 20);
	
	GetActualPosition(mirror_stepper, &position);   
	
	if(position == target_A)
		target_A++;
	
	msb_target_A=target_A>>8;
	lsb_target_A=target_A & 0xff;
	msb_target_B=target_B>>8;
	lsb_target_B=target_B & 0xff;
		    
   	data[0] = 0x88;		   					//RunInit command
   	data[1] = 0xFF;
   	data[2] = 0xFF;
   	data[3] = (mirror_stepper->_init_maxvelocity<<4 | mirror_stepper->_init_minvelocity);	//Vmax/Vmin
   	data[4] = msb_target_A;		  			//Target position 1 (2 bytes)
   	data[5] = lsb_target_A;
   	data[6] = msb_target_B;		  			//Target position 2 (2 bytes)  
   	data[7] = lsb_target_B;
   	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	if(ftdi_controller_i2c_write_bytes(mirror_stepper->_controller, mirror_stepper->_i2c_chip_address, 8, data) != FT_OK) {
		return MIRROR_STEPPER_ERROR;
	}

	#else

	if(GCI_Out_PIC_multiPort (mirror_stepper->_com_port, mirror_stepper->_i2c_bus,
		mirror_stepper->_i2c_chip_type, mirror_stepper->_i2c_chip_address, 8, data) < 0) {
		return MIRROR_STEPPER_ERROR;
	}

	#endif

	return MIRROR_STEPPER_SUCCESS;
}


static int GetFullStatus(MirrorStepper* mirror_stepper)
{
	unsigned char val1[10];
	int address1;
	int IrunIhold,VmaxVmin,status1,status2,status3,na1,na2;

	memset(val1, 0, 10);
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

   	val1[0] = 0x81;		   			//GetFullStatus command

	ftdi_controller_get_lock(mirror_stepper->_controller);
	
	if(ftdi_controller_i2c_write_bytes(mirror_stepper->_controller, mirror_stepper->_i2c_chip_address, 1, val1) != FT_OK) {
		ftdi_controller_release_lock(mirror_stepper->_controller);
		return MIRROR_STEPPER_ERROR;
	}

	if(ftdi_controller_i2c_read_bytes(mirror_stepper->_controller, mirror_stepper->_i2c_chip_address, 9, val1) != FT_OK) {
		ftdi_controller_release_lock(mirror_stepper->_controller);
		return MIRROR_STEPPER_ERROR;
	}

	ftdi_controller_release_lock(mirror_stepper->_controller);
	
	#else

	val1[0] = mirror_stepper->_i2c_chip_type;	//Address
   	val1[1] = 0x81;		   			//GetFullStatus command

	GetI2CPortLock(mirror_stepper->_com_port, "Mirror GetFullStatus");    
	
   	if (GCI_writeI2C_multiPort(mirror_stepper->_com_port, 2, val1, mirror_stepper->_i2c_bus, "Mirror GetFullStatus GCI_writeI2C_multiPort") < 0)
   	{
		ReleaseI2CPortLock(mirror_stepper->_com_port, "Mirror GetFullStatus");  	
		return MIRROR_STEPPER_ERROR; 	
	}
   			
   	val1[0] = mirror_stepper->_i2c_chip_type | 0x01;
   				
    if (GCI_readI2C_multiPort(mirror_stepper->_com_port, 9, val1, mirror_stepper->_i2c_bus, "Mirror GetFullStatus GCI_readI2C_multiPort") < 0)	//Read response
	{
		ReleaseI2CPortLock(mirror_stepper->_com_port, "Mirror GetFullStatus");  	
		return MIRROR_STEPPER_ERROR; 	
	}
	
	ReleaseI2CPortLock(mirror_stepper->_com_port, "Mirror GetFullStatus");  
	
	#endif

	address1 = val1[0] & 0xff; 
    IrunIhold = val1[1] & 0xff; 
    VmaxVmin = val1[2] & 0xff; 
    status1= val1[3] & 0xff;
    status2= val1[4] & 0xff;
    status3= val1[5] & 0xff;
    na1= val1[6] & 0xff;
    na2= val1[7] & 0xff;
    			
	//printf("val array - [%d %d %d %d %d %d %d %d\n]", val1[0], val1[1], val1[2], val1[3], val1[4], val1[5], val1[6], val1[7]);
	//printf("address1 %d\nIrunIhold %d\nVmaxVmin %d\nstatus1 %d\nstatus2 %d\nstatus3 %d\nna1 %d\nna2 %d\n", 
	//	address1, IrunIhold, VmaxVmin, status1, status2, status3, na1, na2);

    mirror_stepper->_motion = status3 & 0xE0;	  //Motion status	 
    mirror_stepper->_motion = mirror_stepper->_motion>>5;

    return MIRROR_STEPPER_SUCCESS;	
}


int mirror_stepper_set_offset(MirrorStepper* mirror_stepper, int mirrorpos, int offset)
{
	int targetposition;

	mirror_stepper->_current_pos = mirrorpos;
	mirror_stepper->_posoffsets[mirrorpos] = offset;

	if (mirrorpos>0)  // modifying an individual offset
	{	
		SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INIT, 1);

		targetposition = (mirror_stepper->_posoffsets[0] + offset + ((mirrorpos-1)*(3200/4)));
		GetInitParamVals(mirror_stepper);		 	//Get running parameter settings
		if (SetInitMotorParam(mirror_stepper))
	   		goto Error;
		if (SetPosition(mirror_stepper, targetposition)) 	  	//Set target position
	   		goto Error;
	}
	
	return MIRROR_STEPPER_SUCCESS;
	
Error:

	DisableSwitches(mirror_stepper, 0); 	//Enable panel switches 
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INIT, 0);
	return MIRROR_STEPPER_ERROR;
	
}

static int CVICALLBACK mirror_stepper_set_pos_thread(void *callback)
{
	MirrorStepper* mirror_stepper = (MirrorStepper*) callback;
	unsigned short current_target_pos = 0;
	int target_pos;

	DisableSwitches(mirror_stepper, 1); //Disable panel switches   

//	GetInitParamVals(mirror_stepper);	//Get initalisation parameter settings

	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INIT, 1);
	
	// Lets check that we are not already in the target position as that
	// can cause the stepper to go into a dead locked state.
	GetActualPosition(mirror_stepper, &current_target_pos);  
	
	target_pos = (mirror_stepper->_posoffsets[0] 
		+ mirror_stepper->_posoffsets[mirror_stepper->_requested_pos] + ((mirror_stepper->_requested_pos-1)*(3200/4)));  
	
	if(current_target_pos == target_pos)
		goto Success;    
	
	if (SetPosition(mirror_stepper, target_pos) == MIRROR_STEPPER_ERROR) 	  	//Set target position
   		goto Error;

	mirror_stepper->_current_pos = mirror_stepper->_requested_pos;
	
	DisableSwitches(mirror_stepper, 0); 	// Enable panel switches 
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INIT, 0);
	
	mirror_stepper_on_change(mirror_stepper);
          
Success:

	mirror_stepper->_move_abort = 0;

	return MIRROR_STEPPER_SUCCESS;

Error:
	
	ui_module_send_error(UIMODULE_CAST(mirror_stepper), "Mirror Stepper Error", "Mirror Stepper failed to move to requested position");

	mirror_stepper->_current_pos = -1;
	
	return MIRROR_STEPPER_ERROR;
}

static int abort_move_thread(MirrorStepper* mirror_stepper)
{
	int thread_status;
	double start_time = Timer();

	mirror_stepper->_move_abort = 1;

	CmtGetThreadPoolFunctionAttribute (gci_thread_pool(), mirror_stepper->_set_pos_thread_id,
			ATTR_TP_FUNCTION_EXECUTION_STATUS, &thread_status);

	// Thread no longer executing. Must be shutdown time.
	if(thread_status >= kCmtThreadFunctionPostExecution) {
		mirror_stepper->_move_abort = 0;
		return MIRROR_STEPPER_SUCCESS;
	}

	while(mirror_stepper->_move_abort == 1) {

		if((Timer() - start_time) > 10.0) {
			logger_log(UIMODULE_LOGGER(mirror_stepper), LOGGER_ERROR, "The mirror stepper failed to abort thread");
			mirror_stepper->_move_abort = 0;
			return MIRROR_STEPPER_ERROR;
		}

		if(thread_status >= kCmtThreadFunctionPostExecution) {
			mirror_stepper->_move_abort = 0;
			return MIRROR_STEPPER_SUCCESS;
		}

		Delay(0.1);
		ProcessSystemEvents();
	}

	mirror_stepper->_move_abort = 0;

	return MIRROR_STEPPER_SUCCESS;
}

int mirror_stepper_set_pos(MirrorStepper* mirror_stepper, int mirrorpos)
{
	unsigned short current_target_pos = 0; 	
	
	mirror_stepper->_requested_pos = mirrorpos;

	if(mirror_stepper->_set_pos_thread_id >= 0)
		abort_move_thread(mirror_stepper);

	CmtScheduleThreadPoolFunction (gci_thread_pool(), mirror_stepper_set_pos_thread, mirror_stepper, &mirror_stepper->_set_pos_thread_id);
	CmtWaitForThreadPoolFunctionCompletion(gci_thread_pool(), mirror_stepper->_set_pos_thread_id, OPT_TP_PROCESS_EVENTS_WHILE_WAITING);
	CmtReleaseThreadPoolFunctionID(gci_thread_pool(), mirror_stepper->_set_pos_thread_id);
	mirror_stepper->_set_pos_thread_id = -1;

	Delay(0.1);

	return MIRROR_STEPPER_SUCCESS;
}

int mirror_stepper_get_pos(MirrorStepper* mirror_stepper)
{
	return mirror_stepper->_current_pos;
}

int mirror_stepper_initialise(MirrorStepper* mirror_stepper, int move_to_default)
{
	unsigned char val1[20] = "";
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	ftdi_controller_get_lock(mirror_stepper->_controller);
	
	GetInitParamVals(mirror_stepper);

   	val1[0] = 0x81;		   						//GetFullStatus command

	if(RecoverFromDeadLock(mirror_stepper) == MIRROR_STEPPER_ERROR)
		goto Error; 	

	if(ftdi_controller_i2c_write_bytes(mirror_stepper->_controller, mirror_stepper->_i2c_chip_address, 1, val1) != FT_OK) {
		ftdi_controller_release_lock(mirror_stepper->_controller);
		return MIRROR_STEPPER_ERROR;
	}

	if (SetInitMotorParam(mirror_stepper)) {
   		goto Error;
	}

	mirror_stepper->_init_mirror_pos_flag = 0;			//Reset flag

	//Set to initial position. 
	//Have to do this twice as first attempt can be inaccurate depending on where it starts from.
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INIT, 1);
	
	if (runInit(mirror_stepper, mirror_stepper->_target_1, mirror_stepper->_target_2))		//Run init 1
   		goto Error;
 
	if (wait_move_complete(mirror_stepper) == MIRROR_STEPPER_ERROR)
   		goto Error;
 
	
	if (runInit(mirror_stepper, mirror_stepper->_target_1, mirror_stepper->_target_2))		//Run init 1
   		goto Error;

	Delay(0.5);
	
	if (wait_move_complete(mirror_stepper) == MIRROR_STEPPER_ERROR)
   		goto Error;

	ftdi_controller_release_lock(mirror_stepper->_controller);
	

	#else

	GetI2CPortLock(mirror_stepper->_com_port, "Mirror mirror_stepper_initialise");  
	
	GetInitParamVals(mirror_stepper);

	val1[0] = mirror_stepper->_i2c_chip_type;	//Address
   	val1[1] = 0x81;		   						//GetFullStatus command

	if(RecoverFromDeadLock(mirror_stepper) == MIRROR_STEPPER_ERROR)
		goto Error; 	
	
   	if (GCI_writeI2C_multiPort(mirror_stepper->_com_port, 2, val1, mirror_stepper->_i2c_bus, "Mirror mirror_stepper_initialise")) { //Needs this to activate motor
		goto Error;
	}

	if (SetInitMotorParam(mirror_stepper)) {
   		goto Error;
	}

	mirror_stepper->_init_mirror_pos_flag = 0;			//Reset flag

	//Set to initial position. 
	//Have to do this twice as first attempt can be inaccurate depending on where it starts from.
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INIT, 1);
	
	if (runInit(mirror_stepper, mirror_stepper->_target_1, mirror_stepper->_target_2))		//Run init 1
   		goto Error;
 
	if (wait_move_complete(mirror_stepper) == MIRROR_STEPPER_ERROR)
   		goto Error;
 
	
	if (runInit(mirror_stepper, mirror_stepper->_target_1, mirror_stepper->_target_2))		//Run init 1
   		goto Error;

	Delay(0.5);
	
	if (wait_move_complete(mirror_stepper) == MIRROR_STEPPER_ERROR)
   		goto Error;

	ReleaseI2CPortLock(mirror_stepper->_com_port, "Mirror mirror_stepper_initialise"); 
	
	#endif

	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INIT, 0);

	if(move_to_default)
		mirror_stepper_set_pos(mirror_stepper, mirror_stepper->_default_pos);	
	
	return MIRROR_STEPPER_SUCCESS;

#ifdef FTDI_NO_VIRTUAL_COMPORT

Error:
	
	ftdi_controller_release_lock(mirror_stepper->_controller);
	DisableSwitches(mirror_stepper, 0); 	//Enable panel switches 
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INIT, 0);
	return MIRROR_STEPPER_ERROR;


#else

Error:
	
	ReleaseI2CPortLock(mirror_stepper->_com_port, "Mirror mirror_stepper_initialise"); 
	DisableSwitches(mirror_stepper, 0); 	//Enable panel switches 
	SetCtrlVal(mirror_stepper->_calib_ui_panel, MS_CALPNL_INIT, 0);
	return MIRROR_STEPPER_ERROR;

#endif
}

int mirror_stepper_init (MirrorStepper* mirror_stepper)
{
	mirror_stepper->_current_pos = 1;

	if (RestoreCalData(mirror_stepper) == MIRROR_STEPPER_ERROR) {
		GCI_MessagePopup("Error", "Error loading file for mirror stepper"); 	
		return MIRROR_STEPPER_ERROR;
	}
  	
  	return MIRROR_STEPPER_SUCCESS;
}


int mirror_stepper_signal_hide_handler_connect (MirrorStepper* mirror_stepper, MIRROR_STEPPER_EVENT_HANDLER handler, void *callback_data)
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(mirror_stepper), "Hide", handler, callback_data) == SIGNAL_ERROR) {
		send_mirror_stepper_error_text(mirror_stepper, "Can not connect signal handler for MirrorStepper Hide signal");
		return MIRROR_STEPPER_ERROR;
	}

	return MIRROR_STEPPER_SUCCESS;
}


int mirror_stepper_changed_handler_connect(MirrorStepper* mirror_stepper, MIRROR_STEPPER_EVENT_HANDLER handler, void *data )
{
	if( GCI_Signal_Connect(UIMODULE_SIGNAL_TABLE(mirror_stepper), "MirrorStepperChanged", handler, data) == SIGNAL_ERROR) {
		send_mirror_stepper_error_text(mirror_stepper, "Can not connect signal handler for MirrorStepper Change signal");
		return MIRROR_STEPPER_ERROR;
	}

	return MIRROR_STEPPER_SUCCESS;
}

MirrorStepper* mirror_stepper_new(const char *name, const char *description)
{
	MirrorStepper* mirror_stepper = (MirrorStepper*) malloc(sizeof(MirrorStepper));
	
	memset(mirror_stepper, 0, sizeof(MirrorStepper));

	ui_module_constructor(UIMODULE_CAST(mirror_stepper), name);         
	ui_module_set_description(UIMODULE_CAST(mirror_stepper), description);      

	mirror_stepper_set_description(mirror_stepper, description);
    mirror_stepper_set_name(mirror_stepper, name);
	
	mirror_stepper->_set_pos_thread_id = -1;
	mirror_stepper->_current_pos = -1;
	mirror_stepper->_requested_pos = -1;
	mirror_stepper->_motion = 0;
	mirror_stepper->_move_abort = 0;
	mirror_stepper->_main_ui_panel = -1;
	mirror_stepper->_calib_ui_panel = -1;
	mirror_stepper->_current_pos = 1;
	
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(mirror_stepper), "Hide", MIRROR_STEPPER_PTR_MARSHALLER);
	GCI_Signal_New(UIMODULE_SIGNAL_TABLE(mirror_stepper), "MirrorStepperChanged", MIRROR_STEPPER_PTR_MARSHALLER);

	mirror_stepper_set_error_handler(mirror_stepper, default_error_handler);

	mirror_stepper->_main_ui_panel = ui_module_add_panel(UIMODULE_CAST(mirror_stepper), "ATD_StepperMotorUI_A.uir", MS_PANEL, 1);
 	mirror_stepper->_calib_ui_panel = ui_module_add_panel(UIMODULE_CAST(mirror_stepper), "ATD_StepperMotorUI_A.uir", MS_CALPNL, 0);
	
    if ( InstallCtrlCallback (mirror_stepper->_main_ui_panel, MS_PANEL_CAL, cb_mirror_stepper_cal, mirror_stepper) < 0)
		return NULL;
  	
    if ( InstallCtrlCallback (mirror_stepper->_main_ui_panel, MS_PANEL_INIT, cb_mirror_stepper_init, mirror_stepper) < 0)
		return NULL; 
	
    if ( InstallCtrlCallback (mirror_stepper->_main_ui_panel, MS_PANEL_MIRRORPOS, cb_mirror_stepper_setpos, mirror_stepper) < 0)
		return NULL; 
  	
    if ( InstallCtrlCallback (mirror_stepper->_main_ui_panel, MS_PANEL_QUIT, cb_mirror_stepper_quit, mirror_stepper) < 0)
		return NULL; 
  	
	SetPanelAttribute (mirror_stepper->_main_ui_panel, ATTR_TITLE, mirror_stepper->_description);
  	
    if ( InstallCtrlCallback (mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET, cb_mirror_stepper_posoffset, mirror_stepper) < 0)
		return NULL; 
	
	if ( InstallCtrlCallback (mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET_1, cb_mirror_stepper_posoffset1, mirror_stepper) < 0)
		return NULL; 
	
	if ( InstallCtrlCallback (mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET_2, cb_mirror_stepper_posoffset2, mirror_stepper) < 0)
		return NULL; 
	
	if ( InstallCtrlCallback (mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET_3, cb_mirror_stepper_posoffset3, mirror_stepper) < 0)
		return NULL; 
	
	if ( InstallCtrlCallback (mirror_stepper->_calib_ui_panel, MS_CALPNL_POSOFFSET_4, cb_mirror_stepper_posoffset4, mirror_stepper) < 0)
		return NULL; 
	
    if ( InstallCtrlCallback (mirror_stepper->_calib_ui_panel, MS_CALPNL_DEFAULT_1, cb_mirror_stepper_set_default, mirror_stepper) < 0)
		return NULL; 
  	
    if ( InstallCtrlCallback (mirror_stepper->_calib_ui_panel, MS_CALPNL_DEFAULT_2, cb_mirror_stepper_set_default, mirror_stepper) < 0)
		return NULL; 
  	
    if ( InstallCtrlCallback (mirror_stepper->_calib_ui_panel, MS_CALPNL_DEFAULT_3, cb_mirror_stepper_set_default, mirror_stepper) < 0)
		return NULL; 
  	
    if ( InstallCtrlCallback (mirror_stepper->_calib_ui_panel, MS_CALPNL_DEFAULT_4, cb_mirror_stepper_set_default, mirror_stepper) < 0)
		return NULL; 
  	
    if ( InstallCtrlCallback (mirror_stepper->_calib_ui_panel, MS_CALPNL_CLOSE_CAL, cb_mirror_stepper_close_cal, mirror_stepper) < 0)
		return NULL; 
  	
    if ( InstallCtrlCallback (mirror_stepper->_calib_ui_panel, MS_CALPNL_INIT_2, cb_mirror_stepper_init, mirror_stepper) < 0)
		return NULL; 
	
	if ( InstallCtrlCallback (mirror_stepper->_calib_ui_panel, MS_CALPNL_SAVE, cb_mirror_stepper_save, mirror_stepper) < 0)
		return NULL; 

	return mirror_stepper;
}

int mirror_stepper_save_data(MirrorStepper* mirror_stepper)
{
	char dataPath[GCI_MAX_PATHNAME_LEN] = "", data_dir[GCI_MAX_PATHNAME_LEN] = "";  
	char name[UIMODULE_NAME_LEN] = "", name_with_ext[UIMODULE_NAME_LEN] = "";
	FILE *fp=NULL;
  
	ui_module_get_name(UIMODULE_CAST(mirror_stepper), name);

	sprintf(name_with_ext, "%s.txt", name);

//	if (FileSelectPopup (UIMODULE_GET_DATA_DIR(mirror_stepper), name_with_ext, "*.txt", "Save Parameters", VAL_SAVE_BUTTON, 0, 1, 1, 1, dataPath) <= 0)
//		return MIRROR_STEPPER_ERROR;

	strcpy(dataPath, UIMODULE_GET_DATA_DIR(UIMODULE_CAST(mirror_stepper)));
	strcat(dataPath, "\\");
	strcat(dataPath, name_with_ext);
	
	GetInitParamVals(mirror_stepper);
	GetInitParamVals(mirror_stepper);

  	fp = fopen (dataPath, "w");

	if (fp == NULL){
		GCIDialog(0, "Configuration Save Error", IDI_EXCLAMATION, GCI_OK_BUTTON, "Configuration FAILED to save to:\n%s", dataPath);
		return MIRROR_STEPPER_ERROR;
	}
	
	fprintf(fp,  "%i,\t", mirror_stepper->_init_holdcurrent);
	fprintf(fp,  "%i,\t", mirror_stepper->_init_runcurrent);
	fprintf(fp,  "%i,\t", mirror_stepper->_init_maxvelocity);
	fprintf(fp,  "%i,\t", mirror_stepper->_init_minvelocity);
	fprintf(fp,  "%i,\t", mirror_stepper->_init_acc);
	fprintf(fp,  "%i,\t", mirror_stepper->_init_accshape);
	fprintf(fp,  "%i,\t", mirror_stepper->_init_shaft);
	fprintf(fp,  "%i,\t", mirror_stepper->_init_resolution);
	fprintf(fp,  "%i,\t", mirror_stepper->_target_1);
	fprintf(fp,  "%i,\t", mirror_stepper->_target_2);
	fprintf(fp,  "%i,\t", mirror_stepper->_target_3);
	fprintf(fp,  "%i,\t", mirror_stepper->_target_4);
	fprintf(fp,  "%i,\t", mirror_stepper->_default_pos);
	fprintf(fp,  "%i,\t", mirror_stepper->_posoffsets[0]);
	fprintf(fp,  "%i,\t", mirror_stepper->_posoffsets[1]);      
	fprintf(fp,  "%i,\t", mirror_stepper->_posoffsets[2]);      
	fprintf(fp,  "%i,\t", mirror_stepper->_posoffsets[3]);      
	fprintf(fp,  "%i,\t", mirror_stepper->_posoffsets[4]);      
	
	fclose(fp);    
  
	GCIDialog(0, "Configuration Saved", IDI_INFORMATION, GCI_OK_BUTTON, "Configuration saved successfully to:\n%s", dataPath);

	return MIRROR_STEPPER_SUCCESS;
}	

int mirror_stepper_destroy(MirrorStepper* mirror_stepper)
{
	ui_module_destroy(UIMODULE_CAST(mirror_stepper));

  	free(mirror_stepper);
  	
  	return MIRROR_STEPPER_SUCCESS;
}


int mirror_stepper_set_com_port_i2c_details(MirrorStepper* mirror_stepper, int port, int pic, int bus, int address)
{
	mirror_stepper->_com_port = port;
	mirror_stepper->_i2c_chip_type = pic; 
	mirror_stepper->_i2c_bus = bus;
	mirror_stepper->_i2c_chip_address = address;

	return MIRROR_STEPPER_SUCCESS;  
}

int mirror_stepper_set_ftdi_controller(MirrorStepper* mirror_stepper, FTDIController* controller, int pic, int bus, int address)
{
	mirror_stepper->_controller = controller;
	mirror_stepper->_i2c_chip_type = pic; 
	mirror_stepper->_i2c_bus = bus;
	mirror_stepper->_i2c_chip_address = address;

	return MIRROR_STEPPER_SUCCESS;  
}

void mirror_stepper_set_error_handler(MirrorStepper* mirror_stepper, UI_MODULE_ERROR_HANDLER handler)
{
	ui_module_set_error_handler(UIMODULE_CAST(mirror_stepper), handler, mirror_stepper);  
}


int  mirror_stepper_set_description(MirrorStepper* mirror_stepper, const char* description)
{
	ui_module_set_description(UIMODULE_CAST(mirror_stepper), description);
   
  	return MIRROR_STEPPER_SUCCESS;
}


int  mirror_stepper_get_description(MirrorStepper* mirror_stepper, char *description)
{
	ui_module_get_description(UIMODULE_CAST(mirror_stepper), description);

    return MIRROR_STEPPER_SUCCESS;
}


int mirror_stepper_set_name(MirrorStepper* mirror_stepper, const char* name)
{
	ui_module_set_name(UIMODULE_CAST(mirror_stepper), name);
  	
  	return MIRROR_STEPPER_SUCCESS;
}


int mirror_stepper_get_name(MirrorStepper* mirror_stepper, char *name)
{
  	ui_module_set_name(UIMODULE_CAST(mirror_stepper), name);
  
  	return MIRROR_STEPPER_SUCCESS;
}


int mirror_stepper_display_main_ui(MirrorStepper* mirror_stepper)
{
	if(mirror_stepper->_main_ui_panel != -1) {
		mirror_stepper_read_or_write_main_panel_registry_settings(mirror_stepper, mirror_stepper->_main_ui_panel, 0); 
		DisplayPanel(mirror_stepper->_main_ui_panel);
	}
	
	return MIRROR_STEPPER_SUCCESS;
}

int mirror_stepper_hide_main_ui(MirrorStepper* mirror_stepper)
{
	if(mirror_stepper->_main_ui_panel != -1) {
		mirror_stepper_read_or_write_main_panel_registry_settings(mirror_stepper, mirror_stepper->_main_ui_panel, 1);
		HidePanel(mirror_stepper->_main_ui_panel);
	}

	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(mirror_stepper), "Hide", GCI_VOID_POINTER, mirror_stepper); 

	return MIRROR_STEPPER_SUCCESS;
}

int mirror_stepper_is_main_ui_visible(MirrorStepper* mirror_stepper)
{
	int visible;
	
	GetPanelAttribute(mirror_stepper->_main_ui_panel, ATTR_VISIBLE, &visible);
	
	return visible;
}

int mirror_stepper_display_calib_ui(MirrorStepper* mirror_stepper, int parent_panel)
{
	if(mirror_stepper->_calib_ui_panel != -1) {
		mirror_stepper_read_or_write_main_panel_registry_settings(mirror_stepper, mirror_stepper->_calib_ui_panel, 0); 
		GCI_ShowPasswordProtectedPanel(mirror_stepper->_calib_ui_panel, parent_panel);
		//DisplayPanel(mirror_stepper->_calib_ui_panel);
	}
	
	return MIRROR_STEPPER_SUCCESS;
}

int mirror_stepper_hide_calib_ui(MirrorStepper* mirror_stepper)
{
	if(mirror_stepper->_calib_ui_panel != -1) {
		mirror_stepper_read_or_write_main_panel_registry_settings(mirror_stepper, mirror_stepper->_calib_ui_panel, 1);
		HidePanel(mirror_stepper->_calib_ui_panel);
	}

	return MIRROR_STEPPER_SUCCESS;
}

