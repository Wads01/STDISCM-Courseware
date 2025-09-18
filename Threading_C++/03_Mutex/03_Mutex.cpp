#include "main.hpp"

// MUTEX (MUTUAL EXCLUSION)
// 1. A mutex is a synchronization primitive that can be used to protect shared data from being simultaneously accessed by multiple threads.
// 2. A mutex can be locked and unlocked using the lock() and unlock() methods.
// 3. When a thread locks a mutex, other threads that try to lock the same mutex will be blocked until the mutex is unlocked.

// RACE CONDITION
// 1. A race condition occurs when two or more threads access shared data and try to change it at the same time.
// 2. If the access to the shared data is not synchronized, it can lead to inconsistent or incorrect results

constexpr int LIMIT = 5;
constexpr int INCREMENT = 500;

int moneyNoMutex = 0;
int moneyWithMutex = 0;


void addMoneyNoMutex(int x) {
    for (int i = 0; i < x; i++) {
        moneyNoMutex++;
	}
}

void addMoneyWithMutex(int x, std::mutex& mtx) {
    for (int i = 0; i < x; i++) {
        mtx.lock();
        moneyWithMutex++;
        mtx.unlock();
    }
}

int main()
{
    std::cout << " ========================= PROGRAM START ========================= " << std::endl;

    for (int cycle = 0; cycle < LIMIT; ++cycle) {

        moneyNoMutex = 0;

        std::thread t1(addMoneyNoMutex, INCREMENT);
        std::thread t2(addMoneyNoMutex, INCREMENT);

        t1.join();
        t2.join();

        // The final money value is expected to be 1000
        std::cout << "CYCLE No: " << cycle
            << " | Final money (without mutex): " << moneyNoMutex << std::endl;
    }

    std::cout << "\n---------------------------------------------------------------\n\n";


    for (int cycle = 0; cycle < LIMIT; ++cycle) {

        moneyWithMutex = 0;
        std::mutex mtx;

        std::thread t3(addMoneyWithMutex, INCREMENT, std::ref(mtx));
        std::thread t4(addMoneyWithMutex, INCREMENT, std::ref(mtx));

        t3.join();
        t4.join();

        // The final money value is expected to be 1000
        std::cout << "CYCLE No: " << cycle
            << " | Final money (with mutex): " << moneyWithMutex << std::endl;
    }

    std::cout << " ========================== PROGRAM END ========================== " << std::endl;

    return 0;
}