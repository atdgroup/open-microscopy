#ifndef __LASER__
#define __LASER__

#include "HardwareTypes.h"
#include "HardwareDevice.h"
#include "signals.h" 

#define LASER_SUCCESS 0
#define LASER_ERROR -1

#define LASER_VTABLE_PTR(ob, member) ((ob)->lpVtbl.member)
#define LASER_VTABLE(ob, member) (*((ob)->lpVtbl.member))

#define CHECK_LASER_VTABLE_PTR(ob, member) if(LASER_VTABLE_PTR(ob, member) == NULL) { \
    send_laser_error_text(ob, "member not implemented"); \
    return LASER_ERROR; \
}  

#define CALL_LASER_VTABLE_PTR(ob, member) if(LASER_VTABLE(ob, member)(ob) == LASER_ERROR ) { \
	send_laser_error_text(ob, "member failed");  \
	return LASER_ERROR; \
}

typedef struct
{
	int (*destroy) (Laser* laser);
	int (*get_power) (Laser* laser, float *power);

} LaserVtbl;


struct _Laser
{
  HardwareDevice parent;
  
  LaserVtbl lpVtbl;
};

int  send_laser_error_text (Laser* laser, char fmt[], ...);

void laser_constructor(Laser* laser, const char *name, const char *description, const char *data_dir);

int laser_get_power(Laser* laser, float *power);

int laser_destroy(Laser* laser);

#endif

 
