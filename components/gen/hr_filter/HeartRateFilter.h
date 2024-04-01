//
// File: HeartRateFilter.h
//
// MATLAB Coder version            : 23.2
// C/C++ source code generated on  : 01-Apr-2024 15:34:30
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
  void hr_filter(const float x_data[], const int x_size[2], float y_data[],
                 int y_size[2]);
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
