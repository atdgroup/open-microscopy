#include "device_list.h"


typedef struct
{
	int somedata;
	
} Objective;

typedef struct _ObjectiveManager
{
	ModuleDeviceConfigurator parent;
	
	
} ObjectiveManager;


ObjectiveManager* objective_manager_new(void)
{
	ObjectiveManager* om = (ObjectiveManager*) malloc(sizeof(ObjectiveManager));
	
	device_conf_constructor(DEVICE_CONF_CAST(om)); 
	
	return om;
}


int __stdcall WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPSTR lpszCmdLine, int nCmdShow)
{
	ObjectiveManager *om;
	Objective *ob1, *ob2, *ob3, *ob4, *ob5, *ob6;
	
  	if (InitCVIRTE (hInstance, 0, 0) == 0)
		return -1;    /* out of memory */

	om = objective_manager_new(); 
	
	device_conf_set_max_num__of_devices(DEVICE_CONF_CAST(om), 10); 
	device_conf_set_max_active_num_devices(DEVICE_CONF_CAST(om), 10); 

	
	ob1 = malloc(sizeof(Objective));
	ob2 = malloc(sizeof(Objective));
	ob3 = malloc(sizeof(Objective));
	ob4 = malloc(sizeof(Objective));
	ob5 = malloc(sizeof(Objective));
	ob6 = malloc(sizeof(Objective));
	
	device_conf_add_device(DEVICE_CONF_CAST(om), ob1, "Object1", -1);
	device_conf_add_device(DEVICE_CONF_CAST(om), ob2, "Object2", -1);
	device_conf_add_device(DEVICE_CONF_CAST(om), ob3, "Object3", -1);
	device_conf_add_device(DEVICE_CONF_CAST(om), ob4, "Object4", -1);
	device_conf_add_device(DEVICE_CONF_CAST(om), ob5, "Object5", -1);
	device_conf_add_device(DEVICE_CONF_CAST(om), ob6, "Object6", -1);

	
	device_conf_display_panel(DEVICE_CONF_CAST(om)); 
	
	RunUserInterface();
	
  	return 0;
}
