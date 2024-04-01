//
// File: hr_filter_types.h
//
// MATLAB Coder version            : 23.2
// C/C++ source code generated on  : 01-Apr-2024 15:40:09
//

#ifndef HR_FILTER_TYPES_H
#define HR_FILTER_TYPES_H

// Include Files
#include "BiquadFilter.h"
#include "rtwtypes.h"

// Type Definitions
namespace gen {
struct hr_filterPersistentData {
  coder::dspcodegen::BiquadFilter Hd;
  boolean_T Hd_not_empty;
};

struct hr_filterStackData {
  hr_filterPersistentData *pd;
};

} // namespace gen

#endif
//
// File trailer for hr_filter_types.h
//
// [EOF]
//
