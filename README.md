# tiny-thread-pool

A simple cross-platform thread pool based on the C++

## 1. Features

- Cross-platform: All implementations are based on the C++11 standard library,
so any compiler that implements this standard can compile it.
- Simple: All you need to know is to define different task types based on your
actual needs, and then drop task instances to the thread pool.
- Lightweight: The implementation and comments combined are no more than 150
lines.
- Easy: All implementations follow the RAII principle as much as possible, which
means you don’t need to worry about creating and destroying threads. The thread
pool will automatically handle these problems when it is created and destroyed.

## 2. Installation

tiny-thread-pool is header-only, so you just need to copy the `tiny_tp.hpp` file
into your project to use it.

## 3. Usage

### 3.1. Define your task type

tiny-thread-pool uses a callable object-like form to construct its own task
types by inheriting the `ITask` interface, allowing for different types of
tasks. You can store any data in your task class in any form, instead of using a
pointer to a specific struct.

```c++
// To define a task, you need to inherit ITask interface and to override the
// only pure virtual function `Run`.

class TraditionalTask : public tiny_tp::ITask {
 public:
  TraditionalTask(std::shared_ptr<void> context)
      : ctx_{ context } {}
  std::shared_ptr<void> Run() override {
    // Do something with `context_`.
    return nullptr;
  }
 private:
  std::shared_ptr<void> context_;
};

class CommonTask : public tiny_tp::ITask {
 public:
  CommonTask(const std::string& arg1, int arg2, double arg3)
      : arg1_{ arg1 }, arg2_{ arg2 }, arg3_{ arg3 } {}
  std::shared_ptr<void> Run() override {
    // Do something with `arg1_`, `arg2_` and `arg3_`.
    return nullptr;
  }
 private:
  std::string arg1_;
  int arg2_;
  double arg3_;
};
```

### 3.2. Create a thread pool instance

You can create multiple thread pool instances in your program, and they do not
interfere with each other.

```c++
// The number of threads equals the number of system cpu cores.
tiny_tp::ThreadPool tp1;
// Specify number of threads and the maximum capacity of the queue.
tiny_tp::ThreadPool tp2(4, 20);
// Specify number of threads only.
tiny_tp::ThreadPool tp3(8);
```

### 3.3. Interact with the thread pool

You can drop any type of task instance (implemented the `ITask` interface) to
the thread pool. For tasks with results (the result of `Run` method is not
`nullptr`), you can alsow retrieve them from the thread pool, but the order is
not guaranteed.

```c++
// Drop some tasks to thread pool.
for (int i = 0; i < 5; ++i) {
  tp1.Drop(std::make_shared<CommonTask>("Hello, world!", i, 3.14));
}
sleep(5);
// Retrieve results from thread pool.
auto results = tp1.GrabAllResults();
for (auto result : results) {
  // Do something with result.
}
```

### 3.4. An example

Here is an example that simulates the producer-consumer model.

```c++
```

Through the above example, we see how to easily manage concurrent task execution
using tiny-thread-pool. While it may not be suitable for all scenarios, it
provides a lightweight and easy-to-use solution for simple concurrency needs.

## 4. Summary

This is a very simple thread pool, but I believe it should be able to handle
many complex tasks. This is my first public library, and it is certainly not
perfect. Feel free to try it out and provide valuable feedback and suggestions.
The next goal is to write or collect some nice examples to prove its worth.
