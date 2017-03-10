/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2011. All Rights Reserved.          */
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
#define  ABOUT_PNL_TEXTBOX               2
#define  ABOUT_PNL_CLOSE_BUTTON          3

#define  BG_PNL                          2
#define  BG_PNL_COLOUR_NUMERIC           2
#define  BG_PNL_OK_BUTTON                3

#define  CNTER_PNL                       3
#define  CNTER_PNL_LAST_INTENSITY        2
#define  CNTER_PNL_ACTIVE_ID             3
#define  CNTER_PNL_RESET                 4
#define  CNTER_PNL_NAME                  5
#define  CNTER_PNL_COLOUR                6
#define  CNTER_PNL_COUNT                 7
#define  CNTER_PNL_ID                    8

#define  COUNT_PNL                       4
#define  COUNT_PNL_EXPORT_BUTTON         2
#define  COUNT_PNL_ADD_BUTTON            3
#define  COUNT_PNL_OK_BUTTON             4
#define  COUNT_PNL_LAST_INTENSITY        5

#define  DEBUG_PNL                       5
#define  DEBUG_PNL_TEXTBOX               2

#define  FFT_PNL                         6
#define  FFT_PNL_OK_BUTTON               2
#define  FFT_PNL_DEC                     3

#define  GRID_PANEL                      7
#define  GRID_PANEL_GRID_SIZE            2
#define  GRID_PANEL_COLOUR_NUMERIC       3
#define  GRID_PANEL_OK_BUTTON            4
#define  GRID_PANEL_CANCEL_BUTTON        5

#define  HISTPNL                         8
#define  HISTPNL_EXPORT                  2
#define  HISTPNL_QUIT                    3
#define  HISTPNL_X_LABEL                 4
#define  HISTPNL_FIX_YSCALE              5
#define  HISTPNL_FIX_XSCALE              6
#define  HISTPNL_GRAPH                   7
#define  HISTPNL_AVERAGE_LABEL           8

#define  INFO                            9
#define  INFO_REMOVE                     2
#define  INFO_ADD                        3
#define  INFO_EXPORT                     4
#define  INFO_STRING                     5
#define  INFO_OK_BUTTON                  6
#define  INFO_TREE                       7

#define  LSCALE_PNL                      10
#define  LSCALE_PNL_LOWER_NUMERIC        2
#define  LSCALE_PNL_OK_BUTTON            3
#define  LSCALE_PNL_UPPER_NUMERIC        4
#define  LSCALE_PNL_TEXTMSG              5
#define  LSCALE_PNL_GRAPH                6
#define  LSCALE_PNL_DECORATION_4         7
#define  LSCALE_PNL_TEXTMSG_9            8

#define  MAN_SCALE                       11
#define  MAN_SCALE_OK_BUTTON             2
#define  MAN_SCALE_CANCEL_BUTTON         3
#define  MAN_SCALE_AXIS_MAX              4
#define  MAN_SCALE_AXIS_MIN              5

#define  MD_PNL                          12
#define  MD_PNL_CLOSE_BUTTON             2
#define  MD_PNL_RGB_INTERPRET            3

#define  MD_SLIDER                       13
#define  MD_SLIDER_AVERAGE_BUTTON        2
#define  MD_SLIDER_PLAY_BUTTON           3
#define  MD_SLIDER_SUM_BUTTON            4
#define  MD_SLIDER_MAX_BUTTON            5
#define  MD_SLIDER_DECORATION_18         6
#define  MD_SLIDER_DIMENSION             7
#define  MD_SLIDER_TIMER                 8
#define  MD_SLIDER_TEXTMSG_9             9

#define  MD_V_AXIS                       14
#define  MD_V_AXIS_VIEW_AXIS2            2
#define  MD_V_AXIS_VIEW_AXIS1            3
#define  MD_V_AXIS_TEXTMSG               4

#define  PROF_PNL                        15
#define  PROF_PNL_EXPORT                 2
#define  PROF_PNL_QUIT                   3
#define  PROF_PNL_X_LABEL                4
#define  PROF_PNL_GRAPH                  5
#define  PROF_PNL_FIX_SCALE              6
#define  PROF_PNL_PIXEL_VAL              7

#define  REC_PNL                         16
#define  REC_PNL_RECORD_BUTTON           2
#define  REC_PNL_OK_BUTTON               3
#define  REC_PNL_RING                    4
#define  REC_PNL_TEXTMSG                 5

#define  SCALE_PNL                       17
#define  SCALE_PNL_SCALEBAR_LEN_RING     2
#define  SCALE_PNL_COLOUR_NUMERIC        3
#define  SCALE_PNL_TEXTMSG               4
#define  SCALE_PNL_OK_BUTTON             5
#define  SCALE_PNL_CANCEL_BUTTON         6
#define  SCALE_PNL_MICRON_LEN_NUMERIC    7

#define  STREAM_PNL                      18
#define  STREAM_PNL_SNAP_BUTTON          2
#define  STREAM_PNL_LIVE_BUTTON          3
#define  STREAM_PNL_RECORD_BUTTON        4
#define  STREAM_PNL_OK_BUTTON            5
#define  STREAM_PNL_INPUT_CHANNEL_RING   6
#define  STREAM_PNL_DEVICE_RING          7
#define  STREAM_PNL_CODEC_RING           8
#define  STREAM_PNL_DECORATION           9
#define  STREAM_PNL_DECORATION_2         10
#define  STREAM_PNL_TEXTMSG              11

#define  WAVE_PNL                        19
#define  WAVE_PNL_TEXTMSG                2
#define  WAVE_PNL_TEXTMSG_2              3
#define  WAVE_PNL_OK_BUTTON              4
#define  WAVE_PNL_TEXTMSG_3              5
#define  WAVE_PNL_TEXTMSG_4              6
#define  WAVE_PNL_TEXTMSG_5              7
#define  WAVE_PNL_TEXTMSG_6              8
#define  WAVE_PNL_TEXTMSG_7              9
#define  WAVE_PNL_WAVELENGTH_NUMERIC     10

#define  WDM_PNL                         20
#define  WDM_PNL_RECORD_BUTTON           2
#define  WDM_PNL_LIVE_BUTTON             3
#define  WDM_PNL_SNAP_BUTTON             4
#define  WDM_PNL_OK_BUTTON               5
#define  WDM_PNL_UF_RING                 6
#define  WDM_PNL_INPUT_CHANNEL_RING      7
#define  WDM_PNL_FORMAT_RING             8
#define  WDM_PNL_DEVICE_RING             9
#define  WDM_PNL_CODEC_RING              10
#define  WDM_PNL_DECORATION              11
#define  WDM_PNL_DECORATION_2            12
#define  WDM_PNL_TEXTMSG                 13


     /* Menu Bars, Menus, and Menu Items: */

#define  MENUBAR                         1
#define  MENUBAR_MENU                    2
#define  MENUBAR_MENU_VID_CAP_FILTER     3
#define  MENUBAR_MENU_VID_CAP_PIN        4
#define  MENUBAR_MENU_VID_CAP_CB         5
#define  MENUBAR_MENU_VID_CAP_CB2        6
#define  MENUBAR_MENU_SEPARATOR          7
#define  MENUBAR_MENU_FAST_DISPLAY       8
#define  MENUBAR_MENU_FORCE_8BIT         9


     /* (no callbacks specified in the resource file) */ 


#ifdef __cplusplus
    }
#endif
