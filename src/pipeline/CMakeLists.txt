add_library(DefaultTasks readers/BlockReader.cpp searchers/Searcher.cpp result_collectors/grep.cpp processors/decompressors.cpp)
target_link_libraries(DefaultTasks PUBLIC SearchUtils DataChunk zstd lz4)
add_library(TaskManager TaskManager.cpp Task.cpp)
