//
// File: _coder_hr_filter_api.h
//
// MATLAB Coder version            : 23.2
// C/C++ source code generated on  : 01-Apr-2024 15:40:09
//

#ifndef _CODER_HR_FILTER_API_H
#define _CODER_HR_FILTER_API_H

// Include Files
#include "emlrt.h"
#include "mex.h"
#include "tmwtypes.h"
#include <algorithm>
#include <cstring>

// Variable Declarations
extern emlrtCTX emlrtRootTLSGlobal;
extern emlrtContext emlrtContextGlobal;

// Function Declarations
void hr_filter(real32_T x_data[], int32_T x_size[2], real32_T y_data[],
               int32_T y_size[2]);

void hr_filter_api(const mxArray *prhs, const mxArray **plhs);

void hr_filter_atexit();

void hr_filter_initialize();

void hr_filter_terminate();

void hr_filter_xil_shutdown();

void hr_filter_xil_terminate();

#endif
//
// File trailer for _coder_hr_filter_api.h
//
// [EOF]
//
