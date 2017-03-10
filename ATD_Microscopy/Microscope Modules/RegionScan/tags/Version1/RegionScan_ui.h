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

#define  ROI_SCAN                        1
#define  ROI_SCAN_QUIT                   2       /* callback function: cbRegionScanClose */
#define  ROI_SCAN_Y_START                3
#define  ROI_SCAN_Y_LENGTH               4
#define  ROI_SCAN_X_START                5
#define  ROI_SCAN_NEXT                   6       /* callback function: cbRegionScanNextFrame */
#define  ROI_SCAN_X_LENGTH               7
#define  ROI_SCAN_PREV                   8       /* callback function: cbRegionScanPrevFrame */
#define  ROI_SCAN_FRAMES                 9
#define  ROI_SCAN_FRAME                  10
#define  ROI_SCAN_OVERLAP                11      /* callback function: cbRegionScanSetOverlap */
#define  ROI_SCAN_Y_STEP                 12
#define  ROI_SCAN_DECO_6                 13
#define  ROI_SCAN_X_STEP                 14
#define  ROI_SCAN_SCAN_DISP              15      /* callback function: cbRegionScanGotoFrame */
#define  ROI_SCAN_PAUSE                  16      /* callback function: cbRegionScanPause */
#define  ROI_SCAN_STOP                   17      /* callback function: cbRegionScanStop */
#define  ROI_SCAN_SET_ROI                18      /* callback function: cbRegionScanSetROI */
#define  ROI_SCAN_SET_PLANE              19      /* callback function: cbRegionScanSetFocalPlane */
#define  ROI_SCAN_START                  20      /* callback function: cbRegionScanStart */
#define  ROI_SCAN_Z_DISP                 21
#define  ROI_SCAN_TEXT_6                 22
#define  ROI_SCAN_ACTION                 23
#define  ROI_SCAN_DO_CORRECTIONS         24
#define  ROI_SCAN_TIME_TAKEN             25


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK cbRegionScanClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRegionScanGotoFrame(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRegionScanNextFrame(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRegionScanPause(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRegionScanPrevFrame(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRegionScanSetFocalPlane(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRegionScanSetOverlap(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRegionScanSetROI(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRegionScanStart(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK cbRegionScanStop(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
