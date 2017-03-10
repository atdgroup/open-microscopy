#include <cviauto.h>
#include <rs232.h>

#include "gci_utils.h"
#include "ATD_Scanner_A.h"
#include "ScannerUI.h"
#include "iniparser.h"

#include "ATD_UsbInterface_A.h"

#include "profile.h"

#include <userint.h>
#include <formatio.h>
#include <utility.h>

////////////////////////////////////////////////////////////////////////////
//GP/RJL/RGN April 2007
//HTS Microscope system. 
//Scanner control.
////////////////////////////////////////////////////////////////////////////

//#define DEBUG

#define Round  RoundRealToNearestInteger 

// The hyst offset (line overscan) is adjusted according to the speed index, BV/GP 28 July 2009
static int hyst_dividers[7] = {1, 1, 2, 5, 10, 50, 100};
// hyst_offset / hyst_dividers[speed]

// the clock frequency for a given speed index, these are the target frequencies, the PIC may not be faithful to these, see atd_scanner_a_scanner_set_rep_rate()
static int frequencies[7] = {0, 800, 400, 200, 100, 50, 25};
	//Speed 0 = Park			   0 KHz	
	//Speed 1 = Very fast		 800 KHz	
	//Speed 2 = Fast			 400 KHz	
	//Speed 3 = Normal			 200 KHz	
	//Speed 4 = Slow			 100 KHz	
	//Speed 5 = Very slow		  50 KHz	
	//Speed 6 = Slowest			  25 KHz	

// The value to be sent to the pic for zoom x1, other zoom values are calculated from this.
// 250 is divisible by 2, 5 and 10 (255 is not :-()
const float zoom_x1_value = 250.0;


static int atd_scanner_a_scanner_save_settings_to_EEPROM(Scanner* scanner)
{
	char data[10]; 
	int err, mode = 11;	   //Store data into EEPROM mode
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner;         
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	data[0] = mode;	    	

	if(ftdi_controller_i2c_write_bytes(atd_scanner_a->_controller, atd_scanner_a->_i2c_chip_address, 5, data) != FT_OK) {
		return SCANNER_ERROR;
	}

	#else

   	data[0] = mode;	    

	err = GCI_Out_PIC_multiPort (atd_scanner_a->_com_port, atd_scanner_a->_i2c_bus,
		atd_scanner_a->_i2c_chip_type, atd_scanner_a->_i2c_chip_address, 5, data);
 	
	if (err) 
		return SCANNER_ERROR;

	#endif

	Delay (0.2);

	return SCANNER_SUCCESS;
}
	
static int atd_scanner_a_scanner_disable_scanner(Scanner* scanner, int disable)
{
	char data[10]; 
	int err, mode = 9;	   //Enable/disable scanner mode
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner;   
	
   	// send 1=enable, 0=standby
	SetCtrlVal(scanner->_main_ui_panel,SCAN_PNL_SCAN_ON_IND, !disable);    
	SetCtrlVal(scanner->_main_ui_panel,SCAN_PNL_SCAN_DISABLE, disable);
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	data[0] = mode;	    
   	data[1] = disable;		

	if(ftdi_controller_i2c_write_bytes(atd_scanner_a->_controller, atd_scanner_a->_i2c_chip_address, 5, data) != FT_OK) {
		return SCANNER_ERROR;
	}

	#else

   	data[0] = mode;	    
   	data[1] = disable;		
	err = GCI_Out_PIC_multiPort (atd_scanner_a->_com_port, atd_scanner_a->_i2c_bus, atd_scanner_a->_i2c_chip_type, atd_scanner_a->_i2c_chip_address, 5, data);
 	
	if (err) 
		return SCANNER_ERROR;

	#endif

	scanner->_scan_disable = disable;

	return SCANNER_SUCCESS;
}

static int atd_scanner_a_scanner_destroy (Scanner* scanner)
{
	int err;
	
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner; 
	
	err =  atd_scanner_a_scanner_save_settings_to_EEPROM(scanner);

	// Put into standby
	atd_scanner_a_scanner_disable_scanner(scanner, 1);  
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT
	ftdi_controller_close(atd_scanner_a->_controller);
	#else
	close_comport(atd_scanner_a->_com_port);
	#endif

	return err;
}

static int atd_scanner_a_scanner_set_dwell_time(Scanner* scanner, int dwell, int clockSelect)
{	// the 'dwell time' is actually the scanner recover time between lines and should be renamed
	char data[10]; 
	int err, mode = 12;		//Set to dwell time mode
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner;  

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	data[0] = mode;	   
   	data[1] = dwell | clockSelect;

	if(ftdi_controller_i2c_write_bytes(atd_scanner_a->_controller, atd_scanner_a->_i2c_chip_address, 5, data) != FT_OK) {
		return SCANNER_ERROR;
	}

	#else

   	data[0] = mode;	   
   	data[1] = dwell | clockSelect;

	err = GCI_Out_PIC_multiPort (atd_scanner_a->_com_port, atd_scanner_a->_i2c_bus,
		atd_scanner_a->_i2c_chip_type, atd_scanner_a->_i2c_chip_address, 5, data);
		
	if (err) 
		return SCANNER_ERROR;

	#endif

	atd_scanner_a->_dwell = dwell;

	return SCANNER_SUCCESS;
}

static int atd_scanner_a_scanner_select_clock(Scanner* scanner, int pixclk, int lineclk, int frameclk)
{
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner;  
	int clockSelect;
	
	clockSelect = pixclk | lineclk<<1 | frameclk<<2;
	return atd_scanner_a_scanner_set_dwell_time(scanner, atd_scanner_a->_dwell, clockSelect);
}

static int atd_scanner_a_scanner_load_line_data(Scanner* scanner, int line)
{
	char data[10]; 
	int msb_line,lsb_line, err;
	int mode = 0; 			 		//Load line data mode   
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner;  
	
	msb_line = line >>8;
	lsb_line = line & 0xff;
				
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	data[0] = mode;	    		
   	data[1] = (msb_line | 0x08);	//Keep msb high
   	data[2] = lsb_line;

	if(ftdi_controller_i2c_write_bytes(atd_scanner_a->_controller, atd_scanner_a->_i2c_chip_address, 5, data) != FT_OK) {
		return SCANNER_ERROR;
	}

	#else

   	data[0] = mode;	    		
   	data[1] = (msb_line | 0x08);	//Keep msb high
   	data[2] = lsb_line;
   	
	err = GCI_Out_PIC_multiPort (atd_scanner_a->_com_port, atd_scanner_a->_i2c_bus,
		atd_scanner_a->_i2c_chip_type, atd_scanner_a->_i2c_chip_address, 5, data);
   				
	if (err) 
		return SCANNER_ERROR;

	#endif

	return SCANNER_SUCCESS;
}

static int atd_scanner_a_scanner_set_rep_rate(Scanner* scanner, int rep_val,int rep_div_1)
{
	char data[10]; 
	double frequency,double_duty_cycle,duty_cycle_freq;
	int divider,duty_cycle;
	int msb_duty_cycle,lsb_duty_cycle;
	int err, mode=2; 			 //Set PWM mode   
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner; 
	
	switch(rep_div_1){			 //Calculate the frequency
		case 0:
			frequency=0;
			divider=1; 
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

	// Store the real frequency to calculate line and frame times
	atd_scanner_a->_current_clock_frequency = frequency;

	//Calculate duty cycle bytes
	//	double_duty_cycle =25.0;	   		//Fix duty cycle to 50%
	double_duty_cycle = 0.005*frequency;   	//Fixes pulse width to 50 ns
	duty_cycle_freq=frequency*(100.0/double_duty_cycle);    
				
	duty_cycle = (int) (20E3/( duty_cycle_freq*divider));
				
	msb_duty_cycle=duty_cycle>>8;
	lsb_duty_cycle=duty_cycle & 0xff;
				
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	data[0]=mode;	     
   	data[1]=rep_val;
   	data[2]=msb_duty_cycle;
   	data[3]=lsb_duty_cycle;
   	data[4]=rep_div_1; 

	if(ftdi_controller_i2c_write_bytes(atd_scanner_a->_controller, atd_scanner_a->_i2c_chip_address, 5, data) != FT_OK) {
		return SCANNER_ERROR;
	}

	#else

   	data[0]=mode;	     
   	data[1]=rep_val;
   	data[2]=msb_duty_cycle;
   	data[3]=lsb_duty_cycle;
   	data[4]=rep_div_1; 
   	
	err = GCI_Out_PIC_multiPort (atd_scanner_a->_com_port, atd_scanner_a->_i2c_bus,
		atd_scanner_a->_i2c_chip_type, atd_scanner_a->_i2c_chip_address, 5, data);
		
	if (err) 
		return SCANNER_ERROR;

	#endif

	return SCANNER_SUCCESS;
}

static int atd_scanner_a_scanner_set_speed(Scanner* scanner, int speed, int hyst_offset)
{
	double set_frequency, set_divider;
	int dwell_times[7] = {0, 0x50, 0x40, 0x30, 0x20, 0x10, 0x00};
	int clockSelect;
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner; 

	if (speed < 1 || speed > 6)
		return SCANNER_ERROR;

	//Set scan speed and hysteresis offset.
	//Speed 1 = Very fast		(SetRepRate(6,4,1);    714.3 KHz	clock1=715)
	//Speed 2 = Fast			(SetRepRate(12,4,1);   384.6 KHz	clock1=385)
	//Speed 3 = Normal			(SetRepRate(24,4,1);   200.0 KHz	clock1=200)
	//Speed 4 = Slow			(SetRepRate(48,4,1);   102.0 KHz	clock1=102)
	//Speed 5 = Very slow		(SetRepRate(96,4,1);   51.5  KHz	clock1=52)
	//Speed 6 = Slowest			(SetRepRate(192,4,1);  25.91 KHz	clock1=26)

	//09/09
	// Also sets the 'dwell time' (recovery time between lines)
	// This is currently works out to be around 700us, independant of speed

	//03/10
	// The frequencies above are not use currently
	// they seem to be, 833, 384, 200, 100, 50, 25 kHz
	
	scanner->_hyst_offset = hyst_offset = min(hyst_offset, 2047);
	
	set_frequency = (double) frequencies[speed];  //Set frequency in KHz
	set_divider = Round((5000.0/set_frequency)-1);	

	if (atd_scanner_a_scanner_set_rep_rate(scanner, (int) set_divider, 4) == SCANNER_ERROR)
		goto Error; 

	//if (atd_scanner_a_scanner_load_line_data(scanner, hyst_offset) == SCANNER_ERROR)
	//	goto Error;	 //This number is or'ed with 0x80 in the routine

	if (atd_scanner_a_scanner_load_line_data(scanner, hyst_offset / hyst_dividers[speed]) == SCANNER_ERROR)
		goto Error;	 //This number is or'ed with 0x80 in the routine

	//clock1= ceil (set_frequency);
	clockSelect = scanner->_pixel_clock | scanner->_line_clock<<1 | scanner->_frame_clock<<2;
	if (atd_scanner_a_scanner_set_dwell_time(scanner, dwell_times[speed], clockSelect) == SCANNER_ERROR) goto Error;
	
	scanner->_speed = speed;
	
	return SCANNER_SUCCESS;

Error:	

	return SCANNER_ERROR;
}

static int atd_scanner_a_scanner_set_resolution(Scanner* scanner, int resolution)
{
	char data[10]; 
	int err, mode = 3, val;	   //Set resolution mode
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner; 
	
	//resolution 1 - 2048 x 2048 etc. on this scanner
	if      (resolution == 2048)
		val = 1;
	else if (resolution == 1024)
		val = 2;
	else if (resolution == 512)
		val = 3;
	else if (resolution == 256)
		val = 4;
	else if (resolution == 128)
		val = 5;
	else if (resolution == 64)
		val = 6;
	else if (resolution == 32)
		val = 7;
	else 
		return SCANNER_ERROR;   // no other values allowed on this scanner
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	data[0] = mode;	   
 	data[1] = val;

	if(ftdi_controller_i2c_write_bytes(atd_scanner_a->_controller, atd_scanner_a->_i2c_chip_address, 5, data) != FT_OK) {
		return SCANNER_ERROR;
	}

	#else

 	data[0] = mode;	   
 	data[1] = val;
	err = GCI_Out_PIC_multiPort (atd_scanner_a->_com_port, atd_scanner_a->_i2c_bus, 
		atd_scanner_a->_i2c_chip_type, atd_scanner_a->_i2c_chip_address, 5, data);
	
	if (err)
		return SCANNER_ERROR;

	#endif

	scanner->_resolution = resolution;
	
	return SCANNER_SUCCESS;
}


static int atd_scanner_a_scanner_set_zoom(Scanner* scanner, int zoom)
{
	char data[10]; 
	int err, val, mode = 4;	   //Set zoom mode
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner; 

	// calc a value to send to the pic from the x1 value
	if (zoom == 0)
		val = 0;		// park
	else	
		val = RoundRealToNearestInteger(zoom_x1_value / (float)zoom); 

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	data[0] = mode;	   
 	data[1] = val;

	if(ftdi_controller_i2c_write_bytes(atd_scanner_a->_controller, atd_scanner_a->_i2c_chip_address, 5, data) != FT_OK) {
		return SCANNER_ERROR;
	}

	#else

 	data[0] = mode;	   
 	data[1] = val;
	err = GCI_Out_PIC_multiPort (atd_scanner_a->_com_port, atd_scanner_a->_i2c_bus, atd_scanner_a->_i2c_chip_type, atd_scanner_a->_i2c_chip_address, 5, data);
 	
	if (err) 
		return SCANNER_ERROR;
	
	#endif

	return SCANNER_SUCCESS;
}

static int atd_scanner_a_scanner_set_x_shift(Scanner* scanner, int x_shift)
{
	char data[10]; 
	int err, mode = 6;	   //Set line shift mode
	int msb_x_shift, lsb_x_shift;
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner; 
	
	msb_x_shift = x_shift >>8;
	lsb_x_shift = x_shift & 0xff;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	data[0] = mode;	    
   	data[1] = msb_x_shift;		
   	data[2] = lsb_x_shift;
   		
	if(ftdi_controller_i2c_write_bytes(atd_scanner_a->_controller, atd_scanner_a->_i2c_chip_address, 5, data) != FT_OK) {
		return SCANNER_ERROR;
	}

	#else

   	data[0] = mode;	    
   	data[1] = msb_x_shift;		
   	data[2] = lsb_x_shift;
	err = GCI_Out_PIC_multiPort (atd_scanner_a->_com_port, atd_scanner_a->_i2c_bus, atd_scanner_a->_i2c_chip_type, atd_scanner_a->_i2c_chip_address, 5, data);
 	
	if (err)
		return SCANNER_ERROR;

	#endif

	scanner->_x_shift = x_shift;

	return SCANNER_SUCCESS;
}

static int atd_scanner_a_scanner_set_y_shift(Scanner* scanner, int y_shift)
{
	char data[10]; 
	int err, mode = 5;	   //Set frame shift mode
	int msb_y_shift, lsb_y_shift;
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner; 
	
	msb_y_shift = y_shift >>8;
	lsb_y_shift = y_shift & 0xff;

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	data[0] = mode;	    
   	data[1] = msb_y_shift;		
   	data[2] = lsb_y_shift;
   		
	if(ftdi_controller_i2c_write_bytes(atd_scanner_a->_controller, atd_scanner_a->_i2c_chip_address, 5, data) != FT_OK) {
		return SCANNER_ERROR;
	}

	#else

   	data[0] = mode;	    
   	data[1] = msb_y_shift;		
   	data[2] = lsb_y_shift;
	err = GCI_Out_PIC_multiPort (atd_scanner_a->_com_port, atd_scanner_a->_i2c_bus, atd_scanner_a->_i2c_chip_type, atd_scanner_a->_i2c_chip_address, 5, data);
 	
	if (err)
		return SCANNER_ERROR;

	#endif

	scanner->_y_shift = y_shift;

	return SCANNER_SUCCESS;
}

static int atd_scanner_get_min_max_x_shift(Scanner* scanner, int *min, int *max)
{
	// correct the fsd of 4095 for the value sent to multiplying DAC, 255 gives full scale but this may not be used
	*min = 0;
	*max = RoundRealToNearestInteger((4095.0 / 255.0) * zoom_x1_value);

	return SCANNER_SUCCESS;
}

static int atd_scanner_get_min_max_y_shift(Scanner* scanner, int *min, int *max)
{
	// correct the fsd of 4095 for the value sent to multiplying DAC, 255 gives full scale but this may not be used
	*min = 0;
	*max = RoundRealToNearestInteger((4095.0 / 255.0) * zoom_x1_value);

	return SCANNER_SUCCESS;
}

static int atd_scanner_get_spc_left_position(Scanner* scanner, int res, int *pos)
{
	*pos = ((scanner->_hyst_offset / hyst_dividers[scanner->_speed]) * res) / 2048;

	return SCANNER_SUCCESS;
}

static int atd_scanner_a_scanner_reverse_scan(Scanner* scanner, int direction)
{
	char data[10]; 
	int err, mode = 10;	   //Reverse scan mode
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner; 

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	data[0] = mode;	    
   	data[1] = direction;	
   		
	if(ftdi_controller_i2c_write_bytes(atd_scanner_a->_controller, atd_scanner_a->_i2c_chip_address, 5, data) != FT_OK) {
		return SCANNER_ERROR;
	}

	#else

   	data[0] = mode;	    
   	data[1] = direction;		
	err = GCI_Out_PIC_multiPort (atd_scanner_a->_com_port, atd_scanner_a->_i2c_bus, atd_scanner_a->_i2c_chip_type, atd_scanner_a->_i2c_chip_address, 5, data);
 	
	if (err)
		return SCANNER_ERROR;

	#endif

	return SCANNER_SUCCESS;
}

static int atd_scanner_a_scanner_line_scan(Scanner* scanner, int enable)
{
	char data[10]; 
	int err, mode = 7;	   //Enable line scan mode
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner; 
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	data[0] = mode;	    
   	data[1] = enable;	
   		
	if(ftdi_controller_i2c_write_bytes(atd_scanner_a->_controller, atd_scanner_a->_i2c_chip_address, 5, data) != FT_OK) {
		return SCANNER_ERROR;
	}

	#else

   	data[0] = mode;	    
   	data[1] = enable;		
	err = GCI_Out_PIC_multiPort (atd_scanner_a->_com_port, atd_scanner_a->_i2c_bus, atd_scanner_a->_i2c_chip_type, atd_scanner_a->_i2c_chip_address, 5, data);
 	
	if (err)
		return SCANNER_ERROR;

	#endif

	scanner->_line_scan = enable;

	return SCANNER_SUCCESS;
}

static int atd_scanner_a_scanner_set_no_frames(Scanner* scanner, int frames)
{
	char data[10]; 
	int err, mode = 8;	   //Set no frames mode
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner; 
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	data[0] = mode;	    
   	data[1] = frames;	
   		
	if(ftdi_controller_i2c_write_bytes(atd_scanner_a->_controller, atd_scanner_a->_i2c_chip_address, 5, data) != FT_OK) {
		return SCANNER_ERROR;
	}

	#else

   	data[0] = mode;	    
   	data[1] = frames;		
	err = GCI_Out_PIC_multiPort (atd_scanner_a->_com_port, atd_scanner_a->_i2c_bus, atd_scanner_a->_i2c_chip_type, atd_scanner_a->_i2c_chip_address, 5, data);
 	
	if (err) 
		return SCANNER_ERROR;

	#endif

	return SCANNER_SUCCESS;
}


static int atd_scanner_a_scanner_get_line_time(Scanner* scanner, double *time)
{
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner; 
	//Get line time in seconds - but just the acquisition part, not any overscan

	//*time = 2048.0 / (double)frequencies[scanner->_speed] / 1000.0;
	*time = 2048.0 / atd_scanner_a->_current_clock_frequency / 1000.0;

	return SCANNER_SUCCESS;
}

static int atd_scanner_a_scanner_get_pixel_time(Scanner* scanner, double *time)
{
	//Get pixel time in seconds
	double line_time;

	atd_scanner_a_scanner_get_line_time(scanner, &line_time);

	*time = line_time / (double)scanner->_resolution;

	return SCANNER_SUCCESS;
}

static int atd_scanner_a_scanner_get_frame_time(Scanner* scanner, double *frame_time)
{
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner;   
	double dwell;

	//Get frame time in seconds
	// old method from Rob
	//*frame_time = scanner->_resolution/(frequencies[scanner->_speed]/(3.0+(scanner->_hyst_offset/1000.0)));

	// 09/09 new calculation
	// should be: *frame_time = scanner->_resolution/(frequencies[scanner->_speed]/(2048 + 'something realted to the 'dwell time' + scanner->_hyst_offset)/1000.0);
	// where the dwell time is the recovery time at the end of each line

	// 03/10 measured the dwell time with Rob and it is set in HW to be roughly constant with speed (through tricky digital logic)
	// approx 640 us when very fast, 800 us at slowest, about 700 us in between
	// added calculation below, look at notes for 17/03/10

//	*frame_time = # of lines                        * (steps in linear rising part    / step frequency                        + constant recovery time per line);

	dwell = (pow(2.0, (atd_scanner_a->_dwell>>4)+4) + pow(2.0, 3)) / atd_scanner_a->_current_clock_frequency / 1000.0;

	*frame_time = scanner->_resolution * ((2048 + scanner->_hyst_offset/hyst_dividers[scanner->_speed]) / 1000.0 / atd_scanner_a->_current_clock_frequency + dwell);
	
	printf ("frame time: %f s\n", *frame_time);

	return SCANNER_SUCCESS;
}

static int atd_scanner_a_scanner_start_scan(Scanner* scanner, int frames)
{
	char data[10]; 
	int err, mode = 1;	   //Start scan mode
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner;   
	
	SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_SCAN_ON_IND, 1);
		
	if (scanner->_line_scan == 1) 
		frames = 0;

	//Calculate time for scan
	if (frames == 0) {			 //continuous scanning
		scanner->_scan_time = -1;
		scanner->_start_time = -1;
		err = atd_scanner_a_scanner_set_no_frames(scanner, 0);   
	}
	else {
//		scanner->_scan_time = frames*(framePulses[scanner->_resolution]/(frequencies[scanner->_speed]/(3.0+(scanner->_hyst_offset/1000.0))));
		atd_scanner_a_scanner_get_frame_time(scanner, &scanner->_scan_time);
		scanner->_start_time = Timer();
		err = atd_scanner_a_scanner_set_no_frames(scanner, frames);
	}

	if (err) {
		SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_SCAN_ON_IND, 0);  
		return SCANNER_ERROR;
	}
		
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	data[0] = mode;	    
   	data[1] = 1;		//Start
   		
	if(ftdi_controller_i2c_write_bytes(atd_scanner_a->_controller, atd_scanner_a->_i2c_chip_address, 5, data) != FT_OK) {
		SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_SCAN_ON_IND, 0);  
		return SCANNER_ERROR;
	}

	if(scanner->_line_scan)
		atd_scanner_a_scanner_line_scan(scanner, 1);

	#else

   	data[0] = mode;	    
   	data[1] = 1;		//Start

	err = GCI_Out_PIC_multiPort (atd_scanner_a->_com_port, atd_scanner_a->_i2c_bus, atd_scanner_a->_i2c_chip_type, atd_scanner_a->_i2c_chip_address, 5, data);
 	
	if(scanner->_line_scan)
		atd_scanner_a_scanner_line_scan(scanner, 1);

	if (err) { 
		SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_SCAN_ON_IND, 0);  
		return SCANNER_ERROR;
	}

	#endif

	return SCANNER_SUCCESS;
}
	
static int atd_scanner_a_scanner_stop_scan(Scanner* scanner)
{
	char data[10]; 
	int err, mode = 1;	   //Start scan mode
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner;   
	
	SetCtrlVal(scanner->_main_ui_panel,SCAN_PNL_SCAN_ON_IND, 0);    
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	data[0] = mode;	    
   	data[1] = 0;		
   		
	if(ftdi_controller_i2c_write_bytes(atd_scanner_a->_controller, atd_scanner_a->_i2c_chip_address, 5, data) != FT_OK) { 
		return SCANNER_ERROR;
	}

	#else

   	data[0] = mode;	    
   	data[1] = 0;		
	
	err = GCI_Out_PIC_multiPort (atd_scanner_a->_com_port, atd_scanner_a->_i2c_bus, atd_scanner_a->_i2c_chip_type, atd_scanner_a->_i2c_chip_address, 5, data);
 	
	if (err) 
		return SCANNER_ERROR;

	#endif

	return SCANNER_SUCCESS;
}
	
	
static int atd_scanner_a_scanner_read_error_signal(Scanner* scanner, int *scannerServoError)
{
	unsigned char data[10] = ""; 
	int err;	  
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner;   
	
	*scannerServoError = 0;
		
   	data[0] = 254;	 // Read error signal    		
	
	#ifdef FTDI_NO_VIRTUAL_COMPORT

	ftdi_controller_get_lock(atd_scanner_a->_controller);
	
	if(ftdi_controller_i2c_write_bytes(atd_scanner_a->_controller, atd_scanner_a->_i2c_chip_address, 5, data) != FT_OK) {
		ftdi_controller_release_lock(atd_scanner_a->_controller);
		return SCANNER_ERROR;
	}

	if(ftdi_controller_i2c_read_bytes(atd_scanner_a->_controller, atd_scanner_a->_i2c_chip_address, 1, data) != FT_OK) {
		ftdi_controller_release_lock(atd_scanner_a->_controller);
		return SCANNER_ERROR;
	}

	ftdi_controller_release_lock(atd_scanner_a->_controller);
	
	#else

	GetI2CPortLock(atd_scanner_a->_com_port, "Scanner");    

	err = GCI_Out_PIC_multiPort (atd_scanner_a->_com_port, atd_scanner_a->_i2c_bus,
		atd_scanner_a->_i2c_chip_type, atd_scanner_a->_i2c_chip_address, 5, data);
   				
	if (err) {
		ReleaseI2CPortLock(atd_scanner_a->_com_port, "Scanner"); 
		return SCANNER_ERROR;
	}
		
	data[0]=atd_scanner_a->_i2c_chip_type | (atd_scanner_a->_i2c_chip_address <<1) | 0x01;
	err = GCI_readI2C_multiPort(atd_scanner_a->_com_port, 1, data, atd_scanner_a->_i2c_bus, "Scanner");
	
	if (err) {
		ReleaseI2CPortLock(atd_scanner_a->_com_port, "Scanner"); 
		return SCANNER_ERROR;
	}

	ReleaseI2CPortLock(atd_scanner_a->_com_port, "Scanner");   
	
	#endif

	*scannerServoError = data[0] & 0x01; 

	return SCANNER_SUCCESS;
}

static void ftdi_error_handler (FTDIController *controller, const char *title, const char *error_string, void *callback_data)    
{
	printf("%s: %s\n", title, error_string);
}

static int atd_scanner_a_scanner_hw_init (Scanner* scanner)
{
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner;  

	#ifdef FTDI_NO_VIRTUAL_COMPORT

	char device_sn[FTDI_SERIAL_NUMBER_LINE_LENGTH] = "";

	atd_scanner_a->_controller = ftdi_controller_new();

	ftdi_controller_get_lock(atd_scanner_a->_controller);
	
	get_device_string_param_from_ini_file(UIMODULE_GET_NAME(atd_scanner_a), "FTDI_SN", device_sn);  
	get_device_int_param_from_ini_file   (UIMODULE_GET_NAME(atd_scanner_a), "I2C_DEVICE_BUS", &(atd_scanner_a->_i2c_bus));
	get_device_int_param_from_ini_file   (UIMODULE_GET_NAME(atd_scanner_a), "I2C_DEVICE_ADDRESS", &(atd_scanner_a->_i2c_chip_address));  

//	ftdi_controller_set_debugging(atd_scanner_a->_controller, 1);
	ftdi_controller_set_error_handler(atd_scanner_a->_controller, ftdi_error_handler, NULL);
	ftdi_controller_open(atd_scanner_a->_controller, device_sn);

	if (scanner_set_init_values(scanner) == SCANNER_ERROR) {
		ftdi_controller_release_lock(atd_scanner_a->_controller);
		return SCANNER_ERROR;	
	}

	//Send 0 to enable, 1 for standby
	SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_SCAN_DISABLE, 0);
	scanner_disable_scanner(scanner, 0);
			
	scanner_on_change(scanner, -1); //update uir
	
	ftdi_controller_release_lock(atd_scanner_a->_controller);
	
	#else

	get_device_param_from_ini_file(UIMODULE_GET_NAME(atd_scanner_a), "i2c_Bus", &(atd_scanner_a->_i2c_bus));
	get_device_param_from_ini_file(UIMODULE_GET_NAME(atd_scanner_a), "i2c_ChipAddress", &(atd_scanner_a->_i2c_chip_address));  
	get_device_param_from_ini_file(UIMODULE_GET_NAME(atd_scanner_a), "i2c_ChipType", &(atd_scanner_a->_i2c_chip_type));  
	get_device_param_from_ini_file(UIMODULE_GET_NAME(atd_scanner_a), "COM_Port", &(atd_scanner_a->_com_port));  
	
	GetI2CPortLock(atd_scanner_a->_com_port, "Scanner"); 
	
	if(initialise_comport(atd_scanner_a->_com_port, 9600) < 0) {
		ReleaseI2CPortLock(atd_scanner_a->_com_port, "Scanner");    
		return SCANNER_ERROR;	
	}

	if (scanner_set_init_values(scanner) == SCANNER_ERROR) {
		ReleaseI2CPortLock(atd_scanner_a->_com_port, "Scanner");     
		return SCANNER_ERROR;	
	}

	//Send 0 to enable, 1 for standby
	SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_SCAN_DISABLE, 0);
	scanner_disable_scanner(scanner, 0);
			
	scanner_on_change(scanner, -1); //update uir
	
	ReleaseI2CPortLock(atd_scanner_a->_com_port, "Scanner");     
	
	#endif

	return SCANNER_SUCCESS;   
}


int atd_scanner_a_scanner_save_settings(Scanner* scanner, const char*filepath, const char *flags)
{
	FILE *fd;
    int standby;
	dictionary *d = dictionary_new(20);
	
	fd = fopen(filepath, flags);
	
	dictionary_set(d, "Scanner", NULL);

	GetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_SCAN_DISABLE, &standby);  
	dictionary_setint(d, "Standby", standby);     
	
    dictionary_setint(d, "Line_scan", scanner->_line_scan);
	dictionary_setint(d, "Frames", scanner->_frames);            
	dictionary_setint(d, "Zoom", scanner->_zoom);        
	dictionary_setint(d, "Speed", scanner->_speed);        
	dictionary_setint(d, "Resolution", scanner->_resolution);        
	dictionary_setint(d, "Pixel_clock", scanner->_pixel_clock);        
	dictionary_setint(d, "Line_clock", scanner->_line_clock);        
	dictionary_setint(d, "Frame_clock", scanner->_frame_clock);        
	dictionary_setint(d, "Scan_reversed", scanner->_reverse_scan);        
	dictionary_setint(d, "Hyst_offset", scanner->_hyst_offset);  
	dictionary_setint(d, "x_offset", scanner->_x_offset);
	dictionary_setint(d, "y_offset", scanner->_y_offset);
	
	iniparser_save(d, fd); 
	
	fclose(fd);
	dictionary_del(d);
	
    return SCANNER_SUCCESS;
}


int atd_scanner_a_scanner_load_settings(Scanner* scanner, const char*filepath)
{
	dictionary* d = NULL;  
	int tmp, file_size = 0;
	
	if(!FileExists(filepath, &file_size))
		return SCANNER_SUCCESS;	 	
	
	d = iniparser_load(filepath);  

	if(d != NULL) {
		
		tmp = dictionary_getint(d, "Scanner:Standby", -1);  
		
		if(tmp != -1) {

			if(tmp == 1)
				scanner_disable_scanner(scanner, 1);    
			else
				scanner_disable_scanner(scanner, 0);    
		}
		
    	tmp = dictionary_getint(d, "Scanner:Line_scan", 0);
    
		if(tmp > 0) {
			scanner->_line_scan = tmp;
			SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_LINE_SCAN, scanner->_line_scan);
		}
	
		tmp = dictionary_getint(d, "Scanner:Frames", 0);
    
		if(tmp > 0) {
			scanner->_frames = tmp;
			SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_FRAME_NUM, scanner->_frames);    
		}
	
		tmp = dictionary_getint(d, "Scanner:Zoom", 0);
    
		if(tmp > 0) {
			scanner_set_zoom(scanner, tmp);
		}
		
		tmp = dictionary_getint(d, "Scanner:Speed", 0);
    
		if(tmp > 0) {
			scanner->_speed = tmp;
			SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_SPEED, scanner->_speed);  
		}
	
		tmp = dictionary_getint(d, "Scanner:Resolution", 0);
    
		if(tmp > 0) {
			scanner->_resolution = tmp;
			SetCtrlVal(scanner->_main_ui_panel, SCAN_PNL_RESOLUTION, scanner->_resolution);  
		}
	
		tmp = dictionary_getint(d, "Scanner:Pixel_clock", 0);
    
		if(tmp > 0) {
			scanner->_pixel_clock = tmp;
			SetCtrlVal(scanner->_cal_ui_panel, SCNCALPNL_PIXCLK, scanner->_pixel_clock);   
		}
		
		tmp = dictionary_getint(d, "Scanner:Line_clock", 0);
    
		if(tmp > 0) {
			scanner->_line_clock = tmp;
			SetCtrlVal(scanner->_cal_ui_panel, SCNCALPNL_LINECLK, scanner->_line_clock);   
		}
	
		tmp = dictionary_getint(d, "Scanner:Frame_clock", 0);
    
		if(tmp > 0) {
			scanner->_frame_clock = tmp;
			SetCtrlVal(scanner->_cal_ui_panel, SCNCALPNL_FRAMECLK, scanner->_frame_clock);     
		}
	
		tmp = dictionary_getint(d, "Scanner:Scan_reversed", 0);
    
		if(tmp > 0) {
			scanner->_reverse_scan = tmp;
			SetCtrlVal(scanner->_cal_ui_panel, SCNCALPNL_REV_SCAN, scanner->_reverse_scan);      
		}
	
		tmp = dictionary_getint(d, "Scanner:Hyst_offset", 0);
    
		if(tmp > 0) {
			scanner->_hyst_offset = tmp;
			SetCtrlVal(scanner->_cal_ui_panel, SCNCALPNL_HYST_OFFSET, scanner->_hyst_offset);  
		}
		
		tmp = 0;  // offset can be -ve, reset tmp to 0 as will always use the value
		tmp = dictionary_getint(d, "Scanner:x_offset", 0);
    
		scanner->_x_offset = tmp;
		SetCtrlVal(scanner->_cal_ui_panel, SCNCALPNL_X_OFFSET, scanner->_x_offset);  

		tmp = 0;  // offset can be -ve, reset tmp to 0 as will always use the value
		tmp = dictionary_getint(d, "Scanner:y_offset", 0);
    
		scanner->_y_offset = tmp;
		SetCtrlVal(scanner->_cal_ui_panel, SCNCALPNL_Y_OFFSET, scanner->_y_offset);  

		dictionary_del(d);   
	}
	
	PROFILE_START("load_microscope_settings - scanner_set_init_values");

	scanner_set_init_values (scanner);
	
	PROFILE_STOP("load_microscope_settings - scanner_set_init_values");


    return SCANNER_SUCCESS;
}


Scanner* atd_scanner_a_new(char *name, char *description,  const char *data_dir, const char *filepath, UI_MODULE_ERROR_HANDLER error_handler, void *data)
{
	Scanner* scanner = scanner_new(name, description, data_dir, filepath, sizeof(ATD_Scanner_A));
	
	ATD_Scanner_A *atd_scanner_a = (ATD_Scanner_A *) scanner;
	
	ui_module_set_error_handler(UIMODULE_CAST(scanner), error_handler, scanner);
										    
	SCANNER_VTABLE_PTR(scanner, hw_init) = atd_scanner_a_scanner_hw_init;
	SCANNER_VTABLE_PTR(scanner, destroy) = atd_scanner_a_scanner_destroy; 
	SCANNER_VTABLE_PTR(scanner, scanner_select_clock) = atd_scanner_a_scanner_select_clock; 
	SCANNER_VTABLE_PTR(scanner, scanner_load_line_data) = atd_scanner_a_scanner_load_line_data; 
	SCANNER_VTABLE_PTR(scanner, scanner_set_rep_rate) = atd_scanner_a_scanner_set_rep_rate;
	SCANNER_VTABLE_PTR(scanner, scanner_set_speed) = atd_scanner_a_scanner_set_speed;
	SCANNER_VTABLE_PTR(scanner, scanner_set_resolution) = atd_scanner_a_scanner_set_resolution;
 	SCANNER_VTABLE_PTR(scanner, scanner_set_zoom) = atd_scanner_a_scanner_set_zoom;
	SCANNER_VTABLE_PTR(scanner, scanner_set_x_shift) = atd_scanner_a_scanner_set_x_shift;
	SCANNER_VTABLE_PTR(scanner, scanner_set_y_shift) = atd_scanner_a_scanner_set_y_shift;
	SCANNER_VTABLE_PTR(scanner, scanner_get_min_max_x_shift) = atd_scanner_get_min_max_x_shift;
	SCANNER_VTABLE_PTR(scanner, scanner_get_min_max_y_shift) = atd_scanner_get_min_max_y_shift;
	SCANNER_VTABLE_PTR(scanner, scanner_get_spc_left_position) = atd_scanner_get_spc_left_position;
	SCANNER_VTABLE_PTR(scanner, scanner_reverse_scan) = atd_scanner_a_scanner_reverse_scan;
	SCANNER_VTABLE_PTR(scanner, scanner_line_scan) = atd_scanner_a_scanner_line_scan;
	SCANNER_VTABLE_PTR(scanner, scanner_set_no_frames) = atd_scanner_a_scanner_set_no_frames;
	SCANNER_VTABLE_PTR(scanner, scanner_get_frame_time) = atd_scanner_a_scanner_get_frame_time;
	SCANNER_VTABLE_PTR(scanner, scanner_get_line_time) = atd_scanner_a_scanner_get_line_time;
	SCANNER_VTABLE_PTR(scanner, scanner_get_pixel_time) = atd_scanner_a_scanner_get_pixel_time;
	SCANNER_VTABLE_PTR(scanner, scanner_start_scan) = atd_scanner_a_scanner_start_scan;
	SCANNER_VTABLE_PTR(scanner, scanner_stop_scan) = atd_scanner_a_scanner_stop_scan;
	SCANNER_VTABLE_PTR(scanner, scanner_disable_scanner) = atd_scanner_a_scanner_disable_scanner;
	SCANNER_VTABLE_PTR(scanner, scanner_save_settings_to_EEPROM) = atd_scanner_a_scanner_save_settings_to_EEPROM;
	SCANNER_VTABLE_PTR(scanner, scanner_read_error_signal) = atd_scanner_a_scanner_read_error_signal;
	SCANNER_VTABLE_PTR(scanner, save_settings) = atd_scanner_a_scanner_save_settings;
	SCANNER_VTABLE_PTR(scanner, load_settings) = atd_scanner_a_scanner_load_settings;   	

	return scanner;
}
