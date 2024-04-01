//
// File: HeartRateFilter.h
//
// MATLAB Coder version            : 23.2
// C/C++ source code generated on  : 01-Apr-2024 16:42:51
//

#ifndef HEARTRATEFILTER_H
#define HEARTRATEFILTER_H

// Include Files
#include "hr_filter_types.h"
#include "rtwtypes.h"
#include <cstddef>
#include <cstdlib>

// Type Definitions
namespace gen {
class HeartRateFilter {
public:
  HeartRateFilter();
  ~HeartRateFilter();
  /**
   * @brief filters input x and returns output y.
   * @param x_data input data
   * @param x_size input data size
   * @param y_data output data
   * @note y_data must be pre-allocated with the same size as x_data
   * @note x_data and y_data seems can be the same array
   * @warning this code is generated from MATLAB code. I have totally no idea
   * what and how it works
   */
  void hr_filter(const real32_T x_data[], int32_T x_size, real32_T y_data[]);
  hr_filterStackData *getStackData();

private:
  hr_filterPersistentData pd_;
  hr_filterStackData SD_;
};

} // namespace gen

#endif
//
// File trailer for HeartRateFilter.h
//
// [EOF]
//
