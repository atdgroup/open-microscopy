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

#define  ABOUT_PNL                       1
#define  ABOUT_PNL_QUIT_ABOUT            2
#define  ABOUT_PNL_INFO                  3

#define  INIT_PNL                        2
#define  INIT_PNL_ABORT_INIT             2       /* callback function: OnStageAbortInit */
#define  INIT_PNL_TEXT                   3

#define  STAGE_PNL                       3
#define  STAGE_PNL_X_POS                 2
#define  STAGE_PNL_Y_POS                 3
#define  STAGE_PNL_Z_POS                 4
#define  STAGE_PNL_JOYSTICK_XY_SPEED     5       /* callback function: OnStageJoystickSpeed */
#define  STAGE_PNL_GOTO_Z_DATUM          6       /* callback function: OnGotoZDatum */
#define  STAGE_PNL_GOTO_XY_DATUM         7       /* callback function: OnGotoXYDatum */
#define  STAGE_PNL_ABORT_MOVE            8       /* callback function: OnStageAbortMove */
#define  STAGE_PNL_SET_DATUM_XY          9       /* callback function: OnSetXYDatum */
#define  STAGE_PNL_SET_DATUM_Z           10      /* callback function: OnSetZDatum */
#define  STAGE_PNL_ACTIVE                11
#define  STAGE_PNL_GOTO_XYZ_DATUM        12      /* callback function: OnGotoXYZDatum */
#define  STAGE_PNL_SET_DATUM_XYZ         13      /* callback function: OnSetXYZDatum */
#define  STAGE_PNL_ADVANCED              14      /* callback function: OnStageAdvancedButton */
#define  STAGE_PNL_QUIT                  15      /* callback function: OnStageQuit */
#define  STAGE_PNL_Z_DISP                16
#define  STAGE_PNL_MOVE_TIME             17
#define  STAGE_PNL_ABOUT                 18      /* callback function: OnAbout */
#define  STAGE_PNL_BOTTOM_RIGHT          19      /* callback function: MoveBottomRight */
#define  STAGE_PNL_MOVE_REL_X            20      /* callback function: MoveByX */
#define  STAGE_PNL_MOVE_REL_Y            21      /* callback function: MoveByY */
#define  STAGE_PNL_MOVE_REL_Z            22      /* callback function: MoveByZ */
#define  STAGE_PNL_MOVE_ABS_Z            23      /* callback function: MoveToZ */
#define  STAGE_PNL_MOVE_ABS_Y            24      /* callback function: MoveToY */
#define  STAGE_PNL_MOVE_ABS_X            25      /* callback function: MoveToX */
#define  STAGE_PNL_MOVE_ABS_XYZ          26      /* callback function: MoveToXYZ */
#define  STAGE_PNL_Y_REL                 27
#define  STAGE_PNL_BOTTOM_LEFT           28      /* callback function: MoveBottomLeft */
#define  STAGE_PNL_X_REL                 29
#define  STAGE_PNL_Z_REL                 30
#define  STAGE_PNL_X_ABS                 31
#define  STAGE_PNL_Y_ABS                 32
#define  STAGE_PNL_Z_ABS                 33
#define  STAGE_PNL_TOP_LEFT              34      /* callback function: MoveTopLeft */
#define  STAGE_PNL_Z_BOTTOM              35      /* callback function: MoveZBottom */
#define  STAGE_PNL_MOVE_REL_XYZ          36      /* callback function: MoveByXYZ */
#define  STAGE_PNL_Z_TOP                 37      /* callback function: MoveZTop */
#define  STAGE_PNL_TOP_RIGHT             38      /* callback function: MoveTopRight */
#define  STAGE_PNL_INITIALISE            39      /* callback function: OnReinit */
#define  STAGE_PNL_XY_DISP               40
#define  STAGE_PNL_SHOW_SETTINGS         41      /* callback function: OnSettings */
#define  STAGE_PNL_TEXT1_5               42
#define  STAGE_PNL_XCL                   43
#define  STAGE_PNL_DECORATION            44
#define  STAGE_PNL_DECORATION_6          45
#define  STAGE_PNL_DECORATION_3          46
#define  STAGE_PNL_TEXT_8                47
#define  STAGE_PNL_JOY_ON                48
#define  STAGE_PNL_DECORATION_4          49
#define  STAGE_PNL_YCL                   50
#define  STAGE_PNL_ZCL                   51
#define  STAGE_PNL_DECORATION_5          52
#define  STAGE_PNL_TEXT1_6               53
#define  STAGE_PNL_TEXT1_7               54
#define  STAGE_PNL_TEXT                  55
#define  STAGE_PNL_TEXT_2                56
#define  STAGE_PNL_JOYSTICK_ENABLE       57      /* callback function: OnStageJoystickToggled */
#define  STAGE_PNL_TEXTMSG               58
#define  STAGE_PNL_TIMER                 59      /* callback function: OnStageTimerTick */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK MoveBottomLeft(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveBottomRight(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveByX(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveByXYZ(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveByY(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveByZ(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveTopLeft(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveTopRight(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveToX(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveToXYZ(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveToY(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveToZ(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveZBottom(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK MoveZTop(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnAbout(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnGotoXYDatum(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnGotoXYZDatum(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnGotoZDatum(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnReinit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSettings(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSetXYDatum(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSetXYZDatum(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSetZDatum(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStageAbortInit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStageAbortMove(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStageAdvancedButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStageJoystickSpeed(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStageJoystickToggled(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStageQuit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStageTimerTick(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
