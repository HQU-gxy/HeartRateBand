//
// File: _coder_hr_filter_api.h
//
// MATLAB Coder version            : 23.2
// C/C++ source code generated on  : 01-Apr-2024 16:42:51
//

#ifndef _CODER_HR_FILTER_API_H
#define _CODER_HR_FILTER_API_H

// Include Files
#include "emlrt.h"
#include "mex.h"
#include "tmwtypes.h"
#include <algorithm>
#include <cstring>

// Type Definitions
struct hr_filter_api {
  real32_T x_data[2048];
  real32_T y_data[2048];
};

struct hr_filterStackData {
  hr_filter_api f0;
};

// Variable Declarations
extern emlrtCTX emlrtRootTLSGlobal;
extern emlrtContext emlrtContextGlobal;

// Function Declarations
void b_hr_filter_api(hr_filterStackData *SD, const mxArray *prhs,
                     const mxArray **plhs);

void hr_filter(hr_filterStackData *SD, real32_T x_data[], int32_T x_size[2],
               real32_T y_data[], int32_T y_size[2]);

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
