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

#define  FOCAL_PNL                       1
#define  FOCAL_PNL_OK                    2
#define  FOCAL_PNL_SET_F_OFFSET          3
#define  FOCAL_PNL_SET_FOCAL_PLANE       4
#define  FOCAL_PNL_INSIDE_ROI            5
#define  FOCAL_PNL_OUTSIDE_ROI           6
#define  FOCAL_PNL_DECORATION            7
#define  FOCAL_PNL_TEXTMSG               8

#define  MSGPANEL                        2
#define  MSGPANEL_OK                     2       /* callback function: cbMessageOK */
#define  MSGPANEL_TEXTMSG_3              3
#define  MSGPANEL_CANCEL                 4
#define  MSGPANEL_TEXTMSG                5

#define  ROI_PANEL                       3
#define  ROI_PANEL_OK_2                  2       /* callback function: onRoiCancel */
#define  ROI_PANEL_OK                    3       /* callback function: onRoiOk */
#define  ROI_PANEL_CANVAS                4
#define  ROI_PANEL_SET_PLANE             5       /* callback function: cbRoiSetFocalPlane */
#define  ROI_PANEL_STRING                6

#define  ROI_TAB1                        4
#define  ROI_TAB1_Y_END                  2       /* callback function: cbRoiTab1_pointChange */
#define  ROI_TAB1_Y_START                3       /* callback function: cbRoiTab1_pointChange */
#define  ROI_TAB1_X_END                  4       /* callback function: cbRoiTab1_pointChange */
#define  ROI_TAB1_Y_LENGTH               5       /* callback function: cbRoiTab1_dimChange */
#define  ROI_TAB1_X_START                6       /* callback function: cbRoiTab1_pointChange */
#define  ROI_TAB1_X_LENGTH               7       /* callback function: cbRoiTab1_dimChange */
#define  ROI_TAB1_SET_END                8       /* callback function: cbRoiTab1_SetEnd */
#define  ROI_TAB1_SET_START              9       /* callback function: cbRoiTab1_SetStart */
#define  ROI_TAB1_TEXTMSG_4              10
#define  ROI_TAB1_TEXTMSG_2              11
#define  ROI_TAB1_TEXTMSG                12
#define  ROI_TAB1_DECORATION             13
#define  ROI_TAB1_TEXTMSG_6              14
#define  ROI_TAB1_TEXTMSG_5              15
#define  ROI_TAB1_TEXTMSG_3              16

#define  ROI_TAB2                        5
#define  ROI_TAB2_X_END                  2       /* callback function: cbRoiTab2_pointChange */
#define  ROI_TAB2_Y_END                  3       /* callback function: cbRoiTab2_pointChange */
#define  ROI_TAB2_Y_START                4       /* callback function: cbRoiTab2_pointChange */
#define  ROI_TAB2_X_START                5       /* callback function: cbRoiTab2_pointChange */
#define  ROI_TAB2_SET_Y_END              6       /* callback function: cbRoiTab2_pointSetup */
#define  ROI_TAB2_SET_X_END              7       /* callback function: cbRoiTab2_pointSetup */
#define  ROI_TAB2_SET_X_START            8       /* callback function: cbRoiTab2_pointSetup */
#define  ROI_TAB2_SET_Y_START            9       /* callback function: cbRoiTab2_pointSetup */
#define  ROI_TAB2_DECORATION             10
#define  ROI_TAB2_DECORATION_3           11
#define  ROI_TAB2_DECORATION_5           12
#define  ROI_TAB2_DECORATION_4           13
#define  ROI_TAB2_DECORATION_2           14

#define  ROI_TAB3                        6
#define  ROI_TAB3_RADIUS                 2       /* callback function: cbRoiTab3_valChange */
#define  ROI_TAB3_CENTRE_Y               3       /* callback function: cbRoiTab3_valChange */
#define  ROI_TAB3_CENTRE_X               4       /* callback function: cbRoiTab3_valChange */
#define  ROI_TAB3_SET_CENTRE             5       /* callback function: cbRoiTab3_SetCentre */
#define  ROI_TAB3_SET_RADIUS             6       /* callback function: cbRoiTab3_SetRadius */
#define  ROI_TAB3_TEXTMSG                7
#define  ROI_TAB3_TEXTMSG_2              8
#define  ROI_TAB3_DECORATION             9
#define  ROI_TAB3_TEXTMSG_7              10
#define  ROI_TAB3_TEXTMSG_6              11
#define  ROI_TAB3_DECORATION_2           12
#define  ROI_TAB3_TEXTMSG_3              13

#define  ROI_TAB4                        7
#define  ROI_TAB4_X_END                  2
#define  ROI_TAB4_Y_END                  3
#define  ROI_TAB4_Y_START                4
#define  ROI_TAB4_X_START                5
#define  ROI_TAB4_GO_MIDDLE              6       /* callback function: cbRoiTab4_GoPos */
#define  ROI_TAB4_GO_4                   7       /* callback function: cbRoiTab4_GoPos */
#define  ROI_TAB4_GO_3                   8       /* callback function: cbRoiTab4_GoPos */
#define  ROI_TAB4_DECORATION             9
#define  ROI_TAB4_GO_2                   10      /* callback function: cbRoiTab4_GoPos */
#define  ROI_TAB4_GO_8                   11      /* callback function: cbRoiTab4_GoPos */
#define  ROI_TAB4_GO_7                   12      /* callback function: cbRoiTab4_GoPos */
#define  ROI_TAB4_GO_6                   13      /* callback function: cbRoiTab4_GoPos */
#define  ROI_TAB4_GO_5                   14      /* callback function: cbRoiTab4_GoPos */
#define  ROI_TAB4_GO_1                   15      /* callback function: cbRoiTab4_GoPos */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK cbMessageOK(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiSetFocalPlane(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab1_dimChange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab1_pointChange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab1_SetEnd(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab1_SetStart(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab2_pointChange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab2_pointSetup(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab3_SetCentre(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab3_SetRadius(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab3_valChange(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRoiTab4_GoPos(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onRoiCancel(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onRoiOk(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
