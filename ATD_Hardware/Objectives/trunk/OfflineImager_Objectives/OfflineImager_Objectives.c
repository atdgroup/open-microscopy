#include "OfflineImager_Objectives.h"
#include "gci_ui_module.h"
#include "ObjectivesUI.h"
#include "ATD_UsbInterface_A.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

//Address 0: all outputs (Chip PCF8574)
// bit 0		camera power	 (0 for on, 1 for off)
// bit 1		lamp power		 (0 for on, 1 for off)
// bit 2		stage power		 (0 for on, 1 for off)
// bit 3		Hg lamp power	 (0 for on, 1 for off)
// bit 7		shutter state	 (0 to open, 1 to shut)
//
//Address 1: all inputs
// bit 5		lamp enabled	 (1 enabled, 0 disabled)
// bit 6		shutter enabled	 (1 enabled, 0 disabled)
// bit 7		shutter state	 (1 closed, 0 open)
//
//Address 7:
// bits 0-2		Turret pos feedback		output  (0-7)
// bit 3		Turret moving			output  (0 moving)
// bits 4-6		Turret pos				input   (0-7)
// bit 7		Turret enable			input   (1 enabled, 0 disabled) 
//
//Address 0: (Chip MAX521)
// DAC 3		Lamp on				(0 for on, 255 for off)
// DAC 4		Scope lamp control  (0 to enable, 255 to disable)
// DAC 5		Scope int'y control (0 to enable, 255 to disable)
// DAC 6		Lamp fine control   (255 max)
// DAC 7		Lamp coarse control (255 max)
//

static int dirs[8] = {0, 0xff, 0, 0, 0, 0, 0, 0x0f};

static int GCI_Out_Byte_multiPort (int port, int bus, byte chip_type, byte address, byte dirs, byte patt )
{
	int err;
	byte val[3]; 									  
 
	//Send a byte. Any input lines must be set high in dirs.
	
	GetI2CPortLock(port, "GCI_Out_Byte_multiPort");

   	val[0] = chip_type | (address <<1);
   	val[1] = patt | dirs;						//keeps input lines high

   	err = GCI_writeI2C_multiPort(port, 2, val, bus, "GCI_Out_Byte_multiPort");

	ReleaseI2CPortLock(port, "GCI_Out_Byte_multiPort"); 

	return err;
}

static int GCI_In_Byte_multiPort (int port, int bus, byte chip_type, byte address, byte *i_patt)
{
	byte val[3]; 
 
	GetI2CPortLock(port, "GCI_In_Byte_multiPort");

    val[0] = chip_type | (address <<1) | 0x01;
    if (GCI_readI2C_multiPort(port, 2, val, bus, "GCI_In_Byte_multiPort")) {
		ReleaseI2CPortLock(port, "GCI_In_Byte_multiPort"); 
    	return -1;	//problem
    }
    *i_patt = (int)val[1];

	ReleaseI2CPortLock(port, "GCI_In_Byte_multiPort"); 

    return 0;
}

static int readTurret(ObjectiveManager* objective_manager)
{
	byte data;
	OfflineImagerObjectiveManager *oom = (OfflineImagerObjectiveManager *) objective_manager;

	if (GCI_In_Byte_multiPort (oom->_com_port, oom->_i2c_bus, oom->_i2c_address, 7, &data))
		return OBJECTIVE_MANAGER_ERROR;

	return data;
}

static int writeTurret(ObjectiveManager* objective_manager, byte mdata)
{
	OfflineImagerObjectiveManager *oom = (OfflineImagerObjectiveManager *) objective_manager;

	return GCI_Out_Byte_multiPort (oom->_com_port, oom->_i2c_bus, oom->_i2c_address, 7, dirs[7], mdata);
}

static int offlineimager_destroy (ObjectiveManager* objective_manager)
{
	return OBJECTIVE_MANAGER_SUCCESS;
}

static int offlineimager_get_current_turret_position(ObjectiveManager* objective_manager, int *position)
{
	OfflineImagerObjectiveManager *oom = (OfflineImagerObjectiveManager *) objective_manager;
	byte data;	
	
	data = readTurret(objective_manager);	
	data &= 0x8f;						//keep "enable" and A0-A2
	*position = (~data) & 0x07;			//current turret pos

	return OBJECTIVE_MANAGER_SUCCESS;
}

static int offlineimager_is_turret_moving(ObjectiveManager* objective_manager)
{
	OfflineImagerObjectiveManager *oom = (OfflineImagerObjectiveManager *) objective_manager;
	byte data;	
	
	data = (unsigned char) readTurret(objective_manager);	
	data = !(data & 0x08);  // Bit 3 is set
	return data;			// is moving
}

static int offlineimager_is_turret_enabled(ObjectiveManager* objective_manager)
{
	OfflineImagerObjectiveManager *oom = (OfflineImagerObjectiveManager *) objective_manager;
	byte data;	
	
	data = (unsigned char) readTurret(objective_manager);	 // Bit 7 is set
	return ((data & 0x80) >> 7);		 // is enabled
}

static int offlineimager_move_to_turret_position(ObjectiveManager* objective_manager, int position)
{
	OfflineImagerObjectiveManager *oom = (OfflineImagerObjectiveManager *) objective_manager;
	unsigned char tmp;
	int read_position;
	double start_time;

	tmp = (unsigned char)readTurret(objective_manager);	
	tmp &= 0x8f;						//keep "enable" and A0-A2
	
	tmp |= (((unsigned char) position)<<4);				//set bits 4-6 to mobjNum

	writeTurret(objective_manager, tmp);

	// Wait until position has been reached.
	start_time = Timer();
	read_position = -1;
	while(offlineimager_is_turret_moving(objective_manager)) {

		if((Timer() - start_time) > 5.0) {
			return OBJECTIVE_MANAGER_ERROR;
		}

		ProcessSystemEvents();
		Delay(0.1);
	}

	return OBJECTIVE_MANAGER_SUCCESS;
}

static int offlineimager_hw_init (ObjectiveManager* objective_manager)
{
	int enabled = -1;

	OfflineImagerObjectiveManager *oom = (OfflineImagerObjectiveManager *) objective_manager;

	// only needs the com port the master pic is on, works directly off its fast lines.
	get_device_param_from_ini_file(UIMODULE_GET_NAME(oom), "COM_Port", &(oom->_com_port)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(oom), "i2c_ChipAddress", &(oom->_i2c_address));      
	get_device_param_from_ini_file(UIMODULE_GET_NAME(oom), "i2c_Bus", &(oom->_i2c_bus));      

	// Don't know what the Robs or Ros's intention was here
	//writeTurret(objective_manager, ((readTurret(objective_manager) & 0x7F) | 0x80));

	//disable
	//writeTurret(objective_manager, (readTurret(objective_manager) & 0x7F));

	// enable
	//writeTurret(objective_manager, (readTurret(objective_manager) | 0x80));

	//enabled = offlineimager_is_turret_enabled(objective_manager);

	return OBJECTIVE_MANAGER_SUCCESS;        	
}

ObjectiveManager* offlineimager_objective_manager_new(const char *name, const char *description, const char *data_dir, const char *filepath)
{
	ObjectiveManager* objective_manager = objective_manager_new(name, description, data_dir, filepath, sizeof(OfflineImagerObjectiveManager));

	if(objective_manager == NULL)
		return NULL;
	
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, hw_init) = offlineimager_hw_init;   
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, destroy) = offlineimager_destroy;
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, move_to_turret_position) = offlineimager_move_to_turret_position;
	OBJECTIVE_MANAGER_VTABLE_PTR(objective_manager, get_current_turret_position) = offlineimager_get_current_turret_position;
				
	return objective_manager;
}
