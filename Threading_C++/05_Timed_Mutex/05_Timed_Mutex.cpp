#include "main.hpp"

// TIMED MUTEX
// 1. std::timed_mutex is a synchronization primitive that can be used to protect shared data from being simultaneously accessed by multiple threads.
// 2. It is similar to std::mutex, but it provides additional member functions that allow threads to attempt to acquire the mutex with a timeout.

// Member functions of std::timed_mutex:
// - lock(): Locks the mutex. If the mutex is already locked, the calling thread will block until the mutex becomes available.
// - try_lock(): Tries to lock the mutex without blocking. Returns true if the lock was acquired, false otherwise.
// - try_lock_for(const std::chrono::duration& rel_time): Tries to lock the mutex for a specified duration. Returns true if the lock was acquired, false otherwise.
// - try_lock_until(const std::chrono::time_point& abs_time): Tries to lock the mutex until a specified time point. Returns true if the lock was acquired, false otherwise.
// - unlock(): Unlocks the mutex. The calling thread must own the mutex.

int amount = 0;
std::timed_mutex t_mtx;

void incrementTryLockFor() {
    if (t_mtx.try_lock_for(std::chrono::seconds(1))) {
        ++amount;
        std::this_thread::sleep_for(std::chrono::seconds(2));
		std::cout << "Thread " << std::this_thread::get_id() << " incremented amount to " << amount << std::endl;
        t_mtx.unlock();
    }
    else {
        std::cout << "Thread " << std::this_thread::get_id() << " could not lock the mutex within 1 millisecond." << std::endl;
    }
}

void incrementTryLockUntil() {
    auto now = std::chrono::steady_clock::now();
    if (t_mtx.try_lock_until(now + std::chrono::seconds(1))) {
        ++amount;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "Thread " << std::this_thread::get_id() << " incremented amount to " << amount << std::endl;
        t_mtx.unlock();
    }
    else {
        std::cout << "Thread " << std::this_thread::get_id() << " could not lock the mutex within 1 millisecond." << std::endl;
    }
}

int main()
{
    std::cout << " ========================= PROGRAM START ========================= " << std::endl;

    std::thread t1(incrementTryLockFor);
    std::thread t2(incrementTryLockFor);

	if (t1.joinable()) t1.join();
	if (t2.joinable()) t2.join();

	std::cout << "Final amount value: " << amount << std::endl;

	std::cout << "\n----------------------------------------\n\n";

    amount = 0;

	std::thread t3(incrementTryLockUntil);
	std::thread t4(incrementTryLockUntil);

	if (t3.joinable()) t3.join();
	if (t4.joinable()) t4.join();

	std::cout << "Final amount value: " << amount << std::endl;

    std::cout << " ========================== PROGRAM END ========================== " << std::endl;

    return 0;
}