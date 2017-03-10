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
#define  EXTRA_PNL_QUIT                  2       /* callback function: C9100_13_Camera_onExtrasQuit */
#define  EXTRA_PNL_SUBWINDOW_AUTOCENTRE  3       /* callback function: C9100_13_Camera_onSetSizePosition */
#define  EXTRA_PNL_SUBWINDOW_HEIGHT      4       /* callback function: C9100_13_Camera_onSetSizePosition */
#define  EXTRA_PNL_SUBWINDOW_WIDTH       5       /* callback function: C9100_13_Camera_onSetSizePosition */
#define  EXTRA_PNL_SUBWINDOW_TOP         6       /* callback function: C9100_13_Camera_onSetSizePosition */
#define  EXTRA_PNL_DECORATION_22         7
#define  EXTRA_PNL_DECORATION_23         8
#define  EXTRA_PNL_DECORATION_21         9
#define  EXTRA_PNL_DECORATION_19         10
#define  EXTRA_PNL_DECORATION_18         11
#define  EXTRA_PNL_SUBWINDOW_LEFT        12      /* callback function: C9100_13_Camera_onSetSizePosition */
#define  EXTRA_PNL_SUBWINDOW_CANVAS      13
#define  EXTRA_PNL_BINNING               14      /* callback function: C9100_13_Camera_onBinning */
#define  EXTRA_PNL_DECORATION            15
#define  EXTRA_PNL_PHOTONMODE            16      /* callback function: C9100_13_Camera_SetPhotonMode */
#define  EXTRA_PNL_DATAMODE              17      /* callback function: C9100_13_Camera_onDataMode */
#define  EXTRA_PNL_SUBWINDOW_PRSETWINDOW 18      /* callback function: C9100_13_Camera_onPresetSubWindow */
#define  EXTRA_PNL_DECORATION_2          19
#define  EXTRA_PNL_DECORATION_12         20
#define  EXTRA_PNL_TEXTMSG_3             21
#define  EXTRA_PNL_TEXTMSG_5             22
#define  EXTRA_PNL_LIGHTMODE             23
#define  EXTRA_PNL_TEXTMSG_8             24
#define  EXTRA_PNL_TEXTMSG_7             25
#define  EXTRA_PNL_TEXTMSG_4             26
#define  EXTRA_PNL_BLACKLEVEL            27
#define  EXTRA_PNL_DEGS_C                28
#define  EXTRA_PNL_TEXTMSG_6             29
#define  EXTRA_PNL_TEXTMSG               30
#define  EXTRA_PNL_TEXTMSG_2             31
#define  EXTRA_PNL_TIMER                 32      /* callback function: C9100_13_Camera_TimerTick */

#define  INIT_PNL                        2
#define  INIT_PNL_TEXTMSG                2
#define  INIT_PNL_INIT_TEXT              3


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK C9100_13_Camera_onBinning(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK C9100_13_Camera_onDataMode(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK C9100_13_Camera_onExtrasQuit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK C9100_13_Camera_onPresetSubWindow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK C9100_13_Camera_onSetSizePosition(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK C9100_13_Camera_SetPhotonMode(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK C9100_13_Camera_TimerTick(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
