// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/results/base/Result.h>
#include <xsearch/tasks/base/DataProvider.h>
#include <xsearch/tasks/base/InplaceProcessor.h>
#include <xsearch/tasks/base/ReturnProcessor.h>
#include <xsearch/utils/InlineBench.h>
#include <xsearch/utils/TSQueue.h>
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
  Executor(int num_threads,
           std::unique_ptr<task::base::DataProvider<DataT>> reader,
           std::vector<std::unique_ptr<task::base::InplaceProcessor<DataT>>>
               inplace_processors,
           std::unique_ptr<task::base::ReturnProcessor<DataT, PartResT>>
               return_processor,
           std::unique_ptr<ResT> initial_result = nullptr)
      : _result(std::move(initial_result)),
        _reader(std::move(reader)),
        _inplace_processors(std::move(inplace_processors)),
        _return_processor(std::move(return_processor)),
        _workers(num_threads) {
    if (_result == nullptr) {
      _result = std::make_unique<ResT>();
    }
    _worker_threads.resize(num_threads);
    _running.store(true);
    if (_reader->get_max_readers() == 1) {
      _reader_thread = std::thread(&Executor::reader_task_single_read, this);
      for (auto& t : _worker_threads) {
        t = std::thread(&Executor::worker_task_single_read, this);
      }
    } else {
      for (auto& t : _worker_threads) {
        t = std::thread(&Executor::pipeline_multi_read, this);
      }
    }
  }

  ~Executor() { join(); }

  ResT* getResult() { return _result.get(); }

  bool isRunning() { return _running.load(); }

  void join() {
    std::unique_lock lock(_join_mutex);
    for (auto& t : _worker_threads) {
      if (t.joinable()) {
        t.join();
      }
    }
    if (_reader_thread.joinable()) {
      _reader_thread.join();
    }
    _running.store(false);
  }

  void force_stop() {
    _stop.store(true);
    join();
  }

 private:
  /**
   * Each thread initialized starts reading and then runs all processing on the
   * data it read in a linear manner
   */
  void pipeline_multi_read() {
    while (true) {
      if (_stop.load()) {
        break;
      }
      auto optPair = reader_task();
      if (!optPair.has_value()) {
        break;
      }
      auto& chunk_index_pair = optPair.value();
      processors_task(&chunk_index_pair.first, chunk_index_pair.second);
    }
    if (_workers.fetch_sub(1) == 1) {
      _running.store(false);
      _result->done();
    }
  }

  /**
   * A single reader task reads data and writes them onto the data queue.
   */
  void reader_task_single_read() {
    while (true) {
      if (_stop.load()) {
        _data.close();
        break;
      }
      auto optData = reader_task();
      if (!optData) {
        _data.close();
        break;
      }
      _data.push(std::move(optData.value()));
    }
  }

  /**
   * The worker threads pop data from the data queue and run the processing on
   * them
   */
  void worker_task_single_read() {
    while (true) {
      auto optPair = _data.pop();
      if (_stop.load()) {
        break;
      }
      if (!optPair) {
        break;
      }
      processors_task(&optPair.value().first, optPair.value().second);
    }
    if (_workers.fetch_sub(1) == 1) {
      _running.store(false);
      _result->done();
    }
  }

  /**
   * The default worker task: run inplace processors and the return processor on
   * the data. Last, write the results to the result.
   * @param chunk
   * @param index
   */
  void processors_task(DataChunk* chunk, chunk_index index) {
    inplace_processors_task(chunk);
    if (_return_processor != nullptr) {
      auto res = return_processor_task(chunk);
      results_join_task(res, index);
    }
  }

  std::optional<std::pair<DataT, uint64_t>> reader_task() {
    return _reader->getNextData();
  }

  void inplace_processors_task(DataT* chunk) {
    for (auto& processor : _inplace_processors) {
      processor->process(chunk);
    }
  }

  PartResT return_processor_task(DataT* chunk) {
    PartResT res;
    res = _return_processor->process(chunk);
    return res;
  }

  void results_join_task(PartResT partial_results, uint64_t index) {
    _result->add(std::move(partial_results), index);
  }

  std::unique_ptr<ResT> _result;

  std::unique_ptr<task::base::DataProvider<DataT>> _reader;
  std::vector<std::unique_ptr<task::base::InplaceProcessor<DataT>>>
      _inplace_processors;
  std::unique_ptr<task::base::ReturnProcessor<DataT, PartResT>>
      _return_processor;

  std::vector<std::thread> _worker_threads;
  std::thread _reader_thread;

  utils::TSQueue<std::pair<DataChunk, chunk_index>> _data;

  std::atomic<bool> _running{false};
  std::atomic<int> _workers{1};
  std::atomic<bool> _stop{false};
  std::mutex _join_mutex;
};

}  // namespace xs