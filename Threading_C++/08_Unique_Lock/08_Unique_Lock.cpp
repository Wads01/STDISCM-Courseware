#include "main.hpp"

// UNIQUE LOCK
// 1. std::unique_lock is a more flexible and feature-rich locking mechanism compared to std::lock_guard.
// 2. It provides more control over the locking and unlocking of a mutex, allowing for deferred locking, timed locking, and manual unlocking.
// 3. std::unique_lock can be moved, which allows for transferring ownership of the lock between different scopes or functions.
// 4. It also supports condition variables, making it suitable for more complex synchronization scenarios.

// Locking Strategies with std::unique_lock:
// 1. defer_lock: The mutex is not locked upon construction. The lock can be acquired later using the lock() member function.
// 2. try_to_lock: The mutex is attempted to be locked upon construction. If the lock cannot be acquired, the unique_lock object will not own the mutex.
// 3. adopt_lock: The mutex is assumed to be already locked by the calling thread. The unique_lock object will take ownership of the lock without attempting to lock it again.

int buffer = 0;
std::mutex mtx;

void noStrategy(int x) {
    std::unique_lock<std::mutex> lock(mtx);
    for (int i = 0; i < x; ++i) {
        ++buffer;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        std::cout << "Thread " << std::this_thread::get_id() << " incremented buffer to " << buffer << std::endl;
    }
    // Lock is automatically released when 'lock' goes out of scope
}

void deferLock(int x) {
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
    lock.lock(); // Locking later
    for (int i = 0; i < x; ++i) {
        ++buffer;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        std::cout << "Thread " << std::this_thread::get_id() << " incremented buffer to " << buffer << std::endl;
    }
    // Lock is automatically released when 'lock' goes out of scope
}

void tryToLock(int x) {
    std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
    if (lock.owns_lock()) { // Check if the lock was acquired
        for (int i = 0; i < x; ++i) {
            ++buffer;
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            std::cout << "Thread " << std::this_thread::get_id() << " incremented buffer to " << buffer << std::endl;
        }
    } else {
        std::cout << "Thread " << std::this_thread::get_id() << " could not acquire the lock." << std::endl;
    }
    // Lock is automatically released when 'lock' goes out of scope
}

// Note: Be cautious when using adopt_lock, as it assumes the mutex is already locked by the current thread.
void adoptLock(int x) {
    mtx.lock(); // Manually lock the mutex first
    std::unique_lock<std::mutex> lock(mtx, std::adopt_lock); // Adopt the already locked mutex
    for (int i = 0; i < x; ++i) {
        ++buffer;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        std::cout << "Thread " << std::this_thread::get_id() << " incremented buffer to " << buffer << std::endl;
    }
    // Lock is automatically released when 'lock' goes out of scope
}

int main()
{
    std::cout << " ========================= PROGRAM START ========================= " << std::endl;

    std::thread t1(noStrategy, 5);
    std::thread t2(noStrategy, 5);

	if (t1.joinable()) t1.join();
	if (t2.joinable()) t2.join();

	std::cout << "Final buffer value: " << buffer << std::endl;

    std::cout << " ========================== PROGRAM END ========================== " << std::endl;

    return 0;
}