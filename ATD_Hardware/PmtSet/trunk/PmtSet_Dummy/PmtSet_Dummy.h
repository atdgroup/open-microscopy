#ifndef __DUMMY_PMT__
#define __DUMMY_PMT__ 

#include "PmtSet.h"

typedef struct
{
	PmtSet parent;
	
	int manual_position;
    
} ManualPmtSet;

PmtSet* manual_pmtset_new(const char *name, const char *description, const char* data_dir, const char *filepath);

#endif
