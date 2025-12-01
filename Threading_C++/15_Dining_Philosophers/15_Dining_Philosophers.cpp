#include "main.hpp"
#include <semaphore>
#include <array>

// DINING PHILOSOPHERS PROBLEM
// 1. Classic synchronization problem involving resource allocation and deadlock avoidance.
// 2. Five philosophers sit at a round table with five forks (one between each pair of philosophers).
// 3. Each philosopher alternates between thinking and eating.
// 4. To eat, a philosopher needs BOTH the left and right fork.
// 5. Challenge: Design a solution that avoids deadlock, starvation, and ensures mutual exclusion.

// The Problem:
// - If all philosophers pick up their left fork simultaneously, no one can pick up their right fork
// - This creates a DEADLOCK - everyone is waiting forever

// We will explore THREE solutions:
// 1. Naïve Semaphore Solution (HAS DEADLOCK PROBLEM!)
// 2. Asymmetric Solution (prevents circular wait - SAFE)
// 3. Monitor Solution (elegant and safe)

constexpr int NUM_PHILOSOPHERS = 5;
constexpr int EATING_TIME = 1000;      // milliseconds
constexpr int THINKING_TIME = 1000;    // milliseconds
constexpr int MEALS_PER_PHILOSOPHER = 3;

// ============================================================================
// SOLUTION 1: NAÏVE SEMAPHORE SOLUTION (DEADLOCK POSSIBLE!)
// ============================================================================
// Problem: All philosophers pick up left fork, then wait for right fork forever

std::array<std::binary_semaphore, NUM_PHILOSOPHERS> forks_naive{
    std::binary_semaphore{1}, std::binary_semaphore{1}, std::binary_semaphore{1},
    std::binary_semaphore{1}, std::binary_semaphore{1}
};

void philosopherNaive(int id) {
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;
 
    for (int meal = 0; meal < MEALS_PER_PHILOSOPHER; ++meal) {
        // Think
        std::cout << "Philosopher " << id << " is thinking..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(THINKING_TIME));
        
        // Pick up forks (DEADLOCK CAN HAPPEN HERE!)
        std::cout << "Philosopher " << id << " is hungry and reaching for forks..." << std::endl;
        
        forks_naive[left].acquire();  // Pick up left fork
        std::cout << "Philosopher " << id << " picked up left fork " << left << std::endl;
    
        // Small delay increases chance of deadlock for demonstration
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
        forks_naive[right].acquire(); // Pick up right fork (might wait forever!)
        std::cout << "Philosopher " << id << " picked up right fork " << right << std::endl;
        
        // Eat
        std::cout << "Philosopher " << id << " is EATING (meal " << (meal + 1) << "/" 
        << MEALS_PER_PHILOSOPHER << ")" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(EATING_TIME));
        
        // Put down forks
        forks_naive[left].release();
        forks_naive[right].release();
        std::cout << "Philosopher " << id << " finished eating and put down forks." << std::endl;
    }
    
  std::cout << "Philosopher " << id << " is done!" << std::endl;
}

void naiveSemaphoreSolution() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "SOLUTION 1: NAÏVE SEMAPHORE (DEADLOCK RISK!)" << std::endl;
    std::cout << "========================================\n" << std::endl;
    std::cout << "WARNING: This solution can deadlock if all philosophers" << std::endl;
    std::cout << "   pick up their left fork at the same time!\n" << std::endl;
    
    std::array<std::thread, NUM_PHILOSOPHERS> philosophers;
    
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        philosophers[i] = std::thread(philosopherNaive, i);
    }
    
    for (auto& p : philosophers) {
        if (p.joinable()) p.join();
    }
    
    std::cout << "\nAll philosophers finished (if no deadlock occurred)!" << std::endl;
}

// ============================================================================
// SOLUTION 2: ASYMMETRIC SOLUTION (PREVENTS CIRCULAR WAIT - SAFE!)
// ============================================================================
// Fix: Even-numbered philosophers pick up left first, odd-numbered pick up right first
// This breaks the circular wait condition and prevents deadlock

std::array<std::binary_semaphore, NUM_PHILOSOPHERS> forks_asymmetric{
    std::binary_semaphore{1}, std::binary_semaphore{1}, std::binary_semaphore{1},
    std::binary_semaphore{1}, std::binary_semaphore{1}
};

void philosopherAsymmetric(int id) {
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    
    for (int meal = 0; meal < MEALS_PER_PHILOSOPHER; ++meal) {
        // Think
        std::cout << "Philosopher " << id << " is thinking..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(THINKING_TIME));
        
        std::cout << "Philosopher " << id << " is hungry and reaching for forks..." << std::endl;
    
        // ASYMMETRIC PICKUP ORDER
        if (id % 2 == 0) {
            // Even philosophers: pick up left fork first
            forks_asymmetric[left].acquire();
                std::cout << "Philosopher " << id << " (even) picked up left fork " << left << std::endl;
    
            forks_asymmetric[right].acquire();
            std::cout << "Philosopher " << id << " (even) picked up right fork " << right << std::endl;
        } else {
            // Odd philosophers: pick up right fork first
            forks_asymmetric[right].acquire();
                std::cout << "Philosopher " << id << " (odd) picked up right fork " << right << std::endl;
         
                forks_asymmetric[left].acquire();
                std::cout << "Philosopher " << id << " (odd) picked up left fork " << left << std::endl;
        }
     
        // Eat
        std::cout << "Philosopher " << id << " is EATING (meal " << (meal + 1) << "/" 
        << MEALS_PER_PHILOSOPHER << ")" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(EATING_TIME));
   
        // Put down forks (order doesn't matter)
        forks_asymmetric[left].release();
        forks_asymmetric[right].release();
        std::cout << "Philosopher " << id << " finished eating and put down forks." << std::endl;
    }
    
    std::cout << "Philosopher " << id << " is done!" << std::endl;
}

void asymmetricSolution() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "SOLUTION 2: ASYMMETRIC SOLUTION (SAFE!)" << std::endl;
    std::cout << "========================================\n" << std::endl;
    std::cout << "Even philosophers pick up LEFT first, odd pick up RIGHT first." << std::endl;
    std::cout << "This breaks circular wait and prevents deadlock!\n" << std::endl;
    
    std::array<std::thread, NUM_PHILOSOPHERS> philosophers;
    
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        philosophers[i] = std::thread(philosopherAsymmetric, i);
    }
    
    for (auto& p : philosophers) {
        if (p.joinable()) p.join();
    }
    
    std::cout << "\nAll philosophers finished successfully!" << std::endl;
}

// ============================================================================
// SOLUTION 3: MONITOR SOLUTION (ELEGANT AND SAFE!)
// ============================================================================
// Uses a monitor to coordinate fork access, ensuring no deadlock

enum class State { THINKING, HUNGRY, EATING };

class DiningPhilosophersMonitor {
private:
    std::array<State, NUM_PHILOSOPHERS> states;
    std::array<std::condition_variable, NUM_PHILOSOPHERS> canEat;
    std::mutex mtx;
    
    int left(int i) { return (i + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS; }
    int right(int i) { return (i + 1) % NUM_PHILOSOPHERS; }
    
    void test(int i) {
        // Can eat if hungry and both neighbors are not eating
        if (states[i] == State::HUNGRY &&
            states[left(i)] != State::EATING &&
            states[right(i)] != State::EATING) {
      
            states[i] = State::EATING;
            canEat[i].notify_one();
        }
    }
    
public:
    DiningPhilosophersMonitor() {
    for (auto& state : states) {
            state = State::THINKING;
        }
    }
    
    void pickupForks(int i) {
        std::unique_lock<std::mutex> lock(mtx);
        
        states[i] = State::HUNGRY;
        std::cout << "Philosopher " << i << " is HUNGRY" << std::endl;
        
        test(i);  // Try to acquire forks
        
        // Wait if we can't eat yet
        while (states[i] != State::EATING) {
            std::cout << "Philosopher " << i << " is WAITING for forks..." << std::endl;
            canEat[i].wait(lock);
        }
        
        std::cout << "Philosopher " << i << " picked up both forks and is EATING" << std::endl;
    }
    
    void putdownForks(int i) {
        std::unique_lock<std::mutex> lock(mtx);
  
        states[i] = State::THINKING;
        std::cout << "Philosopher " << i << " put down forks" << std::endl;
        
        // Check if neighbors can now eat
        test(left(i));
        test(right(i));
    }
};

void philosopherMonitor(int id, DiningPhilosophersMonitor& monitor) {
    for (int meal = 0; meal < MEALS_PER_PHILOSOPHER; ++meal) {
        // Think
        std::cout << "Philosopher " << id << " is thinking..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(THINKING_TIME));
        
        // Pick up forks (monitor handles synchronization)
        monitor.pickupForks(id);
        
        // Eat
        std::cout << "Philosopher " << id << " is EATING (meal " << (meal + 1) << "/" 
          << MEALS_PER_PHILOSOPHER << ")" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(EATING_TIME));
        
        // Put down forks
        monitor.putdownForks(id);
    }
    
    std::cout << "Philosopher " << id << " is done!" << std::endl;
}

void monitorSolution() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "SOLUTION 3: MONITOR SOLUTION (ELEGANT!)" << std::endl;
    std::cout << "========================================\n" << std::endl;
    std::cout << "Monitor ensures philosophers only eat when both forks are available." << std::endl;
    std::cout << "No deadlock, no starvation!\n" << std::endl;
    
    DiningPhilosophersMonitor monitor;
    std::array<std::thread, NUM_PHILOSOPHERS> philosophers;
    
    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        philosophers[i] = std::thread(philosopherMonitor, i, std::ref(monitor));
    }
    
    for (auto& p : philosophers) {
        if (p.joinable()) p.join();
    }
    
    std::cout << "\nAll philosophers finished successfully!" << std::endl;
}

int main()
{
    std::cout << " ========================= PROGRAM START ========================= " << std::endl;
 
    std::cout << "\n***** DINING PHILOSOPHERS PROBLEM *****" << std::endl;
    std::cout << "5 philosophers, 5 forks, each needs 2 forks to eat\n" << std::endl;
    
    // Uncomment ONE solution at a time to test:
    
    // WARNING: This might deadlock! If it hangs, press Ctrl+C
    // naiveSemaphoreSolution();
    
    asymmetricSolution();
    
    // monitorSolution();

    std::cout << " ========================== PROGRAM END ========================== " << std::endl;

    return 0;
}