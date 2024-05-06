#include <chrono>
#include <cstdio>
#include <memory>
#include <random>

#include "../tiny_tp.hpp"

class CommonTask : public tiny_tp::ITask {
 public:
  CommonTask(int id, unsigned execution_time)
      : id_{id}, execution_time_{execution_time} {}
  std::shared_ptr<void> Execute() override {
    std::printf("Task[%d] is executing (%d seconds)...\n", id_,
                execution_time_);
    std::this_thread::sleep_for(std::chrono::seconds(execution_time_));
    return std::make_shared<int>(id_);
  }

 private:
  int id_;
  unsigned execution_time_;
};

static int kTaskSequence = 10000;
constexpr unsigned kWaitingSeconds = 3;
int main(void) {
  std::default_random_engine e;
  std::uniform_int_distribution<unsigned> rt(1, 5);
  std::uniform_int_distribution<unsigned> rc(3, 5);

  tiny_tp::ThreadPool tp;
  while (true) {
    unsigned task_cnt = rc(e);
    for (unsigned i = 0; i < task_cnt; ++i) {
      unsigned execution_time = rt(e);
      tp.Drop(std::make_shared<CommonTask>(kTaskSequence++, execution_time));
    }
    std::printf("Waiting for %d seconds\n", kWaitingSeconds);
    std::this_thread::sleep_for(std::chrono::seconds(kWaitingSeconds));
    auto results = tp.GrabAllResults();
    for (auto result : results) {
      auto task_id = std::static_pointer_cast<int>(result);
      std::printf("Task[%d] is complete...\n", *task_id);
    }
    printf("\n");
  }
  return 0;
}
