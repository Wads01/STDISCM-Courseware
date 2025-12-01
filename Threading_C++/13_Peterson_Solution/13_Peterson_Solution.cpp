#include "main.hpp"
#include <atomic>

// PETERSON'S SOLUTION
// 1. Peterson's Solution is a classic software-based mutual exclusion algorithm for two processes/threads.
// 2. It ensures that two threads can safely access a critical section without race conditions using only shared memory.
// 3. It guarantees three key properties: Mutual Exclusion, Progress, and Bounded Waiting.
// 4. Uses two key variables: flag[] (interest in entering critical section) and turn (whose turn it is).
// 5. Named after Gary L. Peterson who published it in 1981.

// Algorithm Components:
// - flag[i]: Indicates if thread i wants to enter the critical section
// - turn: Indicates whose turn it is to enter (used for tie-breaking)

// Properties Guaranteed:
// 1. Mutual Exclusion: Only one thread can be in the critical section at a time
// 2. Progress: If no thread is in the critical section, one waiting thread will eventually enter
// 3. Bounded Waiting: A thread will not wait indefinitely to enter the critical section

// Note: We use std::atomic to ensure proper memory ordering and prevent compiler optimizations that could break the algorithm.

// Example 1: Basic Peterson's Solution
std::atomic<bool> flag0(false);
std::atomic<bool> flag1(false);
std::atomic<int> turn(0);
int sharedCounter = 0;

void thread0Work(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        // Entry section
        flag0.store(true, std::memory_order_seq_cst);
        turn.store(1, std::memory_order_seq_cst); // Give priority to thread 1
        
        // Wait while the other thread wants to enter and it's their turn
        while (flag1.load(std::memory_order_seq_cst) && turn.load(std::memory_order_seq_cst) == 1) {
            // Busy wait
        }
        
        // Critical section
        sharedCounter++;
        std::cout << "Thread 0 incremented counter to " << sharedCounter << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Exit section
        flag0.store(false, std::memory_order_seq_cst);
        
        // Remainder section
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void thread1Work(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        // Entry section
        flag1.store(true, std::memory_order_seq_cst);
        turn.store(0, std::memory_order_seq_cst); // Give priority to thread 0
        
        // Wait while the other thread wants to enter and it's their turn
        while (flag0.load(std::memory_order_seq_cst) && turn.load(std::memory_order_seq_cst) == 0) {
            // Busy wait
        }
        
        // Critical section
        sharedCounter++;
        std::cout << "Thread 1 incremented counter to " << sharedCounter << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Exit section
        flag1.store(false, std::memory_order_seq_cst);
        
        // Remainder section
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void basicPetersonExample() {
    std::cout << "\n--- Basic Peterson's Solution ---\n" << std::endl;
    
    sharedCounter = 0;
    flag0.store(false);
    flag1.store(false);
    turn.store(0);
    
    std::thread t0(thread0Work, 5);
    std::thread t1(thread1Work, 5);
    
    if (t0.joinable()) t0.join();
    if (t1.joinable()) t1.join();
    
    std::cout << "\nFinal counter value: " << sharedCounter << " (expected: 10)" << std::endl;
}

// Example 2: Peterson's Solution preventing race conditions
std::atomic<bool> flag0_bank(false);
std::atomic<bool> flag1_bank(false);
std::atomic<int> turn_bank(0);
int balance = 100;

void deposit(int amount, int threadId) {
    // Entry section for Peterson's algorithm
    if (threadId == 0) {
        flag0_bank.store(true, std::memory_order_seq_cst);
        turn_bank.store(1, std::memory_order_seq_cst);
        while (flag1_bank.load(std::memory_order_seq_cst) && turn_bank.load(std::memory_order_seq_cst) == 1) {
            // Wait
        }
    } else {
        flag1_bank.store(true, std::memory_order_seq_cst);
        turn_bank.store(0, std::memory_order_seq_cst);
        while (flag0_bank.load(std::memory_order_seq_cst) && turn_bank.load(std::memory_order_seq_cst) == 0) {
            // Wait
        }
    }
    
    // Critical section
    int temp = balance;
    std::cout << "Thread " << threadId << ": Read balance = " << temp << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    temp += amount;
    balance = temp;
    std::cout << "Thread " << threadId << ": Deposited " << amount << ", new balance = " << balance << std::endl;
    
    // Exit section
    if (threadId == 0) {
        flag0_bank.store(false, std::memory_order_seq_cst);
    } else {
        flag1_bank.store(false, std::memory_order_seq_cst);
    }
}

void bankAccountExample() {
    std::cout << "\n--- Bank Account Example (Race Condition Prevention) ---\n" << std::endl;
    
    balance = 100;
    flag0_bank.store(false);
    flag1_bank.store(false);
    turn_bank.store(0);
    
    std::cout << "Initial balance: " << balance << std::endl;
    std::cout << "Both threads will deposit 50...\n" << std::endl;
 
    std::thread t0(deposit, 50, 0);
    std::thread t1(deposit, 50, 1);
    
    if (t0.joinable()) t0.join();
    if (t1.joinable()) t1.join();
    
    std::cout << "\nFinal balance: " << balance << " (expected: 200)" << std::endl;
}

// Example 3: Peterson's Solution with detailed step-by-step logging
std::atomic<bool> flag0_detail(false);
std::atomic<bool> flag1_detail(false);
std::atomic<int> turn_detail(0);

void thread0Detailed() {
    std::cout << "Thread 0: Starting" << std::endl;
    std::cout << "Thread 0: Setting flag0 = true" << std::endl;
    flag0_detail.store(true, std::memory_order_seq_cst);
    
    std::cout << "Thread 0: Setting turn = 1 (giving priority to thread 1)" << std::endl;
    turn_detail.store(1, std::memory_order_seq_cst);
    
    std::cout << "Thread 0: Checking entry condition..." << std::endl;
    while (flag1_detail.load(std::memory_order_seq_cst) && turn_detail.load(std::memory_order_seq_cst) == 1) {
        std::cout << "Thread 0: Waiting (thread 1 has priority)..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Thread 0: >>> ENTERED CRITICAL SECTION <<<" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "Thread 0: >>> LEAVING CRITICAL SECTION <<<" << std::endl;
    
    flag0_detail.store(false, std::memory_order_seq_cst);
    std::cout << "Thread 0: Released lock (flag0 = false)" << std::endl;
}

void thread1Detailed() {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::cout << "Thread 1: Starting" << std::endl;
    std::cout << "Thread 1: Setting flag1 = true" << std::endl;
    flag1_detail.store(true, std::memory_order_seq_cst);
    
    std::cout << "Thread 1: Setting turn = 0 (giving priority to thread 0)" << std::endl;
    turn_detail.store(0, std::memory_order_seq_cst);
    
    std::cout << "Thread 1: Checking entry condition..." << std::endl;
    while (flag0_detail.load(std::memory_order_seq_cst) && turn_detail.load(std::memory_order_seq_cst) == 0) {
        std::cout << "Thread 1: Waiting (thread 0 has priority)..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Thread 1: >>> ENTERED CRITICAL SECTION <<<" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "Thread 1: >>> LEAVING CRITICAL SECTION <<<" << std::endl;
    
    flag1_detail.store(false, std::memory_order_seq_cst);
    std::cout << "Thread 1: Released lock (flag1 = false)" << std::endl;
}

void detailedExample() {
    std::cout << "\n--- Detailed Step-by-Step Example ---\n" << std::endl;
    
    flag0_detail.store(false);
    flag1_detail.store(false);
    turn_detail.store(0);
    
    std::thread t0(thread0Detailed);
    std::thread t1(thread1Detailed);
    
    if (t0.joinable()) t0.join();
    if (t1.joinable()) t1.join();
}

int main()
{
    std::cout << " ========================= PROGRAM START ========================= " << std::endl;

    basicPetersonExample();
    
    bankAccountExample();
    
    detailedExample();

    std::cout << " ========================== PROGRAM END ========================== " << std::endl;

    return 0;
}