#ifndef __FILTERSET_DUMMY__
#define __FILTERSET_DUMMY__ 

#include "FilterSet.h"

////////////////////////////////////////////////////////////////////////////
//RJL November 2006
//Fluorescent cube control for manual microscope.
////////////////////////////////////////////////////////////////////////////

typedef struct
{
	FilterSetCollection parent;

	int stored_position;
	
} FilterSetCollectionManual;


FilterSetCollection* manual_filterset_new(const char *name, const char *description,
								const char *data_dir, const char *filepath,
								UI_MODULE_ERROR_HANDLER error_handler, void *data);


#endif
