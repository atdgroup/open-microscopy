#ifndef __CORVUS_STAGE__
#define __CORVUS_STAGE__

#include "stage.h"

typedef struct _CorvusStage CorvusStage;

struct _CorvusStage {

  Stage stage;
  
};

Stage* Corvus_stage_new(void);

#endif
