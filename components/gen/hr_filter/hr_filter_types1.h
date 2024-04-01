//
// File: hr_filter_types1.h
//
// MATLAB Coder version            : 23.2
// C/C++ source code generated on  : 01-Apr-2024 15:40:09
//

#ifndef HR_FILTER_TYPES1_H
#define HR_FILTER_TYPES1_H

// Include Files
#include "rtwtypes.h"
#include <cstddef>
#include <cstdlib>

// Type Definitions
namespace gen {
struct dsp_BiquadFilter_0 {
  int32_T S0_isInitialized;
  real32_T W0_FILT_STATES[8192];
  int32_T W1_PreviousNumChannels;
  real32_T P0_ICRTP;
  real32_T P1_RTP1COEFF[6];
  real32_T P2_RTP2COEFF[4];
  real32_T P3_RTP3COEFF[3];
  boolean_T P4_RTP_COEFF3_BOOL[3];
};

} // namespace gen

#endif
//
// File trailer for hr_filter_types1.h
//
// [EOF]
//
