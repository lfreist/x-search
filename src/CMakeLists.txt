add_subdirectory(utils)
add_subdirectory(string_search)
add_library(DataChunk DataChunk.cpp)
add_library(MetaFile MetaFile.cpp)

add_library(Executor Executor.cpp)
target_link_libraries(Executor PUBLIC DataChunk MetaFile)

add_library(xsearch xsearch.cpp)
target_link_libraries(xsearch PUBLIC StringUtils Executor SearchUtils lz4 zstd)
