#include "uir_files\gci_camera_jvc_ui.h"
#include "jvc\gci_jvc_camera.h"
#include <userint.h>
#include <utility.h>

#include "asynctmr.h" 

int CVICALLBACK JvcCamera_onExtrasQuit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData;
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			gci_camera_hide_extra_ui(camera);

			break;
		}
	return 0;
}


int CVICALLBACK JvcCamera_onTestPatternDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData;
	GciCamera *camera = (GciCamera*) jvc_camera;
	
	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
    		if(live)
				gci_camera_set_snap_mode(camera);

			gci_jvc_camera_set_test_pattern_mode(jvc_camera, val);
			
			if (live) {
			
    			gci_camera_set_live_mode(camera);
    			gci_camera_activate_live_display(camera);
  			}
  			else {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onTestPatternLevelDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;
	
	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
    		if(live)
				gci_camera_set_snap_mode(camera);

			gci_jvc_camera_set_test_pattern_level(jvc_camera, val);

			if (live) {
			
    			gci_camera_set_live_mode(camera);
    			gci_camera_activate_live_display(camera);
  			}
  			else {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onFreezeCancelDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
    		if(live)
				gci_camera_set_snap_mode(camera);

			gci_jvc_camera_set_freeze_cancel (jvc_camera, val); 

			if (live) {
			
    			gci_camera_set_live_mode(camera);
    			gci_camera_activate_live_display(camera);
  			}
  			else {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}

int CVICALLBACK JvcCamera_onCameraMemorySave (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:

			//GCI_KYF75CameraMemorySave ();

			break;
		}
	return 0;
}

int CVICALLBACK JvcCamera_onColourDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);

			gci_jvc_camera_set_colour_type(jvc_camera, val);

  			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
	return 0;
}

					  
int CVICALLBACK JvcCamera_onResolutionDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_resolution(jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onGainModeDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_gain_mode(jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}

int CVICALLBACK JvcCamera_onGainLevelChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live;
	int val;
	
	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_camera_set_gain(camera, CAMERA_CHANNEL1, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onColourMatrixLevelChanged (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);

			switch (control)
			{
			
				case COLMAT_CMATLEVEL0:
				
					gci_jvc_camera_set_colour_matrix (jvc_camera, MATRIX0, val); 
					break;
				
				case COLMAT_CMATLEVEL1:
				
					gci_jvc_camera_set_colour_matrix (jvc_camera, MATRIX1, val);
					break;
				
				case COLMAT_CMATLEVEL2:
				
					gci_jvc_camera_set_colour_matrix (jvc_camera, MATRIX2, val);
					break;
					
				case COLMAT_CMATLEVEL3:
		
					gci_jvc_camera_set_colour_matrix (jvc_camera, MATRIX3, val);
					break;
					
				case COLMAT_CMATLEVEL4:
			
					gci_jvc_camera_set_colour_matrix (jvc_camera, MATRIX4, val);
					break;
			
				case COLMAT_CMATLEVEL5:
		
					gci_jvc_camera_set_colour_matrix (jvc_camera, MATRIX5, val);
					break;
				
				case COLMAT_CMATLEVEL6:
				
					gci_jvc_camera_set_colour_matrix (jvc_camera, MATRIX6, val);
					break;
			
				case COLMAT_CMATLEVEL7:
				
					gci_jvc_camera_set_colour_matrix (jvc_camera, MATRIX7, val);
					break;
				
				case COLMAT_CMATLEVEL8:
			
					gci_jvc_camera_set_colour_matrix (jvc_camera, MATRIX8, val);
					break;
				}

				if(!live) {
  			
  					gci_camera_snap_image(camera);
				}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onWhiteBalanceColourTemp (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_colour_temp (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onWhiteBalanceModeDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_white_balance_mode (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onWhiteBalanceAuto (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_auto_whitebalance(jvc_camera);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onWhiteBalanceLevelRed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_white_balance_red_level (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onWhiteBalanceLevelBlue (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_white_balance_blue_level (jvc_camera, val); 

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onWhiteBalanceBaseLevelRed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_white_balance_base_red (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onWhiteBalanceBaseLevelBlue (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_white_balance_base_blue (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onShadingDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_shading_mode (jvc_camera, val);  

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onShadingLevelRed (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_shading_level_red  (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onShadingLevelGreen (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_shading_level_green  (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onShadingLevelBlue (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_shading_level_blue  (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onColourMatrixModeDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_colour_matrix_mode (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onProcessDetailDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_detail_mode (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onProcessDetailLevel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_detail_level (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onProccessDetailDepLevel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_detail_level_depend (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onFlareDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_flare_mode (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onFlareRedLevel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_flare_level_red (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onFlareBlueLevel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_flare_level_blue (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onGammaDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_gamma_mode (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onGammaLevel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_gamma_level (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onABLDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_abl_mode (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onAblLevel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_abl_level (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onNoiseSupDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_detail_noise_suppression (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onNegaDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_nega_mode (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onDspBypass (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_dsp_bypass (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onMasterBlackLevel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_master_black_level (jvc_camera, val); 

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onPixelCompLevel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_pixel_compensation (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onPixelCheck (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_pixel_check (jvc_camera); 

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onIrisModeDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_iris_mode (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onAEAreaDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_ae_area (jvc_camera, val, 1);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onAEDetectDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_ae_detect  (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onAEAreaTimer (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_TIMER_TICK:

			live = gci_camera_is_live_mode(camera);
    
			// remove the area from the image
			gci_jvc_camera_draw_ae_area (jvc_camera, 0); 
		
			// stop timer
			SetCtrlAttribute(jvc_camera->_exposure_panel, jvc_camera->_ae_area_timer, ATTR_ENABLED, 0);

			if(!live) {
  			
				// gci_jvc_camera_draw_ae_area slow to respond ?
				Delay(1.0);
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onAELevel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_ae_level (jvc_camera, val);  

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onIrisLevel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_iris_level (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onAlcMaxDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_alc_max (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onEeilimitDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_eei_limit (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onShutterSpeedLevel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_shutter_level (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onShutterModeDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_shutter_mode (jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}


int CVICALLBACK JvcCamera_onShutterRestart (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_restart_shutter (jvc_camera);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}

int CVICALLBACK JvcCamera_onAspectDropDown (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int live, val;

	GciJvcCamera* jvc_camera = (GciJvcCamera*) callbackData; 
	GciCamera *camera = (GciCamera*) jvc_camera;

	switch (event)
		{
		case EVENT_COMMIT:

			GetCtrlVal(panel, control, &val);

			live = gci_camera_is_live_mode(camera);
    
			gci_jvc_camera_set_aspect(jvc_camera, val);

			if(!live) {
  			
  				gci_camera_snap_image(camera);
			}

			break;
		}
		
	return 0;
}
