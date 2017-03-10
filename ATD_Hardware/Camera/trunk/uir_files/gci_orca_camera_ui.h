/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2008. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  EXTRA_PNL                       1
#define  EXTRA_PNL_QUIT                  2       /* callback function: Orca_Camera_onExtrasQuit */
#define  EXTRA_PNL_SUBWINDOW_AUTOCENTRE  3       /* callback function: Orca_Camera_onSetSizePosition */
#define  EXTRA_PNL_SUBWINDOW_HEIGHT      4       /* callback function: Orca_Camera_onSetSizePosition */
#define  EXTRA_PNL_SUBWINDOW_WIDTH       5       /* callback function: Orca_Camera_onSetSizePosition */
#define  EXTRA_PNL_SUBWINDOW_TOP         6       /* callback function: Orca_Camera_onSetSizePosition */
#define  EXTRA_PNL_DECORATION_18         7
#define  EXTRA_PNL_DECORATION_17         8
#define  EXTRA_PNL_DECORATION_16         9
#define  EXTRA_PNL_DECORATION            10
#define  EXTRA_PNL_SUBWINDOW_LEFT        11      /* callback function: Orca_Camera_onSetSizePosition */
#define  EXTRA_PNL_SUBWINDOW_CANVAS      12
#define  EXTRA_PNL_BINNING               13      /* callback function: Orca_Camera_onBinning */
#define  EXTRA_PNL_DATAMODE              14      /* callback function: Orca_Camera_onDataMode */
#define  EXTRA_PNL_SUBWINDOW_PRSETWINDOW 15      /* callback function: Orca_Camera_onPresetSubWindow */
#define  EXTRA_PNL_DECORATION_2          16
#define  EXTRA_PNL_DECORATION_12         17
#define  EXTRA_PNL_TEXTMSG_3             18
#define  EXTRA_PNL_TEXTMSG_4             19
#define  EXTRA_PNL_TEXTMSG               20
#define  EXTRA_PNL_BLACKLEVEL            21      /* callback function: Orca_Camera_onBlackLevel */
#define  EXTRA_PNL_LIGHTMODE             22      /* callback function: Orca_Camera_onLightMode */

#define  INIT_PNL                        2
#define  INIT_PNL_TEXTMSG                2
#define  INIT_PNL_INIT_TEXT              3


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK Orca_Camera_onBinning(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Orca_Camera_onBlackLevel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Orca_Camera_onDataMode(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Orca_Camera_onExtrasQuit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Orca_Camera_onLightMode(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Orca_Camera_onPresetSubWindow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Orca_Camera_onSetSizePosition(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
