#include "main.hpp"
#include <queue>

// MONITORS
// 1. A monitor is a high-level synchronization construct that encapsulates shared data and the operations on that data.
// 2. It provides mutual exclusion automatically - only one thread can execute inside the monitor at a time.
// 3. Monitors combine mutexes and condition variables into a single, easy-to-use abstraction.
// 4. In C++, we implement monitors using a class with private data, mutex, and condition variables.
// 5. Monitors are safer than raw mutexes because the synchronization logic is hidden within the class.

// Key Components of a Monitor:
// - Shared data (private member variables)
// - Mutex (for mutual exclusion)
// - Condition variables (for waiting/signaling)
// - Public methods (operations that access shared data)

// Benefits:
// 1. Encapsulation: Synchronization logic is hidden inside the class
// 2. Safety: Less prone to errors (no forgotten locks/unlocks)
// 3. Clarity: Cleaner code compared to manual mutex management

// Example 1: Simple Counter Monitor
class CounterMonitor {
private:
    int count;
    std::mutex mtx;
    
public:
    CounterMonitor() : count(0) {}
    
    void increment() {
       std::lock_guard<std::mutex> lock(mtx);
       count++;
       std::cout << "Counter incremented to: " << count << std::endl;
    }
    
    void decrement() {
        std::lock_guard<std::mutex> lock(mtx);
        count--;
        std::cout << "Counter decremented to: " << count << std::endl;
    }
    
    int getCount() {
        std::lock_guard<std::mutex> lock(mtx);
        return count;
    }
};

void counterWorker(CounterMonitor& monitor, int id, int operations) {
    for (int i = 0; i < operations; ++i) {
        if (id % 2 == 0) {
            monitor.increment();
        } else {
            monitor.decrement();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void simpleCounterExample() {
    std::cout << "\n--- Simple Counter Monitor ---\n" << std::endl;
    
    CounterMonitor monitor;
    
    std::thread t1(counterWorker, std::ref(monitor), 1, 3);
    std::thread t2(counterWorker, std::ref(monitor), 2, 3);
    
    if (t1.joinable()) t1.join();
    if (t2.joinable()) t2.join();
    
    std::cout << "\nFinal count: " << monitor.getCount() << std::endl;
}

// Example 2: Bounded Buffer Monitor (Producer-Consumer Problem)
template<typename T>
class BoundedBufferMonitor {
private:
    std::queue<T> buffer;
    size_t capacity;
    std::mutex mtx;
    std::condition_variable notFull;   // Signaled when buffer is not full
    std::condition_variable notEmpty;  // Signaled when buffer is not empty
    
public:
  BoundedBufferMonitor(size_t cap) : capacity(cap) {}
    
    // Producer calls this to add items
    void produce(const T& item) {
        std::unique_lock<std::mutex> lock(mtx);
    
        // Wait while buffer is full
        notFull.wait(lock, [this]() { return buffer.size() < capacity; });
        
        buffer.push(item);
        std::cout << "Produced: " << item << " | Buffer size: " << buffer.size() << std::endl;
        
        // Signal that buffer is not empty
        notEmpty.notify_one();
    }
    
    // Consumer calls this to remove items
    T consume() {
        std::unique_lock<std::mutex> lock(mtx);
 
        // Wait while buffer is empty
        notEmpty.wait(lock, [this]() { return !buffer.empty(); });
        
        T item = buffer.front();
        buffer.pop();
        std::cout << "Consumed: " << item << " | Buffer size: " << buffer.size() << std::endl;
        
        // Signal that buffer is not full
        notFull.notify_one();
      
        return item;
    }
    
    size_t size() {
        std::lock_guard<std::mutex> lock(mtx);
        return buffer.size();
    }
};

void producer(BoundedBufferMonitor<int>& buffer, int id, int itemsToProduce) {
    for (int i = 1; i <= itemsToProduce; ++i) {
        int item = id * 100 + i;
        buffer.produce(item);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    std::cout << "Producer " << id << " finished." << std::endl;
}

void consumer(BoundedBufferMonitor<int>& buffer, int id, int itemsToConsume) {
    for (int i = 0; i < itemsToConsume; ++i) {
        int item = buffer.consume();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << "Consumer " << id << " finished." << std::endl;
}

void boundedBufferExample() {
    std::cout << "\n--- Bounded Buffer Monitor (Capacity: 3) ---\n" << std::endl;
    
    BoundedBufferMonitor<int> buffer(3);
  
    std::thread p1(producer, std::ref(buffer), 1, 5);
    std::thread p2(producer, std::ref(buffer), 2, 5);
    std::thread c1(consumer, std::ref(buffer), 1, 5);
    std::thread c2(consumer, std::ref(buffer), 2, 5);
    
    if (p1.joinable()) p1.join();
    if (p2.joinable()) p2.join();
    if (c1.joinable()) c1.join();
    if (c2.joinable()) c2.join();
    
    std::cout << "\nAll producers and consumers finished!" << std::endl;
}

// Example 3: Bank Account Monitor
class BankAccountMonitor {
private:
    int balance;
    std::mutex mtx;
    std::condition_variable sufficientFunds;
  
public:
    BankAccountMonitor(int initialBalance) : balance(initialBalance) {}
    
    void deposit(int amount) {
        std::lock_guard<std::mutex> lock(mtx);
        balance += amount;
      std::cout << "Deposited: " << amount << " | New balance: " << balance << std::endl;
        
        // Notify waiting threads that funds are now available
        sufficientFunds.notify_all();
    }
    
    void withdraw(int amount) {
        std::unique_lock<std::mutex> lock(mtx);
        
   // Wait until we have sufficient funds
     std::cout << "Attempting to withdraw: " << amount << " | Current balance: " << balance << std::endl;
  sufficientFunds.wait(lock, [this, amount]() { return balance >= amount; });
        
balance -= amount;
        std::cout << "Withdrew: " << amount << " | New balance: " << balance << std::endl;
    }
    
    int getBalance() {
        std::lock_guard<std::mutex> lock(mtx);
        return balance;
 }
};

void bankDepositor(BankAccountMonitor& account, int amount) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    account.deposit(amount);
}

void bankWithdrawer(BankAccountMonitor& account, int amount) {
    account.withdraw(amount);
}

void bankAccountExample() {
    std::cout << "\n--- Bank Account Monitor ---\n" << std::endl;
    
    BankAccountMonitor account(100);
    std::cout << "Initial balance: " << account.getBalance() << "\n" << std::endl;
    
    // Try to withdraw more than available (will wait)
 std::thread w1(bankWithdrawer, std::ref(account), 150);
    
    // Deposit money after a delay
    std::thread d1(bankDepositor, std::ref(account), 100);
    
    if (w1.joinable()) w1.join();
    if (d1.joinable()) d1.join();
    
    std::cout << "\nFinal balance: " << account.getBalance() << std::endl;
}

int main()
{
    std::cout << " ========================= PROGRAM START ========================= " << std::endl;

    simpleCounterExample();
    
    boundedBufferExample();
    
    bankAccountExample();

    std::cout << " ========================== PROGRAM END ========================== " << std::endl;

    return 0;
}