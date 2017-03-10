#include <formatio.h>
#include <utility.h>
#include <cvirte.h>
#include <userint.h>
#include <ansi_c.h>

#include "toolbox.h"

#include "comport_hash.h"
#include "string_utils.h"
#include "com_port_control.h"
#include "comport_ui.h"

#include "gci_utils.h"

static char *data_dir = NULL;
static comport_hash_t device_data;
static int comport_panel = 0, configured_devices = 0, user_has_configured = 0;

int numVisibleItems;
int listboxWidth, firstListboxItemTop;
int leftListboxEdge, rightListboxEdge;
int vertDivision1, vertDivision2;
int selectionIndex = 0;
int textHeight, textWidth;
int numItems = 0;


int GCI_ComPortControlSetDataDir(char *dir)
{
	if(dir != NULL) {
		data_dir = (char *) malloc (strlen(dir) + 1);
		
		strcpy(data_dir, dir);
		
		return COMM_CONTROL_SUCCESS;
	}
	
	return COMM_CONTROL_ERROR;
}


static int GCI_ComPortControlGetFilePath(char *path)
{
	char filename[GCI_MAX_PATHNAME_LEN];
	
	memset(filename, 0, GCI_MAX_PATHNAME_LEN );
	
	/* Get the path of the configuration file. */
	strcat(filename, data_dir);   
	strcat(filename, "comm_port_data.txt");

	strcpy(path, filename);

	return COMM_CONTROL_SUCCESS;
}


static int ComPortControlReadData(FILE* fp, char *key, comport_info *data)
{
	return fscanf( fp, "%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
				   key, data->deviceName, &data->port, &data->baudRate, &data->parity, &data->dataBits,
				   &data->stopBits, &data->inputQueueSize, &data->outputQueueSize );
}


static int ComPortControlReadConfigureFile(void)
{
	comport_info *data;
	char dataPath[GCI_MAX_PATHNAME_LEN];
	char device_key[MAX_DEVICE_NAME_LEN];
	int fsize;
	FILE *fp;

	configured_devices = 0;

	data = comport_hash_data_new();
	
	GCI_ComPortControlGetFilePath(dataPath);
	
	// Recall comms ports data
    	if (FileExists (dataPath, &fsize)) {
    
        fp = fopen (dataPath, "r");
        if (fp != NULL) {
        
        	while (ComPortControlReadData(fp, device_key, data) != EOF) {
            
            	/* Dont add devices not specified by the client */
            	if((comport_hash_lookup(&device_data, device_key)) != NULL) {
            		// Update the comport_hash table
            		data->configured = 1;
            		configured_devices++;
            		comport_hash_insert (&device_data, device_key, data);
            	}
            }
            
            fclose(fp);
            
            
            //GCI_MessagePopup("Test", "Configured devices %d, Inserted Devices %d", configured_devices, comport_hash_entry_count(&device_data) );
            
            if(configured_devices != comport_hash_entry_count(&device_data) ) {
            	return COMM_CONTROL_ERROR;
            }
            else {
            	return COMM_CONTROL_SUCCESS;
            } 
        }
    }
    
    comport_hash_data_destroy(data);
    
    return COMM_CONTROL_ERROR;
}

static void ComPortControlWriteData(const char *key, comport_info* data, void *extra)
{
	fprintf( (FILE*)extra, "%s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
		key, data->deviceName, data->port, data->baudRate, data->parity, data->dataBits,
		data->stopBits, data->inputQueueSize, data->outputQueueSize );
}


static int ComPortControlWriteConfigureFile(void)
{
	char dataPath[GCI_MAX_PATHNAME_LEN];
	int fsize;
	FILE *fp;
	
	GCI_ComPortControlGetFilePath(dataPath);

	/* If the conf file does exist clear any read only bit */
	if (FileExists (dataPath, &fsize))
	{
    	SetFileAttrs (dataPath, 0, -1, -1, -1);
	}

    fp = fopen (dataPath, "w");
    
    if (fp == NULL)
    	return COMM_CONTROL_ERROR;
    	
    comport_hash_foreach (&device_data, ComPortControlWriteData, fp);
    
    fclose(fp);
    
    /* set read-only */
    SetFileAttrs (dataPath, 1, -1, -1, -1);

	return COMM_CONTROL_SUCCESS;
}

static void ComPortUpdateDeviceList(const char *key, comport_info* data, void *extra)
{
	char colour_device[50];
	
	if (data->configured == 1)
		sprintf(colour_device, "%s", data->deviceName);
	else
		sprintf(colour_device, "\033fgff0000%s", data->deviceName);
	
	InsertListItem (comport_panel, COM_PANEL_DEVICE_LISTBOX, -1, colour_device, data->deviceName);
}

int GCI_ComPortControlAddDevice(char *device)
{
	/* default port settings */
	struct comport_info data;

	if (strlen(device) > MAX_DEVICE_NAME_LEN)
		return COMM_CONTROL_ERROR;
		
	if (str_contains_whitespace(device))
		return COMM_CONTROL_ERROR;

	data.port = 1;
	data.deviceName = device;
	data.baudRate = 9600;
	data.parity = 0;
	data.dataBits = 8;
	data.stopBits = 1;
	data.inputQueueSize = 164;
	data.outputQueueSize = 164;
	data.configured = 0;
		
	comport_hash_insert (&device_data, device, &data);
	
	return COMM_CONTROL_SUCCESS;
}


static void UpdateDeviceInfoInterface (int index)
{
	struct comport_info *data;
	char device_key[MAX_DEVICE_NAME_LEN];
	
	data = comport_hash_data_new();
	
	/* Get Device name from list index */
	GetValueFromIndex (comport_panel, COM_PANEL_DEVICE_LISTBOX, index, device_key);
	
	/* Update device label */
	SetCtrlAttribute (comport_panel, COM_PANEL_DEVICE_TEXT, ATTR_CTRL_VAL, device_key);
	
	/* Update device properties */
	if ( (data = comport_hash_lookup(&device_data, device_key)) != NULL ) {
        
        	SetCtrlAttribute (comport_panel, COM_PANEL_PORT_RING, ATTR_CTRL_VAL, data->port);
			SetCtrlAttribute (comport_panel, COM_PANEL_BAUD_RING, ATTR_CTRL_VAL, data->baudRate);
			SetCtrlAttribute (comport_panel, COM_PANEL_PARITY_RING, ATTR_CTRL_VAL, data->parity);
			SetCtrlAttribute (comport_panel, COM_PANEL_DATA_BITS_RING, ATTR_CTRL_VAL, data->dataBits);
			SetCtrlAttribute (comport_panel, COM_PANEL_STOP_BITS_RING, ATTR_CTRL_VAL, data->stopBits);
			SetCtrlAttribute (comport_panel, COM_PANEL_INPUT_QUEUE_NUMERIC, ATTR_CTRL_VAL, data->inputQueueSize);
			SetCtrlAttribute (comport_panel, COM_PANEL_OUTPUT_QUEUE_NUMERIC, ATTR_CTRL_VAL, data->outputQueueSize);
	}
	
	return;
}

int GCI_ComPortControlLoadConfig(void)
{
	/* Read saved comm info for the devices in the comport_hash table */
	if(ComPortControlReadConfigureFile() == COMM_CONTROL_ERROR) {
		/* We cant read the file so we must ask the user for the port info */
    	GCI_ComPortControlDisplayConfigurePanel();
    }
    
	return COMM_CONTROL_SUCCESS;
}


void GCI_ComPortControlInit(void)
{
	configured_devices = 0;

	comport_hash_init(&device_data, 16);
}


static int GCI_ComPortControlGetDevicePortName(int port, char *port_string)
{
	sprintf(port_string, "COM%d", port);
	
	return COMM_CONTROL_SUCCESS;
}


int GCI_ComPortControlGetDeviceProperties(const char *device, int *port, char *port_string, int *parity, long *baudRate,
										  int *dataBits, int *stopBits, int *inputQueueSize, int *outputQueueSize)
{
	comport_info* data;
	
	if ( (data = comport_hash_lookup (&device_data, device)) == NULL )
		return COMM_CONTROL_ERROR;
	
	*port = data->port;
	
	GCI_ComPortControlGetDevicePortName(*port, port_string);
	
	*parity = data->parity;
	*baudRate = data->baudRate;
	*dataBits = data->dataBits;
	*stopBits = data->stopBits;
	*inputQueueSize = data->inputQueueSize;
	*outputQueueSize = data->outputQueueSize;
	
	return COMM_CONTROL_SUCCESS;
}


/*
int GCI_ComPortControlGetDevicePort(const char *device)
{
	comport_info* data;
	
	if ( (data = comport_hash_lookup (&device_data, device)) == NULL )
		return COMM_CONTROL_ERROR;
		
	return data->port;
}

int GCI_ComPortControlGetDeviceParity(const char *device)
{
	comport_info* data;
	
	if ( (data = comport_hash_lookup (&device_data, device)) == NULL )
		return COMM_CONTROL_ERROR;
		
	return data->parity;
}

long  GCI_ComPortControlGetDeviceBaudRate(const char *device)
{
	comport_info* data;
	
	if ( (data = comport_hash_lookup (&device_data, device)) == NULL )
		return COMM_CONTROL_ERROR;
		
	return data->baudRate;
}

int  GCI_ComPortControlGetDeviceDataBits(const char *device)
{
	comport_info* data;
	
	if ( (data = comport_hash_lookup (&device_data, device)) == NULL )
		return COMM_CONTROL_ERROR;
		
	return data->dataBits;
}

int  GCI_ComPortControlGetDeviceStopBits(const char *device)
{
	comport_info* data;
	
	if ( (data = comport_hash_lookup (&device_data, device)) == NULL )
		return COMM_CONTROL_ERROR;
		
	return data->stopBits;
}

int  GCI_ComPortControlGetDeviceInputQueueSize(const char *device)
{
	comport_info* data;
	
	if ( (data = comport_hash_lookup (&device_data, device)) == NULL )
		return COMM_CONTROL_ERROR;
		
	return data->inputQueueSize;
}

int  GCI_ComPortControlGetDeviceOutputQueueSize(const char *device)
{
	comport_info* data;
	
	if ( (data = comport_hash_lookup (&device_data, device)) == NULL )
		return COMM_CONTROL_ERROR;
		
	return data->outputQueueSize;
}
*/


int GCI_ComPortControlDisplayConfigurePanel()
{
	int panel, control, event_result;

	user_has_configured = 0;

	if (!comport_panel) {
	
        comport_panel = FindAndLoadUIR(0, "comport_ui.uir", COM_PANEL); 
	}
	
	ClearListCtrl (comport_panel, COM_PANEL_DEVICE_LISTBOX);
	
	/* get list box attributes */
    GetCtrlAttribute(comport_panel, COM_PANEL_DEVICE_LISTBOX, ATTR_TOP, &firstListboxItemTop);
    firstListboxItemTop += 4; /* skip list box frame */
    
    GetCtrlAttribute (comport_panel, COM_PANEL_DEVICE_LISTBOX, ATTR_WIDTH, &listboxWidth);
    listboxWidth -= (8 + VAL_LARGE_SCROLL_BARS);    /* subtract left and right list box frame and scroll bar */
    
    GetCtrlAttribute(comport_panel, COM_PANEL_DEVICE_LISTBOX, ATTR_LEFT, &leftListboxEdge);
    leftListboxEdge += 4;   /* skip list box frame */
    rightListboxEdge = leftListboxEdge + listboxWidth;

    
    GetCtrlAttribute (comport_panel, COM_PANEL_DEVICE_LISTBOX, ATTR_VISIBLE_LINES, &numVisibleItems);
    CreateMetaFont ("Editor14", VAL_EDITOR_FONT, 14, 0, 0, 0, 0);
    GetTextDisplaySize ("  0xFFFFFF  ", "Editor14", &textHeight, &textWidth);

	numItems = comport_hash_entry_count(&device_data);
	
	/* Loop through devices and write the to the list control */
	comport_hash_foreach (&device_data, ComPortUpdateDeviceList, NULL );

	UpdateDeviceInfoInterface (0);
	
	/* explicitly set active list box item highlight and swallow event */
    SetCtrlIndex (comport_panel, COM_PANEL_DEVICE_LISTBOX, 0);
    SetActiveCtrl (comport_panel, COM_PANEL_DEVICE_LISTBOX);
	
	InstallPopup(comport_panel);
	
	/* Freeze execution until the user has setup the neccessary ports */
	while ( !user_has_configured ) {
		event_result = GetUserEvent (1, &panel, &control);
	
		if(event_result < 1)
			return COMM_CONTROL_ERROR;
	}

	return COMM_CONTROL_SUCCESS;
}

int CVICALLBACK onOkButton (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			
			// Write comport_hash table to conf file 
       		ComPortControlWriteConfigureFile();
			
			HidePanel (comport_panel);
			
			user_has_configured = 1;
			
			break;
		}
		
	return 0;
}

int CVICALLBACK onListSelection (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if ((event == EVENT_LEFT_CLICK) || (event == EVENT_LEFT_DOUBLE_CLICK)) {
        /* eventData2 is x pixel value of click */
        if (eventData2 < leftListboxEdge || rightListboxEdge < eventData2)
            return 0;
            
        /* eventData1 is y pixel value of click */
        if (eventData1 < firstListboxItemTop)
            return 0;
            
        GetCtrlAttribute (comport_panel, COM_PANEL_DEVICE_LISTBOX, ATTR_FIRST_VISIBLE_LINE, &selectionIndex);
        selectionIndex += (eventData1 - firstListboxItemTop) / textHeight;
        
        if (selectionIndex >= numItems)
            selectionIndex = numItems ? numItems - 1 : 0;
            
        /* explicitly set active list box item highlight and swallow event */
        SetCtrlIndex (comport_panel, COM_PANEL_DEVICE_LISTBOX, selectionIndex);
        SetActiveCtrl (comport_panel, COM_PANEL_DEVICE_LISTBOX);
      
      
        UpdateDeviceInfoInterface (selectionIndex);
    }
    
    return 0;
}

int CVICALLBACK onApplyButton (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	struct comport_info *data;
	char device_key[15];

	switch (event)
		{
		case EVENT_COMMIT:
		
			data = comport_hash_data_new();
		
			GetCtrlAttribute (comport_panel, COM_PANEL_DEVICE_TEXT, ATTR_CTRL_VAL, device_key);
			strcpy(data->deviceName, device_key);
			GetCtrlAttribute (comport_panel, COM_PANEL_PORT_RING, ATTR_CTRL_VAL, &data->port);
			GetCtrlAttribute (comport_panel, COM_PANEL_BAUD_RING, ATTR_CTRL_VAL, &data->baudRate);
			GetCtrlAttribute (comport_panel, COM_PANEL_PARITY_RING, ATTR_CTRL_VAL, &data->parity);
			GetCtrlAttribute (comport_panel, COM_PANEL_DATA_BITS_RING, ATTR_CTRL_VAL, &data->dataBits);
			GetCtrlAttribute (comport_panel, COM_PANEL_STOP_BITS_RING, ATTR_CTRL_VAL, &data->stopBits);
			GetCtrlAttribute (comport_panel, COM_PANEL_INPUT_QUEUE_NUMERIC, ATTR_CTRL_VAL, &data->inputQueueSize);
			GetCtrlAttribute (comport_panel, COM_PANEL_OUTPUT_QUEUE_NUMERIC, ATTR_CTRL_VAL, &data->outputQueueSize);

            if((comport_hash_lookup(&device_data, device_key)) != NULL) {
            	data->configured = 1;
            	comport_hash_insert (&device_data, device_key, data);
            }
            	
            ClearListCtrl (comport_panel, COM_PANEL_DEVICE_LISTBOX);
            
			/* Loop through devices and update the list control */
			comport_hash_foreach (&device_data, ComPortUpdateDeviceList, NULL );
			
			/* explicitly set active list box item highlight and swallow event */
    		SetCtrlIndex (comport_panel, COM_PANEL_DEVICE_LISTBOX, selectionIndex);
    		SetActiveCtrl (comport_panel, COM_PANEL_DEVICE_LISTBOX);
            		
			break;
		}
		
	return 0;
}
