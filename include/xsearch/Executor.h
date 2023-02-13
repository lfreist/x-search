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

template <class DataT, class ResT, class PartResT, typename... ResArgs>
class Executor {
 public:
  // ---------------------------------------------------------------------------
  Executor(std::string pattern, int num_threads, int max_readers,
           std::unique_ptr<tasks::BaseDataProvider<DataT>> reader,
           std::vector<std::unique_ptr<tasks::BaseProcessor<DataT>>> processors,
           std::unique_ptr<tasks::BaseSearcher<DataT, PartResT>> searchers,
           ResArgs&&... result_args)
      : _result(new ResT(std::forward<ResArgs>(result_args)...)),
        _reader(std::move(reader)),
        _processors(std::move(processors)),
        _searcher(std::move(searchers)) {
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
      t = std::thread(&Executor::main_task, this);
    }
  }

  ~Executor() { join(); }

  /*
  iterator begin() { return iterator(*this, 0); }
  iterator end() { return iterator(*this, 0); }
*/

  ResT* getResult() { return _result.get(); }

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
    while (true) {
      auto optPair = reader_task();
      if (!optPair.has_value()) {
        break;
      }
      auto& chunk_index_pair = optPair.value();
      processors_task(chunk_index_pair.first);
      auto partial_results = searchers_task(chunk_index_pair.first);
      results_join_task(std::move(partial_results), chunk_index_pair.second);
    }
    if (_workers.fetch_sub(1) == 1) {
      _running.store(false);
      _result->done();
    }
  }

  std::optional<std::pair<DataT, uint64_t>> reader_task() {
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
    auto chunk_index_pair = _reader->getNextData();
    INLINE_BENCHMARK_WALL_STOP("reader task");
    reader_lock.lock();
    _max_readers++;
    _reader_cv.notify_one();
    return chunk_index_pair;
  }

  void processors_task(DataT& chunk) {
    INLINE_BENCHMARK_WALL_START("processor task");
    for (auto& processor : _processors) {
      processor->process(&chunk);
    }
    INLINE_BENCHMARK_WALL_STOP("processor task");
  }

  PartResT searchers_task(DataT& chunk) {
    INLINE_BENCHMARK_WALL_START("searcher task");
    PartResT res;
    if (_regex) {
      res = _searcher->search(_regex_pattern.get(), &chunk);
    } else {
      res = _searcher->search(_pattern, &chunk);
    }
    INLINE_BENCHMARK_WALL_STOP("searcher task");
    return res;
  }

  void results_join_task(PartResT partial_results, uint64_t index) {
    INLINE_BENCHMARK_WALL_START("results join task");
    _result->add(std::move(partial_results), index);
    INLINE_BENCHMARK_WALL_STOP("results join task");
  }

  std::string _file_path;
  std::string _meta_file_path;
  std::string _pattern;
  bool _regex = false;
  std::unique_ptr<re2::RE2> _regex_pattern;

  std::unique_ptr<ResT> _result;

  std::unique_ptr<tasks::BaseDataProvider<DataT>> _reader;
  std::vector<std::unique_ptr<tasks::BaseProcessor<DataT>>> _processors;
  std::unique_ptr<tasks::BaseSearcher<DataT, PartResT>> _searcher;

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