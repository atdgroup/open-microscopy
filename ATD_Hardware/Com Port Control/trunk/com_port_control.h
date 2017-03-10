/*
  Functions to set com ports being used for a project.
  The ports are saved after initially asking the user 
  which port a device appears on.
  
  
  Written by Glenn Pierce 11/10/04
*/

#ifndef __COM_PORT_CONTROL__
#define __COM_PORT_CONTROL__

#ifdef WIN32
/*** Windows Specifics ***/

#ifdef BUILD_COMPORT_DLL

#define COMPORT_EXPORT __declspec(dllexport)

#else

#ifdef USE_COMPORT_DLL
#define COMPORT_EXPORT __declspec(dllimport)
#else
#define COMPORT_EXPORT
#endif

#endif

#endif

#define COMM_CONTROL_ERROR -1
#define COMM_CONTROL_SUCCESS 0

#define MAX_COM_PORTS 5
#define MAX_DEVICES 10
					
COMPORT_EXPORT void  GCI_ComPortControlInit(void);

COMPORT_EXPORT int   GCI_ComPortControlAddDevice(char *device);

COMPORT_EXPORT int	 GCI_ComPortControlSetDataDir(char *dir);

COMPORT_EXPORT int   GCI_ComPortControlLoadConfig(void);

COMPORT_EXPORT int   GCI_ComPortControlGetDeviceProperties(
						const char *device, int *port, char *port_string, int *parity, long *baudRate,
						int *dataBits, int *stopBits, int *inputQueueSize, int *outputQueueSize);

COMPORT_EXPORT int   GCI_ComPortControlDisplayConfigurePanel(void);

#endif

