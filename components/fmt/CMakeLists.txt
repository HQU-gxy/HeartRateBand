set(GIT_DIR_LOOKUP_POLICY ALLOW_LOOKING_ABOVE_CMAKE_SOURCE_DIR)
idf_component_register(
  INCLUDE_DIRS
  fmt/include
)

target_compile_definitions(${COMPONENT_LIB} INTERFACE -DFMT_HEADER_ONLY=1 -DFMT_OS=0)
