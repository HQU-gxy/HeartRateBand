//
// File: _coder_hr_filter_mex.h
//
// MATLAB Coder version            : 23.2
// C/C++ source code generated on  : 01-Apr-2024 16:42:51
//

#ifndef _CODER_HR_FILTER_MEX_H
#define _CODER_HR_FILTER_MEX_H

// Include Files
#include "_coder_hr_filter_api.h"
#include "emlrt.h"
#include "mex.h"
#include "tmwtypes.h"

// Function Declarations
MEXFUNCTION_LINKAGE void mexFunction(int32_T nlhs, mxArray *plhs[],
                                     int32_T nrhs, const mxArray *prhs[]);

emlrtCTX mexFunctionCreateRootTLS();

void unsafe_hr_filter_mexFunction(hr_filterStackData *SD, int32_T nlhs,
                                  mxArray *plhs[1], int32_T nrhs,
                                  const mxArray *prhs[1]);

#endif
//
// File trailer for _coder_hr_filter_mex.h
//
// [EOF]
//
