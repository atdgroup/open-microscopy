#ifndef __LSTEP_STAGE__
#define __LSTEP_STAGE__

#include "stage.h"

typedef struct _LStepStage LStepStage;

enum TriggerMode {INTERNAL, EXTERNAL};

struct _LStepStage {

  Stage stage;
  
};

Stage* lstep_stage_new(void);

#endif
