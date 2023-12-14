// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/concepts.h>
#include <xsearch/results/base/Result.h>
#include <xsearch/tasks/base/DataProvider.h>
#include <xsearch/tasks/base/InplaceProcessor.h>
#include <xsearch/tasks/base/ReturnProcessor.h>
#include <xsearch/utils/InlineBench.h>
#include <xsearch/utils/TSQueue.h>
#include <xsearch/utils/utils.h>

#include <coroutine>
#include <future>
#include <memory>
#include <semaphore>
#include <string>
#include <thread>
#include <vector>

namespace xs {

enum class execute { async, blocking };

template <typename ReaderT, typename SearcherT, typename ResultT, typename PartResT, typename ResIterator,
          typename DataT = DataChunk>
  requires xs::ReaderC<ReaderT, DataT> && xs::SearcherC<SearcherT, PartResT, DataT> &&
           xs::ResultC<ResultT, PartResT, ResIterator>
class Searcher {
 public:  // --- public functions --------------------------------------------------------------------------------------
  Searcher(ReaderT&& reader, SearcherT&& searcher, int num_threads, int num_concurrent_reads = 1)
      : _reader(std::move(reader)),
        _searcher(std::move(searcher)),
        _threads(num_threads),
        _read_semaphore(num_concurrent_reads) {}

  ~Searcher() { join(); }

  /// not copyable/movable
  Searcher(Searcher&&) = delete;
  Searcher(const Searcher&) = delete;
  Searcher operator=(Searcher&&) = delete;
  Searcher operator=(const Searcher&) = delete;

  /**
   * Join all threads
   */
  void join() {
    for (auto& t : _threads) {
      if (t.joinable()) {
        t.join();
      }
    }
    _is_running.store(false);
  }

  /**
   * Execute the Searcher.
   *  Execution can be blocking (a) or asynchronous (b) depending on the template argument.
   * (a) execute::blocking
   *     Calling thread blocks until the searcher is done. const ResultT reference is returned.
   * (b) execute::async
   *     Searcher runs asynchronous. std::future<const ResultT&> is returned.
   */
  template <execute e = execute::blocking>
  auto execute() {
    _is_running.store(true);
    if constexpr (e == execute::async) {
      return run_async();
    } else if constexpr (e == execute::blocking) {
      return run_blocking();
    }
  }

  /**
   * Const pointer access to the result while Searcher is running
   *
   * Enabled, if ResIterator is not void
   */
  [[nodiscard]] std::enable_if<!std::is_void_v<ResIterator>, const ResultT*> live_result() const { return &_result; }

  /**
   * Return if Searcher is running.
   */
  [[nodiscard]] bool running() const { return _is_running.load(); }

 private:  // --- helper functions -------------------------------------------------------------------------------------
  const ResultT& run_blocking() {
    while (true) {
      if (_force_stop.load()) {
        break;
      }
      auto opt_data = _read_semaphore.access([&]() { return _reader.read(); });
      if (!opt_data) {
        break;
      }
      auto opt_result = _searcher.search(&opt_data.value());
      if (opt_result) {
        _result.add(std::move(opt_result.value()));
      }
    }
    _is_running.store(false);
    return _result;
  }

  std::future<ResultT> run_async() {
    std::future<ResultT> future_result =
        std::async(std::launch::async, [&]() { return run_blocking(); });
    return future_result;
  }

 private:  // --- members ----------------------------------------------------------------------------------------------
  std::atomic<bool> _is_running = false;
  std::atomic<bool> _force_stop = false;

  ReaderT _reader;
  SearcherT _searcher;
  ResultT _result;

  utils::TSQueue<DataT> _read_queue;

  std::vector<std::thread> _threads;
  xs::utils::Semaphore _read_semaphore;
};

}  // namespace xs