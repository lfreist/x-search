// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/ResultTypes.h>
#include <xsearch/tasks/DataProvider.h>
#include <xsearch/tasks/InplaceProcessors.h>
#include <xsearch/tasks/ReturnProcessors.h>
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
  Executor(int num_threads, int max_readers,
           std::unique_ptr<tasks::BaseDataProvider<DataT>> reader,
           std::vector<std::unique_ptr<tasks::BaseInplaceProcessor<DataT>>>
               inplace_processors,
           std::unique_ptr<tasks::BaseReturnProcessor<DataT, PartResT>>
               return_processor,
           ResArgs&&... result_args)
      : _result(new ResT(std::forward<ResArgs>(result_args)...)),
        _reader(std::move(reader)),
        _inplace_processors(std::move(inplace_processors)),
        _return_processor(std::move(return_processor)),
        _workers(num_threads) {
    _threads.resize(num_threads);
    _max_readers = max_readers;
    _running.store(true);
    for (auto& t : _threads) {
      t = std::thread(&Executor::main_task, this);
    }
  }

  ~Executor() { join(); }

  ResT* getResult() { return _result.get(); }

  bool isRunning() { return _running.load(); }

  void join() {
    std::unique_lock lock(_join_mutex);
    for (auto& t : _threads) {
      if (t.joinable()) {
        t.join();
      }
    }
    _running.store(false);
  }

  void force_stop() {
    _stop.store(true);
    join();
  }

 private:
  void main_task() {
    while (true) {
      if (_stop.load()) {
        break;
      }
      auto optPair = reader_task();
      if (!optPair.has_value()) {
        break;
      }
      auto& chunk_index_pair = optPair.value();
      inplace_processors_task(&chunk_index_pair.first);
      if (_return_processor != nullptr) {
        auto partial_results = return_processor_task(&chunk_index_pair.first);
        results_join_task(std::move(partial_results), chunk_index_pair.second);
      }
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

  void inplace_processors_task(DataT* chunk) {
    INLINE_BENCHMARK_WALL_START("processor task");
    for (auto& processor : _inplace_processors) {
      processor->process(chunk);
    }
    INLINE_BENCHMARK_WALL_STOP("processor task");
  }

  PartResT return_processor_task(DataT* chunk) {
    INLINE_BENCHMARK_WALL_START("searcher task");
    PartResT res;
    res = _return_processor->process(chunk);
    INLINE_BENCHMARK_WALL_STOP("searcher task");
    return res;
  }

  void results_join_task(PartResT partial_results, uint64_t index) {
    INLINE_BENCHMARK_WALL_START("results join task");
    _result->add(std::move(partial_results), index);
    INLINE_BENCHMARK_WALL_STOP("results join task");
  }

  std::unique_ptr<ResT> _result;

  std::unique_ptr<tasks::BaseDataProvider<DataT>> _reader;
  std::vector<std::unique_ptr<tasks::BaseInplaceProcessor<DataT>>>
      _inplace_processors;
  std::unique_ptr<tasks::BaseReturnProcessor<DataT, PartResT>>
      _return_processor;

  std::vector<std::thread> _threads;

  std::mutex _reader_worker_mutex;
  std::mutex _check_reader_worker_mutex;
  std::condition_variable _reader_cv;
  int _max_readers = 2;

  std::atomic<bool> _running{false};
  std::atomic<int> _workers{1};
  std::atomic<bool> _stop{false};
  std::mutex _join_mutex;
};

}  // namespace xs