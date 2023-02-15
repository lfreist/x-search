add_library(StringManipulation string_manipulation.cpp)
target_compile_options(StringManipulation PUBLIC "-mavx2")