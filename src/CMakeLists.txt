add_subdirectory(utils)
add_subdirectory(string_search)

add_library(xsearch xsearch.cpp)
target_link_libraries(xsearch PUBLIC StringUtils SearchUtils)
