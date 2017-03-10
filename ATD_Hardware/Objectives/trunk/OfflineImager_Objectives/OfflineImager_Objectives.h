#ifndef __OFFLINEIMAGER_OBJECTIVE_MANAGER__ 
#define __OFFLINEIMAGER_OBJECTIVE_MANAGER__

#include "objectives.h" 

typedef struct
{
	ObjectiveManager parent;
	
	int	 	 _com_port;
	int 	 _i2c_address;
	int 	 _i2c_bus;
	
} OfflineImagerObjectiveManager;

ObjectiveManager* offlineimager_objective_manager_new(const char *name, const char *description, const char *data_dir, const char *filepath);

#endif
