// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xsearch/utils/safe_read.h>

namespace xs::utils::read {

// _____________________________________________________________________________
DataChunk read(int fd, size_t size, size_t offset, size_t file_size) {
  size_t max_oversize = 65536;
  DataChunk dc(size + max_oversize);
  ::lseek(fd, offset, SEEK_SET);
  ssize_t bytes_read = ::read(fd, dc.data(), dc.size());
  if (bytes_read < 0) {
    throw std::runtime_error("Error reading file...");
  }
  if (bytes_read + offset == file_size) {
    // EOF read
    dc.resize(bytes_read);
    return dc;
  }
  // Not at EOF --> search first new line char starting at end
  for (;; --bytes_read) {
    if (dc.data()[bytes_read] == '\n') {
      break;
    }
    if (bytes_read == 0) {
      throw std::runtime_error("Error searching new line char while reading");
    }
  }
  dc.resize(bytes_read);
  return dc;
}

// _____________________________________________________________________________
DataChunk mmap_with_fallback(int fd, size_t size, size_t offset,
                             size_t file_size) {
  static size_t page_size = sysconf(_SC_PAGE_SIZE);
  size_t page_offset = offset % page_size;
  void* mapped = ::mmap(nullptr, size + page_offset, PROT_READ, MAP_PRIVATE, fd,
                        offset - page_offset);
  if (mapped == MAP_FAILED) {
    return safe_read(fd, size, offset, file_size);
  }
  char* buffer = static_cast<char*>(mapped);
  if (file_size - offset <= size) {
    // EOF
    DataChunk dc;
    dc.assign_mmap_data(buffer, file_size - offset, page_offset);
    return dc;
  }
  // Not EOF -> search new line character starting at end
  size_t buffer_offset = page_offset + size;
  for (;; --buffer_offset) {
    if (buffer[buffer_offset] == '\n') {
      break;
    }
    if (buffer_offset == 0) {
      throw std::runtime_error("Error searching new line char while reading");
    }
  }
  DataChunk dc;
  dc.assign_mmap_data(static_cast<char*>(mapped), buffer_offset - page_offset,
                      page_offset);
  return dc;
}

}  // namespace xs::utils::read