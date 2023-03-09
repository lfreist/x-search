add_library(StringUtils string_utils.cpp)
target_compile_options(StringUtils PUBLIC "-mavx2")

add_library(SafeRead safe_read.cpp)
target_link_libraries(SafeRead PUBLIC DataChunk)