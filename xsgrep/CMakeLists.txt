add_subdirectory(tasks)

add_executable(xs Main.cpp)
target_link_libraries(xs PUBLIC GrepTasks xsearch boost_program_options)