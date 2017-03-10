#include "GL_CVIRegistry.h"

#include <formatio.h>
#include "toolbox.h"
#include <userint.h>
#include <float.h>

// Easy Registry commands for CVI, Paul Barber, Feb 2002, Aug 2002

// Make a procedure like this in the main project:

//int readOrWriteRegistry (int write)
//{
//
//	checkRegistryValueForString           (write, REGKEY_HKCU, "software\\GCI\\Trace3d",        "imagePath",   imagePath);
//	checkRegistryValueForString           (write, REGKEY_HKCU, "software\\GCI\\Trace3d",        "tracePath",   tracePath);
//
//	// main panel
//	checkRegistryValueForPanelAttribInt   (write, REGKEY_HKCU, "software\\GCI\\Trace3d\\main",  "top",   panelHandle, ATTR_TOP);
//	checkRegistryValueForPanelAttribInt   (write, REGKEY_HKCU, "software\\GCI\\Trace3d\\main",  "left",   panelHandle, ATTR_LEFT);
//	checkRegistryValueForPanelAttribInt   (write, REGKEY_HKCU, "software\\GCI\\Trace3d\\main",  "width",   panelHandle, ATTR_WIDTH);
//	checkRegistryValueForPanelAttribInt   (write, REGKEY_HKCU, "software\\GCI\\Trace3d\\main",  "height",   panelHandle, ATTR_HEIGHT);
//	checkRegistryValueForCtrlAttribDouble (write, REGKEY_HKCU, "software\\GCI\\Trace3d\\main",  "zoom",     panelHandle, PANEL_ZOOMPERC, ATTR_CTRL_VAL);
//	checkRegistryValueForCtrlAttribInt    (write, REGKEY_HKCU, "software\\GCI\\Trace3d\\main",  "autoZ",     panelHandle, PANEL_AUTOZ, ATTR_CTRL_VAL);
//
//	return(0);	
//}

//	call readOrWriteRegistry(FALSE) at program start
//	call readOrWriteRegistry(TRUE)  on exit

#ifndef VS_COMPILER

// Standard C NAN definition for very old compilers.
#define NAN (0./0.)
#define INF (1./0.)
#define NEG_INF (-1./0.)

#else

/* These are classes, what use are these?
#define _FPCLASS_SNAN   0x0001  //* signaling NaN 
#define _FPCLASS_PINF   0x0200  //* positive infinity 
#define _FPCLASS_NINF   0x0004  //* negative infinity 

#define NAN _FPCLASS_SNAN
#define INF _FPCLASS_PINF
#define NEG_INF _FPCLASS_NINF
*/

#define NAN sqrt(-1.0)
#define INF DBL_MAX
#define NEG_INF -DBL_MAX

#endif


int RegWriteLong (unsigned int rootKey, const char subkeyName[], const char valueName[], long dataValue)
{
	char buffer[100];
	
	sprintf(buffer, "%d", dataValue);  
	
	return RegWriteString (rootKey, subkeyName, valueName, buffer);
}


int RegReadLong (unsigned int rootKey, const char subkeyName[], const char valueName[], long *uLongData)
{
	char buffer[100];  	
	int realStringSize = 0;
	
	int err = RegReadString (rootKey, subkeyName, valueName, buffer, 100, &realStringSize);
	
	// make sure the string is terminated correctly
	buffer[realStringSize] = 0;
	
	sscanf(buffer, "%d", uLongData);

	return err;
}

int RegistrySavePanelSizePosition (unsigned int userRootKey, const char* userSubKeyName, int panel)
{
	long left = 0, top = 0, width = 0, height = 0, error = 0;

	GetPanelAttribute (panel, ATTR_LEFT, &left);
	GetPanelAttribute (panel, ATTR_TOP, &top);
	GetPanelAttribute (panel, ATTR_WIDTH, &width);
	GetPanelAttribute (panel, ATTR_HEIGHT, &height);

	// If we have silly values don't save
	if(width < 1 || height < 1)
		return -1;
	
	if((error = RegWriteLong (userRootKey, userSubKeyName, "left", left)) < 0)
		return error;

	if((error = RegWriteLong (userRootKey, userSubKeyName, "top", top)) < 0)       
		return error;

	if((error = RegWriteLong (userRootKey, userSubKeyName, "width", width)) < 0)       
		return error;

	if((error = RegWriteLong (userRootKey, userSubKeyName, "height", height)) < 0)       
		return error;
	
	return 0;
}


int RegistryReadPanelSizePosition (unsigned int userRootKey, const char* userSubKeyName, int panel)
{
	long left = 0, top = 0, width = 0, height = 0;
	int error = 0;

	if((error = RegReadLong (userRootKey, userSubKeyName, "left", &left)) < 0)
		return error;

	if((error = RegReadLong (userRootKey, userSubKeyName, "top", &top)) < 0)       
		return error;

	if((error = RegReadLong (userRootKey, userSubKeyName, "width", &width)) < 0)       
		return error;

	if((error = RegReadLong (userRootKey, userSubKeyName, "height", &height)) < 0)       
		return error;
	
	// If we have silly value set to something sensible
	if(width < 1)
		width = 500;
	
	if(height < 1)
		height = 400;

	SetPanelAttribute (panel, ATTR_LEFT, left);
	SetPanelAttribute (panel, ATTR_TOP, top);
	SetPanelAttribute (panel, ATTR_WIDTH, width);
	SetPanelAttribute (panel, ATTR_HEIGHT, height);
	
	return 0;
}


int RegistrySavePanelPosition (unsigned int userRootKey, const char* userSubKeyName, int panel)
{
	long left = 0, top = 0;
	int error = 0;

	GetPanelAttribute (panel, ATTR_LEFT, &left);
	GetPanelAttribute (panel, ATTR_TOP, &top);

	if((error = RegWriteLong (userRootKey, userSubKeyName, "left", left)) < 0)
		return error;

	if((error = RegWriteLong (userRootKey, userSubKeyName, "top", top)) < 0)       
		return error;
	
	return 0;
}

int RegistryReadPanelPosition (unsigned int userRootKey, const char* userSubKeyName, int panel)
{
	long left = 0, top = 0;
	int error = 0;

	if((error = RegReadLong (userRootKey, userSubKeyName, "left", &left)) < 0)
		return error;

	if((error = RegReadLong (userRootKey, userSubKeyName, "top", &top)) < 0)       
		return error;
	
	SetPanelAttribute (panel, ATTR_LEFT, left);
	SetPanelAttribute (panel, ATTR_TOP, top);
	
	return 0;
}


int RegistrySavePanelVisibility (unsigned int userRootKey, const char* userSubKeyName, int panel)
{
	int vis = 0, error = 0;

	GetPanelAttribute (panel, ATTR_VISIBLE, &vis);

	if((error = RegWriteLong (userRootKey, userSubKeyName, "visible", vis)) < 0)
		return error;
	
	return 0;
}


//********************************* checkRegistryValueForPanelAttribInt **********************************************************************

int checkRegistryValueForPanelAttribInt (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName, int panel, int attribute)
{
	long Lval;
	int Ival, visible, error=0;

	if (!write) 
	{
		error = RegReadLong (userRootKey, userSubKeyName, userValName, &Lval);
		if (!error)
			SetPanelAttribute (panel, attribute, (int)Lval);
		else 
			write = TRUE;
	}

	if(error < 0) {  

		if((error == ToolErr_MissingKeyValue) || (error == ToolErr_MissingKey))
		{
			// Key not found lets create it
			GetPanelAttribute (panel, attribute, &Ival);
			return RegWriteLong (userRootKey, userSubKeyName, userValName, Ival);
		}
		else {
			// not sure what to do in this case, the code below can cause memory problems from str
//			char *str = GetGeneralErrorString (error);
//			printf("Error: %s\n", str);
//			free(str);
		}
	}

	if (write)
	{
		GetPanelAttribute (panel, ATTR_VISIBLE, &visible);
		
		if(!visible)
			return 0;

		GetPanelAttribute (panel, attribute, &Ival);
		error = RegWriteLong (userRootKey, userSubKeyName, userValName, Ival);
	}
	
	return error;
}

//********************************* checkRegistryValueForMenuAttribInt **********************************************************************

int checkRegistryValueForMenuAttribInt (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName, int panel, int menuItem, int attribute)
{
	long ULval;
	int Ival, error=0, menuBar;
 
	menuBar = GetPanelMenuBar (panel);

	if (!write) 
	{
		error = RegReadLong (userRootKey, userSubKeyName, userValName, &ULval);
		if (!error)
			SetMenuBarAttribute (menuBar, menuItem, attribute, (int)ULval);
		else write = TRUE;
	}

	if (write)
	{
		GetMenuBarAttribute (menuBar, menuItem, attribute, &Ival);
		error = RegWriteLong (userRootKey, userSubKeyName, userValName, (long)Ival);
	}
	return(error);
}

//********************************* checkRegistryValueForCtrlAttribInt **********************************************************************

int checkRegistryValueForCtrlAttribInt (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName, int panel, int ctrl, int attribute)
{
	long ULval;
	int Ival, error=0;
 
	if (!write) 
	{
		error = RegReadLong (userRootKey, userSubKeyName, userValName, &ULval);
		if (!error)
			SetCtrlAttribute (panel, ctrl, attribute, (int)ULval);
		else write = TRUE;
	}

	if (write)
	{
		GetCtrlAttribute (panel, ctrl, attribute, &Ival);
		error = RegWriteLong (userRootKey, userSubKeyName, userValName, (long)Ival);
	}
	return(error);
}

//********************************* checkRegistryValueForCtrlAttribDouble **********************************************************************

int checkRegistryValueForCtrlAttribDouble (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName, int panel, int ctrl, int attribute)
{
	unsigned char buffer[256];
	int error=0;
	unsigned int realSize;
	double fval;
	double dval;
 
	if (!write) 
	{
		error = RegReadString (userRootKey, userSubKeyName, userValName, buffer, 256, &realSize);
		if (!error)
		{
			// make sure the string is terminated correctly
			buffer[realSize] = 0;

			if (strcmp(buffer, "+Inf")==0) fval=INF;
			else if (strcmp(buffer, "1.#INF")==0) fval=INF;
			else if (strcmp(buffer, "-Inf")==0) fval=NEG_INF;
			else if (strcmp(buffer, "-1.#INF")==0) fval=NEG_INF;
			else if (strcmp(buffer, "NaN")==0) fval=NAN;
			else Scan((char *)buffer, "%s>%f", &fval);

			SetCtrlAttribute (panel, ctrl, attribute, fval);
		}
		else write = TRUE;
	}

	if (write)
	{
		GetCtrlAttribute (panel, ctrl, attribute, &dval);
		sprintf((char *)buffer, "%f", (float)dval);
		error = RegWriteString (userRootKey, userSubKeyName, userValName, buffer);
	}
	return(error);
}

//********************************* checkRegistryValueForCtrlAttribFloat **********************************************************************

int checkRegistryValueForCtrlAttribFloat (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName, int panel, int ctrl, int attribute)
{
	unsigned char buffer[256];
	int error=0;
	unsigned int realSize;
	float val;
	double fval;
 
	if (!write) 
	{
		error = RegReadString (userRootKey, userSubKeyName, userValName, buffer, 256, &realSize);
		if (!error)
		{
			// make sure the string is terminated correctly
			buffer[realSize] = 0;

			if (strcmp(buffer, "+Inf")==0) fval=INF;
			else if (strcmp(buffer, "1.#INF")==0) fval=INF;
			else if (strcmp(buffer, "-Inf")==0) fval=NEG_INF;
			else if (strcmp(buffer, "-1.#INF")==0) fval=NEG_INF;
			else if (strcmp(buffer, "NaN")==0) fval=NAN;
			else Scan((char *)buffer, "%s>%f", &fval);
			val = (float)fval;

			SetCtrlAttribute (panel, ctrl, attribute, val);
		}
		else write = TRUE;
	}

	if (write)
	{
		GetCtrlAttribute (panel, ctrl, attribute, &val);
		sprintf((char *)buffer, "%g", val);
		error = RegWriteString (userRootKey, userSubKeyName, userValName, buffer);
	}
	return(error);
}

//********************************* checkRegistryValueForCtrlAttribString **********************************************************************

int checkRegistryValueForCtrlAttribString (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName, int panel, int ctrl, int attribute)
{
	unsigned char buffer[256];
	int error=0;
	unsigned int realSize;
 
	if (!write) 
	{
		error = RegReadString (userRootKey, userSubKeyName, userValName, buffer, 256, &realSize);
		if (!error)
		{
			// make sure the string is terminated correctly
			buffer[realSize] = 0;
			SetCtrlAttribute (panel, ctrl, attribute, buffer);
		}
		else write = TRUE;
	}

	if (write)
	{
		GetCtrlAttribute (panel, ctrl, attribute, buffer);
		error = RegWriteString (userRootKey, userSubKeyName, userValName, buffer);
	}
	return(error);
}

//********************************* checkRegistryValueForInt **********************************************************************

int checkRegistryValueForInt   (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName,   int *value)
{
	long ULval;
	int error=0;
 
	if (!write) 
	{
		error = RegReadLong (userRootKey, userSubKeyName, userValName, &ULval);
		if (!error)
			*value = (int)ULval;
		else write = TRUE;
	}

	if (write)
	{
		error = RegWriteLong (userRootKey, userSubKeyName, userValName, *value);
	}
	return(error);
}

//********************************* checkRegistryValueForDouble **********************************************************************
//#define NaN   sqrt(-1)
//#define Inf  (1.0/0.0)

int checkRegistryValueForDouble   (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName,   double *value)
{
	unsigned char buffer[256];
	int error=0;
	unsigned int realSize;
	double fval;
 
	if (!write) 
	{
		error = RegReadString (userRootKey, userSubKeyName, userValName, buffer, 256, &realSize);
		if (!error)
		{
			// make sure the string is terminated correctly
			buffer[realSize] = 0;
			if (strcmp(buffer, "+Inf")==0) fval=INF;
			else if (strcmp(buffer, "1.#INF")==0) fval=INF;
			else if (strcmp(buffer, "-Inf")==0) fval=NEG_INF;
			else if (strcmp(buffer, "-1.#INF")==0) fval=NEG_INF;
			else if (strcmp(buffer, "NaN")==0) fval=NAN;
			else Scan((char *)buffer, "%s>%f", &fval);
			
			*value = fval;
		}
		else write = TRUE;
	}

	if (write)
	{
		sprintf((char *)buffer, "%g", (float)*value);
		error = RegWriteString (userRootKey, userSubKeyName, userValName, buffer);
	}
	return(error);
}

//********************************* checkRegistryValueForFloat **********************************************************************

int checkRegistryValueForFloat   (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName,   float *value)
{
	unsigned char buffer[256];
	int error=0;
	unsigned int realSize;
	double fval;
 
	buffer[0]=0;
	
	if (!write) 
	{
		error = RegReadString (userRootKey, userSubKeyName, userValName, buffer, 256, &realSize);
		if (!error)
		{
			// make sure the string is terminated correctly
			buffer[realSize] = 0;
			if (strcmp(buffer, "+Inf")==0) fval=INF;
			else if (strcmp(buffer, "1.#INF")==0) fval=INF;
			else if (strcmp(buffer, "-Inf")==0) fval=NEG_INF;
			else if (strcmp(buffer, "-1.#INF")==0) fval=NEG_INF;
			else if (strcmp(buffer, "NaN")==0) fval=NAN;
			else Scan((char *)buffer, "%s>%f", &fval);
			*value = (float)fval;
		}
		else write = TRUE;
	}

	if (write)
	{
		sprintf((char *)buffer, "%g", *value);
		error = RegWriteString (userRootKey, userSubKeyName, userValName, buffer);
	}
	return(error);
}
#undef NaN  
#undef Inf  

//********************************* checkRegistryValueForString **********************************************************************

int checkRegistryValueForString           (int write, unsigned int userRootKey, const char* userSubKeyName, const char* userValName,   char *string)
{
	unsigned char buffer[256];
	int error=0;
	unsigned int realSize;
 
	if (!write) 
	{
		error = RegReadString (userRootKey, userSubKeyName, userValName, buffer, 256, &realSize);
		if (!error && realSize>0)
		{
			// make sure the string is terminated correctly
			buffer[realSize] = 0;
			sprintf(string, "%s", (char *)buffer);
		}
		else write = TRUE;
	}

	if (write)
	{
		sprintf((char *)buffer, "%s", string);
		error = RegWriteString (userRootKey, userSubKeyName, userValName, buffer);
	}
	return(error);
}

