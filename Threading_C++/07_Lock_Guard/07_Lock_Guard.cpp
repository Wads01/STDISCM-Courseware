#include "main.hpp"

// LOCK GUARD
// 1. std::lock_guard is a simple wrapper around a mutex that provides a convenient RAII-style mechanism for owning a mutex for the duration of a scoped block.
// 2. It attempts to acquire the mutex it is given. When the std::lock_guard object goes out of scope, its destructor is called, and the mutex is automatically released.
// 3. std::lock_guard is not copyable or movable, which prevents accidental copying or moving of the lock guard object, ensuring that the mutex is only owned by one lock guard at a time.
// 4. This helps to ensure that the mutex is properly released even if an exception is thrown within the scope.

int buffer = 0;
std::mutex mtx;

void incrementNoLockGuard(int x) {
	mtx.lock();
    for (int i = 0; i < x; ++i) {
        ++buffer;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        std::cout << "Thread " << std::this_thread::get_id() << " incremented buffer to " << buffer << std::endl;
	}
	mtx.unlock();
}

void incrementLockGuard(int x){
    std::lock_guard<std::mutex> lock(mtx);
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

    std::thread t1(incrementLockGuard, 5);
    std::thread t2(incrementLockGuard, 5);

	if (t1.joinable()) t1.join();
	if (t2.joinable()) t2.join();

	std::cout << "Final buffer value: " << buffer << std::endl;

    std::cout << " ========================== PROGRAM END ========================== " << std::endl;

    return 0;
}