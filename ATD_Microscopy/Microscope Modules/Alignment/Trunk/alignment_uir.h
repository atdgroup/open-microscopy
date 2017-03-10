/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2005. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  ALIGNPANEL                      1
#define  ALIGNPANEL_CANCEL               2
#define  ALIGNPANEL_OK                   3
#define  ALIGNPANEL_TEXTMSG              4
#define  ALIGNPANEL_TEXTMSG_13           5
#define  ALIGNPANEL_PINHOLE_DIAM         6       /* callback function: cbPinholeDiameter */
#define  ALIGNPANEL_TEXTMSG_2            7

#define  INFO_PANEL                      2
#define  INFO_PANEL_OK                   2
#define  INFO_PANEL_TEXTMSG              3

#define  PROG_PNL                        3
#define  PROG_PNL_ABORT                  2
#define  PROG_PNL_TEXTMSG                3

#define  RESULT_PNL                      4
#define  RESULT_PNL_OK                   2
#define  RESULT_PNL_TEXTMSG              3
#define  RESULT_PNL_RESULT_TEXT          4
#define  RESULT_PNL_SLOPE                5       /* callback function: cbSetSlope */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK cbPinholeDiameter(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbSetSlope(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
