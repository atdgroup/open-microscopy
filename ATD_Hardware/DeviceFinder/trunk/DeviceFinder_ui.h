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

#define  FTDI_DEV                        1
#define  FTDI_DEV_PORT                   2
#define  FTDI_DEV_ID                     3       /* callback function: cbDeviceID */
#define  FTDI_DEV_OK                     4
#define  FTDI_DEV_TEXTMSG                5

#define  PANEL                           2
#define  PANEL_NUMDEVICES                2
#define  PANEL_DEVICE                    3
#define  PANEL_DESCRIPTION_5             4
#define  PANEL_DESCRIPTION_4             5
#define  PANEL_DESCRIPTION_3             6
#define  PANEL_DESCRIPTION_2             7
#define  PANEL_DESCRIPTION_1             8
#define  PANEL_GETPORT                   9       /* callback function: cbGetPort */
#define  PANEL_GETDEVICES                10      /* callback function: cbGETDEVICES */
#define  PANEL_COMPORT_6                 11
#define  PANEL_QUIT                      12      /* callback function: cbQUIT */
#define  PANEL_COMPORT_5                 13
#define  PANEL_COMPORT_4                 14
#define  PANEL_COMPORT_3                 15
#define  PANEL_COMPORT_2                 16
#define  PANEL_COMPORT_1                 17

#define  PANEL_2                         3
#define  PANEL_2_CANCEL                  2
#define  PANEL_2_OK                      3
#define  PANEL_2_COM_PORTS               4


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK cbDeviceID(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbGETDEVICES(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbGetPort(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbQUIT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
