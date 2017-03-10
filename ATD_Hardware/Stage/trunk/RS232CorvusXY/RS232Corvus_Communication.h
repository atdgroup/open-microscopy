#ifndef _OL_CORVUS_COMMUNICATION_
#define _OL_CORVUS_COMMUNICATION_               

#include "RS232CorvusXY.h"  

#define RS232_ARRAY_SIZE 200

/****************************************************************************************************************/ 
//Basic communications
int RS232CorvusSend(CorvusXYStage *corvus_stage, char* fmt, ...);
	
int RS232CorvusSendandBlockUntilFinished(CorvusXYStage *corvus_stage, char* fmt, ...);

int RS232CorvusBlockUntilFinished(CorvusXYStage *corvus_stage);

void STAGE_PrintCurrentSettings(CorvusXYStage *stage);

int Corvus_get_response (CorvusXYStage* stage, char* response);

int STAGE_SendString(CorvusXYStage*, char* str);

int STAGE_ReadString(CorvusXYStage*, char *retval);
int STAGE_SendReadString(CorvusXYStage*, char* str, char* retval);
int STAGE_ReadInt(CorvusXYStage*, int *retval);

int STAGE_SendReadInt(CorvusXYStage*, char* str, int *retval);

int STAGE_ReadDouble(CorvusXYStage*, double *retval);

int STAGE_SendReadDouble(CorvusXYStage*, char* str, double *retval);

int Corvus_initRS232Port(CorvusXYStage* stage);

#endif
