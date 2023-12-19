#add_executable(example_grep grep.cpp)
#target_link_libraries(example_grep PUBLIC xsearch boost_program_options)

add_executable(playground playground.cpp)
target_link_libraries(playground xsearch)