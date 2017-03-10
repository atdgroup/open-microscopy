#include <errno.h>
#include <stdio.h>
#include <math.h>

#ifndef _CVI_

int _GetErrno (void)
{
    return errno;
}

FILE **_GetFilesArray (void)
{
    static FILE *wrapper_files[3] = {NULL, NULL, NULL};

    wrapper_files[0] = stdin;
    wrapper_files[1] = stdout;
    wrapper_files[2] = stderr;

    return wrapper_files;
}

double _GetDoubleInf(void)
{
    return _HUGE;
}

#endif