add_executable(re2_literal re2_literal.cpp)
target_link_libraries(re2_literal re2::re2)

add_executable(std_string_find std_string_find.cpp)

add_executable(std_strstr std_strstr.cpp)

add_executable(simd_strstr simd_strstr.cpp)
target_link_libraries(simd_strstr xsearch)