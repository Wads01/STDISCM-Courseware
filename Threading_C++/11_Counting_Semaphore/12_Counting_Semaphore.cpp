#include "main.hpp"
#include <semaphore>
#include <vector>

// COUNTING SEMAPHORE
// 1. std::counting_semaphore is a synchronization primitive that maintains a counter representing available resources.
// 2. Unlike binary_semaphore (which can only be 0 or 1), counting_semaphore can have any non-negative integer value.
// 3. It is commonly used to control access to a pool of resources (e.g., database connections, thread pool slots).
// 4. The counter is decremented by acquire() and incremented by release().
// 5. When the counter reaches 0, threads calling acquire() will block until another thread calls release().

// Template Parameter:
// std::counting_semaphore<LeastMaxValue> - specifies the minimum guaranteed maximum value the semaphore can hold.

// Key Methods:
// 1. acquire(): Decrements the counter. Blocks if counter is 0.
// 2. release(update = 1): Increments the counter by 'update' (default 1).
// 3. try_acquire(): Tries to acquire without blocking. Returns true if successful, false otherwise.

// Example 1: Resource Pool - Only 3 workers can access the resource at the same time
constexpr int MAX_CONCURRENT = 3;
std::counting_semaphore<MAX_CONCURRENT> resourcePool(MAX_CONCURRENT);

void worker(int id) {
    std::cout << "Worker " << id << " is waiting to access the resource..." << std::endl;
    
  resourcePool.acquire(); // Try to get access (will block if 3 workers are already using it)
    
    std::cout << "Worker " << id << " got access! Working..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::cout << "Worker " << id << " finished and releasing access." << std::endl;
    
    resourcePool.release(); // Release access for another worker
}

void resourcePoolExample() {
 std::cout << "\n--- Resource Pool Example (Max " << MAX_CONCURRENT << " concurrent workers) ---\n" << std::endl;
    
    std::vector<std::thread> workers;
    
    // Create 6 workers, but only 3 can work at the same time
    for (int i = 1; i <= 6; ++i) {
        workers.emplace_back(worker, i);
    }
    
    for (auto& w : workers) {
        if (w.joinable()) w.join();
    }
}

// Example 2: Parking Lot - Fixed number of parking spots
constexpr int PARKING_SPOTS = 2;
std::counting_semaphore<PARKING_SPOTS> parkingLot(PARKING_SPOTS);

void car(int id) {
    std::cout << "Car " << id << " looking for parking..." << std::endl;
    
    parkingLot.acquire(); // Park (will wait if lot is full)
    
    std::cout << "Car " << id << " parked!" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    std::cout << "Car " << id << " leaving..." << std::endl;
    
    parkingLot.release(); // Free the spot
}

void parkingLotExample() {
    std::cout << "\n--- Parking Lot Example (" << PARKING_SPOTS << " spots) ---\n" << std::endl;

    std::vector<std::thread> cars;
    
    // 5 cars but only 2 spots
    for (int i = 1; i <= 5; ++i) {
        cars.emplace_back(car, i);
    }
    
  for (auto& c : cars) {
        if (c.joinable()) c.join();
    }
}

// Example 3: try_acquire demonstration
std::counting_semaphore<3> tryAcquireSem(2); // Start with 2 available

void tryAcquireExample() {
    std::cout << "\n--- try_acquire Example (Initial count: 2) ---\n" << std::endl;
    
    // Try to acquire 3 times
    if (tryAcquireSem.try_acquire()) {
        std::cout << "First try_acquire: Success!" << std::endl;
    }
    
    if (tryAcquireSem.try_acquire()) {
        std::cout << "Second try_acquire: Success!" << std::endl;
    }
    
 if (tryAcquireSem.try_acquire()) {
        std::cout << "Third try_acquire: Success!" << std::endl;
    } else {
  std::cout << "Third try_acquire: Failed (counter is 0)" << std::endl;
    }
    
    std::cout << "\nReleasing 2 resources at once..." << std::endl;
    tryAcquireSem.release(2); // Release 2 at the same time
    
    if (tryAcquireSem.try_acquire()) {
        std::cout << "After release: Success!" << std::endl;
    }
}

int main()
{
    std::cout << " ========================= PROGRAM START ========================= " << std::endl;

    resourcePoolExample();
    
    parkingLotExample();
    
    tryAcquireExample();

    std::cout << " ========================== PROGRAM END ========================== " << std::endl;

    return 0;
}

