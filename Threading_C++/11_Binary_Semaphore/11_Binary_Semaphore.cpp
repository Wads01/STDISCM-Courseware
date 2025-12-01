#include "main.hpp"
#include <semaphore>

// BINARY SEMAPHORE
// 1. std::binary_semaphore is a synchronization primitive that can have only two states: 0 (not available) or 1 (available).
// 2. It is used to signal between threads, allowing one thread to notify another that a resource or condition is ready.
// 3. Unlike mutexes, semaphores are designed for signaling rather than mutual exclusion.
// 4. A binary semaphore initialized with 0 will block on acquire() until another thread calls release().
// 5. A binary semaphore initialized with 1 will not block on the first acquire().

// Key Methods:
// 1. acquire(): Decrements the semaphore. If the value is 0, the calling thread blocks until release() is called.
// 2. release(): Increments the semaphore and unblocks one waiting thread (if any).
// 3. try_acquire(): Tries to acquire without blocking. Returns true if successful, false otherwise.

std::binary_semaphore smphSignal(0); // Initialized to 0, so first acquire() will block
std::binary_semaphore smphWorkDone(0);

void worker() {
    std::cout << "Worker thread waiting for signal..." << std::endl;
    
    smphSignal.acquire(); // Wait for the signal from main thread
    
    std::cout << "Worker thread received signal! Starting work..." << std::endl;
    
    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    
    std::cout << "Worker thread finished work!" << std::endl;
    
    smphWorkDone.release(); // Signal that work is complete
}

void taskCoordination() {
    std::cout << "\n--- Task Coordination Example ---\n" << std::endl;
    
    std::thread t1(worker);
    
    std::cout << "Main thread doing some preparation..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    std::cout << "Main thread signaling worker to start!" << std::endl;
    smphSignal.release(); // Signal the worker to start
    
    std::cout << "Main thread waiting for worker to complete..." << std::endl;
    smphWorkDone.acquire(); // Wait for worker to finish
    
    std::cout << "Main thread: Worker has completed!" << std::endl;
    
    if (t1.joinable()) t1.join();
}

// Example 2: Simple synchronization between producer and consumer
std::binary_semaphore dataReady(0);
int sharedData = 0;

void producer() {
    std::cout << "Producer: Preparing data..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    
    sharedData = 42; // Produce data
    std::cout << "Producer: Data ready (value = " << sharedData << ")" << std::endl;
    
    dataReady.release(); // Signal that data is ready
}

void consumer() {
    std::cout << "Consumer: Waiting for data..." << std::endl;
    
    dataReady.acquire(); // Wait for data to be ready
    
    std::cout << "Consumer: Received data (value = " << sharedData << ")" << std::endl;
}

void producerConsumerExample() {
    std::cout << "\n--- Producer-Consumer Example ---\n" << std::endl;
    
    std::thread t1(consumer);
    std::thread t2(producer);
 
    if (t1.joinable()) t1.join();
    if (t2.joinable()) t2.join();
}

// Example 3: try_acquire demonstration
std::binary_semaphore tryAcquireSem(0);

void tryAcquireExample() {
    std::cout << "\n--- try_acquire Example ---\n" << std::endl;
    
    if (tryAcquireSem.try_acquire()) {
      std::cout << "Successfully acquired semaphore on first try!" << std::endl;
    } else {
        std::cout << "Could not acquire semaphore (as expected - initialized to 0)" << std::endl;
  }
    
    std::cout << "Releasing semaphore..." << std::endl;
    tryAcquireSem.release();
 
    if (tryAcquireSem.try_acquire()) {
        std::cout << "Successfully acquired semaphore after release!" << std::endl;
    } else {
        std::cout << "Could not acquire semaphore" << std::endl;
    }
}

int main()
{
    std::cout << " ========================= PROGRAM START ========================= " << std::endl;

    taskCoordination();
    
    producerConsumerExample();
    
    tryAcquireExample();

    std::cout << " ========================== PROGRAM END ========================== " << std::endl;

    return 0;
}