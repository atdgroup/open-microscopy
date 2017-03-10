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

#define  CUBE_CONF                       1
#define  CUBE_CONF_QUIT                  2       /* callback function: OnCubeConfigCloseClicked */
#define  CUBE_CONF_SAVE                  3       /* callback function: OnCubeFileSave */
#define  CUBE_CONF_ADD                   4       /* callback function: OnCubeDetailsAdd */
#define  CUBE_CONF_REMOVE                5       /* callback function: OnCubeRemoveClicked */
#define  CUBE_CONF_EDIT                  6       /* callback function: OnCubeDetailsEdit */
#define  CUBE_CONF_EDIT_ACTIVE           7       /* callback function: OnCubeEditActive */
#define  CUBE_CONF_LOAD                  8       /* callback function: OnCubeFileRecall */
#define  CUBE_CONF_DOWN_ARROW            9       /* callback function: OnCubeDownArrow */
#define  CUBE_CONF_UP_ARROW              10      /* callback function: OnCubeUpArrow */
#define  CUBE_CONF_RIGHT_ARROW           11      /* callback function: OnCubeRightArrow */
#define  CUBE_CONF_LEFT_ARROW            12      /* callback function: OnCubeLeftArrow */
#define  CUBE_CONF_FNAME                 13
#define  CUBE_CONF_ALL_TREE              14      /* callback function: OnCubeTreeValueChanged */
#define  CUBE_CONF_ACTIVE_TREE           15      /* callback function: OnCubeTreeValueChanged */
#define  CUBE_CONF_DEFAULT               16

#define  CUBE_INFO                       2
#define  CUBE_INFO_CLOSE                 2       /* callback function: OnFluorCubeClose */
#define  CUBE_INFO_SETUP                 3       /* callback function: OnCubeSetup */
#define  CUBE_INFO_TURRET_POS            4       /* callback function: OnCubeChanged */
#define  CUBE_INFO_ERROR                 5

#define  EDIT_PANEL                      3
#define  EDIT_PANEL_OK_BUTTON            2       /* callback function: OnCubeAddEditOkClicked */
#define  EDIT_PANEL_NAME                 3
#define  EDIT_PANEL_TEXTMSG              4
#define  EDIT_PANEL_TEXTMSG_4            5
#define  EDIT_PANEL_TEXTMSG_5            6
#define  EDIT_PANEL_TEXTMSG_6            7
#define  EDIT_PANEL_TEXTMSG_8            8
#define  EDIT_PANEL_ADDOREDIT            9
#define  EDIT_PANEL_INDEX                10
#define  EDIT_PANEL_EMMAX                11
#define  EDIT_PANEL_EMMIN                12
#define  EDIT_PANEL_DICHROICNM           13
#define  EDIT_PANEL_EXCNM                14

#define  INSERT_CUB                      4
#define  INSERT_CUB_TURRET_POS           2
#define  INSERT_CUB_CANCEL               3
#define  INSERT_CUB_OK                   4


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */ 

int  CVICALLBACK OnCubeAddEditOkClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeConfigCloseClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeDetailsAdd(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeDetailsEdit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeDownArrow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeEditActive(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeFileRecall(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeFileSave(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeLeftArrow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeRemoveClicked(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeRightArrow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeSetup(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeTreeValueChanged(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnCubeUpArrow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnFluorCubeClose(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif
