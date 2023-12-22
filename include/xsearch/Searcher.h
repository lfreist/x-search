/**
* Copyright 2023, Leon Freist (https://github.com/lfreist)
* Author: Leon Freist <freist.leon@gmail.com>
*
* This file is part of x-search.
*/

#pragma once

#include <xsearch/concepts.h>
#include <xsearch/utils/Semaphore.h>
#include <xsearch/utils/Synchronized.h>
#include <xsearch/utils/UninitializedAllocator.h>
#include <xsearch/utils/utils.h>
#include <xsearch/types.h>

#include <coroutine>
#include <future>
#include <iostream>
#include <memory>
#include <semaphore>
#include <string>
#include <thread>
#include <vector>

namespace xs {

enum class execute { async, blocking, live, lazy };

template <typename ReaderT, typename SearcherT, typename ResultT, typename PartResT, typename ResIterator,
          typename DataT = strtype>
  requires ReaderC<ReaderT, DataT> && SearcherC<SearcherT, PartResT, DataT> && ResultC<ResultT, PartResT>
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
    } else if constexpr (e == execute::live) {
      auto res = run_live();
      return res;
    }
  }

  /**
   * Return if Searcher is running.
   */
  [[nodiscard]] bool running() const { return _is_running.load(); }

  /*
  template <typename T = ReaderT>
  requires InputStreamable<T>
  friend std::istream& operator>>(std::istream& is, Searcher& searcher) {
    if (!searcher.running()) {
      searcher.run_async();
    }
    is >> t;
    return is;
  }
   */

 private:  // --- helper functions -------------------------------------------------------------------------------------
  void run_thread() {
    atomic_fetch_add(&_threads_running, 1);
    while (true) {
      if (_force_stop.load()) {
        break;
      }
      auto opt_data = _read_semaphore.access([&]() { return _reader(); });
      if (!opt_data) {
        break;
      }
      auto opt_result = _searcher(opt_data.value());
      if (opt_result) {
        _result.add(std::move(opt_result.value()));
      }
    }
    atomic_fetch_sub(&_threads_running, 1);
    if (_threads_running.load() == 0) {
      _is_running.store(false);
      _result.close();
    }
  }

  std::future<ResultT&> run_async() {
    std::future<ResultT&> future_result =
        std::async(std::launch::async, [this]() -> ResultT& { return run_blocking().get(); });
    return future_result;
  }

  std::future<ResultT&> run_blocking() {
    auto ret = run_live();
    join();
    return ret;
  }

  std::future<ResultT&> run_live() {
    run();
    std::promise<ResultT&> promise;
    promise.set_value(_result);
    return promise.get_future();
  }

  void run() {
    for (auto& t : _threads) {
      t = std::thread(&Searcher::run_thread, this);
    }
  }

 private:  // --- members ----------------------------------------------------------------------------------------------
  std::atomic<bool> _is_running = false;
  std::atomic<bool> _force_stop = false;
  std::atomic<int> _threads_running;

  ReaderT _reader;
  SearcherT _searcher;
  ResultT _result;

  std::vector<std::thread> _threads;
  xs::utils::Semaphore _read_semaphore;
};

}  // namespace xs