/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2006. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  SHUTTER                         1
#define  SHUTTER_EXIT                    2       /* callback function: cbShutterClose */
#define  SHUTTER_TEXTMSG_32              3
#define  SHUTTER_OPEN                    4       /* callback function: cbShutterOpenButton */
#define  SHUTTER_TRIGGER                 5       /* callback function: cbShutterTriggerButton */
#define  SHUTTER_CLOSE                   6       /* callback function: cbShutterCloseButton */
#define  SHUTTER_CLOSED_LED              7
#define  SHUTTER_OPEN_LED                8
#define  SHUTTER_DECORATION_10           9
#define  SHUTTER_DECORATION_9            10
#define  SHUTTER_OPEN_TIME               11      /* callback function: cbShutterOpenTime */
#define  SHUTTER_TIMER                   12      /* callback function: cbShutturStatusTimer */
#define  SHUTTER_TEST                    13      /* callback function: cbShutterTest */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK cbShutterClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbShutterCloseButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbShutterOpenButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbShutterOpenTime(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbShutterTest(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbShutterTriggerButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbShutturStatusTimer(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
