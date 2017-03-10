#ifndef _USBCONVERTER_AM
#define _USBCONVERTER_AM

#include "ATD_UsbInterface_A.h"

#include "gci_ui_module.h"

#include <userint.h>
#include <rs232.h>
#include <stdio.h>
#include <utility.h>
#include <formatio.h>
#include "toolbox.h"
#include <ansi_c.h>

#include "ThreadDebug.h"

static Logger* i2c_logger = NULL;

#define COM_TIMEOUT 5.0

void gci_set_i2c_logger(Logger* logger)
{
	i2c_logger = logger;
}

///////////////////////////////////////////////////////////////////////////////
// Routines for FTDI USB -> parallel/serial devices
// Andreas Manser 2002
///////////////////////////////////////////////////////////////////////////////
// RJL 18 December 2003
// This is the most recent version as used in the GCI MP system
//
// Changes from A Manser's original
// 1. Commented out error messages in usberr() function as errors should be reported at a higher level
// 2. Made it conform to software guidlines, i.e. export functions are named GCI_
//
///////////////////////////////////////////////////////////////////////////////
// RJL 23 December 2003
// Added function GCI_EnableLowLevelErrorReporting() as error messages may be required
// during initial hardware tests. By default it is disabled.
///////////////////////////////////////////////////////////////////////////////
// RJL 14 April 2004
// Integrated changes from Rob's version of the program 
// Note only PICs with Rob's new firmware return data when using the FAST functions
///////////////////////////////////////////////////////////////////////////////
// RJL July 2004
// Add functions for multiple port operation.
// The single port functions remain for backward compatibility.
// The multi port functions should be used for new projects.
///////////////////////////////////////////////////////////////////////////////
 

typedef unsigned char byte;
static int mi2cbus = GCI_I2C_SINGLE_BUS;	// No switching of the I2C bus

static int reportErrors = 0;
//Will we ever get higher than COM10 ?
static int gPortOpen[10] = {0,0,0,0,0,0,0,0,0,0};

static bus_locks[] = {-1, -1, -1, -1, -1 , -1 , -1, -1, -1, -1};
static got_bus_locks[] = {0, 0, 0, 0, 0 , 0 , 0, 0, 0, 0};
static int lock[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};		

static int CVICALLBACK ThreadedGetOutQLen(void *callback)
{
	int *port = (int *) callback;

	// GetOutQLen Hangs if the communication is broken 
	return GetOutQLen(*port);
}

/*
// Make sure the processor actually writes the output queue data before moving on.
// Has a timeout on this function so it doesn't get stuck in the while loop if the com port is 
// not working.
static int GCI_ComWrt(int COMPort, char buffer[], int count)
{
	double start_time = Timer(), elapsed_time = 0.0;
	int ret, thread_id, thread_ret, thread_status;

	if(count == 0)
		return 0;

	ret = ComWrt(COMPort, buffer, count);    
	
	if (ret <= 0)
		return ret;
	
	CmtScheduleThreadPoolFunction (gci_thread_pool(), ThreadedGetOutQLen, &COMPort, &thread_id);

	elapsed_time = Timer() - start_time;

	// GetOutQLen Hangs if the communication is broken 
	while(elapsed_time < COM_TIMEOUT) {
	
		CmtGetThreadPoolFunctionAttribute (gci_thread_pool(), thread_id, ATTR_TP_FUNCTION_EXECUTION_STATUS, &thread_status);

		if(thread_status > 2) // Finish executing thread
			break;
			
		Delay(0.005);
		elapsed_time = Timer() - start_time;
		ProcessSystemEvents();
	}
	
	// Get the return value GetOutQLen
	CmtGetThreadPoolFunctionAttribute (gci_thread_pool(), thread_id, ATTR_TP_FUNCTION_RETURN_VALUE, &ret);
	
	// Timed out
	if(elapsed_time >= COM_TIMEOUT) {

		CmtTerminateThreadPoolThread (gci_thread_pool(), thread_id, -1);

		// Here we are in the situation that the serial port is not responding
		// I will log this error for now
		GCI_MessagePopup("Critical Error", "The searial port is not responding!");

		return -1;
	}

	CmtReleaseThreadPoolFunctionID (gci_thread_pool(), thread_id);

	return ret;
}
*/

// Make sure the processor actually writes the output queue data before moving on.
static int GCI_ComWrt(int COMPort, unsigned char buffer[], int count)
{
	int ret, len;

	GetI2CPortLock(COMPort, "GCI_ComWrt");
		
	ret = ComWrt(COMPort, buffer, count);    
	
	if (ret != count) {
		ReleaseI2CPortLock(COMPort, "GCI_ComWrt");
		return ret;
	}
	
	// Some FTDI devices need this as GetOutQLen return 0 all the time
	// with the virtual com driver ?
	// I think Im going to assume our PIC code can't cope with anything faster.
	Delay(0.001);

	len = GetOutQLen(COMPort);

	while(len > 0) {
		Delay(0.001);
		continue;
	}
	
	ReleaseI2CPortLock(COMPort, "GCI_ComWrt");

	return ret;
}

static int GCI_ComRd(int portNo, char buf[], int maxCnt)
{
	int ret;

	GetI2CPortLock(portNo, "GCI_ComRd");
		
	ret = ComRd(portNo, buf, maxCnt);    
	
	if (ret != maxCnt) {
		ReleaseI2CPortLock(portNo, "GCI_ComRd");
		return ret;
	}

	ReleaseI2CPortLock(portNo, "GCI_ComRd");

	return ret;
}

////////////////////////////////////////////////////////////////////////
static int usberr(int stat)
{
	//#ifdef VERBOSE_DEBUG
	//printf("Usberr %d %d\n", stat, GetCurrentThreadId());
	//#endif

	switch (stat){
		case 4:     return 0;  //OK
                        
		case 5:     return 0;  //OK
            
		case 2:
		{
			return -2;
		}
            
		case 8:
		{
			return -3;
        }
					
		case 1:
		{
			return -4;
        }
					
		case -99:
		{
			return -99;
        }
					
		default:
		{
			break;
		}
	}
	
	return -1;
}

static int selectBus(int port, int i2cbus)
{
   	unsigned char senddata[10];

	//convert_i2cbus to uchar[]  for selecting it
    senddata[0]=0x02;   //i2c
    senddata[1]=0x04;   //nbr_bytes
    senddata[2]=0x68;   //address
    senddata[3]=0xc0;   //command"switchset"
    switch (i2cbus){
    	case 8: senddata[4]=0x00;   //all switches open
				senddata[5]=0x00;   //that's it
				break;
		case 7: senddata[4]=0x30;   //sw8 sw5
				senddata[5]=0x00;   //that's it
				break;
		case 6: senddata[4]=0x01;   //sw6a 
				senddata[5]=0x20;   //sw3b
				break;
		case 5: senddata[4]=0x02;   //sw6b
				senddata[5]=0x10;   //sw3a
				break;
		case 4: senddata[4]=0x04;   //sw7a
				senddata[5]=0x08;   //sw2b
				break;
		case 3: senddata[4]=0x08;   //sw7b
				senddata[5]=0x04;   //sw2a
				break;
		case 2: senddata[4]=0x00;   //none
				senddata[5]=0x82;   //sw4b sw1b
				break;
		case 1: senddata[4]=0x00;   //none
				senddata[5]=0x41;   //sw4a sw1a
				break;
	}
	 GCI_ComWrt(port, senddata, 6);   //send it
	if (usberr(ComRdByte(port))) return -1;;
                            
	mi2cbus = i2cbus;
    Delay(0.1);                     //makes occasional ack err dissapear!!! 
	return 0;
}
					  

static int write_usbconverter(int port, byte command, int nbr_bytes, byte data[], int i2cbus, const char *name)
{
	int i, stat;
	unsigned char senddata[255];

	GetI2CPortLock(port, "write_usbconverter");

	memset(senddata, 0, 255); 
	
	if((stat = FlushInQ (port)) < 0) {
		if(i2c_logger != NULL)
			logger_log(i2c_logger, LOGGER_ERROR, "Unknown COM write error: code %i", stat); 

		ReleaseI2CPortLock(port, "write_usbconverter");

		return -1;
	}
        
	switch (command){
		case 0x03:              //RS232
			i2cbus = mi2cbus;   //don't switch the i2c bus
                                //don't break!
                        
		case 0x02:						//I2C
			if (i2cbus != mi2cbus){		//switching of i2cbus needed
				if (selectBus(port, i2cbus) < 0) {
					ReleaseI2CPortLock(port, "write_usbconverter");
					return -1;
				}
			}
			senddata[0] = command;
			senddata[1] = nbr_bytes;
			for (i=0; i<nbr_bytes; i++)
				senddata[i+2] = data[i];
			
			if ( GCI_ComWrt(port, senddata, nbr_bytes+2) < 0) {
				ReleaseI2CPortLock(port, "write_usbconverter");
				return -1;
			}
			
			stat = ComRdByte (port);
			
            break;
		case 0x01:                      //FAST write
			senddata[0] = command;
			senddata[1] = nbr_bytes;
			for (i=0; i<nbr_bytes; i++)
				senddata[i+2] = data[i];
			
			if ( GCI_ComWrt(port, senddata, nbr_bytes+2) < 0) {
				ReleaseI2CPortLock(port, "write_usbconverter");
				return -1;
			}
			
			//old PICs don't return anything, so if nothing returned assume success
			if (GetInQLen(port) < 1)
				stat = 4;
			else { 
				
				if(GCI_ComRd(port, data, 1) < 0) {	//just read one byte
					ReleaseI2CPortLock(port, "write_usbconverter");
					return -1;
				}
					
				if (data[0] != 0x04)   	//should come back with 0x04 for I2CFAST PIC
					stat = 2;
				else       
					stat = 4;
			}
            break;
			
		case 0x04:                      //FAST read and write
			
			senddata[0] = 0x01;         //0x01 for fast command
			senddata[1] = nbr_bytes;
			
			for (i=0; i<nbr_bytes; i++)
				senddata[i+2] = data[i];
			
			if ( GCI_ComWrt(port, senddata, nbr_bytes+2) < 0) {
				ReleaseI2CPortLock(port, "write_usbconverter");
				return -1;
			}
            
			stat = GetInQLen(port); 
			
			if (stat>0)
				GCI_ComRd(port, data, i2cbus);	//just read using i2cbus integer    
			
			stat = 4;                    			//as  number of bytes returned  
            break;  
    }
	
	ReleaseI2CPortLock(port, "write_usbconverter");

    return usberr(stat);//stat;
}


static int read_usbconverter(int port, byte command, int nbr_bytes, byte  data[], int i2cbus, const char *name)
{
    int i, stat;
    unsigned char senddata[128];
        
	GetI2CPortLock(port, "read_usbconverter");

	memset(senddata, 0, 128);
	
	if((stat = FlushInQ (port)) < 0) {

		if(i2c_logger != NULL)
			logger_log(i2c_logger, LOGGER_ERROR, "Unknown COM read error: code %i", stat); 

		ReleaseI2CPortLock(port, "read_usbconverter");

		return -1;
	}
                        
    switch (command) {
        case 0x02:                      //I2C
			if (i2cbus != mi2cbus){		//switching of i2cbus needed
				if (selectBus(port, i2cbus) < 0) {
					ReleaseI2CPortLock(port, "read_usbconverter");
					return -1;
				}
			}
            senddata[0] = command;
            senddata[1] = nbr_bytes+1;
            for (i=0; i<nbr_bytes; i++)
                senddata[i+2] = data[i];
            
			if ( GCI_ComWrt(port, senddata, 3) < 0) {
				ReleaseI2CPortLock(port, "read_usbconverter");
				return -1;   //3 bytes for command, nbr_bytes and address (valid only for i2c)
			}

            for (i=0; i<nbr_bytes; i++)
                data[i] = 0x00;
			
            if (GCI_ComRd(port, data, nbr_bytes+1) < 0) {
				ReleaseI2CPortLock(port, "read_usbconverter");
				return -1;      //data + 1 for status, was used for address while sending
			}
			
            stat = data[nbr_bytes];
			
			break;
		
		case 0x01:                      //FAST

			senddata[0] = command;
			nbr_bytes = nbr_bytes+1;    //number of bytes + 1 for RW command
			senddata[1] = nbr_bytes;
			for (i=0; i<nbr_bytes; i++)  
				data[i] = 0x00;
                     
			for (i=0; i<nbr_bytes+1; i++)
				senddata[i+2] = data[i];
			
			if ( GCI_ComWrt(port, senddata, nbr_bytes+3) < 0) {
				ReleaseI2CPortLock(port, "read_usbconverter");
				return -1;
			}
			
			stat = GetInQLen(port); 
                    
			if(GCI_ComRd(port, data, nbr_bytes-1) < 0) { //just read correct number of bytes
				ReleaseI2CPortLock(port, "read_usbconverter");
				return -1;
			}
				
			stat = 4;						//no status byte returned 
			
			break;  
                
		default:                                        //RS232
			stat = GetInQLen(port);
			
			if (stat == 0)
				break;
			
			GCI_ComRd(port, data, stat);  //just read
			stat = 4;				//no status byte returned
            break;
                        
        }          

	ReleaseI2CPortLock(port, "read_usbconverter");

    return usberr(stat);//stat;
}

/////////////////////////////////////////////////////////////////////////////////

void GCI_EnableLowLevelErrorReporting(int enable)
{
    reportErrors = enable;
}

static char* construct_key(char *buffer, const char *section, const char* name)
{
	memset(buffer, 0, 1);
	
	sprintf(buffer, "%s:%s", section, name);
	
	return buffer;
}


void GetI2CPortLock(int port, const char *name)
{
	if (bus_locks[port] < 0) {
		
		char buf[50] = "";

		sprintf(buf, "I2CPortLock-Port%d", port);
		GciCmtNewLock (buf, 0, &bus_locks[port]);  //for multi-threading
	}
		
	if(GciCmtGetLock (bus_locks[port]) < 0) {
		printf("Error Getting Lock");
		return;
	}
}

void ReleaseI2CPortLock(int port, const char *name)
{
	int err;
	
	if((err = GciCmtReleaseLock (bus_locks[port])) < 0) {
		 return;
	}
}

int GCI_readI2C_multiPort(int port, int nbrbytes, byte data[], int i2cbus, const char* name)
{
    int i;
    
	GetI2CPortLock(port, name);      

	i = read_usbconverter(port, 0x2, nbrbytes, data, i2cbus, name);  
		
	ReleaseI2CPortLock(port, name); 

    return i;   
}
    
int GCI_writeI2C_multiPort(int port, int nbrbytes, byte data[], int i2cbus, const char *name)
{
	int ret;
	
	GetI2CPortLock(port, name);
	
	ret = write_usbconverter(port, 0x2, nbrbytes, data, i2cbus, name);

	ReleaseI2CPortLock(port, name); 

    return ret;
}
    
int GCI_writeFAST_multiPort(int port, byte data[], int nbr_bytes, const char *name)
{
	int ret;   
	
	GetI2CPortLock(port, name);     
	
	ret = write_usbconverter(port, 0x01, nbr_bytes, data, 0, name);        
	
	ReleaseI2CPortLock(port, name);  
	
	return ret;
}

int GCI_readFAST_multiPort(int port, byte data[], int nbr_bytes, const char* name)
{
	int ret;   
	
	GetI2CPortLock(port, name);     
	
	ret = read_usbconverter(port, 0x01, nbr_bytes, data, 0, name);  

	ReleaseI2CPortLock(port, name);  
	
	return ret;
}

int GCI_writereadFAST_multiPort(int port, byte data[], int nbr_bytes, int ret_bytes)
{
    return  write_usbconverter(port, 0x04, nbr_bytes, data, ret_bytes, NULL);
}
	
int GCI_Out_PIC_multiPort (int port, int bus, byte chip_type, byte address, int n_bytes, byte *data)
{
	int err, i;
	byte val[1000]; 									  
 
	//Send a byte. Any input lines must be set high in dirs.
	
	GetI2CPortLock(port, "GCI_Out_PIC_multiPort");  

   	val[0] = chip_type | (address <<1);
	
   	for (i=0; i<n_bytes; i++)
   		val[i+1] = data[i];
	
   	err = GCI_writeI2C_multiPort(port, n_bytes+1, val, bus, "GCI_Out_PIC_multiPort");

	ReleaseI2CPortLock(port, "GCI_Out_PIC_multiPort");    
	
	return err;
}

#endif /* _USBCONVERTER_AM */               
