add_library(simd_search simd_search.cpp)
target_compile_options(simd_search PUBLIC "-mavx2")
target_link_libraries(simd_search PUBLIC re2::re2)

add_library(xsearch::simd_search ALIAS simd_search)