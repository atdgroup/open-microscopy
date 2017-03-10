#ifndef __MANUAL_SCANNER__
#define __MANUAL_SCANNER__ 

#include "scanner.h"

typedef struct
{
	Scanner parent;
	
} ScannerManual;

Scanner* manual_scanner_new(char *name, char *description, const char *data_dir, const char *data_file, UI_MODULE_ERROR_HANDLER handler, void* data);

#endif
