add_executable(count_matches count_matches.cpp)
target_link_libraries(count_matches PRIVATE nanobench re2::re2 xsearch)

add_executable(count_matches_icase count_matches_icase.cpp)
target_link_libraries(count_matches_icase PRIVATE nanobench re2::re2 xsearch)