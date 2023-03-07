// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/results/base/Result.h>
#include <xsearch/tasks/base/DataProvider.h>
#include <xsearch/tasks/base/InplaceProcessor.h>
#include <xsearch/tasks/base/ReturnProcessor.h>
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
  Executor(int num_threads,
           std::unique_ptr<task::base::DataProvider<DataT>> reader,
           std::vector<std::unique_ptr<task::base::InplaceProcessor<DataT>>>
               inplace_processors,
           std::unique_ptr<task::base::ReturnProcessor<DataT, PartResT>>
               return_processor,
           ResArgs&&... result_args)
      : _result(new ResT(std::forward<ResArgs>(result_args)...)),
        _reader(std::move(reader)),
        _inplace_processors(std::move(inplace_processors)),
        _return_processor(std::move(return_processor)),
        _workers(num_threads) {
    _threads.resize(num_threads);
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

  std::vector<std::thread> _threads;

  std::atomic<bool> _running{false};
  std::atomic<int> _workers{1};
  std::atomic<bool> _stop{false};
  std::mutex _join_mutex;
};

}  // namespace xs