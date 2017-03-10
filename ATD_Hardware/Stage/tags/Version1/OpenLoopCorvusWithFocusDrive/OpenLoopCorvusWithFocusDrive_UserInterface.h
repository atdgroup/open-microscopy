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
#define  XYZ_PARAMS_Z_ACC                8       /* callback function: OnAcceleration */
#define  XYZ_PARAMS_Y_ACC                9       /* callback function: OnAcceleration */
#define  XYZ_PARAMS_Z_REVERSED           10      /* callback function: OnZdirReversed */
#define  XYZ_PARAMS_Z_ENABLED            11      /* callback function: OnZenabled */
#define  XYZ_PARAMS_Z_PITCH              12      /* callback function: OnZPitch */
#define  XYZ_PARAMS_Y_PITCH              13      /* callback function: OnPitch */
#define  XYZ_PARAMS_DECORATION_5         14
#define  XYZ_PARAMS_Z_LOWER_LIMIT        15      /* callback function: OnZLimitChange */
#define  XYZ_PARAMS_Z_UPPER_LIMIT        16      /* callback function: OnZLimitChange */
#define  XYZ_PARAMS_Z_SPEED              17      /* callback function: OnSpeed */
#define  XYZ_PARAMS_TEXTMSG_26           18
#define  XYZ_PARAMS_BACKLASH_Z           19
#define  XYZ_PARAMS_Y_SPEED              20      /* callback function: OnSpeed */
#define  XYZ_PARAMS_BACKLASH_Y           21
#define  XYZ_PARAMS_X_ACC                22      /* callback function: OnAcceleration */
#define  XYZ_PARAMS_Y_REVERSED           23      /* callback function: OnYdirReversed */
#define  XYZ_PARAMS_Y_ENABLED            24      /* callback function: OnYenabled */
#define  XYZ_PARAMS_TEXTMSG_30           25
#define  XYZ_PARAMS_TEXTMSG_31           26
#define  XYZ_PARAMS_X_PITCH              27      /* callback function: OnPitch */
#define  XYZ_PARAMS_TEXTMSG_32           28
#define  XYZ_PARAMS_TEXTMSG_16           29
#define  XYZ_PARAMS_TEXTMSG_17           30
#define  XYZ_PARAMS_TEXTMSG_18           31
#define  XYZ_PARAMS_TEXTMSG_33           32
#define  XYZ_PARAMS_DECORATION_4         33
#define  XYZ_PARAMS_TEXTMSG_19           34
#define  XYZ_PARAMS_TEXTMSG_22           35
#define  XYZ_PARAMS_XY_CAL_SPEED_2       36      /* callback function: OnCalSpeed */
#define  XYZ_PARAMS_XY_CAL_SPEED         37      /* callback function: OnCalSpeed */
#define  XYZ_PARAMS_X_SPEED              38      /* callback function: OnSpeed */
#define  XYZ_PARAMS_BACKLASH_X           39
#define  XYZ_PARAMS_BAUD                 40      /* callback function: OnSetBaud */
#define  XYZ_PARAMS_X_REVERSED           41      /* callback function: OnXdirReversed */
#define  XYZ_PARAMS_X_ENABLED            42      /* callback function: OnXenabled */
#define  XYZ_PARAMS_FNAME                43
#define  XYZ_PARAMS_TEXTMSG              44
#define  XYZ_PARAMS_TEXTMSG_9            45
#define  XYZ_PARAMS_TEXTMSG_2            46
#define  XYZ_PARAMS_MOVETYPE             47      /* callback function: OnSetMoveType */
#define  XYZ_PARAMS_DECORATION_3         48
#define  XYZ_PARAMS_TEXTMSG_12           49
#define  XYZ_PARAMS_TEXTMSG_15           50
#define  XYZ_PARAMS_TEXTMSG_3            51
#define  XYZ_PARAMS_TEXTMSG_34           52


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK OnAcceleration(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCalSpeed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
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
int  CVICALLBACK OnZdirReversed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnZenabled(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnZLimitChange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnZPitch(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnZtest(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
