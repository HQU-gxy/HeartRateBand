set(GIT_DIR_LOOKUP_POLICY ALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR)
idf_component_register(
  SRCS
  hr_filter/HeartRateFilter.cpp
  hr_filter/BiquadFilter.cpp
  hr_filter/interface/_coder_hr_filter_api.cpp
  hr_filter/interface/_coder_hr_filter_info.cpp
  hr_filter/interface/_coder_hr_filter_mex.cpp
  INCLUDE_DIRS
  hr_filter
  hr_filter/interface
)
