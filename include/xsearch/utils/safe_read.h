// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>

#include <cstddef>
#include <cstdint>

namespace xs::utils::read {

DataChunk safe_read(int fd, size_t size, size_t offset, size_t file_size);

DataChunk mmap_with_fallback(int fd, size_t size, size_t offset,
                             size_t file_size);

}  // namespace xs::utils::read