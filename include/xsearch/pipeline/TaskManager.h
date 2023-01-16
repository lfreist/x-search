#pragma once

#include <xsearch/pipeline/Task.h>

#include <vector>

namespace xs::pipeline {

// _____________________________________________________________________________
template <class ResType>
class TaskManager {
 public:
  TaskManager(ProducerTask producer_task, std::vector<ProcessingTask> tasks,
              CollectorTask<ResType> collector_task)
      : _producer_task(std::move(producer_task)),
        _tasks(std::move(tasks)),
        _collector_task(std::move(collector_task)) {}

  ~TaskManager() {
    if (_executed) {
      for (auto& thread : _threads) {
        if (thread.joinable()) {
          thread.join();
        }
      }
    }
  }

  TaskManager(const TaskManager&) = delete;
  TaskManager(TaskManager&&) = delete;

  void execute(uint16_t num_threads) {
    _threads.resize(num_threads);
    for (auto& thread : _threads) {
      thread = std::thread(&TaskManager::run, this);
    }
    _executed = true;
  }

  ResType join() {
    if (_executed) {
      for (auto& thread : _threads) {
        if (thread.joinable()) {
          thread.join();
        }
      }
    }
    return _collector_task.getResult();
  }

 private:
  void run() {
    bool done = false;
    // done is set to true, if the processors_queue is closed and empty.
    //  After that, the collectors task is run once more to check if there are
    //  data left to process. If this is not the case then this thread is done
    //  working -> done = true.
    // If meanwhile another thread pushes data to one of the queues, the same
    //  thread will run the task that gets data from the corresponding queue
    //  subsequently and thus, all data will be processed completely before all
    //  threads stop working.
    while (!done) {
      _producer_task.run_if_possible(&_processors_queue);
      while (true) {
        bool pop_failed_flag = false;
        // TODO: if readers are slow and many threads working here, we get a
        //  busy join: they cannot pop from queue and cannot work as readers...
        //  FIX THIS!
        std::optional<DataChunk> optData =
            _processors_queue.pop(&pop_failed_flag);
        if (pop_failed_flag) {
          // no data received but there might be pushed data later.
          break;
        }
        if (!optData.has_value()) {
          // no data received and queue is closed -> don't run processing in
          //  further loops
          done = true;
          break;
        }
        // data received
        DataChunk& data = optData.value();
        for (auto& t : _tasks) {
          t.run(&data);
        }
        bool push_warn_flag = false;
        _collectors_queue.push(std::move(data), &push_warn_flag);
        if (push_warn_flag) {
          break;
        }
      }
      _collector_task.run_if_possible(&_collectors_queue);
    }
  }

  utils::TSQueue<DataChunk> _processors_queue;
  utils::TSQueue<DataChunk> _collectors_queue;
  bool _executed = false;
  ProducerTask _producer_task;
  std::vector<ProcessingTask> _tasks;
  CollectorTask<ResType> _collector_task;
  std::vector<std::thread> _threads;
};

}  // namespace xs::pipeline