/*
	Communications interface using the D2XX driver for FTDI

	Filename: FTDI_Utils.h
	Date:     2010
	
	Author: Glenn Pierce
	Company: Gray Institute
	
*/

#ifndef _FTDI_UTILS_
#define _FTDI_UTILS_

#include <windows.h>
#include "FTD2XX.H"

#define FTDI_MAX_NUMBER_OF_DEVICES 10
#define FTDI_SERIAL_NUMBER_LINE_LENGTH 64
#define FTDI_DESCRIPTION_LINE_LENGTH 256

#define SETBITS(mem, bits)      (mem) |= (bits)
#define CLEARBITS(mem, bits)    (mem) &= ~(bits)
#define TOGGLEBITS(mem)			(~(mem))
#define SETLSB(mem)				SETBITS(mem, BIN(0,0,0,0,0,0,0,1));
#define CLEARLSB(mem)			CLEARBITS(mem, BIN(0,0,0,0,0,0,0,1));
#define BIN(b7,b6,b5,b4, b3,b2,b1,b0)                      \
(BYTE)(                                           \
    ((b7)<<7) + ((b6)<<6) + ((b5)<<5) + ((b4)<<4) +        \
    ((b3)<<3) + ((b2)<<2) + ((b1)<<1) + ((b0)<<0)          \
)

#define SET_BIT_ON(mem, bit) (mem |= 1 << bit) 
#define SET_BIT_OFF(mem, bit) (mem &= ~(1 << bit)) 
#define TOGGLE_BIT(mem, bit) (mem ^= 1 << bit) 

#define SET_BIT_WITH_MASK_TO_VALUE(mem, bit, value) (mem ^= (-value ^ mem) & bit)
#define SET_BIT_TO_VALUE(mem, bit, value) ( SET_BIT_WITH_MASK_TO_VALUE(mem, (1 << bit), value) )

#define GET_BIT_FROM_VALUE(value, bit) (value & (1 << bit))

//#define SET_BIT_TO_VALUE(mem, bit_mask, value) (if (value) mem |= bit_mask; else mem &= ~bit_mask;) 

// Example
//int a = 0x123;
//SETBITS(a, BIN(0,0,0,1,1,1,1,0));
//printf("0x%x", a); // should be 0x13F


#define MSB(a) (a = (a | (1<<(sizeof(a)*8-1)) ) | (a)) 

typedef struct _FTDI FTDIController;

typedef FT_STATUS (WINAPI *PtrToOpen)(PVOID, FT_HANDLE *); 
typedef FT_STATUS (WINAPI *PtrToOpenEx)(PVOID, DWORD, FT_HANDLE *); 
typedef FT_STATUS (WINAPI *PtrToListDevices)(PVOID, PVOID, DWORD);
typedef FT_STATUS (WINAPI *PtrToClose)(FT_HANDLE);
typedef FT_STATUS (WINAPI *PtrToRead)(FT_HANDLE, LPVOID, DWORD, LPDWORD);
typedef FT_STATUS (WINAPI *PtrToWrite)(FT_HANDLE, LPVOID, DWORD, LPDWORD);
typedef FT_STATUS (WINAPI *PtrToResetDevice)(FT_HANDLE);
typedef FT_STATUS (WINAPI *PtrToPurge)(FT_HANDLE, ULONG);
typedef FT_STATUS (WINAPI *PtrToSetTimeouts)(FT_HANDLE, ULONG, ULONG);
typedef FT_STATUS (WINAPI *PtrToGetQueueStatus)(FT_HANDLE, LPDWORD);
typedef FT_STATUS (WINAPI *PtrToSetBaudRate)(FT_HANDLE, ULONG);
typedef FT_STATUS (WINAPI *PtrToSetBitMode)(FT_HANDLE, UCHAR, UCHAR);
typedef FT_STATUS (WINAPI *PtrToSetDataCharacteristics)(FT_HANDLE, UCHAR, UCHAR, UCHAR);
typedef FT_STATUS (WINAPI *PtrToSetDeadmanTimeout)(FT_HANDLE, DWORD);

typedef void (*FT_ERROR_HANDLER) (FTDIController *, const char *title, const char *error_string, void *callback_data);       

typedef struct _DLLPointerTable
{
	PtrToOpen pOpen;
	PtrToOpenEx pOpenEx;
	PtrToWrite pRead;
	PtrToWrite pWrite;
	PtrToListDevices pListDevices;
	PtrToClose pClose;
	PtrToResetDevice pResetDevice;
	PtrToPurge pPurge;
	PtrToSetTimeouts pSetTimeouts;
	PtrToGetQueueStatus pGetQueueStatus;
	PtrToSetBaudRate pSetBaudRate;
	PtrToSetBitMode pSetBitMode;
	PtrToSetDataCharacteristics pSetDataCharacteristics;
	PtrToSetDeadmanTimeout pSetDeadmanTimeout;

} DLLPointerTable;

typedef struct _FTDI
{
	HMODULE module;
	FT_HANDLE device_handle;
    int debug;
	int debug_with_ints;
    
	DLLPointerTable dllPointerTable;

	void *callback_data;
	FT_ERROR_HANDLER error_handler;

	char device_id[FTDI_SERIAL_NUMBER_LINE_LENGTH];
};

FTDIController* ftdi_controller_new(void);
void		    ftdi_controller_destroy(FTDIController* controller);
void			ftdi_controller_set_error_handler(FTDIController* controller, FT_ERROR_HANDLER handler, void *callback_data);

int				ftdi_controller_get_lock(FTDIController* controller);
int				ftdi_controller_release_lock(FTDIController* controller);
FT_STATUS		ftdi_controller_open(FTDIController* controller, const char *device_id);
FT_STATUS		ftdi_controller_close(FTDIController* controller);
FT_STATUS 		ftdi_controller_purge_read_queue(FTDIController* controller);
FT_STATUS 		ftdi_controller_purge_write_queue(FTDIController* controller);

FT_STATUS		ftdi_controller_set_deadman_timout(FTDIController* controller, DWORD timeout);
FT_STATUS		ftdi_controller_set_timouts(FTDIController* controller, DWORD readTimeout , DWORD writeTimeout);
FT_STATUS		ftdi_controller_set_baudrate(FTDIController* controller, unsigned long baudrate);
FT_STATUS		ftdi_controller_set_data_characteristics(FTDIController* controller, unsigned char word_length
	, unsigned char stop_bits, unsigned char parity);

FT_STATUS		ftdi_controller_set_bit_bang_mode(FTDIController* controller, unsigned char mask, unsigned char ucMode);
int				ftdi_controller_set_debugging(FTDIController* controller, int debug);
int				ftdi_controller_show_debugging_bytes_as_integers(FTDIController* controller);
int				ftdi_controller_show_debugging_bytes_as_hex(FTDIController* controller);
int				ftdi_controller_get_number_of_connected_devices(FTDIController* controller);
int				ftdi_controller_print_serial_numbers_of_connected_devices(FTDIController* controller);
FT_STATUS		ftdi_controller_add_serial_numbers_of_connected_devices_to_ring_control(FTDIController* controller, int panel, int ctrl);
FT_STATUS		ftdi_controller_get_read_queue_status(FTDIController* controller, LPDWORD amount_in_rx_queue);
void			ftdi_print_byte_array_to_stdout(BYTE* array, int array_length, const char* prefix, int use_ints);

/*
The address passed to these functions are the 8bit address. The original protocol spec for out i2c devices
says that this is shifted one to the left ie make a 7bit address. The LSB is then used to indicate writing
or reading. Rob when passing the address did the shifting at the wrong higher level code so he often gave
me the 7bit equivilent address.
*/
FT_STATUS	ftdi_controller_i2c_write_bytes(FTDIController* controller, int address, int data_length, BYTE *data);
FT_STATUS	ftdi_controller_i2c_read_bytes(FTDIController* controller, int address, int data_length, BYTE *data);

FT_STATUS	ftdi_controller_i2c_fastline_write_bytes(FTDIController* controller, BYTE *data, int data_length);
FT_STATUS	ftdi_controller_i2c_fastline_read_bytes(FTDIController* controller, int data_length, BYTE *data);

FT_STATUS 	ftdi_controller_write_bytes(FTDIController* controller, int data_length, BYTE *data);
FT_STATUS 	ftdi_controller_write_byte(FTDIController* controller, BYTE data);
FT_STATUS	ftdi_controller_write_string(FTDIController* controller, const char* fmt, ...);
FT_STATUS	ftdi_controller_read_bytes(FTDIController* controller, int data_length, BYTE *data);
FT_STATUS	ftdi_controller_read_bytes_availiable_in_rx_queue(FTDIController* controller, BYTE *data, LPDWORD bytes_read);

#endif