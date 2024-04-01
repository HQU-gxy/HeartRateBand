//
// File: _coder_hr_filter_api.cpp
//
// MATLAB Coder version            : 23.2
// C/C++ source code generated on  : 01-Apr-2024 16:42:51
//

// Include Files
#include "_coder_hr_filter_api.h"
#include "_coder_hr_filter_mex.h"

// Variable Definitions
emlrtCTX emlrtRootTLSGlobal{nullptr};

emlrtContext emlrtContextGlobal{
    true,                                                 // bFirstTime
    false,                                                // bInitialized
    131643U,                                              // fVersionInfo
    nullptr,                                              // fErrorFunction
    "hr_filter",                                          // fFunctionName
    nullptr,                                              // fRTCallStack
    false,                                                // bDebugMode
    {2045744189U, 2170104910U, 2743257031U, 4284093946U}, // fSigWrd
    nullptr                                               // fSigMem
};

// Function Declarations
static void b_emlrt_marshallIn(const emlrtStack &sp, const mxArray *src,
                               const emlrtMsgIdentifier *msgId,
                               real32_T ret_data[], int32_T ret_size[2]);

static void emlrt_marshallIn(const emlrtStack &sp, const mxArray *b_nullptr,
                             const char_T *identifier, real32_T y_data[],
                             int32_T y_size[2]);

static void emlrt_marshallIn(const emlrtStack &sp, const mxArray *u,
                             const emlrtMsgIdentifier *parentId,
                             real32_T y_data[], int32_T y_size[2]);

static const mxArray *emlrt_marshallOut(const real32_T u_data[],
                                        const int32_T u_size[2]);

// Function Definitions
//
// Arguments    : const emlrtStack &sp
//                const mxArray *src
//                const emlrtMsgIdentifier *msgId
//                real32_T ret_data[]
//                int32_T ret_size[2]
// Return Type  : void
//
static void b_emlrt_marshallIn(const emlrtStack &sp, const mxArray *src,
                               const emlrtMsgIdentifier *msgId,
                               real32_T ret_data[], int32_T ret_size[2])
{
  static const int32_T dims[2]{1, 2048};
  int32_T iv[2];
  boolean_T bv[2]{false, true};
  emlrtCheckVsBuiltInR2012b((emlrtConstCTX)&sp, msgId, src, "single|double",
                            false, 2U, (const void *)&dims[0], &bv[0], &iv[0]);
  ret_size[0] = iv[0];
  ret_size[1] = iv[1];
  emlrtImportArrayR2015b((emlrtConstCTX)&sp, src, &ret_data[0], 4, false);
  emlrtDestroyArray(&src);
}

//
// Arguments    : const emlrtStack &sp
//                const mxArray *b_nullptr
//                const char_T *identifier
//                real32_T y_data[]
//                int32_T y_size[2]
// Return Type  : void
//
static void emlrt_marshallIn(const emlrtStack &sp, const mxArray *b_nullptr,
                             const char_T *identifier, real32_T y_data[],
                             int32_T y_size[2])
{
  emlrtMsgIdentifier thisId;
  thisId.fIdentifier = const_cast<const char_T *>(identifier);
  thisId.fParent = nullptr;
  thisId.bParentIsCell = false;
  emlrt_marshallIn(sp, emlrtAlias(b_nullptr), &thisId, y_data, y_size);
  emlrtDestroyArray(&b_nullptr);
}

//
// Arguments    : const emlrtStack &sp
//                const mxArray *u
//                const emlrtMsgIdentifier *parentId
//                real32_T y_data[]
//                int32_T y_size[2]
// Return Type  : void
//
static void emlrt_marshallIn(const emlrtStack &sp, const mxArray *u,
                             const emlrtMsgIdentifier *parentId,
                             real32_T y_data[], int32_T y_size[2])
{
  b_emlrt_marshallIn(sp, emlrtAlias(u), parentId, y_data, y_size);
  emlrtDestroyArray(&u);
}

//
// Arguments    : const real32_T u_data[]
//                const int32_T u_size[2]
// Return Type  : const mxArray *
//
static const mxArray *emlrt_marshallOut(const real32_T u_data[],
                                        const int32_T u_size[2])
{
  const mxArray *m;
  const mxArray *y;
  int32_T iv[2];
  int32_T i;
  real32_T *pData;
  y = nullptr;
  iv[0] = u_size[0];
  iv[1] = u_size[1];
  m = emlrtCreateNumericArray(2, &iv[0], mxSINGLE_CLASS, mxREAL);
  pData = static_cast<real32_T *>(emlrtMxGetData(m));
  i = 0;
  for (int32_T b_i{0}; b_i < u_size[1]; b_i++) {
    int32_T c_i;
    c_i = 0;
    while (c_i < u_size[0]) {
      pData[i] = u_data[u_size[0] * b_i];
      i++;
      c_i = 1;
    }
  }
  emlrtAssign(&y, m);
  return y;
}

//
// Arguments    : hr_filterStackData *SD
//                const mxArray *prhs
//                const mxArray **plhs
// Return Type  : void
//
void b_hr_filter_api(hr_filterStackData *SD, const mxArray *prhs,
                     const mxArray **plhs)
{
  emlrtStack st{
      nullptr, // site
      nullptr, // tls
      nullptr  // prev
  };
  int32_T x_size[2];
  int32_T y_size[2];
  st.tls = emlrtRootTLSGlobal;
  // Marshall function inputs
  emlrt_marshallIn(st, emlrtAliasP(prhs), "x", SD->f0.x_data, x_size);
  // Invoke the target function
  hr_filter(SD, SD->f0.x_data, x_size, SD->f0.y_data, y_size);
  // Marshall function outputs
  *plhs = emlrt_marshallOut(SD->f0.y_data, y_size);
}

//
// Arguments    : void
// Return Type  : void
//
void hr_filter_atexit()
{
  emlrtStack st{
      nullptr, // site
      nullptr, // tls
      nullptr  // prev
  };
  mexFunctionCreateRootTLS();
  st.tls = emlrtRootTLSGlobal;
  emlrtEnterRtStackR2012b(&st);
  emlrtDestroyRootTLS(&emlrtRootTLSGlobal);
  hr_filter_xil_terminate();
  hr_filter_xil_shutdown();
  emlrtExitTimeCleanup(&emlrtContextGlobal);
}

//
// Arguments    : void
// Return Type  : void
//
void hr_filter_initialize()
{
  emlrtStack st{
      nullptr, // site
      nullptr, // tls
      nullptr  // prev
  };
  mexFunctionCreateRootTLS();
  st.tls = emlrtRootTLSGlobal;
  emlrtClearAllocCountR2012b(&st, false, 0U, nullptr);
  emlrtEnterRtStackR2012b(&st);
  emlrtFirstTimeR2012b(emlrtRootTLSGlobal);
}

//
// Arguments    : void
// Return Type  : void
//
void hr_filter_terminate()
{
  emlrtDestroyRootTLS(&emlrtRootTLSGlobal);
}

//
// File trailer for _coder_hr_filter_api.cpp
//
// [EOF]
//
