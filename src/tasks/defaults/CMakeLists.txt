add_library(DefaultTasks processors.cpp readers.cpp searchers.cpp)
target_link_libraries(DefaultTasks PUBLIC BaseTasks MetaFile DataChunk StringUtils SearchUtils)