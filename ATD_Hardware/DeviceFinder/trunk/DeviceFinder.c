#include <windows.h>		
#include "toolbox.h"
#include <ansi_c.h>
#include <cvirte.h>		
#include <userint.h>

#include "DeviceFinder.h"
#include "gci_utils.h"
#include <utility.h>
#include <formatio.h>

//#define IS_STANDALONE

#include "DeviceFinder_ui.h"

#define SZ_FTDIBUS "SYSTEM\\CurrentControlSet\\Enum\\FTDIBUS"
#define SZ_PORT_NAME "PortName"

#ifdef IS_STANDALONE
static int panelHandle;
static int comsArray[5];
static int descriptionsArray[5];
#endif

static int foundPort = 0;
static int foundKey = 0;
static int foundSerialNumber = 0;

static int numberOfDevices = 0;
static int portsInUse[MAX_NUMBER_OF_DEVICES];
static char portDescriptions[MAX_NUMBER_OF_DEVICES][MAX_PATH];


#ifdef IS_STANDALONE
int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	Init();
	return 0;
}


int Init()
{
	int port;
	
	if ((panelHandle = LoadPanel (0, "DeviceFinder_ui.uir", PANEL)) < 0)
		return -1;
	InitControls();
	DisplayPanel (panelHandle);
	if (selectPortForDevice("C:\\text.txt", &port) == 0)
		printf("%d\n", port);
	
	RunUserInterface ();
	DiscardPanel (panelHandle);
	return 0;
}

int InitControls()
{
	comsArray[0] = PANEL_COMPORT_1;
	comsArray[1] = PANEL_COMPORT_2;
	comsArray[2] = PANEL_COMPORT_3;
	comsArray[3] = PANEL_COMPORT_4;
	comsArray[4] = PANEL_COMPORT_5;
	
	descriptionsArray[0] = PANEL_DESCRIPTION_1;
	descriptionsArray[1] = PANEL_DESCRIPTION_2;
	descriptionsArray[2] = PANEL_DESCRIPTION_3;
	descriptionsArray[3] = PANEL_DESCRIPTION_4;
	descriptionsArray[4] = PANEL_DESCRIPTION_5;
	
	return 0;
}
#endif
int getDevices()
{
	char pszName[MAX_PATH] = "";
	DWORD dwNameLen = MAX_PATH;
	HKEY hStartKey = HKEY_LOCAL_MACHINE;
	HKEY hKey;
	DWORD dwSubKeys;
	DWORD dwResult;
	DWORD i;
	numberOfDevices = 0;
	foundKey = 0;
	foundPort = 0;
	foundSerialNumber = 0;
	
	//clear arrays
	for(i = 0; i < MAX_NUMBER_OF_DEVICES; i++)
	{
		portsInUse[i] = 0;
		portDescriptions[i][0] = '\0';
	}
	
	//
	// open FTDIBUS key
	//

	dwResult = RegOpenKeyEx(hStartKey, SZ_FTDIBUS, 0L, KEY_READ, &hKey);

	if (ERROR_SUCCESS == dwResult)
	{
		//
		// get number of subkeys under FTDIBUS
		//
		dwResult = RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

		//										
		// enumerate the subkeys under FTDIBUS
		//
		for (i = 0; i < dwSubKeys; i++)
		{
			dwResult = RegEnumKeyEx(hKey, i, pszName, &dwNameLen, NULL, NULL, NULL, NULL);
			if (ERROR_SUCCESS == dwResult)
			{
				//printf("FTDIBUS\\%s : ", pszName);
				EnumerateKey(hKey, pszName/*, 0, 0*/);
			}
			pszName[0] = '\0';
			dwNameLen = MAX_PATH;
		}

		//
		// close FTDIBUS key
		//

		RegCloseKey(hKey);
	}
	else
	{
		//printf("Can't open %s\\%s (%d)\n", hStartKey, SZ_FTDIBUS, dwResult);
	}
	makeDescriptions();	
#ifdef IS_STANDALONE
	displayDevices();
#endif
	return 0;	
}

int getPortForDevice(char *device, int *comport)
{
	char pszName[MAX_PATH] = "";
	DWORD dwNameLen = MAX_PATH;
	HKEY hStartKey = HKEY_LOCAL_MACHINE;
	HKEY hKey;
	DWORD dwSubKeys;
	DWORD dwResult;
	DWORD i;
	char msg[200], str[10];
	int panel, pnl, ctrl, port;
	
	numberOfDevices = 0;
	*comport = 0;
	
	// open FTDIBUS key
	dwResult = RegOpenKeyEx(hStartKey, SZ_FTDIBUS, 0L, KEY_READ, &hKey);

	if (ERROR_SUCCESS == dwResult) {
		// get number of subkeys under FTDIBUS
		dwResult = RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

		// enumerate the subkeys under FTDIBUS
		for (i = 0; i < dwSubKeys; i++) {
			dwResult = RegEnumKeyEx(hKey, i, pszName, &dwNameLen, NULL, NULL, NULL, NULL);
			if (ERROR_SUCCESS == dwResult) {
				if (FindPattern(pszName, 0, -1, device, 0, 0) >= 0) {
					//printf("FTDIBUS\\%s : ", pszName);
					EnumerateKey(hKey, pszName/*, 0, 0*/);
				}
			}
			pszName[0] = '\0';
			dwNameLen = MAX_PATH;
		}

		// close FTDIBUS key
		RegCloseKey(hKey);
	}
	else
	{
		//printf("Can't open %s\\%s (%d)\n", hStartKey, SZ_FTDIBUS, dwResult);
	}
	
	if (numberOfDevices < 1) {
		sprintf(msg, "Device %s not found\n", device);
		MessagePopup("Error", msg);
		return -1;
	}
	
	if (numberOfDevices == 1) {
		*comport = *(getPorts());
		//printf("Device %s on port %d\n", device, *comport);
		return 0;
	}
	
	if ((panel = LoadPanel (0, "DeviceFinder_ui.uir", PANEL_2)) < 0)
		return -1;
		
	//More than one
	ClearListCtrl (panel, PANEL_2_COM_PORTS);
	for (i=0; i<numberOfDevices; i++) {
		port = *(getPorts()+i);
		sprintf(str, "COM%d", port);
		InsertListItem (panel, PANEL_2_COM_PORTS, i, str, port);
	}
	DisplayPanel(panel);
	
	while(1) {
		GetUserEvent (1, &pnl, &ctrl);
		if ((pnl == panel) && (ctrl == PANEL_2_CANCEL)) {
			DiscardPanel(panel);
			return -1;
		}
		if ((pnl == panel) && (ctrl == PANEL_2_OK)) {
			GetCtrlVal(panel, PANEL_2_COM_PORTS, comport);
			DiscardPanel(panel);
			break;
		}
	}
	return 0;
}

int selectPortForDevice(char *filepath, int *port, char *title)
{
	char id[20], port_str[10];
 	char *descArray[MAX_NUMBER_OF_DEVICES];
	char uir_path[500];  
	int id_panel, pnl, ctrl, i;
	FILE *fp;
	
	//If we are using an FTDI gizmo Device Finder should give us the port number
	if ((filepath != NULL) && (FileExists(filepath, NULL))) {
		fp = fopen (filepath, "r");
		if (fp != NULL) {
			if (fscanf(fp, "%s", id) > 0) {
				fclose(fp);
				if (strcmp(id, "NOT_FTDI") == 0) return -1;
				return getPortForDevice(id, port);
			}
		}
	}

	//Ask for the device ID
	getDevices();
	getNumberOfDevices();
	if (numberOfDevices < 1)
		return -1;
		
	find_resource("DeviceFinder_ui.uir", uir_path); 
	id_panel = LoadPanel(0, uir_path, FTDI_DEV);
	if (title != NULL)
		SetPanelAttribute (id_panel, ATTR_TITLE, title);
		
	ClearListCtrl (id_panel, FTDI_DEV_ID);
	ClearListCtrl (id_panel, FTDI_DEV_PORT);
	getDescriptions(descArray);

	for(i = 0; i < numberOfDevices; i++) {
		InsertListItem (id_panel, FTDI_DEV_ID, -1, "empty", "");
		ReplaceListItem (id_panel, FTDI_DEV_ID, i, *(descArray+i), *(descArray+i));
	
		InsertListItem (id_panel, FTDI_DEV_PORT, -1, "empty", -1);
		sprintf(port_str, "COM%d", *(getPorts()+i));
		ReplaceListItem (id_panel, FTDI_DEV_PORT, i, port_str, *(getPorts()+i));
	}
	InsertListItem (id_panel, FTDI_DEV_ID, -1, "empty", "");
	ReplaceListItem (id_panel, FTDI_DEV_ID, i, "NOT_FTDI", "NOT_FTDI");
	
	InsertListItem (id_panel, FTDI_DEV_PORT, -1, "empty", -1);
	ReplaceListItem (id_panel, FTDI_DEV_PORT, i, "", 0);
	
	InstallPopup (id_panel);
	while (1) {
		GetUserEvent (1, &pnl, &ctrl);
		if ((pnl == id_panel) && (ctrl == FTDI_DEV_OK))
			break;
	}
	GetCtrlVal(id_panel, FTDI_DEV_ID, id);
	DiscardPanel(id_panel);

	if (stricmp(id, "NOT_FTDI") != 0) {
		if (getPortForDevice(id, port) != 0) {
			MessagePopup("Error", "Cannot find virtual com port. You will now be asked to enter the port for the stage controller.");
			return -1;
		}
	}
	
	//Save the device ID even if it is "NOT_FTDI" as we don't want to ask every time.
	if ((filepath != NULL)) {
		if (FileExists(filepath, 0)) SetFileAttrs (filepath, 0, -1, -1, -1);   //clear read only atribute
		fp = fopen (filepath, "w");
		if (fp != NULL) {
			fprintf(fp, "%s", id);
			fclose(fp);
		}
		SetFileAttrs (filepath, 1, -1, -1, -1);   //set read only atribute
	}
	
	if (stricmp(id, "NOT_FTDI") == 0) 
		return -1;
	return 0;
}

#ifdef IS_STANDALONE

int displayDevices()
{
 	int i = 0;
 	char *descArray[MAX_NUMBER_OF_DEVICES];
 	SetCtrlAttribute (panelHandle, PANEL_NUMDEVICES, ATTR_CTRL_VAL, getNumberOfDevices());
	getDescriptions(descArray);
		
	//clear boxes
	for(i = 0; i < 5; i++)
	{
		SetCtrlAttribute (panelHandle, comsArray[i], ATTR_CTRL_VAL, 0);
		SetCtrlAttribute (panelHandle, descriptionsArray[i], ATTR_CTRL_VAL, "");
	}
	
	for(i = 0; i < numberOfDevices; i++)
	{
		SetCtrlAttribute (panelHandle, comsArray[i], ATTR_CTRL_VAL, *(getPorts()+i));
		SetCtrlAttribute (panelHandle, descriptionsArray[i], ATTR_CTRL_VAL, *(descArray+i));
	}
	
	if(i == 0)
	{
		MessagePopup ("Connection Problem","No FTDI devices connected.");
	}
	
	return 0;
}
#endif

void EnumerateKey(HKEY hParentKey, char *pszKeyName)
{
	char pszName[MAX_PATH] = "";
	DWORD dwNameLen = MAX_PATH;
	DWORD dwResult;
	DWORD dwKeyCount;
	DWORD dwValueCount;
	DWORD i, gotIt;
	HKEY hKey;
	char Buf[256];
	static char port[10] = "COMxx";
	DWORD BufLen;
	DWORD BufType;
	
	static char serialNumber[MAX_PATH];
	
	//int foundKey = 0;
	//int foundPort = 0;

	dwResult = RegOpenKeyEx(hParentKey, pszKeyName, 0L, KEY_READ, &hKey);
	if (ERROR_SUCCESS != dwResult)
	{
		//printf("Can't open %s\\%s (%d)\n", hParentKey, pszKeyName, dwResult);
		return;
	}
	
	//if the serial number has been found - starts with VID_0403+PID_6001+
	if(pszKeyName[0] == 'V' && pszKeyName[1] == 'I' && pszKeyName[2] == 'D' && pszKeyName[3] == '_')
	{
		int length = strlen(pszKeyName);
		foundSerialNumber = 1;
		for(i = 0; i < length; i++)
		{
			serialNumber[i] = pszKeyName[i];
		}
		serialNumber[i] = '\0';
	}
	
	//
	// get number of subkeys and values for this key
	//
	dwResult = RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwKeyCount, NULL, NULL, &dwValueCount, NULL, NULL, NULL, NULL);
	
	if (ERROR_SUCCESS != dwResult)
		return;
	
	//
	// enumerate values
	//
	for (i = 0, gotIt = 0; i < dwValueCount && gotIt == 0; i++)
	{
		BufLen = 256;
		dwResult = RegEnumValue(hKey, i, pszName, &dwNameLen, NULL, &BufType, Buf, &BufLen);
		if (ERROR_SUCCESS == dwResult)
		{
			//
			// test for PortName value
			//
			if (stricmp(pszName, SZ_PORT_NAME) == 0)
			{
				foundPort = 1;
				for(i = 0; i < BufLen; i++)
				{
					port[i] = Buf[i];
				}
				port[i] = '\0';
				gotIt = 1;
			}
		}

		pszName[0] = '\0';
		dwNameLen = MAX_PATH;
	}

	if((foundKey == 1) && (foundPort == 1) && (foundSerialNumber == 1)) 
	{
		char length = strlen(port);
		char portString[3];
		int portNumber;
		int counter = 0;
		
		for(i = 0; i < length; i++)
		{
			if(port[i] >= '0' && port[i] <= '9')
			{
				break;
			}
		}
		
		while(port[i] >= '0' && port[i] <= '9')
		{
			portString[counter] = port[i];
			i++;
			counter++;
		}
		portString[counter] = '\0';
		portNumber = atoi(portString);
		
		//check still attached
		if(stillAttached(port) == 1)
		{
			portsInUse[numberOfDevices] = portNumber;
			
			length = strlen(serialNumber);
			//printf("serialNumber: %s\n", serialNumber);
			for(i = 0; i < length; i++)
			{
				portDescriptions[numberOfDevices][i] = serialNumber[i];
			}
			portDescriptions[numberOfDevices][i] = '\0';
			numberOfDevices++;
			foundPort = 0;
			foundKey = 0;
			foundSerialNumber = 0;
			
			RegCloseKey(hKey);
			return;
		}
	}

	//
	// enumerate keys
	//
	for (i = 0; i < dwKeyCount && gotIt == 0; i++)
	{
		dwResult = RegEnumKeyEx(hKey, i, pszName, &dwNameLen, NULL, NULL, NULL, NULL);
		if (ERROR_SUCCESS == dwResult)
		{
			//printf("pszName = %s\n", pszName);
			if(stricmp(pszName, "Control") == 0)
			{
				foundKey = 1;
			}
			EnumerateKey(hKey, pszName/*, foundKey, foundPort*/);
		}

		pszName[0] = '\0';
		dwNameLen = MAX_PATH;
	}
	
	RegCloseKey(hKey);
}

int stillAttached(char *port)
{
	char pszName[MAX_PATH] = "";
	DWORD dwNameLen = MAX_PATH;
	HKEY hStartKey = HKEY_LOCAL_MACHINE;
	HKEY hKey;
	DWORD dwResult;
	DWORD i;
	DWORD dwValueCount;
	DWORD BufLen;
	DWORD BufType;
	char Buf[256];
	
	char* location = "HARDWARE\\DEVICEMAP\\SERIALCOMM";
	
	dwResult = RegOpenKeyEx(hStartKey, location, 0L, KEY_READ, &hKey);

	if (ERROR_SUCCESS != dwResult)
	{
		return -1;
	}
	dwResult = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &dwValueCount, NULL, NULL, NULL, NULL);
	
	if (ERROR_SUCCESS != dwResult)
		return -1;
	
	//
	// enumerate values and search for port
	//

	for (i = 0; i < dwValueCount; i++)
	{
		BufLen = 256;
		dwResult = RegEnumValue(hKey, i, pszName, &dwNameLen, NULL, &BufType, Buf, &BufLen);
		if (ERROR_SUCCESS == dwResult)
		{
			if (stricmp(Buf, port) == 0)
			{
				RegCloseKey(hKey);
				return 1;
			}
			//printf("%s\n", pszName);
		}

		pszName[0] = '\0';
		dwNameLen = MAX_PATH;
	}

	RegCloseKey(hKey);
	return 0;
}

int getNumberOfDevices()
{
	return numberOfDevices;
}

int* getPorts()
{
	return portsInUse;
}

void makeDescriptions()
{
	int i, j, jIndex;
	for(i = 0; i < MAX_NUMBER_OF_DEVICES; i++)
	{
		//char[18] to penultimate is description
		j = 18;//first character of description
		while(portDescriptions[i][j])
		{
			portDescriptions[i][j-18] = portDescriptions[i][j];
			j++;
		}
		if(j > 18)
		{
			jIndex = j-18-1;
		}
		else
		{
			jIndex = j-18;
		}
		portDescriptions[i][jIndex] = '\0';
		
	}
}

void getDescriptions(char *array[MAX_NUMBER_OF_DEVICES])
{
	int i;
	for(i = 0; i < MAX_NUMBER_OF_DEVICES; i++)
	{
		array[i] = portDescriptions[i];
	}
}

#ifdef IS_STANDALONE
int CVICALLBACK cbGETDEVICES (int panel, int control, int event, 
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			getDevices();
			break;
		}
	return 0;
}

int CVICALLBACK cbQUIT (int panel, int control, int event, 
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			QuitUserInterface (0);
			break;
		}
	return 0;
}

int CVICALLBACK cbGetPort (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	char device[200];
	int comport;
	
	switch (event)
		{
		case EVENT_COMMIT:
			SetCtrlVal(panelHandle, PANEL_COMPORT_6, -1);
			GetCtrlVal(panelHandle, PANEL_DEVICE, device);
			if (getPortForDevice(device, &comport) >= 0)
				SetCtrlVal(panelHandle, PANEL_COMPORT_6, comport);
			
			break;
		}
	return 0;
}
#endif

int CVICALLBACK cbDeviceID (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int index;
	
	switch (event)
		{
		case EVENT_COMMIT:
			GetCtrlAttribute (panel, FTDI_DEV_ID, ATTR_CTRL_INDEX, &index);
			SetCtrlAttribute (panel, FTDI_DEV_PORT, ATTR_CTRL_INDEX, index);

			break;
		}
	return 0;
}
