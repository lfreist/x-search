add_library(StringManipulation string_utils.cpp)
target_compile_options(StringManipulation PUBLIC "-mavx2")