add_library(StringUtils string_utils.cpp)
target_compile_options(StringUtils PUBLIC "-mavx2")