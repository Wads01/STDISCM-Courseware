#include "main.hpp"

// CONDITION VARIABLE
// 1. std::condition_variable is a synchronization primitive that allows threads to wait for certain conditions to be met before proceeding.
// 2. It is used in conjunction with a mutex to protect shared data and coordinate the execution of multiple threads.
// 3. Threads can wait on a condition variable, releasing the associated mutex while waiting, and can be notified by other threads when the condition changes.

// Notifying Strategies:
// 1. notify_one(): Wakes up one waiting thread. If no threads are waiting, the notification is lost.
// 2. notify_all(): Wakes up all waiting threads. If no threads are waiting, the notification is lost.

// Note - Always use std::unique_lock with std::condition_variable, as it provides the necessary flexibility for locking and unlocking the mutex.

long balance = 0;
std::mutex mtx;
std::condition_variable cv;

void addMoney(long amount) {
	std::lock_guard<std::mutex> lock(mtx);
	balance += amount;
	std::cout << "Added " << amount << ", new balance: " << balance << std::endl;

	cv.notify_all(); // Notify all waiting threads that the balance has been updated
}

void withdrawMoney(long amount) {
	std::cout << "Withdrawing.....Locking the mutex and waiting for balance to be non-zero..." << std::endl;
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [] { return balance != 0; });
    if (balance >= amount) {
        balance -= amount;
        std::cout << "Withdrew " << amount << ", new balance: " << balance << std::endl;
    } else {
        std::cout << "Insufficient funds for withdrawal of " << amount << ", current balance: " << balance << std::endl;
	}

	std::cout << "Withdrawal of " << amount << " completed." << std::endl;
}

int main()
{
    std::cout << " ========================= PROGRAM START ========================= " << std::endl;

    std::thread t1(withdrawMoney, 500);

	std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    std::thread t2(addMoney, 500);

	if (t1.joinable()) t1.join();
	if (t2.joinable()) t2.join();

    std::cout << " ========================== PROGRAM END ========================== " << std::endl;

    return 0;
}