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

#define  DEBUG_PNL                       1
#define  DEBUG_PNL_SHUTTER_EXP_LBL       2
#define  DEBUG_PNL_FRAME_EXP_LBL         3
#define  DEBUG_PNL_LONG_EXP_LBL          4
#define  DEBUG_PNL_TRIGGER_EXP_LBL       5
#define  DEBUG_PNL_BINNING_LBL           6
#define  DEBUG_PNL_SUBWINDOW_LBL         7
#define  DEBUG_PNL_DATA_MODE_LBL         8
#define  DEBUG_PNL_TIMER                 9       /* callback function: OnDebugTimerTick */
#define  DEBUG_PNL_TRIGGER_ENABLED_LBL   10
#define  DEBUG_PNL_EXP_LBL               11
#define  DEBUG_PNL_TRIGGER_EXP           12
#define  DEBUG_PNL_TRIGGER_ENABLED       13
#define  DEBUG_PNL_BINNING               14
#define  DEBUG_PNL_DATA_MODE             15
#define  DEBUG_PNL_SUBWINDOW             16
#define  DEBUG_PNL_FRAME_EXP             17
#define  DEBUG_PNL_LONG_EXP              18
#define  DEBUG_PNL_SHUTTER_EXP           19
#define  DEBUG_PNL_EXP                   20
#define  DEBUG_PNL_LONG_ENABLED          21
#define  DEBUG_PNL_FRAME_ENABLED         22
#define  DEBUG_PNL_SHUTTER_ENABLED       23

#define  EXTRA_PNL                       2
#define  EXTRA_PNL_SW_APPLY              2
#define  EXTRA_PNL_DEBUG                 3       /* callback function: LynxCamera_OnDebugButtonPress */
#define  EXTRA_PNL_QUIT                  4
#define  EXTRA_PNL_TAP                   5
#define  EXTRA_PNL_BINNING               6
#define  EXTRA_PNL_DATAMODE              7
#define  EXTRA_PNL_DECORATION_18         8
#define  EXTRA_PNL_DECORATION_21         9
#define  EXTRA_PNL_DECORATION_20         10
#define  EXTRA_PNL_DECORATION_23         11
#define  EXTRA_PNL_DECORATION_22         12
#define  EXTRA_PNL_DECORATION_19         13
#define  EXTRA_PNL_DECORATION            14
#define  EXTRA_PNL_TEXTMSG_5             15
#define  EXTRA_PNL_TEXTMSG_3             16
#define  EXTRA_PNL_LONG_EXP              17
#define  EXTRA_PNL_TEXTMSG_4             18
#define  EXTRA_PNL_GAINLEVEL2            19
#define  EXTRA_PNL_GAINLEVEL1            20
#define  EXTRA_PNL_SHUTTER_TIME          21
#define  EXTRA_PNL_TEXTMSG               22
#define  EXTRA_PNL_BLACKLEVEL_2          23
#define  EXTRA_PNL_BLACKLEVEL            24
#define  EXTRA_PNL_LEXP_CHECKBOX         25
#define  EXTRA_PNL_SHUTTER_CHECKBOX      26
#define  EXTRA_PNL_SW_CHECKBOX           27
#define  EXTRA_PNL_FRAMETIME_TXT         28

#define  INIT_PNL                        3
#define  INIT_PNL_TEXTMSG_3              2
#define  INIT_PNL_TEXTMSG_2              3
#define  INIT_PNL_TEXTMSG                4
#define  INIT_PNL_INIT_TEXT              5


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK LynxCamera_OnDebugButtonPress(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDebugTimerTick(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
