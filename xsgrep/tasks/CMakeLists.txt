add_library(GrepTasks GrepOutput.cpp GrepSearcher.cpp)
target_link_libraries(GrepTasks PUBLIC xsearch)