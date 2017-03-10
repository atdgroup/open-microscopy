#include "KYF75_CTRL.h"
#include <utility.h>
#include <ansi_c.h>

#define LOGFILE

typedef interface tagKYF75_IKYF75_Interface KYF75_IKYF75_Interface;

typedef struct tagKYF75_IKYF75_VTable
{
	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( KYF75_IKYF75_Interface __RPC_FAR * This, 
	                                                         REFIID riid, 
	                                                         void __RPC_FAR *__RPC_FAR *ppvObject);

	ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( KYF75_IKYF75_Interface __RPC_FAR * This);

	ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( KYF75_IKYF75_Interface __RPC_FAR * This);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( KYF75_IKYF75_Interface __RPC_FAR * This, 
	                                                           UINT __RPC_FAR *pctinfo);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( KYF75_IKYF75_Interface __RPC_FAR * This, 
	                                                      UINT iTInfo, 
	                                                      LCID lcid, 
	                                                      ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( KYF75_IKYF75_Interface __RPC_FAR * This, 
	                                                        REFIID riid, 
	                                                        LPOLESTR __RPC_FAR *rgszNames, 
	                                                        UINT cNames, 
	                                                        LCID lcid, 
	                                                        DISPID __RPC_FAR *rgDispId);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( KYF75_IKYF75_Interface __RPC_FAR * This, 
	                                                 DISPID dispIdMember, 
	                                                 REFIID riid, 
	                                                 LCID lcid, 
	                                                 WORD wFlags, 
	                                                 DISPPARAMS __RPC_FAR *pDispParams, 
	                                                 VARIANT __RPC_FAR *pVarResult, 
	                                                 EXCEPINFO __RPC_FAR *pExcepInfo, 
	                                                 UINT __RPC_FAR *puArgErr);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Initialize_) (KYF75_IKYF75_Interface __RPC_FAR *This);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StartIsoc_) (KYF75_IKYF75_Interface __RPC_FAR *This);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StopIsoc_) (KYF75_IKYF75_Interface __RPC_FAR *This);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLiveImage_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                       long *hdib);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStillImage_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                        long *hdib);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FreezeTrigger_) (KYF75_IKYF75_Interface __RPC_FAR *This);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RestartShutter_) (KYF75_IKYF75_Interface __RPC_FAR *This);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReadDeviceList_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long index, 
	                                                         BSTR *item);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMaxValue_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                      long item, 
	                                                      long *value);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMinValue_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                      long item, 
	                                                      long *value);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetValueText_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                       long item, 
	                                                       BSTR *value);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LoadFile_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                   BSTR filepath);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SaveFile_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                   BSTR filepath);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AutoWhiteBalance_) (KYF75_IKYF75_Interface __RPC_FAR *This);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PixelCheck_) (KYF75_IKYF75_Interface __RPC_FAR *This);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *xGetChromaData_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long passwd, 
	                                                         long targetline, 
	                                                         long *ptr);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MemorySave_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                     long channel);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InqIrisPresence_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InqFocusPresence_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                           long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InqZoomPresence_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InqIrisAeDetect_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long *value);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InqIrisAeLevel_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long *value);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InqAutoWhiteBalance_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long *value);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InqRestartShutter_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                            long *value);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InqPatternLevel_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InqPixelCheck_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                        long *value);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DrawAeArea_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                     long on_off);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *xGetWaveformData_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                           long passwd, 
	                                                           long targetline, 
	                                                           long selectline, 
	                                                           long color, 
	                                                           long direction, 
	                                                           long *ptr);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *xGetFirmwareVersion_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long passwd, 
	                                                              BSTR *value);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *xGetLiveFrameRate_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                            long passwd, 
	                                                            BSTR *value);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *xAsyncTransfer_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long passwd, 
	                                                         long quadlet, 
	                                                         long offset, 
	                                                         long reg, 
	                                                         long rw, 
	                                                         long *value);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDeviceCount_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTargetIndex_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetTargetIndex_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetLicenceString_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                           BSTR newValue);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getiris_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                       long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setiris_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                       long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getiris_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                        long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setiris_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                        long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getiris_ae_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                           long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setiris_ae_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                           long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getiris_ae_area_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setiris_ae_area_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getiris_ae_detect_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                            long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setiris_ae_detect_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                            long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getshutter_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setshutter_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getshutter_step_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setshutter_step_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getshutter_vscan_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                 long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setshutter_vscan_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                 long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getshutter_random_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                  long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setshutter_random_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                  long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getgain_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                       long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setgain_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                       long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getgain_step_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                             long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setgain_step_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                             long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getgain_vgain_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setgain_vgain_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getalc_max_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                           long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setalc_max_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                           long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Geteei_limit_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                             long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Seteei_limit_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                             long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getcolor_temp_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                        long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setcolor_temp_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                        long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getwhite_bal_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                            long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setwhite_bal_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                            long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getwhite_bal_auto_level_r_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                    long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setwhite_bal_auto_level_r_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                    long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getwhite_bal_auto_level_b_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                    long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setwhite_bal_auto_level_b_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                    long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getwhite_bal_auto_base_r_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                   long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setwhite_bal_auto_base_r_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                   long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getwhite_bal_auto_base_b_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                   long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setwhite_bal_auto_base_b_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                   long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getwhite_bal_auto2_level_r_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                     long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setwhite_bal_auto2_level_r_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                     long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getwhite_bal_auto2_level_b_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                     long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setwhite_bal_auto2_level_b_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                     long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getwhite_bal_auto2_base_r_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                    long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setwhite_bal_auto2_base_r_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                    long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getwhite_bal_auto2_base_b_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                    long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setwhite_bal_auto2_base_b_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                    long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getwhite_bal_manual_level_r_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                      long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setwhite_bal_manual_level_r_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                      long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getwhite_bal_manual_level_b_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                      long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setwhite_bal_manual_level_b_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                      long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getshading_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setshading_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getshading_level_r_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                             long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setshading_level_r_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                             long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getshading_level_g_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                             long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setshading_level_g_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                             long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getshading_level_b_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                             long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setshading_level_b_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                             long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getdetail_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setdetail_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getdetail_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setdetail_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getdetail_level_dep_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setdetail_level_dep_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getdetail_noise_supp_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                               long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setdetail_noise_supp_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                               long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getgamma_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                        long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setgamma_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                        long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getgamma_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setgamma_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getnega_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                       long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setnega_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                       long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getdsp_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                      long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setdsp_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                      long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getmaster_black_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setmaster_black_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getflare_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                        long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setflare_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                        long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getflare_level_r_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                           long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setflare_level_r_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                           long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getflare_level_b_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                           long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setflare_level_b_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                           long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getabl_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                      long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setabl_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                      long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getabl_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                       long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setabl_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                       long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getpixel_compen_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setpixel_compen_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getcolor_mat_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                            long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setcolor_mat_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                            long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getcolor_mat_level0_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setcolor_mat_level0_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getcolor_mat_level1_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setcolor_mat_level1_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getcolor_mat_level2_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setcolor_mat_level2_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getcolor_mat_level3_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setcolor_mat_level3_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getcolor_mat_level4_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setcolor_mat_level4_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getcolor_mat_level5_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setcolor_mat_level5_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getcolor_mat_level6_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setcolor_mat_level6_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getcolor_mat_level7_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setcolor_mat_level7_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getcolor_mat_level8_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setcolor_mat_level8_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getlens_focus_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setlens_focus_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                              long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getlens_zoom_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                             long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setlens_zoom_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                             long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getfreeze_cancel_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setfreeze_cancel_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Gettest_pattern_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                               long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Settest_pattern_mode_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                               long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Gettest_pattern_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Settest_pattern_level_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                                long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getstill_color_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setstill_color_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getstill_step_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                        long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setstill_step_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                        long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getstill_aspect_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setstill_aspect_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getstill_bytes_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getstill_height_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                          long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getstill_width_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getlive_color_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                        long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setlive_color_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                        long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getlive_step_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                       long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setlive_step_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                       long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getlive_aspect_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Setlive_aspect_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getlive_bytes_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                        long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getlive_height_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                         long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getlive_width_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                        long *pVal);

	HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Getfreeze_status_) (KYF75_IKYF75_Interface __RPC_FAR *This, 
	                                                           long *pVal);

} KYF75_IKYF75_VTable;

typedef interface tagKYF75_IKYF75_Interface
{
	CONST_VTBL KYF75_IKYF75_VTable __RPC_FAR *lpVtbl;
} KYF75_IKYF75_Interface;

const IID KYF75_IID_IKYF75 =
	{
		0x131C560E, 0xC885, 0x4EB0, 0xBA, 0x31, 0x5B, 0xB1, 0x3A, 0x8, 0xE2, 0x8
	};


HRESULT CVIFUNC KYF75_NewIKYF75 (const char *server, int supportMultithreading,
                                 LCID locale, int reserved,
                                 CAObjHandle *objectHandle)
{
	HRESULT __result = S_OK;
	GUID clsid = {0x1DE5E484, 0x4D31, 0x4211, 0x92, 0xB7, 0xFB, 0xE0, 0xC0,
	              0xE8, 0x4E, 0x42};

	__result = CA_CreateObjectByClassIdEx (&clsid, server, &KYF75_IID_IKYF75,
	                                       supportMultithreading, locale,
	                                       reserved, objectHandle);

	return __result;
}

HRESULT CVIFUNC KYF75_OpenIKYF75 (const char *fileName, const char *server,
                                  int supportMultithreading, LCID locale,
                                  int reserved, CAObjHandle *objectHandle)
{
	HRESULT __result = S_OK;
	GUID clsid = {0x1DE5E484, 0x4D31, 0x4211, 0x92, 0xB7, 0xFB, 0xE0, 0xC0,
	              0xE8, 0x4E, 0x42};

	__result = CA_LoadObjectFromFileByClassIdEx (fileName, &clsid, server,
	                                             &KYF75_IID_IKYF75,
	                                             supportMultithreading, locale,
	                                             reserved, objectHandle);

	return __result;
}

HRESULT CVIFUNC KYF75_ActiveIKYF75 (const char *server,
                                    int supportMultithreading, LCID locale,
                                    int reserved, CAObjHandle *objectHandle)
{
	HRESULT __result = S_OK;
	GUID clsid = {0x1DE5E484, 0x4D31, 0x4211, 0x92, 0xB7, 0xFB, 0xE0, 0xC0,
	              0xE8, 0x4E, 0x42};

	__result = CA_GetActiveObjectByClassIdEx (&clsid, server,
	                                          &KYF75_IID_IKYF75,
	                                          supportMultithreading, locale,
	                                          reserved, objectHandle);

	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Initialize (CAObjHandle objectHandle,
                                        ERRORINFO *errorInfo)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Initialize_ (__vtblIFacePtr));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75StartIsoc (CAObjHandle objectHandle,
                                       ERRORINFO *errorInfo)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->StartIsoc_ (__vtblIFacePtr));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75StopIsoc (CAObjHandle objectHandle,
                                      ERRORINFO *errorInfo)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->StopIsoc_ (__vtblIFacePtr));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75GetLiveImage (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long *hdib)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long hdib__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->GetLiveImage_ (__vtblIFacePtr,
	                                                   &hdib__Temp));

	if (hdib)
		{
		*hdib = hdib__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75GetStillImage (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *hdib)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef = 0;
	int __errorInfoPresent = 0;
	long hdib__Temp = 0;

#ifdef LOGFILE
	gfp = fopen ("c:\\jvc.log", "a");
	fprintf (gfp, "KYF75_IKYF75GetStillImage 1\n");
	fclose (gfp);
#endif

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
#ifdef LOGFILE
	gfp = fopen ("c:\\jvc.log", "a");
	fprintf (gfp, "KYF75_IKYF75GetStillImage 2, __vtblIFacePtr = %d\n", (long)(__vtblIFacePtr->lpVtbl->GetStillImage_));
	fclose (gfp);
#endif

	__caErrChk (__vtblIFacePtr->lpVtbl->GetStillImage_ (__vtblIFacePtr, &hdib__Temp));
#ifdef LOGFILE
	gfp = fopen ("c:\\jvc.log", "a");
	fprintf (gfp, "KYF75_IKYF75GetStillImage 3\n");
	fclose (gfp);
#endif


	if (hdib)
		{
		*hdib = hdib__Temp;
		}

#ifdef LOGFILE
	gfp = fopen ("c:\\jvc.log", "a");
	fprintf (gfp, "KYF75_IKYF75GetStillImage 4\n");
	fclose (gfp);
#endif

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
#ifdef LOGFILE
	gfp = fopen ("c:\\jvc.log", "a");
	fprintf (gfp, "KYF75_IKYF75GetStillImage 5\n");
	fclose (gfp);
#endif

	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
#ifdef LOGFILE
	gfp = fopen ("c:\\jvc.log", "a");
	fprintf (gfp, "KYF75_IKYF75GetStillImage 6\n");
	fclose (gfp);
#endif

	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75FreezeTrigger (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->FreezeTrigger_ (__vtblIFacePtr));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75RestartShutter (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->RestartShutter_ (__vtblIFacePtr));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75ReadDeviceList (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long index,
                                            char **item)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	BSTR item__AutoType = 0;

	if (item)
		*item = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->ReadDeviceList_ (__vtblIFacePtr, index,
	                                                     &item__AutoType));

	if (item)
		__caErrChk (CA_BSTRGetCString (item__AutoType, item));

Error:
	CA_FreeBSTR (item__AutoType);
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	if (FAILED(__result))
		{
		if (item)
			{
			CA_FreeMemory (*item);
			*item = 0;
			}
		}
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75GetMaxValue (CAObjHandle objectHandle,
                                         ERRORINFO *errorInfo, long item,
                                         long *value)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long value__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->GetMaxValue_ (__vtblIFacePtr, item,
	                                                  &value__Temp));

	if (value)
		{
		*value = value__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75GetMinValue (CAObjHandle objectHandle,
                                         ERRORINFO *errorInfo, long item,
                                         long *value)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long value__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->GetMinValue_ (__vtblIFacePtr, item,
	                                                  &value__Temp));

	if (value)
		{
		*value = value__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75GetValueText (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long item,
                                          char **value)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	BSTR value__AutoType = 0;

	if (value)
		*value = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->GetValueText_ (__vtblIFacePtr, item,
	                                                   &value__AutoType));

	if (value)
		__caErrChk (CA_BSTRGetCString (value__AutoType, value));

Error:
	CA_FreeBSTR (value__AutoType);
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	if (FAILED(__result))
		{
		if (value)
			{
			CA_FreeMemory (*value);
			*value = 0;
			}
		}
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75LoadFile (CAObjHandle objectHandle,
                                      ERRORINFO *errorInfo, const char *filepath)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	BSTR filepath__AutoType = 0;

	__caErrChk (CA_CStringToBSTR (filepath, &filepath__AutoType));

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->LoadFile_ (__vtblIFacePtr,
	                                               filepath__AutoType));

Error:
	CA_FreeBSTR (filepath__AutoType);
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75SaveFile (CAObjHandle objectHandle,
                                      ERRORINFO *errorInfo, const char *filepath)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	BSTR filepath__AutoType = 0;

	__caErrChk (CA_CStringToBSTR (filepath, &filepath__AutoType));

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->SaveFile_ (__vtblIFacePtr,
	                                               filepath__AutoType));

Error:
	CA_FreeBSTR (filepath__AutoType);
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75AutoWhiteBalance (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->AutoWhiteBalance_ (__vtblIFacePtr));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75PixelCheck (CAObjHandle objectHandle,
                                        ERRORINFO *errorInfo)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->PixelCheck_ (__vtblIFacePtr));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75xGetChromaData (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long passwd,
                                            long targetline, long *ptr)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long ptr__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->xGetChromaData_ (__vtblIFacePtr,
	                                                     passwd, targetline,
	                                                     &ptr__Temp));

	if (ptr)
		{
		*ptr = ptr__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75MemorySave (CAObjHandle objectHandle,
                                        ERRORINFO *errorInfo, long channel)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->MemorySave_ (__vtblIFacePtr, channel));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75InqIrisPresence (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->InqIrisPresence_ (__vtblIFacePtr,
	                                                      &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75InqFocusPresence (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->InqFocusPresence_ (__vtblIFacePtr,
	                                                       &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75InqZoomPresence (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->InqZoomPresence_ (__vtblIFacePtr,
	                                                      &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75InqIrisAeDetect (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *value)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long value__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->InqIrisAeDetect_ (__vtblIFacePtr,
	                                                      &value__Temp));

	if (value)
		{
		*value = value__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75InqIrisAeLevel (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *value)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long value__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->InqIrisAeLevel_ (__vtblIFacePtr,
	                                                     &value__Temp));

	if (value)
		{
		*value = value__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75InqAutoWhiteBalance (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *value)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long value__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->InqAutoWhiteBalance_ (__vtblIFacePtr,
	                                                          &value__Temp));

	if (value)
		{
		*value = value__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75InqRestartShutter (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long *value)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long value__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->InqRestartShutter_ (__vtblIFacePtr,
	                                                        &value__Temp));

	if (value)
		{
		*value = value__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75InqPatternLevel (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->InqPatternLevel_ (__vtblIFacePtr,
	                                                      &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75InqPixelCheck (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *value)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long value__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->InqPixelCheck_ (__vtblIFacePtr,
	                                                    &value__Temp));

	if (value)
		{
		*value = value__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75DrawAeArea (CAObjHandle objectHandle,
                                        ERRORINFO *errorInfo, long on_off)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->DrawAeArea_ (__vtblIFacePtr, on_off));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75xGetWaveformData (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long passwd,
                                              long targetline, long selectline,
                                              long color, long direction,
                                              long *ptr)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long ptr__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->xGetWaveformData_ (__vtblIFacePtr,
	                                                       passwd, targetline,
	                                                       selectline, color,
	                                                       direction,
	                                                       &ptr__Temp));

	if (ptr)
		{
		*ptr = ptr__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75xGetFirmwareVersion (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long passwd, char **value)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	BSTR value__AutoType = 0;

	if (value)
		*value = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->xGetFirmwareVersion_ (__vtblIFacePtr,
	                                                          passwd,
	                                                          &value__AutoType));

	if (value)
		__caErrChk (CA_BSTRGetCString (value__AutoType, value));

Error:
	CA_FreeBSTR (value__AutoType);
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	if (FAILED(__result))
		{
		if (value)
			{
			CA_FreeMemory (*value);
			*value = 0;
			}
		}
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75xGetLiveFrameRate (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long passwd,
                                               char **value)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	BSTR value__AutoType = 0;

	if (value)
		*value = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->xGetLiveFrameRate_ (__vtblIFacePtr,
	                                                        passwd,
	                                                        &value__AutoType));

	if (value)
		__caErrChk (CA_BSTRGetCString (value__AutoType, value));

Error:
	CA_FreeBSTR (value__AutoType);
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	if (FAILED(__result))
		{
		if (value)
			{
			CA_FreeMemory (*value);
			*value = 0;
			}
		}
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75xAsyncTransfer (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long passwd,
                                            long quadlet, long offset, long reg,
                                            long rw, long *value)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long value__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->xAsyncTransfer_ (__vtblIFacePtr,
	                                                     passwd, quadlet,
	                                                     offset, reg, rw,
	                                                     &value__Temp));

	if (value)
		{
		*value = value__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75GetDeviceCount (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->GetDeviceCount_ (__vtblIFacePtr,
	                                                     &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75GetTargetIndex (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->GetTargetIndex_ (__vtblIFacePtr,
	                                                     &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75SetTargetIndex (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->SetTargetIndex_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75SetLicenceString (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo,
                                              const char *newValue)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	BSTR newValue__AutoType = 0;

	__caErrChk (CA_CStringToBSTR (newValue, &newValue__AutoType));

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->SetLicenceString_ (__vtblIFacePtr,
	                                                       newValue__AutoType));

Error:
	CA_FreeBSTR (newValue__AutoType);
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getiris_mode (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getiris_mode_ (__vtblIFacePtr,
	                                                   &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setiris_mode (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setiris_mode_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getiris_level (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getiris_level_ (__vtblIFacePtr,
	                                                    &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setiris_level (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setiris_level_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getiris_ae_level (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getiris_ae_level_ (__vtblIFacePtr,
	                                                       &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setiris_ae_level (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setiris_ae_level_ (__vtblIFacePtr,
	                                                       pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getiris_ae_area (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getiris_ae_area_ (__vtblIFacePtr,
	                                                      &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setiris_ae_area (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setiris_ae_area_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getiris_ae_detect (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getiris_ae_detect_ (__vtblIFacePtr,
	                                                        &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setiris_ae_detect (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setiris_ae_detect_ (__vtblIFacePtr,
	                                                        pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getshutter_mode (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getshutter_mode_ (__vtblIFacePtr,
	                                                      &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setshutter_mode (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setshutter_mode_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getshutter_step_level (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getshutter_step_level_ (__vtblIFacePtr,
	                                                            &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setshutter_step_level (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setshutter_step_level_ (__vtblIFacePtr,
	                                                            pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getshutter_vscan_level (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getshutter_vscan_level_ (__vtblIFacePtr,
	                                                             &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setshutter_vscan_level (CAObjHandle objectHandle,
                                                    ERRORINFO *errorInfo,
                                                    long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setshutter_vscan_level_ (__vtblIFacePtr,
	                                                             pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getshutter_random_level (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getshutter_random_level_ (__vtblIFacePtr,
	                                                              &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setshutter_random_level (CAObjHandle objectHandle,
                                                     ERRORINFO *errorInfo,
                                                     long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setshutter_random_level_ (__vtblIFacePtr,
	                                                              pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getgain_mode (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getgain_mode_ (__vtblIFacePtr,
	                                                   &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setgain_mode (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setgain_mode_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getgain_step_level (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getgain_step_level_ (__vtblIFacePtr,
	                                                         &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setgain_step_level (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setgain_step_level_ (__vtblIFacePtr,
	                                                         pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getgain_vgain_level (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getgain_vgain_level_ (__vtblIFacePtr,
	                                                          &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setgain_vgain_level (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setgain_vgain_level_ (__vtblIFacePtr,
	                                                          pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getalc_max_level (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getalc_max_level_ (__vtblIFacePtr,
	                                                       &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setalc_max_level (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setalc_max_level_ (__vtblIFacePtr,
	                                                       pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Geteei_limit_level (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Geteei_limit_level_ (__vtblIFacePtr,
	                                                         &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Seteei_limit_level (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Seteei_limit_level_ (__vtblIFacePtr,
	                                                         pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getcolor_temp (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getcolor_temp_ (__vtblIFacePtr,
	                                                    &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setcolor_temp (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setcolor_temp_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_mode (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getwhite_bal_mode_ (__vtblIFacePtr,
	                                                        &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_mode (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setwhite_bal_mode_ (__vtblIFacePtr,
	                                                        pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_auto_level_r (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getwhite_bal_auto_level_r_ (__vtblIFacePtr,
	                                                                &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_auto_level_r (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setwhite_bal_auto_level_r_ (__vtblIFacePtr,
	                                                                pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_auto_level_b (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getwhite_bal_auto_level_b_ (__vtblIFacePtr,
	                                                                &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_auto_level_b (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setwhite_bal_auto_level_b_ (__vtblIFacePtr,
	                                                                pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_auto_base_r (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getwhite_bal_auto_base_r_ (__vtblIFacePtr,
	                                                               &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_auto_base_r (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setwhite_bal_auto_base_r_ (__vtblIFacePtr,
	                                                               pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_auto_base_b (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getwhite_bal_auto_base_b_ (__vtblIFacePtr,
	                                                               &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_auto_base_b (CAObjHandle objectHandle,
                                                      ERRORINFO *errorInfo,
                                                      long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setwhite_bal_auto_base_b_ (__vtblIFacePtr,
	                                                               pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_auto2_level_r (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getwhite_bal_auto2_level_r_ (__vtblIFacePtr,
	                                                                 &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_auto2_level_r (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setwhite_bal_auto2_level_r_ (__vtblIFacePtr,
	                                                                 pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_auto2_level_b (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getwhite_bal_auto2_level_b_ (__vtblIFacePtr,
	                                                                 &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_auto2_level_b (CAObjHandle objectHandle,
                                                        ERRORINFO *errorInfo,
                                                        long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setwhite_bal_auto2_level_b_ (__vtblIFacePtr,
	                                                                 pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_auto2_base_r (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getwhite_bal_auto2_base_r_ (__vtblIFacePtr,
	                                                                &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_auto2_base_r (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setwhite_bal_auto2_base_r_ (__vtblIFacePtr,
	                                                                pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_auto2_base_b (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getwhite_bal_auto2_base_b_ (__vtblIFacePtr,
	                                                                &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_auto2_base_b (CAObjHandle objectHandle,
                                                       ERRORINFO *errorInfo,
                                                       long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setwhite_bal_auto2_base_b_ (__vtblIFacePtr,
	                                                                pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_manual_level_r (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getwhite_bal_manual_level_r_ (__vtblIFacePtr,
	                                                                  &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_manual_level_r (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setwhite_bal_manual_level_r_ (__vtblIFacePtr,
	                                                                  pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getwhite_bal_manual_level_b (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getwhite_bal_manual_level_b_ (__vtblIFacePtr,
	                                                                  &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setwhite_bal_manual_level_b (CAObjHandle objectHandle,
                                                         ERRORINFO *errorInfo,
                                                         long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setwhite_bal_manual_level_b_ (__vtblIFacePtr,
	                                                                  pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getshading_mode (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getshading_mode_ (__vtblIFacePtr,
	                                                      &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setshading_mode (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setshading_mode_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getshading_level_r (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getshading_level_r_ (__vtblIFacePtr,
	                                                         &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setshading_level_r (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setshading_level_r_ (__vtblIFacePtr,
	                                                         pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getshading_level_g (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getshading_level_g_ (__vtblIFacePtr,
	                                                         &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setshading_level_g (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setshading_level_g_ (__vtblIFacePtr,
	                                                         pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getshading_level_b (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getshading_level_b_ (__vtblIFacePtr,
	                                                         &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setshading_level_b (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setshading_level_b_ (__vtblIFacePtr,
	                                                         pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getdetail_mode (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getdetail_mode_ (__vtblIFacePtr,
	                                                     &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setdetail_mode (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setdetail_mode_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getdetail_level (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getdetail_level_ (__vtblIFacePtr,
	                                                      &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setdetail_level (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setdetail_level_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getdetail_level_dep (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getdetail_level_dep_ (__vtblIFacePtr,
	                                                          &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setdetail_level_dep (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setdetail_level_dep_ (__vtblIFacePtr,
	                                                          pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getdetail_noise_supp (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getdetail_noise_supp_ (__vtblIFacePtr,
	                                                           &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setdetail_noise_supp (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setdetail_noise_supp_ (__vtblIFacePtr,
	                                                           pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getgamma_mode (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getgamma_mode_ (__vtblIFacePtr,
	                                                    &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setgamma_mode (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setgamma_mode_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getgamma_level (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getgamma_level_ (__vtblIFacePtr,
	                                                     &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setgamma_level (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setgamma_level_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getnega_mode (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getnega_mode_ (__vtblIFacePtr,
	                                                   &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setnega_mode (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setnega_mode_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getdsp_mode (CAObjHandle objectHandle,
                                         ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getdsp_mode_ (__vtblIFacePtr,
	                                                  &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setdsp_mode (CAObjHandle objectHandle,
                                         ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setdsp_mode_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getmaster_black_level (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getmaster_black_level_ (__vtblIFacePtr,
	                                                            &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setmaster_black_level (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setmaster_black_level_ (__vtblIFacePtr,
	                                                            pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getflare_mode (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getflare_mode_ (__vtblIFacePtr,
	                                                    &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setflare_mode (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setflare_mode_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getflare_level_r (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getflare_level_r_ (__vtblIFacePtr,
	                                                       &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setflare_level_r (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setflare_level_r_ (__vtblIFacePtr,
	                                                       pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getflare_level_b (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getflare_level_b_ (__vtblIFacePtr,
	                                                       &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setflare_level_b (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setflare_level_b_ (__vtblIFacePtr,
	                                                       pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getabl_mode (CAObjHandle objectHandle,
                                         ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getabl_mode_ (__vtblIFacePtr,
	                                                  &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setabl_mode (CAObjHandle objectHandle,
                                         ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setabl_mode_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getabl_level (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getabl_level_ (__vtblIFacePtr,
	                                                   &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setabl_level (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setabl_level_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getpixel_compen (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getpixel_compen_ (__vtblIFacePtr,
	                                                      &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setpixel_compen (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setpixel_compen_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_mode (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getcolor_mat_mode_ (__vtblIFacePtr,
	                                                        &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_mode (CAObjHandle objectHandle,
                                               ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setcolor_mat_mode_ (__vtblIFacePtr,
	                                                        pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level0 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getcolor_mat_level0_ (__vtblIFacePtr,
	                                                          &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level0 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setcolor_mat_level0_ (__vtblIFacePtr,
	                                                          pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level1 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getcolor_mat_level1_ (__vtblIFacePtr,
	                                                          &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level1 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setcolor_mat_level1_ (__vtblIFacePtr,
	                                                          pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level2 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getcolor_mat_level2_ (__vtblIFacePtr,
	                                                          &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level2 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setcolor_mat_level2_ (__vtblIFacePtr,
	                                                          pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level3 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getcolor_mat_level3_ (__vtblIFacePtr,
	                                                          &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level3 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setcolor_mat_level3_ (__vtblIFacePtr,
	                                                          pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level4 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getcolor_mat_level4_ (__vtblIFacePtr,
	                                                          &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level4 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setcolor_mat_level4_ (__vtblIFacePtr,
	                                                          pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level5 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getcolor_mat_level5_ (__vtblIFacePtr,
	                                                          &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level5 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setcolor_mat_level5_ (__vtblIFacePtr,
	                                                          pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level6 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getcolor_mat_level6_ (__vtblIFacePtr,
	                                                          &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level6 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setcolor_mat_level6_ (__vtblIFacePtr,
	                                                          pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level7 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getcolor_mat_level7_ (__vtblIFacePtr,
	                                                          &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level7 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setcolor_mat_level7_ (__vtblIFacePtr,
	                                                          pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getcolor_mat_level8 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getcolor_mat_level8_ (__vtblIFacePtr,
	                                                          &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setcolor_mat_level8 (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setcolor_mat_level8_ (__vtblIFacePtr,
	                                                          pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getlens_focus_level (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo,
                                                 long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getlens_focus_level_ (__vtblIFacePtr,
	                                                          &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setlens_focus_level (CAObjHandle objectHandle,
                                                 ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setlens_focus_level_ (__vtblIFacePtr,
	                                                          pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getlens_zoom_level (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getlens_zoom_level_ (__vtblIFacePtr,
	                                                         &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setlens_zoom_level (CAObjHandle objectHandle,
                                                ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setlens_zoom_level_ (__vtblIFacePtr,
	                                                         pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getfreeze_cancel_mode (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getfreeze_cancel_mode_ (__vtblIFacePtr,
	                                                            &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setfreeze_cancel_mode (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setfreeze_cancel_mode_ (__vtblIFacePtr,
	                                                            pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Gettest_pattern_mode (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Gettest_pattern_mode_ (__vtblIFacePtr,
	                                                           &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Settest_pattern_mode (CAObjHandle objectHandle,
                                                  ERRORINFO *errorInfo,
                                                  long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Settest_pattern_mode_ (__vtblIFacePtr,
	                                                           pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Gettest_pattern_level (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Gettest_pattern_level_ (__vtblIFacePtr,
	                                                            &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Settest_pattern_level (CAObjHandle objectHandle,
                                                   ERRORINFO *errorInfo,
                                                   long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Settest_pattern_level_ (__vtblIFacePtr,
	                                                            pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getstill_color (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getstill_color_ (__vtblIFacePtr,
	                                                     &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setstill_color (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setstill_color_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getstill_step (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getstill_step_ (__vtblIFacePtr,
	                                                    &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setstill_step (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setstill_step_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getstill_aspect (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getstill_aspect_ (__vtblIFacePtr,
	                                                      &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setstill_aspect (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setstill_aspect_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getstill_bytes (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getstill_bytes_ (__vtblIFacePtr,
	                                                     &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getstill_height (CAObjHandle objectHandle,
                                             ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getstill_height_ (__vtblIFacePtr,
	                                                      &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getstill_width (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getstill_width_ (__vtblIFacePtr,
	                                                     &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getlive_color (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getlive_color_ (__vtblIFacePtr,
	                                                    &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setlive_color (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setlive_color_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getlive_step (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getlive_step_ (__vtblIFacePtr,
	                                                   &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setlive_step (CAObjHandle objectHandle,
                                          ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setlive_step_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getlive_aspect (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getlive_aspect_ (__vtblIFacePtr,
	                                                     &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Setlive_aspect (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Setlive_aspect_ (__vtblIFacePtr, pVal));

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getlive_bytes (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getlive_bytes_ (__vtblIFacePtr,
	                                                    &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getlive_height (CAObjHandle objectHandle,
                                            ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getlive_height_ (__vtblIFacePtr,
	                                                     &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getlive_width (CAObjHandle objectHandle,
                                           ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getlive_width_ (__vtblIFacePtr,
	                                                    &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}

HRESULT CVIFUNC KYF75_IKYF75Getfreeze_status (CAObjHandle objectHandle,
                                              ERRORINFO *errorInfo, long *pVal)
{
	HRESULT __result = S_OK;
	KYF75_IKYF75_Interface * __vtblIFacePtr = 0;
	int __didAddRef;
	int __errorInfoPresent = 0;
	long pVal__Temp;

	__caErrChk (CA_GetInterfaceFromObjHandle (objectHandle, &KYF75_IID_IKYF75,
	                                          0, &__vtblIFacePtr, &__didAddRef));
	__caErrChk (__vtblIFacePtr->lpVtbl->Getfreeze_status_ (__vtblIFacePtr,
	                                                       &pVal__Temp));

	if (pVal)
		{
		*pVal = pVal__Temp;
		}

Error:
	if (__vtblIFacePtr && __didAddRef)
		__vtblIFacePtr->lpVtbl->Release (__vtblIFacePtr);
	CA_FillErrorInfoEx (objectHandle, &KYF75_IID_IKYF75, __result, errorInfo,
	                    &__errorInfoPresent);
	if (__errorInfoPresent)
		__result = DISP_E_EXCEPTION;
	return __result;
}
