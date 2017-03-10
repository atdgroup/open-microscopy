/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2007. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  XYZ_PARAMS                      1
#define  XYZ_PARAMS_TEST                 2       /* callback function: OnZtest */
#define  XYZ_PARAMS_LOAD                 3       /* callback function: OnParamsLoad */
#define  XYZ_PARAMS_SAVE                 4       /* callback function: OnParamsSave */
#define  XYZ_PARAMS_READ                 5       /* callback function: OnParamsRead */
#define  XYZ_PARAMS_SEND                 6       /* callback function: OnParamsSend */
#define  XYZ_PARAMS_CLOSE                7       /* callback function: OnParamsClose */
#define  XYZ_PARAMS_Y_ACC                8       /* callback function: OnAcceleration */
#define  XYZ_PARAMS_Z_REVERSED           9       /* callback function: OnZdirReversed */
#define  XYZ_PARAMS_DIAL_ENABLED         10      /* callback function: OnZDialEnabled */
#define  XYZ_PARAMS_Z_ENABLED            11      /* callback function: OnZenabled */
#define  XYZ_PARAMS_Y_PITCH              12      /* callback function: OnPitch */
#define  XYZ_PARAMS_DECORATION_5         13
#define  XYZ_PARAMS_Z_SPEED              14      /* callback function: OnZSpeedChange */
#define  XYZ_PARAMS_Z_LOWER_LIMIT        15      /* callback function: OnZLimitChange */
#define  XYZ_PARAMS_Z_UPPER_LIMIT        16      /* callback function: OnZLimitChange */
#define  XYZ_PARAMS_TEXTMSG_26           17
#define  XYZ_PARAMS_Y_SPEED              18      /* callback function: OnSpeed */
#define  XYZ_PARAMS_BACKLASH_Y           19
#define  XYZ_PARAMS_X_ACC                20      /* callback function: OnAcceleration */
#define  XYZ_PARAMS_Y_REVERSED           21      /* callback function: OnYdirReversed */
#define  XYZ_PARAMS_SENSITIVITY          22      /* callback function: OnZDialSensitivityChange */
#define  XYZ_PARAMS_Y_ENABLED            23      /* callback function: OnYenabled */
#define  XYZ_PARAMS_X_PITCH              24      /* callback function: OnPitch */
#define  XYZ_PARAMS_TEXTMSG_16           25
#define  XYZ_PARAMS_TEXTMSG_17           26
#define  XYZ_PARAMS_TEXTMSG_18           27
#define  XYZ_PARAMS_DECORATION_4         28
#define  XYZ_PARAMS_TEXTMSG_19           29
#define  XYZ_PARAMS_TEXTMSG_22           30
#define  XYZ_PARAMS_X_SPEED              31      /* callback function: OnSpeed */
#define  XYZ_PARAMS_BACKLASH_X           32
#define  XYZ_PARAMS_BAUD                 33      /* callback function: OnSetBaud */
#define  XYZ_PARAMS_X_REVERSED           34      /* callback function: OnXdirReversed */
#define  XYZ_PARAMS_X_ENABLED            35      /* callback function: OnXenabled */
#define  XYZ_PARAMS_FNAME                36
#define  XYZ_PARAMS_TEXTMSG              37
#define  XYZ_PARAMS_TEXTMSG_9            38
#define  XYZ_PARAMS_TEXTMSG_2            39
#define  XYZ_PARAMS_MOVETYPE             40      /* callback function: OnSetMoveType */
#define  XYZ_PARAMS_DECORATION_3         41
#define  XYZ_PARAMS_TEXTMSG_12           42
#define  XYZ_PARAMS_TEXTMSG_15           43
#define  XYZ_PARAMS_TEXTMSG_3            44
#define  XYZ_PARAMS_TEXTMSG_29           45
#define  XYZ_PARAMS_TEXTMSG_30           46
#define  XYZ_PARAMS_Z_TOLERANCE          47      /* callback function: OnZToleranceChange */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK OnAcceleration(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnParamsClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnParamsLoad(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnParamsRead(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnParamsSave(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnParamsSend(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnPitch(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSetBaud(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSetMoveType(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSpeed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnXdirReversed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnXenabled(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnYdirReversed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnYenabled(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnZDialEnabled(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnZDialSensitivityChange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnZdirReversed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnZenabled(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnZLimitChange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnZSpeedChange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnZtest(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnZToleranceChange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
