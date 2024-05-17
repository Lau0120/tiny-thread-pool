# tiny-thread-pool

A simple cross-platform thread pool based on the C++

## 1. Features

- Cross-platform: All implementations are based on the C++11 standard library,
so any compiler that implements this standard can compile it.
- Simple: All you need to know is to define different task types based on your
actual needs, and then drop task instances to the thread pool.
- Lightweight: The implementation and comments combined are no more than 180
lines.

All implementations follow the [RAII](https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization) principle as much as possible,
which means you don’t need to worry about creating and destroying threads. The
thread pool will automatically handle these problems when it is created and destroyed.

## 2. Installation

tiny-thread-pool is header-only, so you just need to copy the `tiny_tp.hpp` file
into your project to use it.

## 3. Compilation

Regardless of the compilation method you choose, you must link the system
library `pthread`. For example, if you’re using the CLI method, you need to add
the option `-lpthread`, and if you’re using CMake, you need to invoke the
`target_link_libraries` command to link the pthread library.

## 4. Usage

### 4.1. Define your task type

tiny-thread-pool uses a callable object-like form to construct its own task
types by inheriting the `ITask` interface, allowing for different types of
tasks. You can store any data in your task class in any form, instead of using a
pointer to a specific struct.

```c++
// To define a task, you need to inherit ITask interface and to override the
// only pure virtual function `Execute`.

class TraditionalTask : public tiny_tp::ITask {
 public:
  TraditionalTask(std::shared_ptr<void> context)
      : ctx_{ context } {}
  std::shared_ptr<void> Execute() override {
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
  std::shared_ptr<void> Execute() override {
    // Do something with `arg1_`, `arg2_` and `arg3_`.
    return nullptr;
  }
 private:
  std::string arg1_;
  int arg2_;
  double arg3_;
};
```

Please note that tiny thread pool only releases IDLE threads, which means that 
if you put an endless loop in `Execute` method of your task, it could
potentially lead to a thread resource leak. So it’s best not to do that, but if
you do, you must take responsibility for exiting such tasks.

### 4.2. Create a thread pool instance

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

### 4.3. Interact with the thread pool

You can drop any type of task instance (implemented the `ITask` interface) to
the thread pool. For tasks with results (the result of `Execute` method is not
`nullptr`), you can also retrieve them from the thread pool, but the order is
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

### 4.4. Examples

Here are some short example programs in `examples` directory, some of which are
inspirational and reusable. Please remember, the thread pool is simply a way to
manage thread resources. That’s why I only provided some of the most basic
functionalities. You can extend it to solve a specific problem in a particular
scenario, just like `timer.cpp` and `timeout.cpp` in the examples.

- basic.cpp
  - A basic example that simulates the producer-consumer model.
- timer.cpp
  - An example that shows how to implement timer task.
- timeout.cpp
  - An example that shows how to implement timeout task.

In the `examples` directory, there is a Makefile. You can compile and run a
specific example by running `make <filename>` in the terminal within this
directory. (Of course, your system must support make for this to work, but even
if it does not, you can refer to the Makefile to write the corresponding
compilation commands.)

Through the above examples, we see how to easily manage concurrent task execution
using tiny-thread-pool. While it may not be suitable for all scenarios, it
provides a lightweight and easy-to-use solution for simple concurrency needs.

## 5. Summary

This is a very simple thread pool, but I believe it should be able to handle
many complex tasks. This is my first public library, and it is certainly not
perfect. Feel free to try it out and provide valuable feedback and suggestions.
The next goal is to write or collect some nice examples to prove its worth.
