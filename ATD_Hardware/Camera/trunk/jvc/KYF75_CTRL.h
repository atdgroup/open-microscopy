#ifndef _KYF75_H
#define _KYF75_H

#if defined(INCLUDE_AFTER_WINDOWS_H) && !defined(_INC_WINDOWS)
#error  This header must be included before utility.h and formatio.h
#error  because it includes cviauto.h which includes Windows SDK headers.
#endif /* INCLUDE_AFTER_WINDOWS_H */

#include <cviauto.h>

#ifdef __cplusplus
    extern "C" {
#endif
extern const IID KYF75_IID_IKYF75;

HRESULT CVIFUNC KYF75_NewIKYF75 (const char *server, int supportMultithreading,
                                 LCID locale, int reserved,
                                 CAObjHandle *objectHandle);

HRESULT CVIFUNC KYF75_OpenIKYF75 (const char *fileName, const char *server,
                                  int supportMultithreading, LCID locale,
                                  int reserved, CAObjHandle *objectHandle);

HRESULT CVIFUNC KYF75_ActiveIKYF75 (const char *server,
                                    int supportMultithreading, LCID locale,
                                    int reserved, CAObjHandle *objectHandle);

HRESULT CVIFUNC KYF75_IKYF75Initialize (CAObjHandle objectHandle,
                                        ERRORINFO *errorInfo);

HRESULT CVIFUNC KYF75_IKYF75StartIsoc (CAObjHandle objectHandle,
                                       ERRORINFO *errorInfo);

HRESULT CVIFUNC KYF75_IKYF75StopIsoc (CAObjHandle objectHandle,
                                      ERRORINFO *errorInfo);

HRESULT CVIFUNC KYF75_IKYF75GetLiveImage (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long *hdib);

HRESULT CVIFUNC KYF75_IKYF75GetStillImage (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *hdib);

HRESULT CVIFUNC KYF75_IKYF75FreezeTrigger (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo);

HRESULT CVIFUNC KYF75_IKYF75RestartShutter (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo);

HRESULT CVIFUNC KYF75_IKYF75ReadDeviceList (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long index,
                                            char **item);

HRESULT CVIFUNC KYF75_IKYF75GetMaxValue (CAObjHandle objectHandle,
                                         ERRORINFO *errorInfo, long item,
                                         long *value);

HRESULT CVIFUNC KYF75_IKYF75GetMinValue (CAObjHandle objectHandle,
                                         ERRORINFO *errorInfo, long item,
                                         long *value);

HRESULT CVIFUNC KYF75_IKYF75GetValueText (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long item,
                                          char **value);

HRESULT CVIFUNC KYF75_IKYF75LoadFile (CAObjHandle objectHandle,
                                      ERRORINFO *errorInfo, const char *filepath);

HRESULT CVIFUNC KYF75_IKYF75SaveFile (CAObjHandle objectHandle,
                                      ERRORINFO *errorInfo, const char *filepath);

HRESULT CVIFUNC KYF75_IKYF75AutoWhiteBalance (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo);

HRESULT CVIFUNC KYF75_IKYF75PixelCheck (CAObjHandle objectHandle,
                                        ERRORINFO *errorInfo);

HRESULT CVIFUNC KYF75_IKYF75xGetChromaData (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long passwd,
                                            long targetline, long *ptr);

HRESULT CVIFUNC KYF75_IKYF75MemorySave (CAObjHandle objectHandle,
                                        ERRORINFO *errorInfo, long channel);

HRESULT CVIFUNC KYF75_IKYF75InqIrisPresence (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75InqFocusPresence (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75InqZoomPresence (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75InqIrisAeDetect (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *value);

HRESULT CVIFUNC KYF75_IKYF75InqIrisAeLevel (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *value);

HRESULT CVIFUNC KYF75_IKYF75InqAutoWhiteBalance (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *value);

HRESULT CVIFUNC KYF75_IKYF75InqRestartShutter (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long *value);

HRESULT CVIFUNC KYF75_IKYF75InqPatternLevel (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75InqPixelCheck (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *value);

HRESULT CVIFUNC KYF75_IKYF75DrawAeArea (CAObjHandle objectHandle,
                                        ERRORINFO *errorInfo, long on_off);

HRESULT CVIFUNC KYF75_IKYF75xGetWaveformData (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long passwd,
                                              long targetline, long selectline,
                                              long color, long direction,
                                              long *ptr);

HRESULT CVIFUNC KYF75_IKYF75xGetFirmwareVersion (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long passwd, char **value);

HRESULT CVIFUNC KYF75_IKYF75xGetLiveFrameRate (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long passwd,
                                               char **value);

HRESULT CVIFUNC KYF75_IKYF75xAsyncTransfer (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long passwd,
                                            long quadlet, long offset, long reg,
                                            long rw, long *value);

HRESULT CVIFUNC KYF75_IKYF75GetDeviceCount (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75GetTargetIndex (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75SetTargetIndex (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75SetLicenceString (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              const char *newValue);

HRESULT CVIFUNC KYF75_IKYF75Getiris_mode (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setiris_mode (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getiris_level (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setiris_level (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getiris_ae_level (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setiris_ae_level (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getiris_ae_area (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setiris_ae_area (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getiris_ae_detect (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setiris_ae_detect (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getshutter_mode (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setshutter_mode (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getshutter_step_level (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setshutter_step_level (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getshutter_vscan_level (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setshutter_vscan_level (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getshutter_random_level (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setshutter_random_level (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getgain_mode (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setgain_mode (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getgain_step_level (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setgain_step_level (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getgain_vgain_level (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setgain_vgain_level (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getalc_max_level (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setalc_max_level (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Geteei_limit_level (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Seteei_limit_level (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getcolor_temp (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setcolor_temp (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_mode (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_mode (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_auto_level_r (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_auto_level_r (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_auto_level_b (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_auto_level_b (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_auto_base_r (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_auto_base_r (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_auto_base_b (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_auto_base_b (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_auto2_level_r (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_auto2_level_r (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_auto2_level_b (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_auto2_level_b (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_auto2_base_r (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_auto2_base_r (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_auto2_base_b (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_auto2_base_b (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_manual_level_r (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_manual_level_r (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_manual_level_b (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_manual_level_b (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getshading_mode (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setshading_mode (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getshading_level_r (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setshading_level_r (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getshading_level_g (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setshading_level_g (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getshading_level_b (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setshading_level_b (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getdetail_mode (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setdetail_mode (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getdetail_level (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setdetail_level (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getdetail_level_dep (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setdetail_level_dep (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getdetail_noise_supp (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setdetail_noise_supp (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getgamma_mode (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setgamma_mode (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getgamma_level (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setgamma_level (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getnega_mode (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setnega_mode (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getdsp_mode (CAObjHandle objectHandle,
                                         ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setdsp_mode (CAObjHandle objectHandle,
                                         ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getmaster_black_level (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setmaster_black_level (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getflare_mode (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setflare_mode (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getflare_level_r (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setflare_level_r (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getflare_level_b (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setflare_level_b (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getabl_mode (CAObjHandle objectHandle,
                                         ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setabl_mode (CAObjHandle objectHandle,
                                         ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getabl_level (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setabl_level (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getpixel_compen (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setpixel_compen (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_mode (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_mode (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level0 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level0 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level1 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level1 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level2 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level2 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level3 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level3 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level4 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level4 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level5 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level5 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level6 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level6 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level7 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level7 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level8 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level8 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getlens_focus_level (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setlens_focus_level (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getlens_zoom_level (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setlens_zoom_level (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getfreeze_cancel_mode (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setfreeze_cancel_mode (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long pVal);

HRESULT CVIFUNC KYF75_IKYF75Gettest_pattern_mode (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Settest_pattern_mode (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  long pVal);

HRESULT CVIFUNC KYF75_IKYF75Gettest_pattern_level (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Settest_pattern_level (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getstill_color (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setstill_color (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getstill_step (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setstill_step (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getstill_aspect (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setstill_aspect (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getstill_bytes (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Getstill_height (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Getstill_width (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Getlive_color (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setlive_color (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getlive_step (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setlive_step (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getlive_aspect (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Setlive_aspect (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long pVal);

HRESULT CVIFUNC KYF75_IKYF75Getlive_bytes (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Getlive_height (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Getlive_width (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *pVal);

HRESULT CVIFUNC KYF75_IKYF75Getfreeze_status (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long *pVal);
#ifdef __cplusplus
    }
#endif
#endif /* _KYF75_H */
