#include "gci_ui_module.h"
#include "RS232Corvus_Communication.h"
#include "stage\stage.h"
#include "status.h" 
#include "gci_utils.h" 
#include "RS232CorvusXY.h"

#include <formatio.h>
#include <utility.h>
#include <rs232.h>
#include <userint.h>

#include "ThreadDebug.h"

// Make sure the processor actually writes the output queue data before moving on.
// Could put a timeout on this function so it doesn't get stuck in the while loop
// No need for delays when sending some messages to the stage now.
static int GCI_ComWrt(int COMPort, char buffer[], int count)
{
	int ret;
	char read_data[RS232_ARRAY_SIZE]="";

	ret = ComWrt(COMPort, buffer, count);    
	
	if (ret < 0)
		return ret;
	
	while(GetOutQLen(COMPort)>0) {
		Delay(0.005);
		continue;
	}
	
	return ret;
}

static int STAGE_CommsErrorMessage(CorvusXYStage *this)
{
	char *msg;
	int err, stat;
	XYStage *stage = (XYStage *) this;  
	
	// Break down in communication has occured
	err = ReturnRS232Err();
	
	if (err < 0) {
	
		if (stage->_log_errors) {
		
			SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);
			msg = GetRS232ErrorString (err);

			stat = GetComStat(this->_com_port);

			logger_log(UIMODULE_LOGGER(stage), LOGGER_ERROR, "XY Stage Controller Communication Error: %s Com Stat %d", msg, stat); 
		}
		
		return err;
	}

	return 0;
}

int RS232CorvusSend(CorvusXYStage *this, char* fmt, ...) 
{
    char buffer[1000];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buffer, fmt, ap);
	va_end(ap);    
    
    GciCmtGetLock (this->_lock);					 //for multi-threading
    
	#ifdef STAGE_DEBUG
        printf(buffer);
    #endif
		
	// flush the Qs
	FlushInQ(this->_com_port);
	FlushOutQ(this->_com_port);	

	// send string 
	if (GCI_ComWrt(this->_com_port, buffer, strlen(buffer)) <= 0) {
		GciCmtReleaseLock (this->_lock);
		return STAGE_CommsErrorMessage(this);
	}
   
	GciCmtReleaseLock (this->_lock);
	
	return 0;
}		
   

int RS232CorvusSendandBlockUntilFinished(CorvusXYStage *this, char* fmt, ...) 
{
    int retval;
	char buffer[1000];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buffer, fmt, ap);
	va_end(ap);    
    
    #ifdef STAGE_DEBUG
        printf(buffer);
    #endif
	
    GciCmtGetLock (this->_lock);					 //for multi-threading
    
	if(SetComTime(this->_com_port, STAGE_LONG_TIMEOUT))
		return STAGE_ERROR;
	
	// flush the Qs
	FlushInQ(this->_com_port);
	FlushOutQ(this->_com_port);	

	// send string 
	if (GCI_ComWrt(this->_com_port, buffer, strlen(buffer)) <= 0) {
		goto Error;
	}
   
	// Block any further commands
	if (GCI_ComWrt(this->_com_port, "ge\n", strlen("ge\n")) <= 0) {
		goto Error;
	}
	
	if (STAGE_SendReadInt(this, "st\n", &retval)) {
		goto Error;
	}
	
	SetComTime(this->_com_port, STAGE_NORMAL_TIMEOUT);   
	GciCmtReleaseLock (this->_lock);   

	return 0;

Error:

	SetComTime(this->_com_port, STAGE_NORMAL_TIMEOUT);   
	GciCmtReleaseLock (this->_lock);   
	
	return STAGE_CommsErrorMessage(this);
}		


int RS232CorvusBlockUntilFinished(CorvusXYStage *this) 
{
	char buffer[1000];
 
    GciCmtGetLock (this->_lock);					 //for multi-threading
    
	// Block any further commands
	if (GCI_ComWrt(this->_com_port, "ge\n", strlen("ge\n")) <= 0) {
		GciCmtReleaseLock (this->_lock);
		return STAGE_CommsErrorMessage(this);
	}
	
	if (RS232CorvusSend(this, "st\n")) {
		GciCmtReleaseLock (this->_lock);
		return STAGE_ERROR;
	}
	
	while (STAGE_ReadString(this, buffer) < 0) {		
		ProcessSystemEvents();
		Delay(0.1);
	}	

	GciCmtReleaseLock (this->_lock);    
	
	return 0;
}		


int STAGE_ReadString(CorvusXYStage *stage, char *retval)
{
	int no_bytes_read;
	char read_data[RS232_ARRAY_SIZE]="";

	// Reads the Stage Port and returns a string
	// Returned character string is always terminated with CR, (ASCII 13)
	// will return 0 if read is successful, -1 otherwise
	GciCmtGetLock (stage->_lock);					 //for multi-threading

	memset(retval, 0, RS232_ARRAY_SIZE);  
	memset(read_data, 0, RS232_ARRAY_SIZE);
	
	no_bytes_read = ComRdTerm(stage->_com_port, read_data, RS232_ARRAY_SIZE - 1, 13);  
	
	if (no_bytes_read <= 0) {
		GciCmtReleaseLock (stage->_lock);
		return STAGE_CommsErrorMessage(stage);
	}
	
	strcpy(retval, read_data);
	
	GciCmtReleaseLock (stage->_lock);
	
	return 0;
}


static int STAGE_PrintParam(CorvusXYStage *stage, char* str)
{
	char retval[RS232_ARRAY_SIZE] = "";
	char buffer[RS232_ARRAY_SIZE] = "";
	
	GciCmtGetLock (stage->_lock);
	
	sprintf(buffer, "%s\n", str);
    RS232CorvusSend(stage, buffer);
	STAGE_ReadString(stage, retval);
    
	printf("%s: %s\n", str, retval);
	
	GciCmtReleaseLock (stage->_lock);
	
	return 0;
}

void STAGE_PrintCurrentSettings(CorvusXYStage *stage)
{
	STAGE_PrintParam(stage, "getdim");	
	STAGE_PrintParam(stage, "-1 getunit");
	STAGE_PrintParam(stage, "-1 getaxis");	
	STAGE_PrintParam(stage, "-1 getumotmin");	
	STAGE_PrintParam(stage, "-1 getumotgrad");	
	STAGE_PrintParam(stage, "getcalvel");	
	STAGE_PrintParam(stage, "getrmvel");	
	STAGE_PrintParam(stage, "getaccelfunc");	
	STAGE_PrintParam(stage, "1 getpitch");
	STAGE_PrintParam(stage, "2 getpitch");
	STAGE_PrintParam(stage, "1 getsw");	 
	STAGE_PrintParam(stage, "2 getsw");	
	STAGE_PrintParam(stage, "1 getclperiod");	 
	STAGE_PrintParam(stage, "2 getclperiod");	 
	STAGE_PrintParam(stage, "-1 getcloop");	 
	STAGE_PrintParam(stage, "-1 getclfactor");	 
	STAGE_PrintParam(stage, "getvel");	 
	STAGE_PrintParam(stage, "getaccel");
	STAGE_PrintParam(stage, "getmanaccel");
	STAGE_PrintParam(stage, "getlimit");
}

int STAGE_SendReadString(CorvusXYStage *stage, char* str, char* retval)
{
	GciCmtGetLock (stage->_lock);   
	
	if(RS232CorvusSend(stage, str) < 0) {
		GciCmtReleaseLock (stage->_lock);   
		return STAGE_ERROR;
	}

	if(STAGE_ReadString(stage, retval) < 0) {
		GciCmtReleaseLock (stage->_lock);   
		return STAGE_ERROR;
	}
    
	GciCmtReleaseLock (stage->_lock);   
	
	return STAGE_SUCCESS;
}


int STAGE_ReadInt(CorvusXYStage *stage, int *retval)
{
	char read_data[RS232_ARRAY_SIZE]="";
	
    if(STAGE_ReadString(stage, read_data) < 0)
		return STAGE_ERROR;

    *retval = atoi(read_data);
    
	return STAGE_SUCCESS;
}

int STAGE_SendReadInt(CorvusXYStage *stage, char* str, int *retval)
{
	GciCmtGetLock (stage->_lock);   
	
	if(RS232CorvusSend(stage, str) < 0) {
		GciCmtReleaseLock (stage->_lock);   
		return STAGE_ERROR;
	}

	if(STAGE_ReadInt(stage, retval) < 0) {
		GciCmtReleaseLock (stage->_lock);   
		return STAGE_ERROR;
	}

	GciCmtReleaseLock (stage->_lock);   
	
	return STAGE_SUCCESS;
}

int STAGE_ReadDouble(CorvusXYStage *stage, double *retval)
{	
	char read_data[RS232_ARRAY_SIZE]="";
	
    if(STAGE_ReadString(stage, read_data) < 0)
		return STAGE_ERROR;

    *retval = atof(read_data);
  
	return STAGE_SUCCESS;
}

int STAGE_SendReadDouble(CorvusXYStage *this, char* str, double *retval)
{
	GciCmtGetLock (this->_lock);   
	
	if(RS232CorvusSend(this, str) < 0) {
		GciCmtReleaseLock (this->_lock);   
		return STAGE_ERROR;
	}

	if(STAGE_ReadDouble(this, retval) < 0) {
		GciCmtReleaseLock (this->_lock);   
		return STAGE_ERROR;
	}
	
	GciCmtReleaseLock (this->_lock);   
	
	return STAGE_SUCCESS;
}

int Corvus_initRS232Port(CorvusXYStage *this)
{
	XYStage *stage = (XYStage *) this;

	//Initialise rs232 communications
	int attempts=0;
	double retval;
	
	GciCmtGetLock (this->_lock);   
	
	if(initialise_comport(this->_com_port, this->_baud_rate) < 0) {
		GciCmtReleaseLock (this->_lock);
		return -1;
	}

	// Turn of handshaking for the corvus serial port
	SetXMode (this->_com_port, 0);
	SetCTSMode (this->_com_port, LWRS_HWHANDSHAKE_OFF);

	// Short timeout so we can tell at once if it's switched off or something
	SetComTime (this->_com_port, STAGE_NORMAL_TIMEOUT);	

	RS232CorvusSend(this, "mode 0\n");	  //Set "host" mode. "terminal" mode is for use with Hyperterminal only.
	
	STAGE_SendReadDouble(this, "version\n", &retval);

	//Get the version number from the controller
	if (retval <= 0) {
		//RS232CorvusSend(this, "reset\n");
		GciCmtReleaseLock (this->_lock);
		return -1;
	}
		
	GciCmtReleaseLock (this->_lock); 
		
	return 0;
}
