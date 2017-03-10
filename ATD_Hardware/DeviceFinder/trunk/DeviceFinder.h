#include <windows.h>		
#include "toolbox.h"

#define MAX_NUMBER_OF_DEVICES 15

int getDevices(void);
void EnumerateKey(HKEY hKey, char *pszKeyName);
int getNumberOfDevices(void);
int* getPorts(void);
void getDescriptions(char *array[MAX_NUMBER_OF_DEVICES]);
void makeDescriptions(void);
int stillAttached(char *port);
int getPortForDevice(char *device, int *comport);
int selectPortForDevice(char *filepath, int *port, char *title);

//#ifdef IS_STANDALONE
int InitControls(void);
int Init(void);
int displayDevices(void);
//#endif
