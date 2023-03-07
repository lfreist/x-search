// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/MetaFile.h>
#include <xsearch/tasks/base/DataProvider.h>
#include <xsearch/utils/Semaphore.h>

#include <functional>
#include <mutex>
#include <string>

namespace xs::task::reader {

/**
 * FileReader: base class for DataProviders that read data from a file.
 * @tparam T
 */
template <typename T>
class FileReader : public base::DataProvider<T> {
 public:
  explicit FileReader(std::string file_path, bool read_binary = false)
      : _file_path(std::move(file_path)) {}

 protected:
  const std::string _file_path;
};

/**
 * MetaReader: base class for DataProviders that access metadata from
 *  xs::MetaFile
 */
class MetaReader {
 public:
  /**
   * Constructor:
   *  We allow reading with multiple threads because we know the positions and
   *  size of the chunks read from a file from the meta data. Thus, we can open
   *  multiple streams to the given file and read simultaneously.
   *  If files are accessed from drives like HDDs, simultaneous reads can
   *  decrease performance heavily (because of the magnetic reader head that
   *  might jump around for while reading different locations of the drive at
   *  the same time...).
   *  Therefor, we introduce the max_readers setting that restricts the number
   *  of simultaneous reads to the provided number using a semaphore.
   * @param meta_file_path
   * @param max_readers
   */
  MetaReader(std::string meta_file_path, int max_readers);

 protected:
  MetaFile _meta_file;
  /// controls simultaneous read operations
  utils::Semaphore<std::function<void(void)>> _semaphore;
};

/**
 * FileBlockMetaReader: inherits FileReader<DataChunk> and MetaReader.
 *  Read data from a file using std::ifstream.
 */
class FileBlockMetaReader : public FileReader<DataChunk>, MetaReader {
 public:
  FileBlockMetaReader(std::string file_path, std::string meta_file_path,
                      int max_readers = 1);

  std::optional<std::pair<DataChunk, chunk_index>> getNextData() override;
};

/**
 * FileBlockMetaReaderMMAP: inherits FileReader<DataChunk> and MetaReader.
 *  Read data from a file using memory mapping.
 *
 *  @remark: if the chunk sizes provided by the meta file are considered to
 *  be too small for mmap, data are read using std::ifstream!
 */
class FileBlockMetaReaderMMAP : public FileReader<DataChunk>, MetaReader {
 public:
  FileBlockMetaReaderMMAP(std::string file_path, std::string meta_file_path,
                          int max_readers = 1);

  std::optional<std::pair<DataChunk, chunk_index>> getNextData() override;

 private:
  std::optional<std::pair<DataChunk, chunk_index>> read_no_mmap(
      ChunkMetaData cmd);
};

/**
 * FileBlockReader: inherits FileReader<DataChunk>
 *  Since we do not know the final size of a chunk (we start reading min_size
 *  bytes and continue until we read a new line character), this reader is
 *  restricted to only ONE reading operation at the same time.
 *  Reads data using std::ifstream.
 */
class FileBlockReader : public FileReader<DataChunk> {
 public:
  explicit FileBlockReader(std::string file_path, size_t min_size = 16777216,
                           size_t max_oversize = 65536,
                           bool read_binary = false);

  std::optional<std::pair<DataChunk, chunk_index>> getNextData() override;

 private:
  const size_t _min_size;
  const size_t _max_oversize;
  uint64_t _current_index = 0;
  uint64_t _current_offset = 0;
  std::ifstream _file_stream;
  std::unique_ptr<std::mutex> _stream_mutex = std::make_unique<std::mutex>();
};

/**
 * FileBlockReaderMMAP:inherits FileReader<DataChunk>
 *  Since we do not know the final size of a chunk (we start reading min_size
 *  bytes and continue until we read a new line character), this reader is
 *  restricted to only ONE reading operation at the same time.
 *  Reads data using memory mapping.
 *
 *  @remark: if min_size is considered to be too small for memory mapping, or if
 *   memory mapping fails for any other reason, data are read using
 *   std::ifstream.
 */
class FileBlockReaderMMAP : public FileReader<DataChunk> {
 public:
  explicit FileBlockReaderMMAP(std::string file_path,
                               size_t min_size = 16777216,
                               size_t max_oversize = 65536,
                               bool read_binary = false);

  std::optional<std::pair<DataChunk, chunk_index>> getNextData() override;

 private:
  std::optional<std::pair<DataChunk, chunk_index>> read_no_mmap();

  const size_t _min_size;
  const size_t _max_oversize;
  const size_t _mmap_read_size;
  size_t _file_size;
  chunk_index _current_index = 0;
  uint64_t _current_offset = 0;
  std::unique_ptr<std::mutex> _stream_mutex = std::make_unique<std::mutex>();
};

}  // namespace xs::task::reader