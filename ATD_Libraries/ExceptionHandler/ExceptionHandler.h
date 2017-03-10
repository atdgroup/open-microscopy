#ifndef __EXCEPTION_DIALOG__
#define __EXCEPTION_DIALOG__

#include <windows.h>

typedef enum {EXCEPTION_DEBUG = 0, EXCEPTION_QUIT} EXCEPTION_ACTION;

int CreateMiniDump(EXCEPTION_POINTERS* pep, DWORD dwExpCode);

EXCEPTION_ACTION ShowExceptionDialog (int parent, char *message);

#endif