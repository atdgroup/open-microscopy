#include "ThreadDebug.h"
#include <toolbox.h>
#include <utility.h>

static FILE *thread_fp = NULL;

int CVIFUNC GciCmtNewLock (const char lockName[], unsigned int options, int *lockHandle)
{
	int ret;

	#ifdef VERBOSE_DEBUG
	if(thread_fp == NULL) {
		thread_fp = fopen("thread.dat", "w");
	}
	#endif

	// We don't use the lock name as we don't want locks accross
	// multiple processes and this would slow us down.
	// However, we do use lockName for our debug log below.
	ret = CmtNewLock(NULL, 0, lockHandle);

	#ifdef VERBOSE_DEBUG
	fprintf(thread_fp, "NewLock,%d,%d,%s\n", *lockHandle, CmtGetCurrentThreadID(), lockName);
	fflush(thread_fp);
	#endif

	return ret;
}

int CVIFUNC GciCmtGetLock (int lockHandle)
{
	#ifdef VERBOSE_DEBUG
	fprintf(thread_fp, "GetLock,%d,%d,%.5f\n", lockHandle, CmtGetCurrentThreadID(), Timer());
	fflush(thread_fp);
	#endif

	return CmtGetLock(lockHandle);
}

int CVIFUNC GciCmtReleaseLock (int lockHandle)
{
	#ifdef VERBOSE_DEBUG
	fprintf(thread_fp, "ReleaseLock,%d,%d,%.5f\n", lockHandle, CmtGetCurrentThreadID(), Timer());
	fflush(thread_fp);
	#endif

	return CmtReleaseLock(lockHandle);
}