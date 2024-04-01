//
// File: HeartRateFilter.h
//
// MATLAB Coder version            : 23.2
// C/C++ source code generated on  : 01-Apr-2024 15:40:09
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
  void hr_filter(const real32_T x_data[], const int32_T x_size[2],
                 real32_T y_data[], int32_T y_size[2]);
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
