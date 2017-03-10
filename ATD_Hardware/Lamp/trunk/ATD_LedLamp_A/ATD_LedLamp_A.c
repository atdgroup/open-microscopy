#include "ATD_LedLampUI_A.h"
#include "ATD_LedLamp_A.h" 
#include <rs232.h>

#include "string_utils.h"
#include "gci_utils.h"
#include "signals.h"

#include "GL_CVIRegistry.h"
#include "toolbox.h"

#include <ansi_c.h> 
#include "ATD_UsbInterface_A.h"
#include <utility.h>

#define AD5241		 0x58 



////////////////////////////////////////////////////////////////////////////
//GCI HTS Microscope system. 
//LED sequence control via PIC and memory chip.
////////////////////////////////////////////////////////////////////////////
//Pic modes 
//mode 0 = controls PWM including duty cycle
//mode 1 = start / stop / program settings
//mode 2 = input trigger polarity
//mode 3 = enable/disable data bus
//mode 4 = input data to memory
//mode 5 = select sequence output
//mode 6 = Off
//mode 7 = On
////////////////////////////////////////////////////////////////////////////

// Program mode - Set while sequence data is being uploaded. (Should only be activated for this short time.
// Free running the chip is cycling through the sequence data.

#define SEQ_SIZE 4096

#define Round  RoundRealToNearestInteger 

typedef enum {ATD_A_LED_SETUP_PROGRAM_MODE = 3, ATD_A_LED_SETUP_NORMAL_MODE=0, ATD_A_LED_SETUP_FREE_RUN_MODE=4} ATD_A_LED_SETUP_MODE;

static int adt_a_led_lamp_send_data(Lamp* lamp, int mode, int val1, int val2, int val3, int val4)
{
    ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;

	char data[20]; 

	data[0]=mode;	    
	data[1]=val1;
	data[2]=val2;
	data[3]=val3;
	data[4]=val4; 
  	
	return GCI_Out_PIC_multiPort (adt_a_led->_com_port, adt_a_led->_i2c_bus, adt_a_led->_i2c_chip_type, adt_a_led->_i2c_chip_address, 5, data);
}


static void adt_a_led_lamp_log_message(Lamp* lamp)
{
	char state_text[16];   
//	int max=0,min=0,ledpower,auxout, ledstate;
//	double intensity;
	
	ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;   
/*	
	GetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_INTENSITY ,&intensity);
	GetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_LED_ENABLE ,&ledpower);
	GetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_AUXOUTPUT ,&auxout);   
	GetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_LEDSTATE, &ledstate);  
	
	switch (ledstate)
	{
		case 1:
			strcpy(state_text, "Off");
			break;
		case 2:
			strcpy(state_text, "On");
			break;
		case 3:
			strcpy(state_text, "Pulsed");
			break;
	}
	
	logger_log(UIMODULE_LOGGER(adt_a_led), LOGGER_INFORMATIONAL, "%s changed: power %s int %f, enable %d, aux %d", 
		UIMODULE_GET_DESCRIPTION(adt_a_led), state_text, intensity, ledpower, auxout);
*/

	switch (adt_a_led->_led_mode)
	{
		case ATD_A_LED_OFF:
			strcpy(state_text, "Off");
			break;
		case ATD_A_LED_ON:
			strcpy(state_text, "On");
			break;
		case ATD_A_LED_PULSED:
			strcpy(state_text, "Pulsed");
			break;
	}

	logger_log(UIMODULE_LOGGER(adt_a_led), LOGGER_INFORMATIONAL, "%s changed: power %s int %f, enable %d, aux %d", 
		UIMODULE_GET_DESCRIPTION(adt_a_led), state_text, adt_a_led->_intensity, adt_a_led->_enabled_status, adt_a_led->_auxout);
}


static int adt_a_led_lamp_send_settings(Lamp *lamp, int enable, double intensity)
{
    ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;
	double range, setting;
	byte data[100];
	
    // We was 1 to mean enable and 0 to mean disable
    enable = !enable;

	range = adt_a_led->_max_intensity - adt_a_led->_min_intensity;
	setting = ((range * intensity) / 100.0) + adt_a_led->_min_intensity;
		
	data[0] = (enable<<4 | adt_a_led->_auxout<<3);	//instruction byte  including logic pins
	data[1] = (byte) setting; 				//data byte
	
	if(GCI_Out_PIC_multiPort (adt_a_led->_com_port, adt_a_led->_i2c_bus, AD5241, adt_a_led->_i2c_ADaddress, 2, data) != 0)
		return LAMP_ERROR;
	
    return LAMP_SUCCESS;
}


static int adt_a_led_lamp_set_freerun_mode(Lamp *lamp)
{
    ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;
	
	if(adt_a_led_lamp_send_data(lamp, 1, ATD_A_LED_FREE_RUN_MODE,0,0,0) == LAMP_ERROR)	//	free run
    	return LAMP_ERROR;

	SetCtrlAttribute(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SELECTOUTPUT, ATTR_DIMMED, 0); 
		
	return LAMP_SUCCESS; 
}

static int adt_a_led_lamp_set_program_mode(Lamp *lamp)
{
    ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;
	
	if(adt_a_led_lamp_send_data(lamp, 1, ATD_A_LED_SETUP_PROGRAM_MODE,0,0,0) == LAMP_ERROR)	//either free run or program state
            return LAMP_ERROR;

	SetCtrlAttribute(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SELECTOUTPUT, ATTR_DIMMED, 1); 
	
	return LAMP_SUCCESS; 
}

static int adt_a_led_lamp_set_normal_mode(Lamp *lamp)
{
    ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;
	
	if(adt_a_led_lamp_send_data(lamp, 1, 1,0,0,0) == LAMP_ERROR)	// Normal Mode
            return LAMP_ERROR;

	SetCtrlAttribute(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SELECTOUTPUT, ATTR_DIMMED, 1); 
	
	return LAMP_SUCCESS; 
}


int adt_a_led_lamp_set_mode(Lamp *lamp, ATD_A_LED_MODE mode, int sequence)
{
    ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;

    if(mode == ATD_A_LED_ON)
    {
		SetCtrlAttribute(lamp->_main_ui_panel, LEDS_PNL_SELECTOUTPUT, ATTR_DIMMED, 1);
		// 1 Off 2 on 3 pulsed 
		SetCtrlVal(lamp->_main_ui_panel, LEDS_PNL_LEDSTATE, 2); 
	
        adt_a_led_lamp_select_output_sequence(lamp, 7); 
        adt_a_led_lamp_send_settings(lamp, ATD_A_LED_ENABLE, adt_a_led->_intensity);
		adt_a_led->_led_mode = ATD_A_LED_ON;
		adt_a_led->_enabled_status = ATD_A_LED_ENABLE;
    }
    else if(mode == ATD_A_LED_OFF)
    {
		// 1 Off 2 on 3 pulsed 
		SetCtrlVal(lamp->_main_ui_panel, LEDS_PNL_LEDSTATE, 1); 
		
        SetCtrlAttribute(lamp->_main_ui_panel, LEDS_PNL_SELECTOUTPUT, ATTR_DIMMED, 1);        
        adt_a_led_lamp_select_output_sequence(lamp, 6);    
        adt_a_led_lamp_send_settings(lamp, ATD_A_LED_DISABLE, adt_a_led->_intensity); 
		adt_a_led->_led_mode = ATD_A_LED_OFF;
		adt_a_led->_enabled_status = ATD_A_LED_DISABLE;  
    }
    else if(mode == ATD_A_LED_PULSED)
    {
		// 1 Off 2 on 3 pulsed 
		SetCtrlVal(lamp->_main_ui_panel, LEDS_PNL_LEDSTATE, 3); 
		
        SetCtrlAttribute(lamp->_main_ui_panel, LEDS_PNL_SELECTOUTPUT, ATTR_DIMMED, 0);         

		adt_a_led_lamp_select_output_sequence(lamp, sequence);
        adt_a_led_lamp_send_settings(lamp, ATD_A_LED_ENABLE, adt_a_led->_intensity); 
		adt_a_led->_led_mode = ATD_A_LED_PULSED;
		adt_a_led->_enabled_status = ATD_A_LED_ENABLE;  
    }
	else if(mode == ATD_A_LED_FREE_RUN_MODE)
    {	
		// 1 Off 2 on 3 pulsed 
		SetCtrlVal(lamp->_main_ui_panel, LEDS_PNL_LEDSTATE, 4);
		
		SetCtrlAttribute(lamp->_main_ui_panel, LEDS_PNL_SELECTOUTPUT, ATTR_DIMMED, 0); 
		SetCtrlVal(lamp->_main_ui_panel, LEDS_PNL_SELECTOUTPUT, sequence); 
        
        if(adt_a_led_lamp_send_data(lamp, 1, ATD_A_LED_FREE_RUN_MODE,0,0,0) == LAMP_ERROR)	//	free run
            return LAMP_ERROR;
            
		adt_a_led_lamp_select_output_sequence(lamp, sequence);
        adt_a_led_lamp_send_settings(lamp, ATD_A_LED_ENABLE, adt_a_led->_intensity);
		adt_a_led->_led_mode = ATD_A_LED_FREE_RUN_MODE;
		adt_a_led->_enabled_status = ATD_A_LED_ENABLE;  
	}
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp);
	
	adt_a_led_lamp_log_message(lamp);   
	
	return LAMP_SUCCESS;       
}


int adt_a_led_lamp_set_numtimeslots(Lamp* lamp, int numtimeslots, double period)
{
	double sequenceTime;
	ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;  
	
	sequenceTime = (numtimeslots * period)/1000;  //Sequence time in ms
	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SEQUENCETIME, sequenceTime);
	
	return LAMP_SUCCESS;
}

int adt_a_led_lamp_set_sequence_time(Lamp* lamp, double sequencetime, int numtimeslots)
{
	double period;
	int PR2,t2,divider,msb_duty_cycle,lsb_duty_cycle, mode;
	ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;  

//	period =  (sequencetime/numtimeslots * 1000);  //In usecs
	period =  (sequencetime/(double)SEQ_SIZE * 1000);  //In usecs
	divider = 1;
	t2 = 4;
	PR2 = Round((period/(0.2*divider))-1);
	if (PR2 < 1.0) PR2=1;
	if (PR2 > 255) {
		divider = 4;
		t2 = 5;
		PR2 = Round((period/(0.2*divider))-1);
		if (PR2 < 1.0) PR2=1; 
	}
	if(PR2 > 255) {
		divider = 16;
		t2 =6 ;
		PR2 = Round((period/(0.2*divider))-1);
		if (PR2<1.0) PR2=1; 
	}
				
	mode=0; 			 //PWM mode   
	msb_duty_cycle=0;
	lsb_duty_cycle=2;    //100ns pulse width
	adt_a_led_lamp_send_data(lamp, mode,PR2,msb_duty_cycle,lsb_duty_cycle,t2);
	
	period = ((PR2+1)*0.2*divider);		   //Slot time
	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SLOTTIME_1, (period*SEQ_SIZE)/numtimeslots); 		
//	period = ((PR2+1)*0.2*divider*numtimeslots)/1000;		 //Sequence time in ms
	period = ((PR2+1)*0.2*divider*SEQ_SIZE)/1000;		 //Sequence time in ms
	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SEQUENCETIME, period);
	adt_a_led->_seq_time = (int) period;
	
	return LAMP_SUCCESS;
}



static int adt_a_led_lamp_set_rep_rate(Lamp* lamp, int rep_val, int rep_div_1, int rep_div_2)
{
	char data[5]; 	 
	double frequency, double_duty_cycle, duty_cycle_freq;
	int divider, duty_cycle, mode;
	int msb_duty_cycle, lsb_duty_cycle;
	ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp; 
	
	switch (rep_div_1) {			 //Calculate the frequency; rep_div_1=t2
		case 0:
		   	divider=1; 
		   	frequency=0;
		   	break;
	   case 4:
		   	divider=1;
		   	frequency=20E6/(((rep_val+1)*divider*4)*1000);    
		   	break;
	   case 5:
		   	divider=4;
		   	frequency=20E6/(((rep_val+1)*divider*4)*1000);    
		   	break;
	   	case 6:
		   	divider=16;
		   	frequency=20E6/(((rep_val+1)*divider*4)*1000);    
		   	break;
			   
		default:
			divider=1;
	}
	 
	//Calculate duty cycle bytes
	double_duty_cycle = 50.0;	   //Fix duty cycle to 50%
	duty_cycle_freq = frequency*(100.0/double_duty_cycle);    
				
	duty_cycle = (int) (20E3/(duty_cycle_freq*divider));
	
	msb_duty_cycle=duty_cycle >> 8;
	lsb_duty_cycle=duty_cycle & 0xff;
			
	mode=0; 			 //PWM mode   
				
	data[0] = mode;
	data[1] = rep_val;			   //PR2
	data[2] = msb_duty_cycle;
	data[3] = lsb_duty_cycle;
	data[4] = rep_div_1;		   //t2
	
	return GCI_Out_PIC_multiPort (adt_a_led->_com_port, adt_a_led->_i2c_bus, adt_a_led->_i2c_chip_type, adt_a_led->_i2c_chip_address, 5, data);
}


int adt_a_led_lamp_set_pw_rep_rate(Lamp* lamp)
{
	double slotTime, sequenceTime;
	int index, numtimeslots, numBits; 

	GetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SETSLOTTIME ,&slotTime);
	GetCtrlAttribute (UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SETSLOTTIME, ATTR_CTRL_INDEX, &index);
	
	GetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_NUMTIMESLOTS ,&numBits); 
	numtimeslots = SEQ_SIZE;
	   
	switch (index) {
		case 0:  //200 ns
			adt_a_led_lamp_set_rep_rate(lamp, 0,4,1);
			break;
		case 1:  //400 ns
			adt_a_led_lamp_set_rep_rate(lamp, 1,4,1);
			break;
		case 2:  //1 us
			adt_a_led_lamp_set_rep_rate(lamp, 4,4,1);
			break;
		case 3:  //2 us
			adt_a_led_lamp_set_rep_rate(lamp, 9,4,1);
			break;
		case 4:  //5 us
			adt_a_led_lamp_set_rep_rate(lamp, 24,4,1);
			break;
		case 5:  //10 us
			adt_a_led_lamp_set_rep_rate(lamp, 49,4,1);
			break;
		case 6:  //20 us
			adt_a_led_lamp_set_rep_rate(lamp, 99,4,1);
			break;
		case 7:  //50 us
			adt_a_led_lamp_set_rep_rate(lamp, 249,4,1);
			break;
		case 8:  //100 us
			adt_a_led_lamp_set_rep_rate(lamp, 124,5,1);
			break;
		case 9:  //200 us
			adt_a_led_lamp_set_rep_rate(lamp, 249,5,1);
			break;
		case 10:  //499.2 us
			adt_a_led_lamp_set_rep_rate(lamp, 155,6,1);
			break;
		case 11:  //668.8 us
			adt_a_led_lamp_set_rep_rate(lamp, 207,6,2);
			break;
		case 12:  //819.2 us
			adt_a_led_lamp_set_rep_rate(lamp, 255,6,2);
			break;
	}
			       
	sequenceTime = (numtimeslots * slotTime)/1000;  //Sequence time in ms
	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SEQUENCETIME ,sequenceTime);  
	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SLOTTIME_1 , (numtimeslots * slotTime)/numBits);  	 	
	
	return LAMP_SUCCESS;
}


int adt_a_led_lamp_load_sequence_data (Lamp* lamp, const char *filepath)
{
	char *tok;
 	FILE *fp; 
	char line_str[10000], name[20];
	int *buffer1=NULL;
	int file_size, datapoints,x,y,z,dataAddress,totByteVal,byteVal, mode, dont_upload_to_eprom = 0;
	
	double stretchMult;
	int intStretchMult,totalBytes,remainderFill,sequence,index;
	int sequenceArray[SEQ_SIZE][10];
    ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;
    
	if(!FileExists(filepath, &file_size))
		return LAMP_ERROR;
	
  	fp = fopen (filepath, "r");
  	
    if (fp == NULL)
        return LAMP_ERROR;
 
	if(strcmp(adt_a_led->_last_uploaded_sequence_filepath, filepath) == 0)
		dont_upload_to_eprom = 1;
	
	adt_a_led_lamp_set_program_mode(lamp);
	 
	SetWaitCursor (1);
	sequence=0;
	index=0;
	
	while (fgets (line_str, 10000, fp) != NULL) {
		tok = strtok (line_str, ",");		//Breaks string to tokens seperated by commas
		strcpy(name, tok);	   //Put name in popup
		if (index<7)
			ReplaceListItem (UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SELECTOUTPUT, index, name, index);
	  
		tok = strtok (NULL, ",");
		datapoints = atoi(tok);
		
		adt_a_led->_number_sequence_points[index] = datapoints;
		
		if(dont_upload_to_eprom) {
			sequence++; 	//Read next line
			index++;	   	//Advance index
			continue;
		}
		
		buffer1 = (int *)calloc(datapoints, sizeof(int));
		x=0;
	 
		while (1) {
			tok = strtok (NULL, ",");
			if (tok == NULL) break;
			buffer1[x] = atoi(tok);
			x++;
			if (x >= datapoints) break;
		}	  
		
		//Calculate stretching needed to fill SEQ_SIZE locations in memory
		stretchMult = SEQ_SIZE/datapoints;
		intStretchMult = (int) floor (stretchMult);   //Integer multiplier
		totalBytes = intStretchMult*datapoints; //Total number stretched
		remainderFill = SEQ_SIZE - totalBytes;		//Remaining bits to fill with 0's
		
		x=0;
		for(dataAddress=0; dataAddress <= (datapoints-1); dataAddress++) {	 //Set array data   
			z=1; 
  			while(z<=intStretchMult) {
				sequenceArray[x][sequence]=buffer1[dataAddress];
				x++;
				z++;
			}
		}
		
		x=x-1;
	  	for ((x=totalBytes);x < (totalBytes+remainderFill);x++) {
			sequenceArray[x][sequence]=0;
		}
		
		sequence++; 	//Read next line
		index++;	   	//Advance index
		
		free(buffer1);
	}  
	
	if(dont_upload_to_eprom)
		goto DATA_LOADED;
  
  	//Now have full 2-D array, need to multiply columns to get bit values
  	for (y=0;y<=(sequence-1);y++) {
		for (x=0;x<SEQ_SIZE;x++) { 
			sequenceArray[x][y]=sequenceArray[x][y]<<y;	//Multiply rows by 2,4,8 etc
		}
	}
	
	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_PROGLED, 1);   
  
	for(x=0;x<SEQ_SIZE;x++) {
		totByteVal=0;		  //Reset byte value
		for(y=0;y<=(sequence-1);y++) {
			byteVal=sequenceArray[x][y];
			totByteVal=byteVal+totByteVal;		//Add each bit value to get the total byte 
		}
		
		mode = 4;		   		//Load data into memory command

		ProcessDrawEvents ();
		adt_a_led_lamp_send_data(lamp, mode, totByteVal, 0, 0, 0); 	  //Send byte to PIC
	}
  
	
	DATA_LOADED:
	
	GetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SELECTOUTPUT, &sequence); 
	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_NUMTIMESLOTS, adt_a_led->_number_sequence_points[sequence]);
	
    SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_PROGLED ,0);
  
	adt_a_led_lamp_set_normal_mode(lamp);  
	
	strcpy(adt_a_led->_last_uploaded_sequence_filepath, filepath);
	
	adt_a_led_lamp_set_pw_rep_rate(lamp);  
	
    SetWaitCursor (0);
    
    return LAMP_SUCCESS;
}


static char* construct_key(Lamp* lamp, char *buffer, const char* name)
{
	memset(buffer, 0, 1);
	
	sprintf(buffer, "%s:%s", UIMODULE_GET_NAME(lamp), name);
	
	return buffer;
}


// Save settings specific to  ATD_LAMP_A
static int adt_ledlamp_a_save_settings(ATD_LAMP_A *adt_a_led)
{
	FILE *fd;
	char data_filepath[GCI_MAX_PATHNAME_LEN];
	Lamp *lamp = (Lamp *) adt_a_led;    
	dictionary *d = dictionary_new(20);

	sprintf(data_filepath, "%s\\%s_%s", UIMODULE_GET_DATA_DIR(lamp), UIMODULE_GET_NAME(lamp), DEFAULT_LAMP_FILENAME_SUFFIX);

	fd = fopen(data_filepath, "w");
	
	dictionary_set(d, UIMODULE_GET_NAME(lamp), NULL);

	dictionary_setint(d, "Mode", adt_a_led->_led_mode);
    dictionary_setdouble(d, "Min Intensity", adt_a_led->_min_intensity);
    dictionary_setdouble(d, "Max Intensity", adt_a_led->_max_intensity);
    dictionary_setdouble(d, "Intensity Increment", adt_a_led->_intensity_increment);
    dictionary_setdouble(d, "Intensity", adt_a_led->_intensity);
 
    dictionary_setint(d, "Input Trigger", adt_a_led->_input_trigger); 
    dictionary_setint(d, "Sequence Time", adt_a_led->_seq_time); 
    dictionary_setint(d, "Selected Sequence", adt_a_led->_selected_sequence);
    
	dictionary_set(d, "Last Loaded Data Sequence", adt_a_led->_last_uploaded_sequence_filepath); 
	
	iniparser_save(d, fd); 
	
	fclose(fd);
	dictionary_del(d);
	
	return LAMP_SUCCESS;
}


// Load settings specific to  ATD_LAMP_A
static int adt_ledlamp_a_load_settings(ATD_LAMP_A *adt_a_led)
{
	dictionary* d = NULL;
	int file_size;
	char *data_filepath = NULL, filepath[GCI_MAX_PATHNAME_LEN], buffer[100];
	Lamp *lamp = (Lamp *) adt_a_led;     
	
	sprintf(filepath, "%s\\%s_%s", UIMODULE_GET_DATA_DIR(lamp), UIMODULE_GET_NAME(lamp), DEFAULT_LAMP_FILENAME_SUFFIX);

	if(!FileExists(filepath, &file_size))
		return LAMP_SUCCESS;	 	
	
	d = iniparser_load(filepath);  

	if(d != NULL) {
		
		adt_a_led->_led_mode = dictionary_getint(d, construct_key(lamp, buffer, "Mode"), ATD_A_LED_OFF); 
		
		adt_a_led->_min_intensity = dictionary_getdouble(d, construct_key(lamp, buffer, "Min Intensity"), 0);
        adt_a_led->_max_intensity = dictionary_getdouble(d, construct_key(lamp, buffer, "Max Intensity"), 255);
        adt_a_led->_intensity_increment = dictionary_getdouble(d, construct_key(lamp, buffer, "Intensity Increment"), 1.0);
        adt_a_led->_intensity = dictionary_getdouble(d, construct_key(lamp, buffer, "Intensity"), 0.0);
        adt_a_led->_input_trigger = dictionary_getint(d, construct_key(lamp, buffer, "Input Trigger"), 0); 
        adt_a_led->_seq_time = dictionary_getint(d, construct_key(lamp, buffer, "Sequence Time"), 0); 
        adt_a_led->_selected_sequence = dictionary_getint(d, construct_key(lamp, buffer, "Selected Sequence"), 0);
			
		lamp_set_intensity_range (lamp, adt_a_led->_min_intensity, adt_a_led->_max_intensity, adt_a_led->_intensity_increment);
        adt_a_led_lamp_set_intensity (lamp, adt_a_led->_intensity);

		data_filepath = dictionary_get(d, construct_key(lamp, buffer, "Last Loaded Data Sequence"), NULL);  
		
		if(data_filepath != NULL)
		{
			strcpy(adt_a_led->_last_uploaded_sequence_filepath, data_filepath);
			adt_a_led_lamp_load_sequence_data (lamp, adt_a_led->_last_uploaded_sequence_filepath);           
		}

		adt_a_led_lamp_set_sequence_time(lamp, adt_a_led->_seq_time, adt_a_led->_number_sequence_points[adt_a_led->_selected_sequence]);
	
		adt_a_led_lamp_set_mode(lamp, adt_a_led->_led_mode, adt_a_led->_selected_sequence); 
		
        dictionary_del(d);
	}
	
	return LAMP_SUCCESS;
}


// Saves to arbitary file - for changing microscope modes.
int adt_a_led_lamp_load_settings (Lamp* lamp, const char *filepath)
{
	dictionary* d = NULL;
	int file_size;
	char *data_filepath = NULL, buffer[100];
	ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;    
	
	if(!FileExists(filepath, &file_size))
		return LAMP_SUCCESS;	 	
	
	d = iniparser_load(filepath);  

	if(d != NULL) {
		
		adt_a_led->_led_mode = dictionary_getint(d, construct_key(lamp, buffer, "Mode"), ATD_A_LED_OFF); 
		adt_a_led->_intensity = dictionary_getdouble(d, construct_key(lamp, buffer, "Intensity"), 0.0);
  		
        dictionary_del(d);
	}
	
	lamp_set_intensity (lamp, adt_a_led->_intensity);

	if(adt_a_led->_led_mode == ATD_A_LED_OFF)
		lamp_off (lamp);
	else
		lamp_on (lamp);

	return LAMP_SUCCESS;	      
}


int adt_a_led_lamp_save_settings (Lamp* lamp, const char *filepath, const char *flags)
{
	FILE *fd;
	ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;
    dictionary *d = dictionary_new(20);
	
	fd = fopen(filepath, flags);
	
	dictionary_set(d, UIMODULE_GET_NAME(lamp), NULL);

	dictionary_setint(d, "Mode", adt_a_led->_led_mode);
    dictionary_setdouble(d, "Intensity", adt_a_led->_intensity);
 
	iniparser_save(d, fd); 
	
	fclose(fd);
	dictionary_del(d);
	
	return LAMP_SUCCESS;	      
}


int adt_a_led_lamp_destroy (Lamp* lamp)
{
    if(lamp == NULL)
		return LAMP_SUCCESS;   	
		
	adt_ledlamp_a_save_settings((ATD_LAMP_A *)lamp);

	lamp_off (lamp);

  	return LAMP_SUCCESS;
}


int adt_a_led_lamp_disable (Lamp* lamp)
{
    ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;

    adt_a_led_lamp_set_normal_mode(lamp);

	adt_a_led_lamp_set_mode(lamp, ATD_A_LED_OFF, 6);    
	
    return LAMP_SUCCESS;
}

int adt_a_led_lamp_enable (Lamp* lamp)
{
    ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;

    adt_a_led_lamp_set_normal_mode(lamp);
	
	adt_a_led_lamp_set_mode(lamp, ATD_A_LED_ON, 7);   
	
    return LAMP_SUCCESS;
}


int adt_a_led_lamp_off (Lamp* lamp)
{
    return adt_a_led_lamp_disable (lamp);
}

int adt_a_led_lamp_on (Lamp* lamp)
{
	return adt_a_led_lamp_enable (lamp);
}


int adt_a_led_lamp_set_aux_out (Lamp* lamp, int aux_out)
{
	ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;        
	
	adt_a_led->_auxout = aux_out;
	
	return LAMP_SUCCESS;   
}

int adt_a_led_lamp_off_on_status (Lamp* lamp, LampStatus *status)
{
	ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;    
	
	if(adt_a_led->_led_mode != ATD_A_LED_OFF)
		*status = LAMP_ON;
	else
		*status = LAMP_OFF;    
		
	return LAMP_SUCCESS;         
}

int adt_a_led_lamp_set_intensity (Lamp* lamp, double intensity)
{
	ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;        
	
	adt_a_led->_intensity = intensity;   
	
    adt_a_led_lamp_send_settings(lamp, adt_a_led->_enabled_status, intensity);

	SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_INTENSITY, intensity); 
	
	GCI_Signal_Emit(UIMODULE_SIGNAL_TABLE(lamp), "LampChanged", GCI_VOID_POINTER, lamp); 
	
	adt_a_led_lamp_log_message(lamp);   
	
	return LAMP_SUCCESS;         
}

int adt_a_led_lamp_get_intensity (Lamp* lamp, double *intensity)
{
	ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp; 
	
    *intensity = adt_a_led->_intensity;
	
	return LAMP_SUCCESS;         
}

int adt_a_led_lamp_set_intensity_range (Lamp *lamp, double min, double max, double increment)
{
	ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;  
	
	adt_a_led->_min_intensity = min;
  	adt_a_led->_max_intensity = max;
  	adt_a_led->_intensity_increment = increment;

	return LAMP_SUCCESS;         
}

int adt_a_led_lamp_set_input_trigger (Lamp* lamp, int trigger)
{
    return adt_a_led_lamp_send_data(lamp, 2, trigger,0,0,0); //Select trigger edge
}
	

int adt_a_led_lamp_log_led_mode (Lamp *lamp, ATD_A_LED_MODE mode)
{
	char state_text[100] = "";
	ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;   
	
    switch (mode)
	{
		case ATD_A_LED_OFF:
			strcpy(state_text, "Off");
			break;
		case ATD_A_LED_ON:
			strcpy(state_text, "On");
			break;
		case ATD_A_LED_PULSED:
			strcpy(state_text, "Pulsed");
			break;
	}
	
	logger_log(UIMODULE_LOGGER(adt_a_led), LOGGER_INFORMATIONAL, "%s changed: power %s int %d, enable %d, aux %d",
		UIMODULE_GET_DESCRIPTION(adt_a_led), state_text, adt_a_led->_intensity, adt_a_led->_enabled_status, adt_a_led->_auxout);
	
    return LAMP_SUCCESS;
}

int adt_a_led_lamp_select_output_sequence(Lamp* lamp, int sequence)
{
	int err=0, mode = 5;					//Select sequence output mode
    ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;
    
    SetCtrlVal(UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SELECTOUTPUT, sequence); 
    
	err = adt_a_led_lamp_send_data(lamp, mode, sequence, 0, 0, 0);
	
	if (err<0) return err;
	
	adt_a_led->_selected_sequence = sequence;
	
	return 0;   
}


int  adt_a_led_lamp_hardware_initialise(Lamp* lamp)
{
	int status = UIMODULE_ERROR_NONE; 
	ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;     
	
	get_device_param_from_ini_file(UIMODULE_GET_NAME(adt_a_led), "i2c_Bus", &(adt_a_led->_i2c_bus));
	get_device_param_from_ini_file(UIMODULE_GET_NAME(adt_a_led), "i2c_ChipAddress", &(adt_a_led->_i2c_chip_address));  
	get_device_param_from_ini_file(UIMODULE_GET_NAME(adt_a_led), "AD5241_i2c_ChipAddress", &(adt_a_led->_i2c_ADaddress)); 
	get_device_param_from_ini_file(UIMODULE_GET_NAME(adt_a_led), "i2c_ChipType", &(adt_a_led->_i2c_chip_type));  
	get_device_param_from_ini_file(UIMODULE_GET_NAME(adt_a_led), "COM_Port", &(adt_a_led->_com_port));  
	
	GetI2CPortLock(adt_a_led->_com_port, "Lamp"); 

	initialise_comport(adt_a_led->_com_port, 9600);
	
	adt_ledlamp_a_load_settings(adt_a_led);   
		
	adt_a_led->_auxout = 0;
	adt_a_led_lamp_load_sequence_data (lamp, adt_a_led->_last_uploaded_sequence_filepath);           
	adt_a_led_lamp_set_pw_rep_rate(lamp);
	
	ReleaseI2CPortLock(adt_a_led->_com_port, "Lamp");    
	
	return LAMP_SUCCESS;
}

int  adt_a_led_lamp_initialise(Lamp* lamp)
{
	int status = UIMODULE_ERROR_NONE; 
	ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;     
	
	lamp_set_main_panel (lamp, ui_module_add_panel(UIMODULE_CAST(adt_a_led), "ATD_LedLampUI_A.uir", LEDS_PNL, 1));

    //Load main panel and install callbacks such that adt_a_led is passed in the callback data
    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_LOADDATA, cb_htsleds_loaddata, adt_a_led) < 0)
		return LAMP_ERROR;
  	
    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_INTENSITY, cb_htsleds_intensity, adt_a_led) < 0)
		return LAMP_ERROR;
  	
    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_LEDSTATE, cb_htsleds_ledstate, adt_a_led) < 0)
		return LAMP_ERROR;
  	
    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SEQUENCETIME, cb_htsleds_sequencetime, adt_a_led) < 0)
		return LAMP_ERROR;

    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_NUMTIMESLOTS, cb_htsleds_numtimeslots, adt_a_led) < 0)
		return LAMP_ERROR;
  	
    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_QUIT, cb_htsleds_quit, adt_a_led) < 0)
		return LAMP_ERROR;
  	
    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_LED_ENABLE, cb_htsleds_ledenable, adt_a_led) < 0)
		return LAMP_ERROR;
  	
    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_AUXOUTPUT, cb_htsleds_auxoutput, adt_a_led) < 0)
		return LAMP_ERROR;
  	
    if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SETSLOTTIME, cbset_htsleds_slottime, adt_a_led) < 0)
		return LAMP_ERROR;
  	
	if ( InstallCtrlCallback (UIMODULE_MAIN_PANEL_ID(lamp), LEDS_PNL_SELECTOUTPUT, cb_htsleds_selectoutput, adt_a_led) < 0)
		return LAMP_ERROR;   
	
	return LAMP_SUCCESS;
}

Lamp* adt_a_led_lamp_new(char *name, char *description, UI_MODULE_ERROR_HANDLER handler, const char *data_dir)
{
	ATD_LAMP_A* adt_a_led = (ATD_LAMP_A*) malloc(sizeof(ATD_LAMP_A));
	Lamp *lamp = (Lamp*) adt_a_led;
	
	memset(adt_a_led, 0, sizeof(ATD_LAMP_A));
	
	lamp_constructor(lamp, name, description, handler, data_dir);
	
	LAMP_VTABLE_PTR(lamp, init) = adt_a_led_lamp_initialise;     
	LAMP_VTABLE_PTR(lamp, hardware_init) = adt_a_led_lamp_hardware_initialise;   
	LAMP_VTABLE_PTR(lamp, destroy) = adt_a_led_lamp_destroy;   
	LAMP_VTABLE_PTR(lamp, lamp_off) = adt_a_led_lamp_off;   
	LAMP_VTABLE_PTR(lamp, lamp_on) = adt_a_led_lamp_on;   
	LAMP_VTABLE_PTR(lamp, lamp_off_on_status) = adt_a_led_lamp_off_on_status;
	LAMP_VTABLE_PTR(lamp, lamp_set_intensity) = adt_a_led_lamp_set_intensity;
	LAMP_VTABLE_PTR(lamp, lamp_get_intensity) = adt_a_led_lamp_get_intensity; 
	LAMP_VTABLE_PTR(lamp, lamp_set_intensity_range) = adt_a_led_lamp_set_intensity_range;
	LAMP_VTABLE_PTR(lamp, save_settings) = adt_a_led_lamp_save_settings;   
	LAMP_VTABLE_PTR(lamp, load_settings) = adt_a_led_lamp_load_settings;   

	return lamp;
}


int adt_a_led_lamp_display_settings_ui(Lamp* lamp)
{
	ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;  
	
	ui_module_display_panel(UIMODULE_CAST(adt_a_led), UIMODULE_MAIN_PANEL_ID(lamp));  
	
	return LAMP_SUCCESS;
}


int adt_a_led_lamp_hide_settings_ui(Lamp* lamp)
{
	ATD_LAMP_A *adt_a_led = (ATD_LAMP_A *) lamp;  
	
	ui_module_hide_panel(UIMODULE_CAST(adt_a_led), UIMODULE_MAIN_PANEL_ID(lamp));   

	return LAMP_SUCCESS;
}
