add_library(SearchUtils offset_mappings.cpp search_wrappers.cpp simd_search.cpp)
target_compile_options(SearchUtils PUBLIC "-mavx2")
target_link_libraries(SearchUtils PUBLIC re2::re2 InlineBench)