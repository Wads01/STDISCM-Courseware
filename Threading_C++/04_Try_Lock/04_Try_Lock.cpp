#include "main.hpp"

// TRY_LOCK
// 1. std::try_lock tries to lock all the mutexes passed to it as arguments.
// 2. If it successfully locks all the mutexes, it returns -1.
// 3. If it fails to lock any mutex, it returns the index of the first mutex that it failed to lock.
// 4. If it fails to lock any mutex, it also unlocks all the mutexes that it successfully locked before the failure.

// YIELD
// 1. std::this_thread::yield() is a hint to the implementation to reschedule the execution of threads, allowing other threads to run.
// 2. It does not guarantee that the current thread will be suspended or that other threads will be scheduled to run.
// 3. It is typically used in spin-wait loops to avoid busy-waiting and to improve the responsiveness of the system.

// WITH YIELD - both threads can succeed in most of their attempts, so the counter can approach 200,000.
// WITHOUT YIELD - one thread can dominate and the other rarely gets a chance, so the counter stays near 100,000.

int counter = 0;
std::mutex mtx;

void increaseCounter() {
    for (int i = 0; i < 100000; ++i) {
        // Try to lock the mutex
        if (mtx.try_lock()) {
            ++counter; // Safely increment the counter
            mtx.unlock(); // Unlock the mutex
        } 
        //else
        //    std::this_thread::yield();
	}
}

int main()
{
    std::cout << " ========================= PROGRAM START ========================= " << std::endl;

    std::thread t1(increaseCounter);
    std::thread t2(increaseCounter);

	if (t1.joinable()) t1.join();
	if (t2.joinable()) t2.join();

	std::cout << "Final counter value: " << counter << std::endl;

    std::cout << " ========================== PROGRAM END ========================== " << std::endl;

    return 0;
}