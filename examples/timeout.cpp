#include <chrono>
#include <condition_variable>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "../tiny_tp.hpp"

class ITimeoutTask : public tiny_tp::ITask {
 public:
  ITimeoutTask(long long timeout) : timeout_{timeout} {}
  std::shared_ptr<void> Execute() final {
    return timeout_ == 0 ? OnTimeout() : OnSuccess();
  }
  virtual std::shared_ptr<void> OnSuccess() = 0;
  virtual std::shared_ptr<void> OnTimeout() = 0;
  void CountDown() { timeout_ = (timeout_ == 0) ? 0 : timeout_ - 1; }

 private:
  long long timeout_;
};

class TimeoutThreadPool {
 public:
  friend class TimePollingTask;

  TimeoutThreadPool(unsigned num_threads) : execution_tp_{num_threads} {
    execution_tp_.Drop(std::make_shared<TimePollingTask>(this));
  }
  ~TimeoutThreadPool() { is_closing_ = true; }

  void Drop(std::shared_ptr<ITimeoutTask> task) {
    std::lock_guard<std::mutex> timeout_tasks_guard{timeout_tasks_mtx_};
    timeout_tasks_.push_back(task);
    timeout_tasks_cond_.notify_one();
  }
  std::vector<std::shared_ptr<void>> GrabAllResults() {
    return execution_tp_.GrabAllResults();
  }

 private:
  class TimePollingTask : public tiny_tp::ITask {
   public:
    TimePollingTask(TimeoutThreadPool* pool) : pool_{pool} {}
    std::shared_ptr<void> Execute() final {
      while (!pool_->is_closing_) {
        std::unique_lock<std::mutex> timeout_tasks_unilock{
            pool_->timeout_tasks_mtx_};
        pool_->timeout_tasks_cond_.wait(timeout_tasks_unilock, [this]() {
          return !pool_->timeout_tasks_.empty();
        });
        for (auto it = pool_->timeout_tasks_.begin();
             it != pool_->timeout_tasks_.end(); ++it) {
          (*it)->CountDown();
        }
        auto num_idle_threads = pool_->execution_tp_.QueryIdleThreadsCount();
        while (num_idle_threads-- && !pool_->timeout_tasks_.empty()) {
          pool_->execution_tp_.Drop(pool_->timeout_tasks_.front());
          pool_->timeout_tasks_.pop_front();
        }
        timeout_tasks_unilock.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
      return nullptr;
    }

   private:
    TimeoutThreadPool* pool_;
  };

  bool is_closing_{false};
  std::list<std::shared_ptr<ITimeoutTask>> timeout_tasks_;
  std::mutex timeout_tasks_mtx_;
  std::condition_variable timeout_tasks_cond_;
  tiny_tp::ThreadPool execution_tp_;
};

class CommonTimeoutTask : public ITimeoutTask {
 public:
  CommonTimeoutTask(int id, long long timeout)
      : ITimeoutTask{timeout}, id_{id} {}
  std::shared_ptr<void> OnSuccess() final {
    std::cout << "Task [" << id_ << "] is executing..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return nullptr;
  }
  std::shared_ptr<void> OnTimeout() final {
    std::cout << "Task [" << id_ << "] is timeout..." << std::endl;
    return nullptr;
  }

 private:
  int id_;
};

int main(void) {
  TimeoutThreadPool ttp(2);
  ttp.Drop(std::make_shared<CommonTimeoutTask>(1001, 3));
  ttp.Drop(std::make_shared<CommonTimeoutTask>(1002, 3));
  while (true) {
    std::cout << "sleeping..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
