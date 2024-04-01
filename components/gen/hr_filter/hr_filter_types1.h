//
// File: hr_filter_types1.h
//
// MATLAB Coder version            : 23.2
// C/C++ source code generated on  : 01-Apr-2024 15:34:30
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
  int S0_isInitialized;
  float W0_FILT_STATES[8192];
  int W1_PreviousNumChannels;
  float P0_ICRTP;
  float P1_RTP1COEFF[6];
  float P2_RTP2COEFF[4];
  float P3_RTP3COEFF[3];
  boolean_T P4_RTP_COEFF3_BOOL[3];
};

} // namespace gen

#endif
//
// File trailer for hr_filter_types1.h
//
// [EOF]
//
