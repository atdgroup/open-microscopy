#include <userint.h>
//#include "password.h"

#include "ATD_ZDrive_A.h"
#include "ATD_ZDriveAutoFocus_A.h"

////////////////////////////////////////////////////////////////////////////
//RJL/RGN June 2007
//GCI HTP Microscope system. 
//Autofocus control.
////////////////////////////////////////////////////////////////////////////

int CVICALLBACK cb_autofocus_quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			ATD_ZDRIVE_A *atd_zdrive_a_zd = (ATD_ZDRIVE_A *) callbackData;  
		
	
			ui_module_hide_panel(UIMODULE_CAST(atd_zdrive_a_zd), atd_zdrive_a_zd->_autofocus_ui_pnl);   
			
			}break;
		}
	return 0;
}

int CVICALLBACK cb_autofocus_input_0 (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			ATD_ZDRIVE_A *atd_zdrive_a_zd = (ATD_ZDRIVE_A *) callbackData;  
	
		
			autofocus_send_vals(atd_zdrive_a_zd);
			}break;
		}
	return 0;
}

int CVICALLBACK cb_autofocus_input_1 (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			ATD_ZDRIVE_A *atd_zdrive_a_zd = (ATD_ZDRIVE_A *) callbackData; 
	
		
			autofocus_send_vals(atd_zdrive_a_zd);
			}break;
		}
	return 0;
}

int CVICALLBACK cb_autofocus_lasercurrent (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:{
	
			ATD_ZDRIVE_A *atd_zdrive_a_zd = (ATD_ZDRIVE_A *) callbackData; 
			Z_Drive *zd = (Z_Drive *) callbackData; 

			GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_LASERCURRENT, &atd_zdrive_a_zd->_autofocus_laser_I);    
	 
		 	if(atd_zdrive_a_zd->_autofocus_laser_I<0){
		 	   atd_zdrive_a_zd->_autofocus_laser_I=0;
		 	   SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_LASERCURRENT ,atd_zdrive_a_zd->_autofocus_laser_I); 
		 	}
		 	if(atd_zdrive_a_zd->_autofocus_laser_I>255){
		 	   atd_zdrive_a_zd->_autofocus_laser_I=255;
		 	  SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_LASERCURRENT ,atd_zdrive_a_zd->_autofocus_laser_I);  
		 	}
	 
		 	if	(atd_zdrive_a_out_byte_max521_multiport ( atd_zdrive_a_zd->_com_port, atd_zdrive_a_zd->_i2c_bus, atd_zdrive_a_zd->_i2c_chip_address, DAC7, atd_zdrive_a_zd->_autofocus_laser_I )) {
				send_z_drive_error_text(zd, "Failed to set laser current");
				return  Z_DRIVE_ERROR ;
		 	}
		
		}break;
	}
	return 0;
}

int CVICALLBACK cb_autofocus_abattn (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
	 
		ATD_ZDRIVE_A *atd_zdrive_a_zd = (ATD_ZDRIVE_A *) callbackData; 
		Z_Drive *zd = (Z_Drive *) callbackData; 

		 GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_ABATTN, &atd_zdrive_a_zd->_autofocus_errorRange); 
	 
	 	if(atd_zdrive_a_zd->_autofocus_errorRange<0){
	 	   atd_zdrive_a_zd->_autofocus_errorRange=0;
	 	    SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_ABATTN ,atd_zdrive_a_zd->_autofocus_errorRange);  
	 	}
	 	if(atd_zdrive_a_zd->_autofocus_errorRange>255){
	 	   atd_zdrive_a_zd->_autofocus_errorRange=255;
	 	   SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_ABATTN ,atd_zdrive_a_zd->_autofocus_errorRange); 
	 	}
	 
	 	if	(atd_zdrive_a_out_byte_max521_multiport ( atd_zdrive_a_zd->_com_port, atd_zdrive_a_zd->_i2c_bus,atd_zdrive_a_zd->_i2c_chip_address, DAC2, atd_zdrive_a_zd->_autofocus_errorRange )){
				send_z_drive_error_text(zd, "Failed to set a+b gain");    
			return  Z_DRIVE_ERROR ;
			}
			}break;
		}
	return 0;
}

int CVICALLBACK cb_autofocus_offsetcoarse (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{

	switch (event)
		{
		case EVENT_COMMIT:{
		ATD_ZDRIVE_A *atd_zdrive_a_zd = (ATD_ZDRIVE_A *) callbackData; 
		Z_Drive *zd = (Z_Drive *) callbackData; 
	//int offsetCourse;
	
		GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_OFFSETCOARSE, &atd_zdrive_a_zd->_autofocus_offsetCoarse);      
	 
	 	if(atd_zdrive_a_zd->_autofocus_offsetCoarse<0){
	 	   atd_zdrive_a_zd->_autofocus_offsetCoarse=0;
	 	  SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_OFFSETCOARSE ,atd_zdrive_a_zd->_autofocus_offsetCoarse);  
	 	}
	 	if(atd_zdrive_a_zd->_autofocus_offsetCoarse>255){
	 	   atd_zdrive_a_zd->_autofocus_offsetCoarse=255;
	 	   SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_OFFSETCOARSE ,atd_zdrive_a_zd->_autofocus_offsetCoarse); 
	 	}
	 
	 if	(atd_zdrive_a_out_byte_max521_multiport ( atd_zdrive_a_zd->_com_port, atd_zdrive_a_zd->_i2c_bus,atd_zdrive_a_zd->_i2c_chip_address, DAC5, atd_zdrive_a_zd->_autofocus_offsetCoarse )){
				send_z_drive_error_text(zd, "Failed to set coarse offset");    
			return  Z_DRIVE_ERROR ;
		  }
			}break;
		}
	return 0;
}

int CVICALLBACK cb_autofocus_offsetfine (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
	 
		ATD_ZDRIVE_A *atd_zdrive_a_zd = (ATD_ZDRIVE_A *) callbackData; 
		Z_Drive *zd = (Z_Drive *) callbackData; 
	//int offsetFine;

	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_OFFSETFINE, &atd_zdrive_a_zd->_autofocus_offsetFine);  
	 
	 	if(atd_zdrive_a_zd->_autofocus_offsetFine<0){
	 	   atd_zdrive_a_zd->_autofocus_offsetFine=0;
	 	   SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_OFFSETFINE ,atd_zdrive_a_zd->_autofocus_offsetFine); 
	 	}
	 	if(atd_zdrive_a_zd->_autofocus_offsetFine>255){
	 	   atd_zdrive_a_zd->_autofocus_offsetFine=255;
	 	   SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_OFFSETFINE ,atd_zdrive_a_zd->_autofocus_offsetFine); 
	 	}
	 
	 	if	(atd_zdrive_a_out_byte_max521_multiport ( atd_zdrive_a_zd->_com_port, atd_zdrive_a_zd->_i2c_bus,atd_zdrive_a_zd->_i2c_chip_address, DAC6, atd_zdrive_a_zd->_autofocus_offsetFine  )){
				send_z_drive_error_text(zd, "Failed to set fine offset");    
			return  Z_DRIVE_ERROR ;
		   }
			}break;
		}
	return 0;
}

int CVICALLBACK cb_autofocus_differentialgain (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
	 
		ATD_ZDRIVE_A *atd_zdrive_a_zd = (ATD_ZDRIVE_A *) callbackData; 
		Z_Drive *zd = (Z_Drive *) callbackData; 
	//int differentialGain;

	GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_DIFFERENTIALGAIN, &atd_zdrive_a_zd->_autofocus_differentialGain);    
	 
	 	if(atd_zdrive_a_zd->_autofocus_differentialGain<0){
	 	   atd_zdrive_a_zd->_autofocus_differentialGain=0;
	 	   SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_DIFFERENTIALGAIN ,atd_zdrive_a_zd->_autofocus_differentialGain); 
	 	}
	 	if(atd_zdrive_a_zd->_autofocus_differentialGain>255){
	 	   atd_zdrive_a_zd->_autofocus_differentialGain=255;
	 	   SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_DIFFERENTIALGAIN ,atd_zdrive_a_zd->_autofocus_differentialGain); 
	 	}
	 
	 if	(atd_zdrive_a_out_byte_max521_multiport ( atd_zdrive_a_zd->_com_port, atd_zdrive_a_zd->_i2c_bus,atd_zdrive_a_zd->_i2c_chip_address, DAC3, atd_zdrive_a_zd->_autofocus_differentialGain )){
				send_z_drive_error_text(zd, "Failed to set differential gain");    
			return  Z_DRIVE_ERROR ;
		   }
			}break;
		}
	return 0;
}

int CVICALLBACK cb_autofocus_lowsignalllimit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
	
		ATD_ZDRIVE_A *atd_zdrive_a_zd = (ATD_ZDRIVE_A *) callbackData; 
	 	Z_Drive *zd = (Z_Drive *) callbackData; 

		GetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_LOWSIGNALLIMIT, &atd_zdrive_a_zd->_autofocus_lowSignalLimit);   
	 
	 	if(atd_zdrive_a_zd->_autofocus_lowSignalLimit<0){
	 	   atd_zdrive_a_zd->_autofocus_lowSignalLimit=0;
	 	   SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_LOWSIGNALLIMIT ,atd_zdrive_a_zd->_autofocus_lowSignalLimit); 
	 	}
	 	if(atd_zdrive_a_zd->_autofocus_lowSignalLimit>255){
	 	   atd_zdrive_a_zd->_autofocus_lowSignalLimit=255;
	 	   SetCtrlVal(atd_zdrive_a_zd->_autofocus_ui_pnl, AUTOFOCUS_LOWSIGNALLIMIT ,atd_zdrive_a_zd->_autofocus_lowSignalLimit); 
	 	}
	 
	 if	(atd_zdrive_a_out_byte_max521_multiport ( atd_zdrive_a_zd->_com_port, atd_zdrive_a_zd->_i2c_bus,atd_zdrive_a_zd->_i2c_chip_address, DAC4, atd_zdrive_a_zd->_autofocus_lowSignalLimit )){
				send_z_drive_error_text(zd, "Failed to set low signal limit");    
			return  Z_DRIVE_ERROR ;
		   }
			}break;
		}
	return 0;
}

int CVICALLBACK cb_autofocus_sendall (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			ATD_ZDRIVE_A *atd_zdrive_a_zd = (ATD_ZDRIVE_A *) callbackData; 
	
	
			autofocus_send_vals(atd_zdrive_a_zd);  
			}break;
		}
	return 0;
}


int CVICALLBACK cb_autofocus_sendall_setup (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:{
			ATD_ZDRIVE_A *atd_zdrive_a_zd = (ATD_ZDRIVE_A *) callbackData; 
	
	
			ui_module_display_panel(UIMODULE_CAST(atd_zdrive_a_zd), atd_zdrive_a_zd->_autofocus_ui_pnl);
			
			}break;
		}
	return 0;
}
