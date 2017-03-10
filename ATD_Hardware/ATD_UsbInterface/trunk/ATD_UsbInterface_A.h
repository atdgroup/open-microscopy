////////////////////////////////////////////////////////////////////////////
//Export functions for usbdetectors.c
//Module of GCI MultiPhotonTimeResolved system
//
//Rosalind Locke - January 2003
////////////////////////////////////////////////////////////////////////////
// RJL 23 December 2003 - Added function GCI_EnableLowLevelErrorReporting() 
////////////////////////////////////////////////////////////////////////////
// RJL July 2004
// Add functions for multiple port operation.
// The single port functions remain for backward compatibility.
// The multi port functions should be used for new projects.
///////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4005)
#endif

#ifdef I2C_DEBUG 
#include "gci_ui_module.h"
#else
typedef struct _Logger Logger;
#endif

//For historical reasons if there's only one bus we initialise it to 2
#define GCI_I2C_SINGLE_BUS	2

typedef unsigned char byte;

void GCI_EnableLowLevelErrorReporting(int enable);

// Locking
void GetI2CPortLock(int port, const char *name);
void ReleaseI2CPortLock(int port, const char *name);

void gci_set_i2c_logger(Logger* logger);

int GCI_initI2C_multiPort(int port);
void GCI_closeI2C_multiPort(int port);
int GCI_writeRS232_multiPort(int port, byte data[], int nbr_bytes);
int GCI_readRS232_multiPort(int port, byte data[]);
int GCI_readI2C_multiPort(int port, int nbrbytes, byte data[], int i2cbus, const char *name);
int GCI_writeI2C_multiPort(int port, int nbrbytes, byte data[], int i2cbus, const char *name);
int GCI_writeFAST_multiPort(int port, byte data[], int nbr_bytes, const char *name);
int GCI_readFAST_multiPort(int port, byte data[], int nbr_bytes, const char *name);
int GCI_writereadFAST_multiPort(int port, byte data[], int nbr_bytes, int ret_bytes);
int GCI_Out_PIC_multiPort (int port, int bus, byte chip_type, byte address, int n_bytes, byte *data);
