// Copyright 2023, Leon Freist
// Author: Leon Freist <freist@informatik.uni-freiburg.de>

#include <xsearch/pipeline/Task.h>

namespace xs::pipeline {

// ===== ProcessingTask ========================================================
// _____________________________________________________________________________
ProcessingTask::ProcessingTask(std::function<void(DataChunk*)> func) {
  _func = func;
}

// _____________________________________________________________________________
void ProcessingTask::run(DataChunk* data) { _func(data); }

// ===== ProducerTask ==========================================================
// _____________________________________________________________________________
ProducerTask::ProducerTask(std::function<std::vector<DataChunk>()> func,
                           int max_workers) {
  _func = func;
  _max_workers = max_workers;
}

// _____________________________________________________________________________
void ProducerTask::run_if_possible(xs::utils::TSQueue<DataChunk>* queue) {
  std::unique_lock locker(*_mutex);
  if (_done || _num_workers >= _max_workers) {
    return;
  }
  _num_workers++;
  locker.unlock();
  while (true) {
    std::vector<DataChunk> dcs = _func();
    if (dcs.empty()) {
      locker.lock();
      _num_workers--;
      if (_num_workers == 0) {
        // there is no other thread reading and reading did not result in new
        //  data -> we mark the reading task and the queue as closed.
        _done = true;
        queue->close();
        return;
      }
      // By setting _max_workers to 1 we avoid an endless loop. The endless
      //  loop could exist if every time a thread checks the if statement above
      //  (_num_workers == 0), another thread already signed himself as a worker
      //  for this task.
      // However, the check above is necessary if _max_workers > 1. In this
      //  case, we cannot just call _queue->close() because the other thread
      //  might try to add data to the queue...
      _max_workers = 1;
      locker.unlock();
      return;
    }
    bool push_warn_flag = false;
    for (auto& dc : dcs) {
      queue->push(std::move(dc), &push_warn_flag, true);
    }
    if (push_warn_flag) {
      break;
    }
  }
  locker.lock();
  _num_workers--;
}

// ===== CollectorTask =========================================================
// is templated on the result type and thus defined within the header file.

}  // namespace xs::pipeline