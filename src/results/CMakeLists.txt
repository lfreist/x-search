add_subdirectory(base)

add_library(Results Results.cpp)
target_link_libraries(Results BaseResults)