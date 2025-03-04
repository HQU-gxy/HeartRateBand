//
// File: _coder_hr_filter_mex.cpp
//
// MATLAB Coder version            : 23.2
// C/C++ source code generated on  : 01-Apr-2024 16:42:51
//

// Include Files
#include "_coder_hr_filter_mex.h"
#include "_coder_hr_filter_api.h"

// Function Definitions
//
// Arguments    : int32_T nlhs
//                mxArray *plhs[]
//                int32_T nrhs
//                const mxArray *prhs[]
// Return Type  : void
//
void mexFunction(int32_T nlhs, mxArray *plhs[], int32_T nrhs,
                 const mxArray *prhs[])
{
  hr_filterStackData *hr_filterStackDataGlobal{nullptr};
  hr_filterStackDataGlobal =
      static_cast<hr_filterStackData *>(new hr_filterStackData);
  mexAtExit(&hr_filter_atexit);
  // Module initialization.
  hr_filter_initialize();
  // Dispatch the entry-point.
  unsafe_hr_filter_mexFunction(hr_filterStackDataGlobal, nlhs, plhs, nrhs,
                               prhs);
  // Module termination.
  hr_filter_terminate();
  delete hr_filterStackDataGlobal;
}

//
// Arguments    : void
// Return Type  : emlrtCTX
//
emlrtCTX mexFunctionCreateRootTLS()
{
  emlrtCreateRootTLSR2022a(&emlrtRootTLSGlobal, &emlrtContextGlobal, nullptr, 1,
                           nullptr, "UTF-8", true);
  return emlrtRootTLSGlobal;
}

//
// Arguments    : hr_filterStackData *SD
//                int32_T nlhs
//                mxArray *plhs[1]
//                int32_T nrhs
//                const mxArray *prhs[1]
// Return Type  : void
//
void unsafe_hr_filter_mexFunction(hr_filterStackData *SD, int32_T nlhs,
                                  mxArray *plhs[1], int32_T nrhs,
                                  const mxArray *prhs[1])
{
  emlrtStack st{
      nullptr, // site
      nullptr, // tls
      nullptr  // prev
  };
  const mxArray *outputs;
  st.tls = emlrtRootTLSGlobal;
  // Check for proper number of arguments.
  if (nrhs != 1) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:WrongNumberOfInputs", 5, 12, 1, 4,
                        9, "hr_filter");
  }
  if (nlhs > 1) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:TooManyOutputArguments", 3, 4, 9,
                        "hr_filter");
  }
  // Call the function.
  b_hr_filter_api(SD, prhs[0], &outputs);
  // Copy over outputs to the caller.
  emlrtReturnArrays(1, &plhs[0], &outputs);
}

//
// File trailer for _coder_hr_filter_mex.cpp
//
// [EOF]
//
