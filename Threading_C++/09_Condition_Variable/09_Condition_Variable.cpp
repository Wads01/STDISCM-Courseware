#include "main.hpp"

// CONDITION VARIABLE
// 1. std::condition_variable is a synchronization primitive that allows threads to wait for certain conditions to be met before proceeding.
// 2. It is used in conjunction with a mutex to protect shared data and coordinate the execution of multiple threads.
// 3. Threads can wait on a condition variable, releasing the associated mutex while waiting, and can be notified by other threads when the condition changes.

// Notifying Strategies:
// 1. notify_one(): Wakes up one waiting thread. If no threads are waiting, the notification is lost.
// 2. notify_all(): Wakes up all waiting threads. If no threads are waiting, the notification is lost.

// Note - Always use std::unique_lock with std::condition_variable, as it provides the necessary flexibility for locking and unlocking the mutex.

ull balance = 0;
std::mutex mtx;
std::condition_variable cv;

void addMoney(int amount() {

}

int main()
{
    std::cout << " ========================= PROGRAM START ========================= " << std::endl;

    std::thread t1();
    std::thread t2();

    std::cout << " ========================== PROGRAM END ========================== " << std::endl;

    return 0;
}