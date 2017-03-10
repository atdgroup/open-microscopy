/*
	Communications interface using the D2XX driver for FTDI

	Filename: FTDI_Utils.c
	Date:     2010
	
	Author: Glenn Pierce
	Company: Gray Institute
	
*/

#include "FTDI_Utils.h"
#include "gci_utils.h"
#include "ThreadDebug.h"
#include "stdarg.h"
#include "dictionary.h"

static dictionary *devices = NULL;
static dictionary *device_open_count = NULL;

static int lock;	// Lock is global to all conroller instances as only one device can write at any one time.

#define DLL_POINTER(ptr_name) (controller->dllPointerTable.ptr_name) 
#define DLL_POINTER_CREATE(function_name, ptr_type, ptr_name) \
{ \
	DLL_POINTER(ptr_name) = (ptr_type) GetProcAddress(controller->module, function_name); \
	if (DLL_POINTER(ptr_name) == NULL) \
	{ \
		GCI_MessagePopup("Error", "Error: Can't find " function_name); \
		return NULL; \
	} \
}

static void ftdi_send_valist_error(FTDIController* controller, const char *title, const char *fmt, va_list ap)
{
	char message[500];

	SetSystemAttribute (ATTR_DEFAULT_MONITOR, 1);  
	
	vsprintf(message, fmt, ap);

	if(controller->error_handler != NULL) {
		controller->error_handler(controller, title, message, controller->callback_data);
	}
}

static void ftdi_send_error(FTDIController* controller, const char *title, const char *fmt, ...)
{
	va_list ap;
	
	va_start(ap, fmt);

	 ftdi_send_valist_error(controller, title, fmt, ap); 
		
	va_end(ap);
}


void ftdi_controller_set_error_handler(FTDIController* controller, FT_ERROR_HANDLER handler, void *callback_data)
{
	controller->error_handler = handler;
	controller->callback_data = callback_data;
}

FTDIController* ftdi_controller_new(void)
{
	FTDIController *controller = (FTDIController *) malloc(sizeof(FTDIController));
	memset(controller, 0, sizeof(FTDIController));

	controller->module = LoadLibrary("Ftd2xx.dll");	
	if(controller->module == NULL)
	{
		GCI_MessagePopup("Error", "Error: Can't Load ftd2xx dll");
		return NULL;
	}

	controller->debug_with_ints = 1;
	controller->debug = 0;

	DLL_POINTER_CREATE("FT_Read", PtrToRead, pRead);
	DLL_POINTER_CREATE("FT_Write", PtrToWrite, pWrite);
	DLL_POINTER_CREATE("FT_Open", PtrToOpen, pOpen);
	DLL_POINTER_CREATE("FT_OpenEx", PtrToOpenEx, pOpenEx);
	DLL_POINTER_CREATE("FT_ListDevices", PtrToListDevices, pListDevices);
	DLL_POINTER_CREATE("FT_Close", PtrToClose, pClose);
	DLL_POINTER_CREATE("FT_ResetDevice", PtrToResetDevice, pResetDevice);
	DLL_POINTER_CREATE("FT_Purge", PtrToPurge, pPurge);
	DLL_POINTER_CREATE("FT_SetTimeouts", PtrToSetTimeouts, pSetTimeouts);
	DLL_POINTER_CREATE("FT_GetQueueStatus", PtrToGetQueueStatus, pGetQueueStatus);
	DLL_POINTER_CREATE("FT_SetBaudRate", PtrToSetBaudRate, pSetBaudRate);
	DLL_POINTER_CREATE("FT_SetBitMode", PtrToSetBitMode, pSetBitMode);
	DLL_POINTER_CREATE("FT_SetDataCharacteristics", PtrToSetDataCharacteristics, pSetDataCharacteristics);
	DLL_POINTER_CREATE("FT_SetDeadmanTimeout", PtrToSetDeadmanTimeout, pSetDeadmanTimeout);

	if(devices == NULL) {
		devices = dictionary_new(20);
		device_open_count = dictionary_new(20);
	}

	return controller;
}

int ftdi_controller_get_lock(FTDIController* controller)
{
	 return GciCmtGetLock (lock);
}

int ftdi_controller_release_lock(FTDIController* controller)
{
	 return GciCmtReleaseLock (lock);
}

FT_STATUS ftdi_controller_open(FTDIController* controller, const char *device_id)
{
	FT_STATUS ftStatus;
	FT_HANDLE handle;

	if(strcmp(device_id, "") == 0)
		return FT_DEVICE_NOT_OPENED;

	handle = (FT_HANDLE) dictionary_getulong(devices, device_id, 0l);

	if(handle != 0l) {
		
		int count;

		controller->device_handle = handle;
		strncpy(controller->device_id, device_id, FTDI_SERIAL_NUMBER_LINE_LENGTH - 1);

		// Increment the open reference count for this device
		count = dictionary_getint(device_open_count, device_id, -1);
		if(count >= 0) {
			count++;
			dictionary_setint(device_open_count, device_id, count);
		}

		return FT_OK;
	}

	ftStatus = controller->dllPointerTable.pOpenEx((char*) device_id, FT_OPEN_BY_SERIAL_NUMBER, &(controller->device_handle));
    
    if (ftStatus != FT_OK)
	{
		if(controller->debug) {
			printf("Failed to open device %s\nAvailable devices are:\n", device_id);
		}

		ftdi_controller_print_serial_numbers_of_connected_devices(controller);

		return FT_DEVICE_NOT_OPENED;
	}

	GciCmtNewLock (device_id, 0, &(lock));
	strncpy(controller->device_id, device_id, FTDI_SERIAL_NUMBER_LINE_LENGTH - 1);
	dictionary_setulong(devices, device_id, (unsigned long) controller->device_handle);
	dictionary_setint(device_open_count, device_id, 1);

	// Set default timeouts
	ftdi_controller_set_timouts(controller, 1000, 1000);

	return FT_OK;
}

FT_STATUS ftdi_controller_set_timouts(FTDIController* controller, DWORD readTimeout , DWORD writeTimeout)
{
	FT_STATUS ftStatus = controller->dllPointerTable.pSetTimeouts(controller->device_handle, readTimeout, writeTimeout);
    
    return ftStatus;
}

FT_STATUS ftdi_controller_set_deadman_timout(FTDIController* controller, DWORD timeout)
{
	FT_STATUS ftStatus = controller->dllPointerTable.pSetDeadmanTimeout(controller->device_handle, timeout);
    
    return ftStatus;
}

FT_STATUS	ftdi_controller_set_baudrate(FTDIController* controller, unsigned long baudrate)
{
	FT_STATUS ftStatus = controller->dllPointerTable.pSetBaudRate(controller->device_handle, baudrate);
    
    return ftStatus;
}

FT_STATUS	ftdi_controller_set_data_characteristics(FTDIController* controller, unsigned char word_length
	, unsigned char stop_bits, unsigned char parity)
{
	return controller->dllPointerTable.pSetDataCharacteristics(controller->device_handle, word_length,
		stop_bits, parity);
}

FT_STATUS ftdi_controller_close(FTDIController* controller)
{
	int count = 0;

	if(controller->device_handle == NULL)
		return FT_DEVICE_NOT_OPENED;
	
	if(device_open_count == NULL)
		return FT_DEVICE_NOT_OPENED;
	
	// Increment the open reference count for this device
	count = dictionary_getint(device_open_count, controller->device_id, -1);
	
	if(count > 0) {
		count--;
		dictionary_setint(device_open_count, controller->device_id, count);
	}

	if(count <= 0) {
		controller->dllPointerTable.pClose(controller->device_handle);
        dictionary_setulong(devices, controller->device_id, 0l);
	}

	return FT_OK;
}

void ftdi_controller_destroy(FTDIController* controller)
{
    ftdi_controller_close(controller);
}

int ftdi_controller_set_debugging(FTDIController* controller, int debug)
{
    controller->debug = debug;

	return FT_OK;
}

int	ftdi_controller_show_debugging_bytes_as_integers(FTDIController* controller)
{
	controller->debug_with_ints = 1;

	return FT_OK;
}

int	ftdi_controller_show_debugging_bytes_as_hex(FTDIController* controller)
{
	controller->debug_with_ints = 0;

	return FT_OK;
}

int ftdi_controller_get_number_of_connected_devices(FTDIController* controller)
{
    FT_STATUS ftStatus;
    int numDevs;

    ftStatus = controller->dllPointerTable.pListDevices(&numDevs, NULL, FT_LIST_NUMBER_ONLY);
    
    if (ftStatus == FT_OK)
        return numDevs;
    else
        return 0;
}


FT_STATUS ftdi_controller_set_bit_bang_mode(FTDIController* controller, unsigned char mask, unsigned char ucMode)
{
    return controller->dllPointerTable.pSetBitMode(controller->device_handle, mask, ucMode);
}

int ftdi_controller_print_serial_numbers_of_connected_devices(FTDIController* controller)
{
   FT_STATUS ftStatus;
   int i, numDevs;

   char **serial_numbers = (char**) malloc(sizeof(char*) * FTDI_MAX_NUMBER_OF_DEVICES);

   for(i=0; i < FTDI_MAX_NUMBER_OF_DEVICES; i++) {

		serial_numbers[i] = (char*) malloc(sizeof(char) * FTDI_SERIAL_NUMBER_LINE_LENGTH);
   }

   ftStatus = controller->dllPointerTable.pListDevices(serial_numbers, &numDevs, 
																 FT_LIST_ALL|FT_OPEN_BY_SERIAL_NUMBER);
   if(ftStatus == FT_OK) {
	   for(i=0; i < numDevs; i++) {
	       printf("%s\n", serial_numbers[i]);
	   }
   }

   for(i=0; i < FTDI_MAX_NUMBER_OF_DEVICES; i++) {

		free(serial_numbers[i]);
		serial_numbers[i] = NULL;
   }

   free(serial_numbers);

   return ftStatus;
}

FT_STATUS ftdi_controller_add_serial_numbers_of_connected_devices_to_ring_control(FTDIController* controller, int panel, int ctrl)
{
   FT_STATUS ftStatus;
   int i, numDevs;

   char **serial_numbers = (char**) malloc(sizeof(char*) * FTDI_MAX_NUMBER_OF_DEVICES);

   for(i=0; i < FTDI_MAX_NUMBER_OF_DEVICES; i++) {

		serial_numbers[i] = (char*) malloc(sizeof(char) * FTDI_SERIAL_NUMBER_LINE_LENGTH);
   }

   ftStatus = controller->dllPointerTable.pListDevices(serial_numbers, &numDevs, 
																 FT_LIST_ALL|FT_OPEN_BY_SERIAL_NUMBER);
   if(ftStatus == FT_OK) {
	   for(i=0; i < numDevs; i++) {
		   InsertListItem (panel, ctrl, i, serial_numbers[i], serial_numbers[i]);    
	   }
   }

   for(i=0; i < FTDI_MAX_NUMBER_OF_DEVICES; i++) {

		free(serial_numbers[i]);
		serial_numbers[i] = NULL;
   }

   free(serial_numbers);

   return ftStatus;
}

static int i2c_error_check(FTDIController* controller, int stat)
{
	switch (stat){

		case 4:
		case 5:
			return FT_OK;  //OK

		case 1:
			ftdi_send_error(controller, "I2C Error", "Interrupt on I2C occurred");
			break;

		case 2:
			ftdi_send_error(controller, "I2C Error", "ACK error");
            break;

		case 8:
			ftdi_send_error(controller, "I2C Error", "I2C bus collision");
			break;

		default:
			break;
	}
	
	return FT_IO_ERROR;
}

static char * sprint_byte_array(BYTE* array, int array_length, const char* prefix, int use_ints, char *buffer)
{
	int i, written;
	char *ptr = buffer;
	const char* fmt = "0x%02x";

	written = sprintf(ptr, "%s [", prefix);
	ptr+=written;

	if (use_ints)
		fmt = "%d";

	for(i=0; i<array_length; i++) {

		written = sprintf(ptr, fmt, array[i]);
		ptr+=written;

		*ptr++ = ',';
	}

	*(--ptr) = ']';	// Get rid of last , and add a ]
	*(++ptr) = '\0';

	return buffer;
}

void ftdi_print_byte_array_to_stdout(BYTE* array, int array_length, const char* prefix, int use_ints)
{
	char buf[500] = "";

	char * b = sprint_byte_array(array, array_length, prefix, use_ints, buf);

	printf("%s\n", b);
	fflush(stdout);
}

FT_STATUS ftdi_controller_get_read_queue_status(FTDIController* controller, LPDWORD amount_in_rx_queue)
{
	FT_STATUS ftStatus;

	ftdi_controller_get_lock(controller);
    
	ftStatus = controller->dllPointerTable.pGetQueueStatus(controller->device_handle, amount_in_rx_queue);

	ftdi_controller_release_lock(controller);

	return ftStatus;  
}

FT_STATUS ftdi_controller_purge_read_queue(FTDIController* controller)
{
	FT_STATUS ftStatus;

	ftdi_controller_get_lock(controller);
    
	ftStatus = controller->dllPointerTable.pPurge(controller->device_handle, FT_PURGE_RX);

	ftdi_controller_release_lock(controller);

	return ftStatus;  
}

FT_STATUS ftdi_controller_purge_write_queue(FTDIController* controller)
{
	FT_STATUS ftStatus;

	ftdi_controller_get_lock(controller);
    
	ftStatus = controller->dllPointerTable.pPurge(controller->device_handle, FT_PURGE_TX);

	ftdi_controller_release_lock(controller);

	return ftStatus;  
}

FT_STATUS ftdi_controller_i2c_write_bytes(FTDIController* controller, int address, int data_length, BYTE *data)
{
	FT_STATUS ftStatus;
	int i, total_raw_bytes;
	DWORD bytes_written_or_read = 0;
    BYTE raw_data[100] = "";
    
	ftdi_controller_get_lock(controller);

	raw_data[0] = 0x02;
	raw_data[1] = data_length + 1;
	raw_data[2] = address << 1;
	total_raw_bytes = data_length + 3;

	// Append data byte array
	for(i=0; i<data_length; i++)
		raw_data[i+3] = data[i];

    if(controller->debug) {
		ftdi_print_byte_array_to_stdout(raw_data, total_raw_bytes, "Writing actual data", controller->debug_with_ints);
    }
    
	ftStatus = controller->dllPointerTable.pWrite(controller->device_handle, raw_data, total_raw_bytes, &bytes_written_or_read);

	if (ftStatus != FT_OK || bytes_written_or_read != total_raw_bytes) {
		
		ftdi_controller_release_lock(controller);
		return ftStatus;
	}

	memset(raw_data, 0, sizeof(raw_data));

	ftStatus = controller->dllPointerTable.pRead(controller->device_handle, raw_data, 1, &bytes_written_or_read);

	if (ftStatus != FT_OK || bytes_written_or_read != 1) {
		
		ftdi_controller_release_lock(controller);
		return ftStatus;
	}

	ftdi_controller_release_lock(controller);

	return i2c_error_check(controller, raw_data[0]);  
}


FT_STATUS ftdi_controller_i2c_fastline_write_bytes(FTDIController* controller, BYTE *data, int data_length)
{
	FT_STATUS ftStatus;
	int i, total_raw_bytes;
	DWORD bytes_written_or_read = 0;
    BYTE raw_data[100] = "";
    
	raw_data[0] = 0x01;
	raw_data[1] = data_length;
	total_raw_bytes = data_length + 2;

	// Append data byte array
	for(i=0; i<data_length; i++)
		raw_data[i+2] = data[i];

    ftdi_controller_get_lock(controller);
    
    if(controller->debug) {
		ftdi_print_byte_array_to_stdout(raw_data, total_raw_bytes, "Writing actual data", controller->debug_with_ints);
    }
    
	ftStatus = controller->dllPointerTable.pWrite(controller->device_handle, raw_data, total_raw_bytes, &bytes_written_or_read);

	if (ftStatus != FT_OK || bytes_written_or_read != total_raw_bytes) {
		
		ftdi_controller_release_lock(controller);
		return ftStatus;
	}

	controller->dllPointerTable.pGetQueueStatus(controller->device_handle, &bytes_written_or_read);

	// Old PICs don't return anything, so if nothing returned assume success
	if (bytes_written_or_read < 1) {
		ftdi_controller_release_lock(controller);
		return i2c_error_check(controller, 4);  
	}

	memset(raw_data, 0, sizeof(raw_data));

	ftStatus = controller->dllPointerTable.pRead(controller->device_handle, raw_data, 1, &bytes_written_or_read);

	if (ftStatus != FT_OK || bytes_written_or_read != 1) {
		
		ftdi_controller_release_lock(controller);
		return ftStatus;
	}

	ftdi_controller_release_lock(controller);

	return i2c_error_check(controller, raw_data[0]);  
}


FT_STATUS ftdi_controller_i2c_read_bytes(FTDIController* controller, int address, int data_length, BYTE *data)
{
	DWORD bytes_written_or_read = 0;
	FT_STATUS ftStatus;
	BYTE raw_data[100] = "";

	ftdi_controller_get_lock(controller);
    
	ftStatus = controller->dllPointerTable.pPurge(controller->device_handle, FT_PURGE_RX | FT_PURGE_TX);

	if (ftStatus != FT_OK) {
		ftdi_controller_release_lock(controller);
		return ftStatus;
	}

	raw_data[0] = 0x02;
    raw_data[1] = data_length + 1;
	raw_data[2] = address << 1;
	raw_data[2] = SETLSB(raw_data[2]);

	ftStatus = controller->dllPointerTable.pWrite(controller->device_handle, raw_data, 3, &bytes_written_or_read);

	if (ftStatus != FT_OK || bytes_written_or_read != 3) {
		
		ftdi_controller_release_lock(controller);
		return ftStatus;
	}

    if(controller->debug) {
		ftdi_print_byte_array_to_stdout(raw_data, 3, "Writing actual data from read", controller->debug_with_ints);
    }

	memset(raw_data, 0, sizeof(raw_data));

	ftStatus = controller->dllPointerTable.pRead(controller->device_handle, data, data_length + 1, &bytes_written_or_read);

	if (ftStatus != FT_OK || bytes_written_or_read != (data_length + 1)) {
		
		ftdi_controller_release_lock(controller);
		return ftStatus;
	}

	if(controller->debug) {
		ftdi_print_byte_array_to_stdout(data, data_length + 1, "Reading data", controller->debug_with_ints);
    }

	ftdi_controller_release_lock(controller);

	return i2c_error_check(controller, data[data_length]);  
}

FT_STATUS ftdi_controller_i2c_fastline_read_bytes(FTDIController* controller, int data_length, BYTE *data)
{
	int i, total_raw_bytes;
	DWORD bytes_written_or_read = 0;
	FT_STATUS ftStatus;
	BYTE raw_data[100] = "";

	ftdi_controller_get_lock(controller);
    
	ftStatus = controller->dllPointerTable.pPurge(controller->device_handle, FT_PURGE_RX | FT_PURGE_TX);

	if (ftStatus != FT_OK) {
		ftdi_controller_release_lock(controller);
		return ftStatus;
	}

	raw_data[0] = 0x01;
	total_raw_bytes = data_length + 1;
    raw_data[1] = total_raw_bytes;
	
	// Append data byte array
	for(i=0; i<total_raw_bytes; i++)
		raw_data[i+2] = data[i];

	ftStatus = controller->dllPointerTable.pWrite(controller->device_handle, raw_data, total_raw_bytes + 3, &bytes_written_or_read);

	if (ftStatus != FT_OK || bytes_written_or_read != (total_raw_bytes + 3)) {
		
		ftdi_controller_release_lock(controller);
		return ftStatus;
	}

    if(controller->debug) {
		ftdi_print_byte_array_to_stdout(raw_data, total_raw_bytes + 3, "Writing actual data from read", controller->debug_with_ints);
    }

	memset(raw_data, 0, sizeof(raw_data));

	ftStatus = controller->dllPointerTable.pRead(controller->device_handle, data, total_raw_bytes - 1, &bytes_written_or_read);

	if (ftStatus != FT_OK) {
		
		ftdi_controller_release_lock(controller);
		return ftStatus;
	}

	if(controller->debug) {
		ftdi_print_byte_array_to_stdout(data, total_raw_bytes - 1, "Reading data", controller->debug_with_ints);
    }

	ftdi_controller_release_lock(controller);

	return i2c_error_check(controller, 4);  // No status byte returned
}


FT_STATUS ftdi_controller_write_bytes(FTDIController* controller, int data_length, BYTE *data)
{
	FT_STATUS ftStatus;
	DWORD bytes_written_or_read = 0;
    
	if(controller->device_handle == 0)
		return FT_DEVICE_NOT_OPENED;
	
	ftdi_controller_get_lock(controller);

    if(controller->debug) {
		ftdi_print_byte_array_to_stdout(data, data_length, "Writing actual data", controller->debug_with_ints);
    }
    
	ftStatus = controller->dllPointerTable.pWrite(controller->device_handle, data, data_length, &bytes_written_or_read);

	if (ftStatus != FT_OK || bytes_written_or_read != data_length) {
		
		printf("Error sending data\n");
		ftdi_controller_release_lock(controller);
		return ftStatus;
	}
	else {
		if(controller->debug) {
			printf("Data send successfully\n");
		}
	}

	ftdi_controller_release_lock(controller);

	return ftStatus;  
}

FT_STATUS 	ftdi_controller_write_byte(FTDIController* controller, BYTE data)
{
	BYTE bytes[1] = {data};

	return ftdi_controller_write_bytes( controller, 1, bytes);
}

FT_STATUS ftdi_controller_write_string(FTDIController* controller, const char* fmt, ...)
{
	FT_STATUS ftStatus;
	char buffer[200] = "";
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buffer, fmt, ap);
	va_end(ap); 

	ftStatus = ftdi_controller_write_bytes(controller, strlen(buffer), buffer);
	
	if(ftStatus != FT_OK)
		return  ftStatus;

	return ftStatus;
}

FT_STATUS ftdi_controller_read_bytes(FTDIController* controller, int data_length, BYTE *data)
{
	DWORD bytes_written_or_read = 0;
	FT_STATUS ftStatus;

	if(controller->device_handle == 0)
		return FT_DEVICE_NOT_OPENED;
	
	ftdi_controller_get_lock(controller);
    
	ftStatus = controller->dllPointerTable.pRead(controller->device_handle, data, data_length, &bytes_written_or_read);

	if (ftStatus != FT_OK || bytes_written_or_read != (data_length)) {
		
		ftdi_controller_release_lock(controller);
		return ftStatus;
	}

	if(controller->debug) {
		ftdi_print_byte_array_to_stdout(data, data_length, "Reading data", controller->debug_with_ints);
    }

	ftdi_controller_release_lock(controller);

	return ftStatus;  
}

FT_STATUS ftdi_controller_read_bytes_availiable_in_rx_queue(FTDIController* controller, BYTE *data, LPDWORD bytes_read)
{	
	FT_STATUS ftStatus;
	DWORD amount_in_rx_queue = 0;

	ftStatus = ftdi_controller_get_read_queue_status(controller, &amount_in_rx_queue);
	
	if(ftStatus != FT_OK)
		return ftStatus;

	if(amount_in_rx_queue == 0)
		return FT_IO_ERROR;

	ftStatus = ftdi_controller_read_bytes(controller, amount_in_rx_queue, data);
	
	if(ftStatus != FT_OK)
		return ftStatus;
	
	*bytes_read = amount_in_rx_queue;

	return ftStatus;
}

