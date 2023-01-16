// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <xsearch/DataChunk.h>
#include <xsearch/utils/TSQueue.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

namespace xs::pipeline {

class ProcessingTask {
 public:
  explicit ProcessingTask(std::function<void(DataChunk*)> func);

  void run(DataChunk* data);

 protected:
  std::function<void(DataChunk*)> _func;
};

// _____________________________________________________________________________
class ProducerTask {
 public:
  explicit ProducerTask(std::function<std::vector<DataChunk>()> func,
                        int max_workers);

  ProducerTask(const ProducerTask&) = delete;
  ProducerTask& operator=(const ProducerTask&) = delete;

  ProducerTask(ProducerTask&&) = default;
  ProducerTask& operator=(ProducerTask&&) = default;

  void run_if_possible(xs::utils::TSQueue<DataChunk>* queue);

 private:
  std::function<std::vector<DataChunk>()> _func;
  int _max_workers;
  int _num_workers = 0;
  std::unique_ptr<std::condition_variable> _cv =
      std::make_unique<std::condition_variable>();
  std::unique_ptr<std::mutex> _mutex = std::make_unique<std::mutex>();
  bool _done = false;
};

// _____________________________________________________________________________
template <class ResType>
class CollectorTask {
 public:
  explicit CollectorTask(std::function<void(DataChunk*)> collecting_func,
                         std::function<ResType()> result_provider_func) {
    _collecting_func = std::move(collecting_func);
    _result_provider_func = result_provider_func;
  }

  void run_if_possible(utils::TSQueue<DataChunk>* queue) {
    _worker_mutex->lock();
    if (_num_workers >= 1) {
      // already processed by a thread -> run not possible
      _worker_mutex->unlock();
      return;
    }
    // run possible -> run it
    _num_workers++;
    _worker_mutex->unlock();
    // start working
    std::unique_lock locker(*_mutex);
    while (true) {
      bool pop_failed_flag = false;
      std::optional<DataChunk> optData = queue->pop(&pop_failed_flag);
      if (pop_failed_flag || !optData.has_value()) {
        // did not get an element -> stop working here
        _worker_mutex->lock();
        _num_workers--;
        _worker_mutex->unlock();
        break;
      }
      _collecting_func(&optData.value());
    }
  }

  ResType getResult() { return _result_provider_func(); }

 private:
  std::function<void(DataChunk*)> _collecting_func;
  std::function<ResType()> _result_provider_func;
  std::unique_ptr<std::condition_variable> _cv =
      std::make_unique<std::condition_variable>();
  std::unique_ptr<std::mutex> _mutex = std::make_unique<std::mutex>();
  std::unique_ptr<std::mutex> _worker_mutex = std::make_unique<std::mutex>();
  int _num_workers = 0;
};

}  // namespace xs::pipeline