/// @file timer.cpp
/// @brief An example that shows how to implement timer task
/// @version 1.0.0
/// @copyright MIT License
/// @author Lau0120
/// @date 2024/05/06

#include <chrono>
#include <cstdio>

#include "../tiny_tp.hpp"

class ITimerTask : public tiny_tp::ITask {
 public:
  ITimerTask(unsigned times, long long interval)
      : times_{times}, interval_{interval} {}
  std::shared_ptr<void> Execute() final {
    std::chrono::milliseconds elapse{interval_};
    for (unsigned i = 0; i < times_; ++i) {
      std::this_thread::sleep_for(elapse);
      OnTimerTick(i + 1);
    }
    return nullptr;
  }
  virtual void OnTimerTick(unsigned tick_count) = 0;

 protected:
  const unsigned times_;
  const long long interval_;
};

class CommonTimer : public ITimerTask {
 public:
  CommonTimer(unsigned id, unsigned times, long long interval)
      : ITimerTask{times, interval}, id_{id} {}
  void OnTimerTick(unsigned tick_count) final {
    std::printf("task[%2u] %2u/%2u(p/t)\n", id_, tick_count, times_);
  }

 private:
  unsigned id_;
};

void ShowInfo(tiny_tp::ThreadPool& tp);

int main(void) {
  tiny_tp::ThreadPool tp;
  ShowInfo(tp);
  std::this_thread::sleep_for(std::chrono::seconds(3));
  for (unsigned i = 0; i < 12; ++i) {
    tp.Drop(std::make_shared<CommonTimer>(i, (i + 1) * 2, 2000));
  }
  while (true) {
    ShowInfo(tp);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::printf("\n");
  }
  return 0;
}

void ShowInfo(tiny_tp::ThreadPool& tp) {
  printf("threads count: %u\n", tp.num_threads());
  printf("idle threads count: %zu\n", tp.QueryIdleThreadsCount());
  printf("waiting task size: %zu\n", tp.QueryWaitingQueueCount());
}
