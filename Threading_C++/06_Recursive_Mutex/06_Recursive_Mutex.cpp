#include "main.hpp"

// RECURSIVE MUTEX
// 1. std::recursive_mutex is a synchronization primitive that can be used to protect shared data from being simultaneously accessed by multiple threads.
// 2. It is similar to std::mutex, but it allows the same thread to acquire the mutex multiple times without causing a deadlock.
// 3. This is useful in scenarios where a function that locks the mutex may be called recursively or from multiple functions that also lock the same mutex.

int buffer = 0;
std::recursive_mutex r_mtx;

void recursionFunction(int loopFor) {
	if (loopFor < 0) return;

	r_mtx.lock();
	std::cout << "Thread " << std::this_thread::get_id() << " acquired lock, buffer: " << buffer << ", loopFor: " << loopFor << std::endl;
    buffer++;
	recursionFunction(loopFor - 1);
	r_mtx.unlock();
}

void loopFunction(int loopFor) {
    while(loopFor-- > 0) {
        r_mtx.lock();
        std::cout << "Thread " << std::this_thread::get_id() << " acquired lock, buffer: " << buffer << ", loopFor: " << loopFor << std::endl;
        buffer++;
        r_mtx.unlock();
	}
}

int main()
{
    std::cout << " ========================= PROGRAM START ========================= " << std::endl;
    
	std::cout << "Recursive Function" << std::endl;
    std::thread t1(recursionFunction, 5);
	if (t1.joinable()) t1.join();

	std::cout << "\n----------------------------------------\n\n";

    buffer = 0;

    std::cout << "Loop Function" << std::endl;
    std::thread t2(loopFunction, 5);
	if (t2.joinable()) t2.join();

    std::cout << " ========================== PROGRAM END ========================== " << std::endl;

    return 0;
}