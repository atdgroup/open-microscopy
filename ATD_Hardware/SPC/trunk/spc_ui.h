/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2012. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                           1
#define  PANEL_DECORATION_29             2
#define  PANEL_PILE_UP_LED               3
#define  PANEL_B_DISP                    4
#define  PANEL_G_DISP                    5
#define  PANEL_R_DISP                    6
#define  PANEL_TEXTMSG_13                7
#define  PANEL_SAVE_RGB                  8
#define  PANEL_BLUE_GAIN                 9
#define  PANEL_GREEN_GAIN                10
#define  PANEL_RED_GAIN                  11
#define  PANEL_SET_PILEUP                12
#define  PANEL_PILEUP_LUT_ON             13

#define  SPC_ERR                         2
#define  SPC_ERR_ADC_RES                 2
#define  SPC_ERR_SCAN_SIZE               3
#define  SPC_ERR_IGNORE                  4
#define  SPC_ERR_TEXTMSG_2               5

#define  SPC_GRAPH                       3
#define  SPC_GRAPH_OS_CLOSE              2
#define  SPC_GRAPH_MAX_COUNT             3
#define  SPC_GRAPH_GRAPH                 4
#define  SPC_GRAPH_AUTOSCALE             5
#define  SPC_GRAPH_SAVE_PROMT            6

#define  SPC_MAIN                        4
#define  SPC_MAIN_SCANNER_ZOOM           2
#define  SPC_MAIN_STATUS                 3
#define  SPC_MAIN_SCANNER_SPEED          4
#define  SPC_MAIN_SCANNER_RESOLUTION     5
#define  SPC_MAIN_SYNC_LED               6
#define  SPC_MAIN_MEAS_LED               7
#define  SPC_MAIN_TEXTMSG_16             8
#define  SPC_MAIN_TEXTMSG_9              9
#define  SPC_MAIN_ACC_FRAMES             10
#define  SPC_MAIN_ACC_TIME               11
#define  SPC_MAIN_ACQ_TIME               12
#define  SPC_MAIN_SET_ROI                13
#define  SPC_MAIN_DISPLAY                14
#define  SPC_MAIN_ACCUMULATE             15
#define  SPC_MAIN_TO_TW                  16
#define  SPC_MAIN_AUTOSAVE               17
#define  SPC_MAIN_DECORATION_9           18
#define  SPC_MAIN_TEXTMSG_3              19
#define  SPC_MAIN_DECORATION_33          20
#define  SPC_MAIN_DECORATION_31          21
#define  SPC_MAIN_DECORATION_27          22
#define  SPC_MAIN_REPEAT                 23
#define  SPC_MAIN_TAC_SEL                24
#define  SPC_MAIN_FROM_TW                25
#define  SPC_MAIN_DISPLAY_TIME           26
#define  SPC_MAIN_MC_TAC_OFFSET          27
#define  SPC_MAIN_TAC_VAL                28
#define  SPC_MAIN_REPEAT_TIME            29
#define  SPC_MAIN_TEXTMSG_14             30
#define  SPC_MAIN_TEXTMSG_12             31
#define  SPC_MAIN_CLOSE                  32
#define  SPC_MAIN_STOP                   33
#define  SPC_MAIN_ADVANCED               34
#define  SPC_MAIN_SCOPE                  35
#define  SPC_MAIN_START_LIVE             36
#define  SPC_MAIN_START                  37
#define  SPC_MAIN_ACQ_LIMIT_TYPE         38
#define  SPC_MAIN_ACQ_LIMIT_VAL          39
#define  SPC_MAIN_ADC_RES                40
#define  SPC_MAIN_PROGRESS               41

#define  SPC_PARAM                       5
#define  SPC_PARAM_LINE_COMPRESSION      2
#define  SPC_PARAM_PIX_CLOCK             3
#define  SPC_PARAM_PIX_CLOCK_POLARITY    4
#define  SPC_PARAM_Y_POLARITY            5
#define  SPC_PARAM_X_POLARITY            6
#define  SPC_PARAM_TOP_BORDER            7
#define  SPC_PARAM_LEFT_BORDER           8
#define  SPC_PARAM_PIX_CLOCK_DIVIDER     9
#define  SPC_PARAM_TEXTMSG_18            10
#define  SPC_PARAM_DECORATION_30         11
#define  SPC_PARAM_DECORATION_31         12
#define  SPC_PARAM_DECORATION_33         13
#define  SPC_PARAM_DECORATION_32         14
#define  SPC_PARAM_DECORATION_29         15
#define  SPC_PARAM_DECORATION_28         16
#define  SPC_PARAM_DECORATION_27         17
#define  SPC_PARAM_TEXTMSG_29            18
#define  SPC_PARAM_TEXTMSG_22            19
#define  SPC_PARAM_TEXTMSG_21            20
#define  SPC_PARAM_QUIT                  21
#define  SPC_PARAM_SAVE                  22
#define  SPC_PARAM_LOAD                  23
#define  SPC_PARAM_TEXTMSG_13            24
#define  SPC_PARAM_TEXTMSG_33            25
#define  SPC_PARAM_TEXTMSG_9             26
#define  SPC_PARAM_OP_MODE               27
#define  SPC_PARAM_DEAD_TIME             28
#define  SPC_PARAM_COUNT_INC             29
#define  SPC_PARAM_MEM_OFFSET            30
#define  SPC_PARAM_TEXTMSG_20            31
#define  SPC_PARAM_SCAN_Y                32
#define  SPC_PARAM_SCAN_X                33
#define  SPC_PARAM_ROUTING_Y             34
#define  SPC_PARAM_ROUTING_X             35
#define  SPC_PARAM_DELAY                 36
#define  SPC_PARAM_OFLO_CTRL             37
#define  SPC_PARAM_CFD_HOLDOFF           38
#define  SPC_PARAM_CFD_ZC                39
#define  SPC_PARAM_CFD_LIM_HIGH          40
#define  SPC_PARAM_CFD_LIM_LOW           41
#define  SPC_PARAM_TEXTMSG_32            42
#define  SPC_PARAM_TEXTMSG_31            43
#define  SPC_PARAM_TEXTMSG_30            44
#define  SPC_PARAM_TEXTMSG_26            45
#define  SPC_PARAM_TEXTMSG_25            46
#define  SPC_PARAM_TEXTMSG_24            47
#define  SPC_PARAM_TEXTMSG_27            48
#define  SPC_PARAM_TEXTMSG_23            49
#define  SPC_PARAM_TAC_LIM_HIGH          50
#define  SPC_PARAM_SYNC_THRESH           51
#define  SPC_PARAM_SYNC_HOLDOFF          52
#define  SPC_PARAM_SYNC_ZC_LEVEL         53
#define  SPC_PARAM_TEXTMSG_14            54
#define  SPC_PARAM_TAC_LIM_LOW           55
#define  SPC_PARAM_TAC_GAIN              56
#define  SPC_PARAM_TAC_OFFSET            57
#define  SPC_PARAM_TAC_TIME_PER_CHAN     58
#define  SPC_PARAM_TAC_RANGE             59
#define  SPC_PARAM_TEXTMSG_15            60
#define  SPC_PARAM_COLLECT_TIME          61
#define  SPC_PARAM_TEXTMSG               62
#define  SPC_PARAM_TEXTMSG_16            63
#define  SPC_PARAM_TEXTMSG_17            64
#define  SPC_PARAM_SYNC_FREQ_DIV         65
#define  SPC_PARAM_DTHER                 66
#define  SPC_PARAM_ADC_RES               67
#define  SPC_PARAM_TRIGGER               68

#define  SPC_PILEUP                      6
#define  SPC_PILEUP_MILD_COLOUR          2
#define  SPC_PILEUP_MILD_VAL             3
#define  SPC_PILEUP_MODERATE_COLOUR      4
#define  SPC_PILEUP_MODERATE_VAL         5
#define  SPC_PILEUP_SEVERE_COLOUR        6
#define  SPC_PILEUP_SEVERE_VAL           7
#define  SPC_PILEUP_QUIT                 8
#define  SPC_PILEUP_SAVE                 9
#define  SPC_PILEUP_LOAD                 10

#define  SPC_RATES                       7
#define  SPC_RATES_ADC_SLIDE             2
#define  SPC_RATES_TAC_SLIDE             3
#define  SPC_RATES_SYNC_LED              4
#define  SPC_RATES_CFD_SLIDE             5
#define  SPC_RATES_SYNC_SLIDE            6
#define  SPC_RATES_TEXTMSG               7
#define  SPC_RATES_TEXTMSG_2             8
#define  SPC_RATES_TEXTMSG_3             9
#define  SPC_RATES_TEXTMSG_4             10
#define  SPC_RATES_TEXTMSG_5             11
#define  SPC_RATES_TEXTMSG_6             12
#define  SPC_RATES_TEXTMSG_7             13
#define  SPC_RATES_ADC                   14
#define  SPC_RATES_TAC                   15
#define  SPC_RATES_CFD                   16
#define  SPC_RATES_SYNC                  17
#define  SPC_RATES_TEXTMSG_8             18


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* (no callbacks specified in the resource file) */ 


#ifdef __cplusplus
    }
#endif
