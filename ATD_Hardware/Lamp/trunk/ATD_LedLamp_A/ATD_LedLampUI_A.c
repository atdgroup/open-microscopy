#include <userint.h>
#include "ATD_LedLamp_A.h"
#include "ATD_LedLampUI_A.h"

////////////////////////////////////////////////////////////////////////////
//RJL/RGN April 2007
//GCI HTS Microscope system. 
//LED sequence control - ui callbacks
////////////////////////////////////////////////////////////////////////////

//Settings panel controls

int CVICALLBACK cb_htsleds_closesettingpnl (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
            Lamp *lamp = (Lamp *) callbackData;
            
			adt_a_led_lamp_hide_settings_ui(lamp);
            
			break;
        }
	}
    
	return 0;
}

int CVICALLBACK cb_htsleds_selectoutput (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
            Lamp *lamp = (Lamp *) callbackData;
            ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp; 
            int sequence;
            
            GetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SELECTOUTPUT, &sequence);
            
			adt_a_led_lamp_select_output_sequence(lamp, sequence);
            
			SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_NUMTIMESLOTS, adt_a_led->_number_sequence_points[sequence]);
	
			break;
		}
    }
    
	return 0;
}

int CVICALLBACK cb_htsleds_loaddata (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
            Lamp *lamp = (Lamp *) callbackData;
            ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp; 
            char path[MAX_PATHNAME_LEN] = "";

            if (FileSelectPopup (path, "*.txt", "*.txt", "Load",
							 	 VAL_LOAD_BUTTON, 0, 1, 1, 0, path) != 1) return 0;
	
			if (!FileExists(path, NULL))
                return 0;
                
			adt_a_led_lamp_load_sequence_data (lamp, path);
            
            break;         
		}
	}
    
	return 0;
}

int CVICALLBACK cb_htsleds_intensity (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
        {
            Lamp *lamp = (Lamp *) callbackData;
            double intensity;
            
            GetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_INTENSITY, &intensity);
			lamp_set_intensity (lamp, intensity);
            
            break;
		}
	}
	return 0;
}

int CVICALLBACK cb_htsleds_ledstate (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;
            
            int mode;
            int sequence;
    
			GetCtrlVal(panel, control, &mode);
				
			GetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SELECTOUTPUT, &sequence);   
			adt_a_led_lamp_set_mode(lamp, mode, sequence);    
					
            break;
		}
	}
    
	return 0;
}

int CVICALLBACK cb_htsleds_sequencetime (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;

            double sequencetime;
            int numtimeslots;

            GetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SEQUENCETIME, &sequencetime);  
            GetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_NUMTIMESLOTS, &numtimeslots);
    
			adt_a_led_lamp_set_sequence_time(lamp, sequencetime, numtimeslots);
            
            break;
		}
	}
    
	return 0;
}

int CVICALLBACK cb_htsleds_loadsettingpanel (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;

			adt_a_led_lamp_display_settings_ui(lamp);
            
            break;
		}
	}
    
	return 0;
}

int CVICALLBACK cb_htsleds_numtimeslots (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;
            double period;
            int numtimeslots; 
	
            GetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SETSLOTTIME ,&period);
            GetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_NUMTIMESLOTS ,&numtimeslots);  
    
            adt_a_led_lamp_set_numtimeslots(lamp, numtimeslots, period);
            
            break;
		}
	}
	return 0;
}

int CVICALLBACK cb_htsleds_quit (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;
			lamp_hide_main_ui(lamp);
            
            break;
		}
	}
    
	return 0;
}

int CVICALLBACK cb_htsleds_ledenable (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;
            int val = 0;
            
            GetCtrlVal(panel, control, &val);
            
            if(val)
                adt_a_led_lamp_enable (lamp);
            else
                adt_a_led_lamp_disable (lamp);
               
            break;
		}
	}
    
	return 0;
}

int CVICALLBACK cb_htsleds_auxoutput (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;
            int val = 0;
            
            GetCtrlVal(panel, control, &val);

            adt_a_led_lamp_set_aux_out (lamp, val);
            
            break;
		}
	}
    
	return 0;
}

int CVICALLBACK cbset_htsleds_slottime (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
        {
			Lamp *lamp = (Lamp *) callbackData;
			adt_a_led_lamp_set_pw_rep_rate(lamp);
            
            break;
		}
	}
    
	return 0;
}
