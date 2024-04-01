//
// File: main.cpp
//
// MATLAB Coder version            : 23.2
// C/C++ source code generated on  : 01-Apr-2024 16:42:51
//

/*************************************************************************/
/* This automatically generated example C++ main file shows how to call  */
/* entry-point functions that MATLAB Coder generated. You must customize */
/* this file for your application. Do not modify this file directly.     */
/* Instead, make a copy of this file, modify it, and integrate it into   */
/* your development environment.                                         */
/*                                                                       */
/* This file initializes entry-point function arguments to a default     */
/* size and value before calling the entry-point functions. It does      */
/* not store or use any values returned from the entry-point functions.  */
/* If necessary, it does pre-allocate memory for returned values.        */
/* You can use this file as a starting point for a main function that    */
/* you can deploy in your application.                                   */
/*                                                                       */
/* After you copy the file, and before you deploy it, you must make the  */
/* following changes:                                                    */
/* * For variable-size function arguments, change the example sizes to   */
/* the sizes that your application requires.                             */
/* * Change the example values of function arguments to the values that  */
/* your application requires.                                            */
/* * If the entry-point functions return values, store these values or   */
/* otherwise use them as required by your application.                   */
/*                                                                       */
/*************************************************************************/

// Include Files
#include "main.h"
#include "HeartRateFilter.h"

// Function Declarations
static void argInit_1xd2048_real32_T(real32_T result_data[],
                                     int32_T result_size[2]);

static real32_T argInit_real32_T();

// Function Definitions
//
// Arguments    : real32_T result_data[]
//                int32_T result_size[2]
// Return Type  : void
//
static void argInit_1xd2048_real32_T(real32_T result_data[],
                                     int32_T result_size[2])
{
  // Set the size of the array.
  // Change this size to the value that the application requires.
  result_size[0] = 1;
  result_size[1] = 2;
  // Loop over the array to initialize each element.
  for (int32_T idx1{0}; idx1 < 2; idx1++) {
    // Set the value of the array element.
    // Change this value to the value that the application requires.
    result_data[idx1] = argInit_real32_T();
  }
}

//
// Arguments    : void
// Return Type  : real32_T
//
static real32_T argInit_real32_T()
{
  return 0.0F;
}

//
// Arguments    : int32_T argc
//                char **argv
// Return Type  : int32_T
//
int32_T main(int32_T, char **)
{
  gen::HeartRateFilter *classInstance;
  classInstance = new gen::HeartRateFilter;
  // Invoke the entry-point functions.
  // You can call entry-point functions multiple times.
  main_hr_filter(classInstance);
  delete classInstance;
  return 0;
}

//
// Arguments    : gen::HeartRateFilter *instancePtr
// Return Type  : void
//
void main_hr_filter(gen::HeartRateFilter *instancePtr)
{
  static real32_T x_data[2048];
  static real32_T y_data[2048];
  int32_T x_size[2];
  int32_T y_size[2];
  // Initialize function 'hr_filter' input arguments.
  // Initialize function input argument 'x'.
  argInit_1xd2048_real32_T(x_data, x_size);
  // Call the entry-point 'hr_filter'.
  instancePtr->hr_filter(x_data, x_size, y_data, y_size);
}

//
// File trailer for main.cpp
//
// [EOF]
//
