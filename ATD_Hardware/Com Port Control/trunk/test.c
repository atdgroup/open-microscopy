#include <ansi_c.h>
#include <rs232.h>
#include <userint.h>
#include <utility.h>

#include "com_port_control.h"

int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	int port, parity, baudRate, dataBits, stopBits, inputQueueSize, outputQueueSize;
	char properties[500];

	if (InitCVIRTE (hInstance, 0, 0) == 0)
    	return -1;

	/* Initialize comport control */
    GCI_ComPortControlInit();

    /* Informs the comport control code to check config data for
     * a particular device. If any extra devices are found within
     * the config file on disk and are not specified with GCI_ComPortControlAddDevice
     * they are ignored.
     */
    GCI_ComPortControlAddDevice("I2C");
    GCI_ComPortControlAddDevice("SerialPort");
    GCI_ComPortControlAddDevice("Shutter");
	GCI_ComPortControlAddDevice("Test");

	/* This tries to read config data from disk 
	 * if no info about a device is found the the user is
	 * asked for them
	 */
	GCI_ComPortControlLoadConfig();

	/* This function retrieves the comport properties of a device */
	GCI_ComPortControlGetDeviceProperties("Shutter", &port, &parity, &baudRate,
				&dataBits, &stopBits, &inputQueueSize, &outputQueueSize);

	
	sprintf(properties, "port: %d, parity: %d, baud rate: %d, data bits: %d"
			", stop bits: %d, input queue size: %d, output queue size: %d", 
			port, parity, baudRate, dataBits, stopBits, inputQueueSize, outputQueueSize);
	
	
	MessagePopup("Device Properties", properties);
	
	/* This displays the com config panel regardless of 
	 * whether or not the devices have already been configured
	 */
	/* GCI_ComPortControlDisplayConfigurePanel(void); */
	
	RunUserInterface();
	
	
	return 0;
}
