#ifndef __90i_ANALYZER__
#define __90i_ANALYZER__ 

#include "analyzer.h"

typedef struct
{
	Analyzer parent;
	
 	CAObjHandle hAnalyzer; 
	
} Analyzer90i;


Analyzer* Nikon90i_analyzer_new(CAObjHandle hNikon90i, const char *name, const char *description,
  UI_MODULE_ERROR_HANDLER error_handler, const char *data_dir, void *data);

#endif
