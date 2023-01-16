// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/ResultTypes.h>

#include <string>
#include <vector>

namespace xs {

/**
 * Result class: Result collects and merges partial results.
 * Class is enabled for:
 *  - restype::byte_positions
 *  - restype::line_numbers
 *  - restype::line_indices
 *  - restype::lines
 *  - restype::count
 *  - restype::full
 *
 * @tparam T
 * @tparam Enable
 */
template <typename T, typename Enable = void>
class Result {
  Result() = default;
};

/**
 * Result class for count matches only.
 * @tparam T
 */
template <typename T>
class Result<T, typename std::enable_if<
                    std::is_same<T, restype::count>::value ||
                    std::is_same<T, restype::count_lines>::value>::type> {
 public:
  class iterator : public std::iterator<std::forward_iterator_tag, size_t> {
   public:
    explicit iterator(Result& r) : _result(r) {}

    size_t operator*() {
      std::unique_lock locker(*(_result._mutex));
      return _result._result;
    }

    iterator& operator++() {
      std::unique_lock locker(*(_result._mutex));
      _result->_cv.get()->wait(
          locker, [&]() { return _result._changed || _result._done; });
      _result->_changed = false;
      return *this;
    }

    bool operator!=(const iterator&) {
      std::unique_lock locker(*(_result._mutex));
      return !_result._done;
    }

   private:
    Result& _result;
  };

  Result() = default;

  iterator begin() { return iterator(*this); }

  iterator end() { return iterator(*this); }

  /**
   * Get the sum of collected partial count results.
   * @return size_t
   */
  [[nodiscard]] size_t getResult() const { return _result; }

  /**
   * Add the count of matches of a partial result.
   * @param data
   */
  void addPartialResult(DataChunk* data);

 private:
  size_t _result = 0;
  bool _changed = true;
  bool _done = false;
  std::unique_ptr<std::mutex> _mutex = std::make_unique<std::mutex>();
  std::unique_ptr<std::condition_variable> _cv =
      std::make_unique<std::condition_variable>();
};

/**
 * Result class for byte_positions, line_numbers or line_indices
 * @tparam T
 */
template <typename T>
class Result<T, typename std::enable_if<
                    std::is_same<T, restype::byte_positions>::value ||
                    std::is_same<T, restype::line_numbers>::value ||
                    std::is_same<T, restype::line_indices>::value>::type> {
 public:
  class iterator : public std::iterator<std::forward_iterator_tag, size_t> {
   public:
    explicit iterator(Result& r) : _result(r) {}

    size_t operator*() {
      std::unique_lock locker(*(_result._mutex));
      return _result._result.back();
    }

    iterator& operator++() {
      std::unique_lock locker(*(_result._mutex));
      _result->_cv.get()->wait(
          locker, [&]() { return _result._changed || _result._done; });
      _result->_changed = false;
      return *this;
    }

    bool operator!=(const iterator&) {
      std::unique_lock locker(*(_result._mutex));
      return !_result._done;
    }

   private:
    Result& _result;
  };

  Result() = default;

  /**
   * Get a merged result of collected partial results.
   * @return std::vector<size_t>
   */
  [[nodiscard]] const std::vector<size_t>& getResult() const { return _result; }

  /**
   * Get a merged result of collected partial results.
   * !!!
   *   MARK: the result is moved here.
   *    Consider using the const getResult() method for retrieving a reference.
   * !!!
   * @return std::vector<size_t>
   */
  [[nodiscard]] std::vector<size_t> getResult() { return std::move(_result); }

  /**
   * Add the results of a partial results to the overall result.
   * @param data
   */
  void addPartialResult(DataChunk* data);

 private:
  std::vector<size_t> _result;
  bool _changed = true;
  bool _done = false;
  std::unique_ptr<std::mutex> _mutex = std::make_unique<std::mutex>();
  std::unique_ptr<std::condition_variable> _cv =
      std::make_unique<std::condition_variable>();
};

/**
 * Result class for matching lines.
 * @tparam T
 */
template <typename T>
class Result<
    T, typename std::enable_if<std::is_same<T, restype::lines>::value>::type> {
 public:
  class iterator
      : public std::iterator<std::forward_iterator_tag, std::string> {
   public:
    explicit iterator(Result& r) : _result(r) {}

    std::string& operator*() {
      std::unique_lock locker(*(_result._mutex));
      return _result._result.back();
    }

    iterator& operator++() {
      std::unique_lock locker(*(_result._mutex));
      _result->_cv.get()->wait(
          locker, [&]() { return _result._changed || _result._done; });
      _result->_changed = false;
      return *this;
    }

    bool operator!=(const iterator&) {
      std::unique_lock locker(*(_result._mutex));
      return !_result._done;
    }

   private:
    Result& _result;
  };

  Result() = default;

  /**
   * Get the merged result of collected partial results.
   * @return
   */
  [[nodiscard]] const std::vector<std::string>& getResult() const {
    return _result;
  }

  /**
   * Get a merged result of collected partial results.
   * !!!
   *   MARK: the result is moved here.
   *    Consider using the const getResult() method for retrieving a reference.
   * !!!
   * @return std::vector<size_t>
   */
  [[nodiscard]] std::vector<std::string> getResult() {
    return std::move(_result);
  }

  /**
   * Add a partial result to the overall result.
   * @param data
   */
  void addPartialResult(DataChunk* data);

 private:
  std::vector<std::string> _result;
  bool _changed = true;
  bool _done = false;
  std::unique_ptr<std::mutex> _mutex = std::make_unique<std::mutex>();
  std::unique_ptr<std::condition_variable> _cv =
      std::make_unique<std::condition_variable>();
};

/**
 * Result class for all collected results.
 * @tparam T
 */
template <typename T>
class Result<
    T, typename std::enable_if<std::is_same<T, restype::full>::value>::type> {
 public:
  class iterator
      : public std::iterator<std::forward_iterator_tag, SearchResults> {
   public:
    explicit iterator(Result& r) : _result(r) {}

    SearchResults operator*() {
      std::unique_lock locker(*(_result._mutex));
      return _result._result.back();
    }

    iterator& operator++() {
      std::unique_lock locker(*(_result._mutex));
      _result->_cv.get()->wait(
          locker, [&]() { return _result._changed || _result._done; });
      _result->_changed = false;
      return *this;
    }

    bool operator!=(const iterator&) {
      std::unique_lock locker(*(_result._mutex));
      return !_result._done;
    }

   private:
    Result& _result;
  };

  Result() = default;

  /**
   * Get a std::vector of collected partial results.
   * @return
   */
  [[nodiscard]] const std::vector<SearchResults>& getResult() const {
    return _result;
  };

  /**
   * Get a merged result of collected partial results.
   * !!!
   *   MARK: the result is moved here.
   *    Consider using the const getResult() method for retrieving a reference.
   * !!!
   * @return std::vector<size_t>
   */
  [[nodiscard]] std::vector<SearchResults> getResult() {
    return std::move(_result);
  }

  /**
   * Add a partial result.
   * @param data
   */
  void addPartialResult(DataChunk* data);

 private:
  std::vector<SearchResults> _result;
  bool _changed = true;
  bool _done = false;
  std::unique_ptr<std::mutex> _mutex = std::make_unique<std::mutex>();
  std::unique_ptr<std::condition_variable> _cv =
      std::make_unique<std::condition_variable>();
};

}  // namespace xs