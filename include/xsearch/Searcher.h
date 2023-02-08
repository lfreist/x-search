// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/ResultTypes.h>
#include <xsearch/tasks/DataProvider.h>
#include <xsearch/tasks/Processor.h>
#include <xsearch/tasks/Searcher.h>
#include <xsearch/utils/InlineBench.h>
#include <xsearch/utils/utils.h>

#include <memory>
#include <semaphore>
#include <string>
#include <thread>
#include <vector>

namespace xs {

template <class DataT, class ResType, class PartResT>
class Searcher {
 public:
  class iterator {
   public:
    iterator(Searcher<DataT, ResType, PartResT>& searcher, size_t index)
        : _searcher(searcher), _index(index) {}

    PartResT& operator*() {
      std::unique_lock lock(_searcher._results_mutex);
      return _searcher._result[_index];
    }

    iterator& operator++() {
      _index++;
      return *this;
    }

    bool operator!=(const iterator& other) {
      std::unique_lock locker(_searcher._results_mutex);
      while (_index >= _searcher._result._merged_result.size()) {
        if (!_searcher.isRunning()) {
          // no results will be added and index is out of range
          return false;
        }
        _searcher._results_condition_variable.wait(locker);
      }
      if (!_searcher.isRunning()) {
        return _index <= _searcher._result._merged_result.size();
      }
      return true;
    }

   private:
    Searcher<DataT, ResType, PartResT>& _searcher;
    size_t _index;
  };

 public:
  // ---------------------------------------------------------------------------
  Searcher(std::string pattern, int num_threads, int max_readers,
           std::unique_ptr<tasks::BaseDataProvider<DataT>> reader,
           std::vector<std::unique_ptr<tasks::BaseProcessor<DataT>>> processors,
           std::vector<std::unique_ptr<tasks::BaseSearcher<DataT, PartResT>>>
               searchers,
           std::unique_ptr<ResType> initial_result)
      : _result(std::move(initial_result)),
        _reader(std::move(reader)),
        _processors(std::move(processors)),
        _searchers(std::move(searchers)) {
    if (max_readers > num_threads) {
      max_readers = num_threads;
    }
    _threads.resize(num_threads);
    _max_readers = max_readers;
    _pattern = std::move(pattern);
    _regex = xs::utils::use_str_as_regex(_pattern);
    if (_regex) {
      _regex_pattern = std::make_unique<re2::RE2>("(" + _pattern + ")");
    }
    _running = true;
    _workers = num_threads;
    for (auto& t : _threads) {
      t = std::thread(&Searcher::main_task, this);
    }
  }

  ~Searcher() { join(); }

  ResType* getResult() {
    std::unique_lock lock(_results_mutex);
    return _result.get();
  }

  bool isRunning() { return _running.load(); }

  void join() {
    for (auto& t : _threads) {
      if (t.joinable()) {
        t.join();
      }
    }
    _running.store(false);
  }

 private:
  void main_task() {
    size_t num_chunks_read = 0;
    while (true) {
      auto chunks = reader_task();
      if (chunks.empty()) {
        break;
      }
      num_chunks_read += chunks.size();
      processors_task(chunks);
      auto partial_results = searchers_task(chunks);
      results_join_task(partial_results);
    }
    if (_workers.fetch_sub(1) == 1) {
      _running.store(false);
      std::unique_lock locker(_results_mutex);
      _results_condition_variable.notify_all();
    }
  }

  std::vector<DataT> reader_task() {
    std::unique_lock reader_lock(_reader_worker_mutex);
    while (true) {
      {
        std::unique_lock check_lock(_check_reader_worker_mutex);
        if (_max_readers > 0) {
          break;
        }
      }
      _reader_cv.wait(reader_lock);
    }
    _max_readers--;
    reader_lock.unlock();
    INLINE_BENCHMARK_WALL_START("reader task");
    auto chunks = _reader->getNextData(1);
    INLINE_BENCHMARK_WALL_STOP("reader task");
    reader_lock.lock();
    _max_readers++;
    _reader_cv.notify_one();
    return chunks;
  }

  void processors_task(std::vector<DataT>& chunks) {
    INLINE_BENCHMARK_WALL_START("processor task");
    for (auto& chunk : chunks) {
      for (auto& processor : _processors) {
        processor->process(&chunk);
      }
    }
    INLINE_BENCHMARK_WALL_STOP("processor task");
  }

  std::vector<PartResT> searchers_task(std::vector<DataT>& chunks) {
    INLINE_BENCHMARK_WALL_START("searcher task");
    std::vector<PartResT> partial_results(chunks.size());
    for (size_t index = 0; index < chunks.size(); ++index) {
      for (auto& searcher : _searchers) {
        if (_regex) {
          searcher->search(_regex_pattern.get(), &chunks[index],
                           &partial_results[index]);
        } else {
          searcher->search(_pattern, &chunks[index], &partial_results[index]);
        }
      }
    }
    INLINE_BENCHMARK_WALL_STOP("searcher task");
    return partial_results;
  }

  void results_join_task(std::vector<PartResT>& partial_results) {
    INLINE_BENCHMARK_WALL_START("results join task");
    std::unique_lock locker(_results_mutex);
    for (auto& part_res : partial_results) {
      _result->addPartialResult(std::move(part_res));
      _results_condition_variable.notify_one();
    }
    INLINE_BENCHMARK_WALL_STOP("results join task");
  }

  std::string _file_path;
  std::string _meta_file_path;
  std::string _pattern;
  bool _regex = false;
  std::unique_ptr<re2::RE2> _regex_pattern;

  std::unique_ptr<ResType> _result;
  std::mutex _results_mutex;
  std::condition_variable _results_condition_variable;

  std::unique_ptr<tasks::BaseDataProvider<DataT>> _reader;
  std::vector<std::unique_ptr<tasks::BaseProcessor<DataT>>> _processors;
  std::vector<std::unique_ptr<tasks::BaseSearcher<DataT, PartResT>>> _searchers;

  std::vector<std::thread> _threads;

  utils::TSQueue<PartResT> _results_queue;

  std::mutex _reader_worker_mutex;
  std::mutex _check_reader_worker_mutex;
  std::condition_variable _reader_cv;
  int _max_readers = 2;

  std::atomic<bool> _running = false;
  std::atomic<int> _workers;
};

}  // namespace xs