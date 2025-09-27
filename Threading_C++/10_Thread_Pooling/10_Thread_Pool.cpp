#include "main.hpp"
#include "ThreadPool.hpp"

// THREAD POOLING
// 1. Thread pooling is a technique used to manage a collection of reusable threads for executing tasks concurrently.
// 2. It helps to reduce the overhead of creating and destroying threads for each task, improving performance and resource utilization.
// 3. A thread pool maintains a fixed number of threads that can be reused to execute multiple tasks, allowing for efficient handling of concurrent workloads.
// 4. Tasks are typically submitted to the thread pool, which assigns them to available threads for execution.


// Basic Components of a Thread Pool:
// 1. Worker Threads: A set of threads that are created and managed by the thread pool. These threads wait for tasks to be assigned and execute them when available.
// 2. Task Queue: A queue that holds the tasks to be executed by the worker threads. Tasks are typically represented as function objects or callable entities.
// 3. Synchronization Mechanisms: Mutexes, condition variables, and other synchronization primitives are used to ensure thread-safe access to shared resources, such as the task queue.
// 4. Task Submission: A mechanism for submitting tasks to the thread pool, which adds them to the task queue and notifies worker threads of new tasks.
// 5. Shutdown Mechanism: A way to gracefully shut down the thread pool, ensuring that all tasks are completed before terminating the worker threads.

// Note: Implementing a full-featured thread pool requires careful consideration of thread management, task scheduling, and synchronization to ensure efficient and safe operation.
int main()
{
    std::cout << " ========================= PROGRAM START ========================= " << std::endl;

    // Create a pool with 4 worker threads
    ThreadPool pool(4);

    // Enqueue a variety of tasks
    auto f1 = pool.enqueue([](int a, int b) { return a + b; }, 5, 7);
    auto f2 = pool.enqueue([](std::string s) { return s + " world"; }, std::string("hello"));

    // Enqueue a task that prints (returns void)
    auto f3 = pool.enqueue([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Task running on thread " << std::this_thread::get_id() << std::endl;
    });

    // Get results
    std::cout << "f1 result: " << f1.get() << std::endl;    // 12
    std::cout << "f2 result: " << f2.get() << std::endl;    // hello world
    f3.get(); // wait for print task

    std::cout << " ========================== PROGRAM END ========================== " << std::endl;

    return 0;
}