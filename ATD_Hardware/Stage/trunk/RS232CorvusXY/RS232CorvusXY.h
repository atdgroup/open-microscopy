#ifndef __CORVUS_STAGE__
#define __CORVUS_STAGE__

#include "stage\stage.h"

typedef struct _CorvusXYStage
{
	XYStage parent;
	
	//int _powered_up;
	//double _power_up_time;
	//int _rs232_initilised;
	int	_baud_rate;
	int _com_port;
	int	_lock;
	double _backlash[AXIS_DATA_ARRAY_SIZE];
	double _pitch[AXIS_DATA_ARRAY_SIZE]; 
	double _cal_velocity_away;
  	double _cal_velocity_toward; 
  	double _rm_velocity_away;
  	double _rm_velocity_toward;
  
} CorvusXYStage;

XYStage* corvus_rs232_xy_stage_new(const char* name, const char* description, UI_MODULE_ERROR_HANDLER error_handler, void *data, const char *data_dir);

int corvus_rs232_set_axis_enable(XYStage* stage, Axis axis, int enable);
int corvus_rs232_xy_send_closed_loop_params(XYStage* stage);

int  CVICALLBACK OnAcceleration(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnParamsClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnParamsLoad(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnParamsSave(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnResetSafeRegion (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnToggleClosedLoop (int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnPitch(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSpeed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnXbacklash(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnXenabled(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnYbacklash(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnYenabled(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#endif
