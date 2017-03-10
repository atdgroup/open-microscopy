#ifndef __CORVUS_STAGE__
#define __CORVUS_STAGE__

#include "stage\stage.h"

typedef struct _LStepXYStage
{
	XYStage parent;
	
	int _powered_up;
	double _power_up_time;
	int _rs232_initilised;
	int _com_port;
	int	_lock;
	double _timeout; // ms
	double _cal_velocity;
	double _cal_acceleration;
	double _backlash[AXIS_DATA_ARRAY_SIZE];
	double _pitch[AXIS_DATA_ARRAY_SIZE]; 
	char   _info[200];

} LStepXYStage;

XYStage* lstep_xy_stage_new(const char* name, const char* description, UI_MODULE_ERROR_HANDLER error_handler, void *data, const char *data_dir);

int lstep_init_comport(int port);

void LogLStepError(int val);


int  CVICALLBACK OnLStepAcceleration(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLStepParamsClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLStepParamsLoad(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLStepParamsSave(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLStepPitch(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLStepSpeed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLStepXbacklash(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLStepXenabled(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLStepYbacklash(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLStepYenabled(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#endif
