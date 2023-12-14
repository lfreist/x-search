// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/utils/Semaphore.h>

#include <cstdint>
#include <functional>
#include <optional>
#include <utility>

namespace xs {

typedef uint64_t chunk_index;

}

namespace xs::task::base {

/**
 * BaseDataProvider: Vase class that must be inherited by all primary data
 *  producing tasks.
 *  E.g.: tasks that read data from a file
 * @tparam DataT
 */
template <typename T>
class DataProvider {
 public:
  DataProvider() = default;
  /**
   * Constructor:
   *  We allow reading with multiple threads.
   *
   * It is important to note, that reading concurrently from secondary memory
   *  with bad random access (like HDDs) decreases reading performance heavily
   *  (because of the magnetic reader head that might jump around while reading
   *  different locations of the drive at the same time...). Therefor, we
   *  introduce the max_readers setting that restricts the number of
   *  simultaneous reads to the provided number using a semaphore.
   * @param meta_file_path
   * @param max_readers
   */
  explicit DataProvider(int max_readers)
      : _semaphore(max_readers), _max_readers(max_readers){};
  virtual ~DataProvider() = default;

  virtual std::optional<std::pair<T, chunk_index>> getNextData() = 0;

  [[nodiscard]] int get_max_readers() const { return _max_readers; }

 protected:
  /// controls simultaneous read operations
  utils::Semaphore _semaphore;
  int _max_readers = 1;
};

}  // namespace xs::task::base