add_library(SearchUtils simd_search.cpp)
target_compile_options(SearchUtils PUBLIC "-mavx2")
target_link_libraries(SearchUtils PUBLIC re2::re2)