#ifndef __ATD_OP_A_OPTICAL_PATH__
#define __ATD_OP_A_OPTICAL_PATH__ 

#include "OpticalPath.h"
#include "ATD_StepperMotor_A\ATD_StepperMotor_A.h"
#include "FTDI_Utils.h"

typedef struct
{
	OpticalPathManager parent;
	MirrorStepper* mirror_stepper;
	FTDIController*		_controller;
    int _com_port;

	char device_sn[FTDI_SERIAL_NUMBER_LINE_LENGTH];
    
} ATD_OP_A;

OpticalPathManager* atd_op_a_optical_path_new(const char *name, const char *description, const char* data_dir, const char *filepath);

#endif
