// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <fcntl.h>
#include <sys/mman.h>
#include <xsearch/tasks/defaults/readers.h>
#include <xsearch/utils/InlineBench.h>
#include <xsearch/utils/string_utils.h>

#include <cstdio>
#include <cstdlib>

namespace xs::task::reader {

// _____________________________________________________________________________
MetaReader::MetaReader(std::string meta_file_path)
    : _meta_file(std::move(meta_file_path), std::ios::in) {}

// _____________________________________________________________________________
FileBlockMetaReader::FileBlockMetaReader(std::string file_path,
                                         std::string meta_file_path,
                                         int max_readers)
    : FileReader<DataChunk>(std::move(file_path), max_readers),
      MetaReader(std::move(meta_file_path)) {}

// _____________________________________________________________________________
std::optional<std::pair<DataChunk, chunk_index>>
FileBlockMetaReader::getNextData() {
  INLINE_BENCHMARK_WALL_START(read, "reading");
  auto optCmd = _meta_file.next_chunk_meta_data();
  if (!optCmd.has_value()) {
    return {};
  }

  xs::DataChunk chunk(std::move(optCmd.value()));

  // semaphore protected read: restricted to max_readers
  _semaphore.access([&]() -> void {
    std::ifstream stream(_file_path);
    stream.seekg(static_cast<int64_t>(chunk.getMetaData().actual_offset),
                 std::ios::beg);
    stream.read(chunk.data(), static_cast<int64_t>(chunk.size()));
  });

  chunk.set_file_name(_file_path);
  return std::make_pair(std::move(chunk), chunk.getMetaData().chunk_index);
}

// _____________________________________________________________________________
FileBlockMetaReaderSingle::FileBlockMetaReaderSingle(std::string file_path,
                                                     std::string meta_file_path)
    : FileReader<DataChunk>(std::move(file_path), 1),
      MetaReader(std::move(meta_file_path)) {
  _file_stream.open(_file_path);
}

// _____________________________________________________________________________
std::optional<std::pair<DataChunk, chunk_index>>
FileBlockMetaReaderSingle::getNextData() {
  std::unique_lock lock(*_stream_mutex);
  INLINE_BENCHMARK_WALL_START(read, "reading");
  auto opt = _meta_file.next_chunk_meta_data();
  if (!opt.has_value()) {
    return {};
  }
  DataChunk chunk(std::move(opt.value()));
  _file_stream.read(chunk.data(), static_cast<int64_t>(chunk.size()));
  chunk.set_file_name(_file_path);
  return std::make_pair(std::move(chunk), chunk.getMetaData().chunk_index);
}

// _____________________________________________________________________________
FileBlockMetaReaderMMAP::FileBlockMetaReaderMMAP(std::string file_path,
                                                 std::string meta_file_path,
                                                 int max_readers)
    : FileReader<DataChunk>(std::move(file_path), max_readers),
      MetaReader(std::move(meta_file_path)) {}

// _____________________________________________________________________________
std::optional<std::pair<DataChunk, chunk_index>>
FileBlockMetaReaderMMAP::getNextData() {
  INLINE_BENCHMARK_WALL_START(read, "reading");
  auto optCmd = _meta_file.next_chunk_meta_data();
  if (!optCmd.has_value()) {
    return {};
  }
  auto cmd = std::move(optCmd.value());

  // do not use mmap if data are too small
  if (cmd.actual_size < static_cast<uint64_t>(1 << 20)) {
    return FileBlockMetaReaderMMAP::read_no_mmap(std::move(cmd));
  }

  size_t page_size = sysconf(_SC_PAGE_SIZE);
  size_t page_offset = cmd.actual_offset % page_size;

  void* mapped;

  // semaphore protected read: restricted to max_readers
  _semaphore.access([&]() {
    int fd = open(_file_path.c_str(), O_RDONLY);
    if (fd < 0) {
      exit(1);
    }
    mapped =
        mmap(nullptr, cmd.actual_size + page_offset, PROT_READ, MAP_PRIVATE, fd,
             static_cast<int64_t>(cmd.actual_offset - page_offset));
    close(fd);
  });

  if (mapped == MAP_FAILED) {
    // mmap failed -> read data using std::ifstream...
    return FileBlockMetaReaderMMAP::read_no_mmap(std::move(cmd));
  }

  xs::DataChunk chunk(std::move(cmd));
  chunk.assign_mmap_data(static_cast<char*>(mapped),
                         chunk.getMetaData().actual_size, page_offset);
  chunk.set_file_name(_file_path);
  return std::make_pair(std::move(chunk), chunk.getMetaData().chunk_index);
}

// _____________________________________________________________________________
std::optional<std::pair<DataChunk, chunk_index>>
FileBlockMetaReaderMMAP::read_no_mmap(ChunkMetaData cmd) {
  xs::DataChunk chunk(std::move(cmd));
  _semaphore.access([&]() {
    std::ifstream stream(_file_path);
    stream.seekg(static_cast<int64_t>(chunk.getMetaData().actual_offset),
                 std::ios::beg);
    stream.read(chunk.data(), static_cast<int64_t>(chunk.size()));
  });
  chunk.set_file_name(_file_path);
  return std::make_pair(std::move(chunk), chunk.getMetaData().chunk_index);
}

// _____________________________________________________________________________
FileBlockReader::FileBlockReader(std::string file_path, size_t min_size,
                                 size_t max_oversize)
    : FileReader<DataChunk>(std::move(file_path)),
      _min_size(min_size),
      _max_oversize(max_oversize),
      _file_stream(_file_path) {
  if (!_file_stream.is_open()) {
    std::cerr << "Could not open file '" << _file_path << "'." << std::endl;
    exit(3);
  }
}

// _____________________________________________________________________________
std::optional<std::pair<DataChunk, chunk_index>>
FileBlockReader::getNextData() {
  std::unique_lock lock(*_stream_mutex);
  INLINE_BENCHMARK_WALL_START(read, "reading");
  ChunkMetaData cmd{_current_index,
                    _current_offset,
                    _current_offset,
                    _min_size + _max_oversize,
                    _min_size + _max_oversize,
                    {}};
  DataChunk chunk(std::move(cmd));
  _file_stream.read(chunk.data(), static_cast<int64_t>(_min_size));
  auto num_bytes_read = _file_stream.gcount();

  if (num_bytes_read > 0 && chunk.data()[num_bytes_read - 1] != '\n' &&
      !_file_stream.eof()) {
    int64_t additional_bytes_read = 0;
    while (true) {
      if (static_cast<size_t>(additional_bytes_read) >= _max_oversize) {
        throw std::runtime_error(
            "ERROR: maximum size exceeded while reading data: " +
            std::to_string(additional_bytes_read + num_bytes_read));
      }
      _file_stream.read(chunk.data() + num_bytes_read + additional_bytes_read,
                        1);
      additional_bytes_read++;
      if (chunk.data()[num_bytes_read + additional_bytes_read - 1] == '\n' ||
          _file_stream.eof()) {
        break;
      }
    }
    num_bytes_read += additional_bytes_read;
  }
  if (num_bytes_read > 0) {
    chunk.resize(num_bytes_read);
    chunk.getMetaData().actual_size = num_bytes_read;
    chunk.getMetaData().original_size = num_bytes_read;
    _current_offset += num_bytes_read;
    chunk.set_file_name(_file_path);
    return std::make_pair(std::move(chunk), _current_index++);
  }
  return {};
}

// _____________________________________________________________________________
FileBlockReaderMMAP::FileBlockReaderMMAP(std::string file_path, size_t min_size,
                                         size_t max_oversize)
    : FileReader<DataChunk>(std::move(file_path)),
      _min_size(min_size),
      _max_oversize(max_oversize),
      // rounding _max size to a factorial of page size
      _mmap_read_size(sysconf(_SC_PAGE_SIZE) *
                      static_cast<int64_t>(
                          ceil(static_cast<double>(min_size + max_oversize) /
                               static_cast<double>(sysconf(_SC_PAGE_SIZE))))) {
  int fd = open(_file_path.c_str(), O_RDONLY);
  _file_size = lseek(fd, 0, SEEK_END);
  close(fd);
}

// _____________________________________________________________________________
std::optional<std::pair<DataChunk, chunk_index>>
FileBlockReaderMMAP::getNextData() {
  std::unique_lock lock(*_stream_mutex);
  INLINE_BENCHMARK_WALL_START(read, "reading");
  if (_current_offset >= _file_size) {
    return {};
  }

  int fd = open(_file_path.c_str(), O_RDONLY);
  if (fd < 0) {
    throw std::runtime_error("ERROR: Could not open file '" + _file_path + "'");
  }

  size_t page_size = sysconf(_SC_PAGE_SIZE);
  size_t page_offset = _current_offset % page_size;
  char* buffer;

  void* mapped =
      mmap(nullptr, _mmap_read_size + page_size, PROT_READ, MAP_PRIVATE, fd,
           static_cast<int64_t>(_current_offset - page_offset));
  close(fd);

  if (mapped == MAP_FAILED) {
    // read data using std::ifstream
    return FileBlockReaderMMAP::read_no_mmap();
  }

  buffer = static_cast<char*>(mapped);
  // search new line char
  size_t size = std::min<size_t>(_file_size - _current_offset, _min_size);

  // get last char of data
  size_t offset = page_offset + size;
  if (size == _min_size) {  // -> we have not read EOF
    while (true) {
      offset++;
      if (offset - page_offset - size > _max_oversize) {
        throw std::runtime_error("ERROR: failed to find new line char.");
      }
      if (buffer[offset] == '\n' ||
          offset - page_offset + _current_offset >= _file_size) {
        offset++;
        break;
      }
    }
  }

  size_t actual_size = offset - page_offset;
  xs::DataChunk chunk;
  chunk.assign_mmap_data(buffer, actual_size, page_offset);
  chunk.getMetaData() = {_current_index, _current_offset, _current_offset,
                         actual_size,    actual_size,     {}};

  _current_offset += actual_size;

  chunk.set_file_name(_file_path);
  return std::make_pair(std::move(chunk), _current_index++);
}

// _____________________________________________________________________________
std::optional<std::pair<DataChunk, chunk_index>>
FileBlockReaderMMAP::read_no_mmap() {
  ChunkMetaData cmd{_current_index,
                    _current_offset,
                    _current_offset,
                    _min_size + _max_oversize,
                    _min_size + _max_oversize,
                    {}};
  DataChunk chunk(std::move(cmd));
  std::ifstream stream(_file_path);
  stream.seekg(static_cast<int64_t>(_current_offset), std::ios::beg);
  stream.read(chunk.data(), static_cast<int64_t>(_min_size));
  auto num_bytes_read = stream.gcount();

  if (num_bytes_read > 0 && chunk.data()[num_bytes_read - 1] != '\n' &&
      !stream.eof()) {
    int64_t additional_bytes_read = 0;
    while (true) {
      if (static_cast<size_t>(additional_bytes_read) > _max_oversize) {
        throw std::runtime_error(
            "ERROR: maximum size exceeded while reading data: " +
            std::to_string(additional_bytes_read + num_bytes_read));
      }
      stream.read(chunk.data() + num_bytes_read + additional_bytes_read, 1);
      additional_bytes_read++;
      if (chunk.data()[num_bytes_read + additional_bytes_read - 1] == '\n' ||
          stream.eof()) {
        break;
      }
    }
    num_bytes_read += additional_bytes_read;
  }
  if (num_bytes_read > 0) {
    chunk.resize(num_bytes_read);
    chunk.getMetaData().actual_size = num_bytes_read;
    chunk.getMetaData().original_size = num_bytes_read;
    _current_offset += num_bytes_read;
    chunk.set_file_name(_file_path);
    return std::make_pair(std::move(chunk), _current_index++);
  }
  return {};
}

}  // namespace xs::task::reader