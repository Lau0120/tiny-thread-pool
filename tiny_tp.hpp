#ifndef TINY_TP_HPP_
#define TINY_TP_HPP_

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace tiny_tp {  // definitions

/// @brief Interface for tasks to be executed by the `ThreadPool`.
class ITask {
 public:
  virtual std::shared_ptr<void> Run() = 0;
};

/// @brief A thread pool class for executing tasks concurrently.
class ThreadPool {
 public:
  /// @brief Default maximum queue size for the `ThreadPool`.
  static constexpr std::uint32_t kDefaultMaxQueueSize{65535};

  /// @brief Constructs a `ThreadPool` with specified number of threads and
  /// maximum queue size.
  /// @param num_threads The number of threads in the `ThreadPool`.
  /// @param max_queue_size The maximum size of the task queue.
  ThreadPool(std::uint32_t num_threads, std::uint32_t max_queue_size);
  ThreadPool()
      : ThreadPool{std::thread::hardware_concurrency(), kDefaultMaxQueueSize} {}
  explicit ThreadPool(std::uint32_t num_threads)
      : ThreadPool{num_threads, kDefaultMaxQueueSize} {};
  ~ThreadPool();

  /// @brief Adds a task to the `ThreadPool` queue (non-blocking).
  /// @param task The task to be added to the queue.
  /// @return True if the task was successfully added, false otherwise.
  bool Drop(std::shared_ptr<ITask> task);
  /// @brief Retrieves all results from the `ThreadPool` queue (non-blocking).
  /// @return A vector containing all results from executed tasks.
  std::vector<std::shared_ptr<void>> GrabAllResults();
  uint32_t max_queue_size() const { return kMaxQueueSize; }
  uint32_t num_threads() const { return kNumThreads; }

 private:
  /// @brief A special task used to signal thread termination.
  class QuitTask : public ITask {
   public:
    QuitTask(std::shared_ptr<std::uint32_t> mark) : exit_mark_{mark} {}
    std::shared_ptr<void> Run() override { return exit_mark_; }

   private:
    std::shared_ptr<std::uint32_t> exit_mark_;
  };

  /// @brief Main function for thread pool execution cycle.
  void Cycle();

  std::queue<std::shared_ptr<ITask>> waiting_queue_;
  std::mutex waiting_queue_mtx_;
  std::condition_variable waiting_queue_cond_;
  std::queue<std::shared_ptr<void>> results_queue_;
  std::mutex results_queue_mtx_;
  const uint32_t kMaxQueueSize;
  const uint32_t kNumThreads;
  /// @brief Shared pointer for marking thread exit.
  std::shared_ptr<std::uint32_t> exit_mark_;
};

}  // namespace tiny_tp

namespace tiny_tp {  // implementations

ThreadPool::ThreadPool(std::uint32_t num_threads, std::uint32_t max_queue_size)
    : kNumThreads{num_threads},
      kMaxQueueSize{max_queue_size},
      exit_mark_{new std::uint32_t{0}} {
  // Create the specified number of threads and start them.
  for (int i = 0; i < kNumThreads; ++i) {
    std::thread(&ThreadPool::Cycle, this).detach();
  }
}

ThreadPool::~ThreadPool() {
  // Terminate threads one by one.
  for (int i = 0; i < kNumThreads; ++i) {
    // Send quit "signal" to each thread.
    Drop(std::make_shared<QuitTask>(exit_mark_));
    while (*exit_mark_ != i + 1) {
      // Reschedule execution of threads (lower the priority of main thread)
      std::this_thread::yield();
    }
  }
}

bool ThreadPool::Drop(std::shared_ptr<ITask> task) {
  std::lock_guard<std::mutex> waiting_queue_guard{waiting_queue_mtx_};
  if (waiting_queue_.size() >= kMaxQueueSize) {
    return false;
  }
  waiting_queue_.push(task);
  waiting_queue_cond_.notify_one();
  return true;
}

std::vector<std::shared_ptr<void>> ThreadPool::GrabAllResults() {
  std::vector<std::shared_ptr<void>> results;
  std::lock_guard<std::mutex> results_guard{results_queue_mtx_};
  while (!results_queue_.empty()) {
    results.push_back(results_queue_.front());
    results_queue_.pop();
  }
  return results;
}

void ThreadPool::Cycle() {
  // Main loop for each thread.
  while (true) {
    // Try to take a task from queue.
    std::unique_lock<std::mutex> waiting_queue_unilock{waiting_queue_mtx_};
    // Keep waiting untile the queue is not empty.
    waiting_queue_cond_.wait(waiting_queue_unilock,
                             [this]() { return !waiting_queue_.empty(); });
    std::shared_ptr<ITask> task = waiting_queue_.front();
    waiting_queue_.pop();
    waiting_queue_unilock.unlock();

    auto result = task->Run();
    // Process the result of the task execution
    if (result != nullptr) {
      if (result == exit_mark_) {
        ++(*exit_mark_);
        break;  // Exit the loop if the result is an exit flag
      }
      std::lock_guard<std::mutex> results_guard{results_queue_mtx_};
      results_queue_.push(result);
    }
  }
}

}  // namespace tiny_tp

#endif  // TINY_TP_HPP_