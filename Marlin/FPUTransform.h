#ifndef __FPUTRANSFORM

#define __FPUTRANSFORM
#include "Marlin.h"
#include "z_probe.h"

#if defined(UMFPUSUPPORT) && (UMFPUSUPPORT > -1) 
  extern bool FPUEnabled;
  void FPUTransform_init();
  void FPUEnable();
  void FPUReset();
  void FPUDisable();
  void FPUTransform_determineBedOrientation();
  void FPUTransform_transformDestination(); 

#else //no UMFPU SUPPORT
  FORCE_INLINE void FPUTransform_init() {};

#endif //UMFPUSUPPORT

#endif //__FPUTRANSFORM
