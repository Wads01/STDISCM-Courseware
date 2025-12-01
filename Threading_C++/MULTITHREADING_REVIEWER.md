# Multithreading Concepts - Complete Reviewer

## Table of Contents
1. [Core Concepts](#core-concepts)
2. [Synchronization Primitives](#synchronization-primitives)
3. [Classic Problems & Solutions](#classic-problems--solutions)
4. [Deadlock](#deadlock)
5. [Advanced Patterns](#advanced-patterns)
6. [Quick Reference](#quick-reference)

---

## Core Concepts

### What is Multithreading?
- **Definition**: Running multiple threads concurrently within a single process
- **Purpose**: Improve performance, responsiveness, and resource utilization
- **Challenge**: Managing shared resources safely without race conditions

### Race Condition
**What it is:**
- Occurs when two or more threads access shared data simultaneously
- At least one thread modifies the data
- The outcome depends on the timing/order of execution

**Example:**
```cpp
int counter = 0;
// Thread 1: counter++;
// Thread 2: counter++;
// Expected: 2, Actual: Could be 1 or 2 (race condition!)
```

**Why it happens:**
- `counter++` is actually 3 operations:
  1. Read value
  2. Increment value
  3. Write value back
- Threads can interleave these operations unpredictably

### Critical Section
- **Definition**: A code segment that accesses shared resources
- **Requirement**: Only one thread should execute it at a time
- **Goal**: Ensure mutual exclusion

---

## Synchronization Primitives

### 1. Mutex (Mutual Exclusion)

**Purpose:** Protect shared data from simultaneous access

**Key Methods:**
- `lock()`: Acquire the mutex (blocks if already locked)
- `unlock()`: Release the mutex
- `try_lock()`: Try to acquire without blocking

**Properties:**
- ✅ Provides mutual exclusion
- ✅ Simple to understand
- ⚠️ Must manually lock/unlock (error-prone)
- ⚠️ Can deadlock if not careful

**When to use:**
- Protecting simple critical sections
- When you need explicit control over lock timing

**Example:**
```cpp
std::mutex mtx;

void criticalSection() {
    mtx.lock();
 // Access shared data
    mtx.unlock();
}
```

---

### 2. Lock Guard

**Purpose:** RAII wrapper for mutex (automatic unlock)

**Key Features:**
- ✅ Automatically unlocks when goes out of scope
- ✅ Exception-safe
- ✅ Simple and safe
- ❌ Cannot be moved or copied
- ❌ Cannot manually unlock early

**When to use:**
- Default choice for most mutex locking
- When you want simple, safe locking for entire scope

**Example:**
```cpp
std::mutex mtx;

void safeFunction() {
    std::lock_guard<std::mutex> lock(mtx);
    // Mutex automatically unlocked when lock goes out of scope
    // Even if exception is thrown!
}
```

---

### 3. Unique Lock

**Purpose:** Flexible RAII wrapper for mutex

**Key Features:**
- ✅ Can be moved (transfer ownership)
- ✅ Can unlock/relock manually
- ✅ Supports deferred locking
- ✅ Works with condition variables
- ✅ More flexible than lock_guard
- ⚠️ Slightly more overhead

**Locking Strategies:**

| Strategy | Description | Use Case |
|----------|-------------|----------|
| **Default** | Locks immediately | Normal usage |
| **defer_lock** | Don't lock on construction | Lock later manually |
| **try_to_lock** | Try to lock, don't block | Check if lock available |
| **adopt_lock** | Assume already locked | Take ownership of existing lock |

**When to use:**
- With condition variables
- When you need to unlock before scope ends
- When you need to transfer lock ownership

**Example:**
```cpp
std::mutex mtx;

void flexibleFunction() {
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
    
    // Do some work without lock
    
    lock.lock(); // Lock when needed
    // Critical section
    lock.unlock(); // Unlock early if needed
    
 // More non-critical work
}
```

---

### 4. Condition Variable

**Purpose:** Wait for a condition to become true / Signal when condition changes

**Key Methods:**
- `wait(lock, predicate)`: Wait until predicate is true
- `notify_one()`: Wake up one waiting thread
- `notify_all()`: Wake up all waiting threads

**How it works:**
1. Thread locks mutex
2. Checks condition
3. If false, calls `wait()` (releases mutex and sleeps)
4. Another thread changes condition and calls `notify_*()`
5. Waiting thread wakes up, reacquires mutex, rechecks condition

**Important:**
- ✅ Always use with `std::unique_lock`
- ✅ Always use a predicate (protect against spurious wakeups)
- ✅ Must hold lock when calling wait/notify

**When to use:**
- Thread needs to wait for a specific condition
- Producer-consumer patterns
- Coordinating thread execution order

**Example:**
```cpp
std::mutex mtx;
std::condition_variable cv;
bool ready = false;

// Waiting thread
void waiter() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [] { return ready; }); // Wait until ready is true
    // Proceed when ready
}

// Signaling thread
void signaler() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
    }
    cv.notify_one(); // Wake up waiter
}
```

---

### 5. Semaphores

#### Binary Semaphore

**Purpose:** Signaling between threads (0 or 1)

**Key Characteristics:**
- Can only be 0 (not available) or 1 (available)
- Unlike mutex, designed for **signaling** not mutual exclusion
- Can be released by different thread than acquired it

**Key Methods:**
- `acquire()`: Decrement (wait if 0)
- `release()`: Increment (signal)
- `try_acquire()`: Non-blocking attempt

**When to use:**
- Signaling between threads
- Producer-consumer with single item
- Coordinating thread execution

**Example:**
```cpp
std::binary_semaphore signal(0); // Start at 0

// Thread 1
void worker() {
    signal.acquire(); // Wait for signal
    // Do work
}

// Thread 2
void coordinator() {
    // Do prep work
 signal.release(); // Signal worker to start
}
```

#### Counting Semaphore

**Purpose:** Control access to multiple identical resources

**Key Characteristics:**
- Can be any non-negative integer
- Represents number of available resources
- Perfect for resource pools

**Key Methods:**
- `acquire()`: Take one resource (decrement)
- `release(n)`: Return n resources (increment by n)
- `try_acquire()`: Try to take without blocking

**When to use:**
- Limited resource pool (connections, threads, etc.)
- Bounded buffer with multiple slots
- Rate limiting

**Example:**
```cpp
constexpr int MAX_CONNECTIONS = 5;
std::counting_semaphore<MAX_CONNECTIONS> pool(MAX_CONNECTIONS);

void useConnection() {
    pool.acquire(); // Get connection from pool
    // Use connection
    pool.release(); // Return connection to pool
}
```

---

### 6. Monitors

**Purpose:** High-level synchronization construct that encapsulates data + synchronization

**Key Characteristics:**
- ✅ Combines mutex + condition variables + data
- ✅ Automatic mutual exclusion for all methods
- ✅ Encapsulates synchronization logic
- ✅ Safer and cleaner than manual locking
- ✅ Professional design pattern

**Components:**
1. Private shared data
2. Mutex (for mutual exclusion)
3. Condition variables (for coordination)
4. Public methods (operations on shared data)

**When to use:**
- Complex shared data structures
- Producer-consumer patterns
- When you want clean encapsulation
- Professional/production code

**Example:**
```cpp
class BoundedBuffer {
private:
    std::queue<int> buffer;
    size_t capacity;
    std::mutex mtx;
    std::condition_variable notFull, notEmpty;
    
public:
    BoundedBuffer(size_t cap) : capacity(cap) {}
    
    void produce(int item) {
        std::unique_lock<std::mutex> lock(mtx);
        notFull.wait(lock, [this] { return buffer.size() < capacity; });
        buffer.push(item);
        notEmpty.notify_one();
    }
    
    int consume() {
        std::unique_lock<std::mutex> lock(mtx);
        notEmpty.wait(lock, [this] { return !buffer.empty(); });
        int item = buffer.front();
        buffer.pop();
        notFull.notify_one();
        return item;
    }
};
```

---

### 7. Atomic Variables

**Purpose:** Thread-safe operations without locks

**Key Characteristics:**
- Operations are **atomic** (indivisible)
- No race conditions on individual operations
- Provides memory ordering guarantees
- Lock-free (no blocking)

**Memory Ordering:**
- `memory_order_seq_cst`: Sequentially consistent (default, safest)
- `memory_order_acquire`: Acquire semantics
- `memory_order_release`: Release semantics
- `memory_order_relaxed`: No ordering constraints

**When to use:**
- Simple counters, flags
- Lock-free algorithms
- When locks are too heavy
- Peterson's algorithm (requires atomics!)

**Example:**
```cpp
std::atomic<int> counter(0);

void increment() {
    counter++; // Atomic operation, no race condition
}

void atomicOps() {
    counter.store(10, std::memory_order_seq_cst);
    int value = counter.load(std::memory_order_seq_cst);
    counter.fetch_add(5);
}
```

---

## Classic Problems & Solutions

### Peterson's Solution

**What it is:**
- Classic software-based mutual exclusion for **2 threads**
- Uses only shared memory (no hardware support)
- Guarantees: Mutual Exclusion, Progress, Bounded Waiting

**Algorithm Components:**
- `flag[i]`: Thread i wants to enter critical section
- `turn`: Whose turn it is (tie-breaker)

**How it works:**
```cpp
// Thread i wants to enter
flag[i] = true;    // I want to enter
turn = j;          // Give priority to other thread
while (flag[j] && turn == j) {
    // Wait
}
// Critical section
flag[i] = false;          // I'm done
```

**Why it works:**
- If both threads want to enter, `turn` breaks the tie
- One thread will always see the other thread's flag as false OR turn will favor it
- Guarantees only one thread in critical section

**Important:**
- ⚠️ Requires atomic variables in C++ (`std::atomic`)
- ⚠️ Only works for 2 threads
- ⚠️ Uses busy waiting (CPU intensive)

**Modern usage:**
- Mostly academic/historical
- Use mutexes in production code

---

### Dining Philosophers Problem

**The Problem:**
- 5 philosophers sit at round table
- 5 forks (one between each pair)
- To eat, philosopher needs BOTH left AND right fork
- Challenge: Avoid deadlock while allowing eating

**Visual:**
```
     P0
  F0    F1
P4        P1
  F4    F2
   P3  P2
     F3
```

#### Solution 1: Naïve (DEADLOCK!)

**Approach:** Everyone picks up left fork, then right fork

**Problem:**
```
All 5 philosophers pick up left fork simultaneously
→ No one can pick up right fork
→ DEADLOCK!
```

**Deadlock conditions present:**
- ✅ Mutual Exclusion (forks can't be shared)
- ✅ Hold-and-Wait (hold left, wait for right)
- ✅ No Preemption (can't steal forks)
- ✅ Circular Wait (P0→P1→P2→P3→P4→P0)

#### Solution 2: Asymmetric (SAFE!)

**Approach:** 
- Even philosophers: Pick up LEFT first
- Odd philosophers: Pick up RIGHT first

**Why it works:**
- Breaks **circular wait** condition
- At least one philosopher can complete pickup sequence

**Example:**
```cpp
if (id % 2 == 0) {
forks[left].acquire();
    forks[right].acquire();
} else {
    forks[right].acquire();  // Odd picks up right first
    forks[left].acquire();
}
```

#### Solution 3: Monitor (ELEGANT!)

**Approach:** Monitor tracks philosopher states

**States:**
- THINKING: Not interested in eating
- HUNGRY: Wants to eat
- EATING: Currently eating

**Rules:**
- Can only eat if BOTH neighbors are NOT eating
- Atomically checks and updates state

**Why it works:**
- Breaks **hold-and-wait** condition
- Acquires both forks atomically or waits
- Clean encapsulation of synchronization

---

## Deadlock

### What is Deadlock?

**Definition:** A state where threads are blocked forever, each waiting for resources held by others

**Example:**
```
Thread 1: Holds Lock A, waits for Lock B
Thread 2: Holds Lock B, waits for Lock A
→ Both wait forever = DEADLOCK
```

### Four Conditions for Deadlock

**ALL FOUR must be present simultaneously for deadlock to occur**

#### 1. Mutual Exclusion 🔒
**Definition:** A resource can only be held by one thread at a time

**Example:**
- Only one thread can hold a mutex
- Only one philosopher can hold a fork

**Can we break it?**
- ❌ Usually NO - often necessary for correctness
- Some resources can't be shared (e.g., printer)

---

#### 2. Hold-and-Wait 🤚⏳
**Definition:** Thread holds at least one resource while waiting for additional resources

**Example:**
```cpp
mutex1.lock();    // HOLD mutex1
// ... doing work ...
mutex2.lock();    // WAIT for mutex2 (while holding mutex1)
```

**How to break it:**
- ✅ Acquire all resources atomically (all or nothing)
- ✅ Release held resources if can't get all needed
- ✅ Use `std::lock(mutex1, mutex2)` to lock multiple at once

---

#### 3. No Preemption 🚫↩️
**Definition:** Resources cannot be forcibly taken away; must be voluntarily released

**Example:**
- Once philosopher picks up fork, no one can steal it
- Thread must voluntarily unlock mutex

**How to break it:**
- ✅ Allow timeouts (`try_lock_for()`)
- ✅ Force release after timeout
```cpp
if (!mutex.try_lock_for(2s)) {
  // Timeout, give up and retry
}
```

---

#### 4. Circular Wait 🔄
**Definition:** Circular chain of threads where each waits for resource held by next

**Example (Dining Philosophers):**
```
P0 waits for fork held by P1
P1 waits for fork held by P2
P2 waits for fork held by P3
P3 waits for fork held by P4
P4 waits for fork held by P0
↑__________________________|
     CIRCULAR DEPENDENCY!
```

**How to break it:**
- ✅ Order resources (always acquire in same order)
- ✅ Asymmetric solution (different threads use different orders)
- ✅ Assign numbers to resources, always acquire lower number first

---

### Deadlock Prevention Strategies

| Strategy | Condition Broken | How |
|----------|------------------|-----|
| **Resource Ordering** | Circular Wait | Always acquire locks in same order |
| **Lock Timeout** | No Preemption | Release locks after timeout |
| **All-or-Nothing** | Hold-and-Wait | Acquire all locks atomically |
| **Asymmetric Ordering** | Circular Wait | Different threads use different orders |
| **Monitor Pattern** | Hold-and-Wait | Atomic resource acquisition |

---

## Advanced Patterns

### Producer-Consumer Pattern

**Problem:** Coordinate producers creating items and consumers using items

**Solutions:**

#### With Condition Variables:
```cpp
std::queue<int> buffer;
std::mutex mtx;
std::condition_variable cv;

void producer() {
    while (true) {
        int item = produce_item();
        {
        std::lock_guard<std::mutex> lock(mtx);
          buffer.push(item);
        }
     cv.notify_one();
    }
}

void consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] { return !buffer.empty(); });
        int item = buffer.front();
        buffer.pop();
        lock.unlock();
      consume_item(item);
    }
}
```

**Explanation:**
- **Producer acquires lock** → Adds item to buffer → **Releases lock** → **Notifies one waiting consumer**
- **Consumer acquires lock** → **Waits with predicate `!buffer.empty()`**
  - If buffer is empty, `wait()` releases the lock and sleeps
  - When notified, wakes up, reacquires lock, and rechecks predicate
  - If still empty (spurious wakeup), goes back to sleep
  - If not empty, proceeds to consume
- **Why the predicate?** Protects against spurious wakeups and ensures buffer has data before consuming
- **Why unlock before consume_item()?** Don't hold lock during slow I/O operations - allows other threads to access buffer

---

#### With Semaphores:
```cpp
std::counting_semaphore<BUFFER_SIZE> empty_slots(BUFFER_SIZE);
std::counting_semaphore<BUFFER_SIZE> filled_slots(0);
std::mutex buffer_mutex;
std::queue<int> buffer;

void producer() {
    int item = produce_item();
    empty_slots.acquire();  // Wait if buffer is full
    {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        buffer.push(item);
    }
    filled_slots.release(); // Signal that one more slot is filled
}

void consumer() {
  filled_slots.acquire(); // Wait if buffer is empty
    int item;
    {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        item = buffer.front();
        buffer.pop();
    }
    empty_slots.release();  // Signal that one more slot is empty
    consume_item(item);
}
```

**Explanation:**
- **Two semaphores track buffer state:**
  - `empty_slots`: Counts available space in buffer (starts at BUFFER_SIZE)
  - `filled_slots`: Counts items in buffer (starts at 0)
- **Producer flow:**
  1. `empty_slots.acquire()` - Wait if buffer full (empty_slots == 0)
  2. Lock mutex, add item, unlock mutex
  3. `filled_slots.release()` - Increment filled count (signal consumer)
- **Consumer flow:**
  1. `filled_slots.acquire()` - Wait if buffer empty (filled_slots == 0)
  2. Lock mutex, remove item, unlock mutex
  3. `empty_slots.release()` - Increment empty count (signal producer)
- **Why mutex + semaphores?** Semaphores handle buffer fullness/emptiness, mutex protects actual data structure
- **Advantage:** More explicit counting of resources compared to CV approach

---

### Reader-Writer Problem

**Problem:** Multiple readers can read simultaneously, but writers need exclusive access

#### Solution 1: With Condition Variables

```cpp
class ReadWriteLock {
private:
    std::mutex mtx;
    std::condition_variable cv;
    int readers = 0;
    bool writer = false;
  
public:
    void read_lock() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !writer; });
        readers++;
  }
    
    void read_unlock() {
        std::lock_guard<std::mutex> lock(mtx);
        readers--;
        if (readers == 0) cv.notify_all();
    }
    
    void write_lock() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !writer && readers == 0; });
        writer = true;
    }
    
    void write_unlock() {
        std::lock_guard<std::mutex> lock(mtx);
        writer = false;
        cv.notify_all();
    }
};
```

**Explanation:**
- **State tracking:**
  - `readers`: Count of active readers (can be multiple)
  - `writer`: Boolean flag (only one writer allowed)
- **read_lock():**
  - **Condition: `!writer`** - Can read if no writer is active
  - Multiple readers can proceed simultaneously
  - Increment reader count
- **read_unlock():**
  - Decrement reader count
  - **If last reader** (`readers == 0`), notify all waiting threads
  - Allows waiting writer to proceed
- **write_lock():**
  - **Condition: `!writer && readers == 0`** - Can write only if:
    - No other writer active (exclusive write access)
    - No readers active (exclusive access to data)
  - Set writer flag to true
- **write_unlock():**
  - Clear writer flag
  - **Notify all** - Wake up all waiting readers and writers
- **Design choice:** Readers don't block each other, only writers block everyone

---

#### Solution 2: With Semaphores

```cpp
class ReadWriteLockSemaphore {
private:
    std::binary_semaphore resource_access{1};  // Protects the resource
    std::binary_semaphore read_count_access{1}; // Protects reader count
    int readers = 0;
  
public:
    void read_lock() {
        read_count_access.acquire();  // Lock reader count
        readers++;
        if (readers == 1) {
            // First reader locks resource (blocks writers)
            resource_access.acquire();
        }
        read_count_access.release();  // Unlock reader count
    }
    
    void read_unlock() {
        read_count_access.acquire();  // Lock reader count
        readers--;
        if (readers == 0) {
            // Last reader unlocks resource (allows writers)
            resource_access.release();
        }
        read_count_access.release();  // Unlock reader count
    }
    
    void write_lock() {
        resource_access.acquire();  // Exclusive access to resource
    }
    
    void write_unlock() {
        resource_access.release();  // Release resource
    }
};
```

**Explanation:**
- **Two semaphores:**
  - `resource_access`: Controls access to the actual shared resource (binary semaphore)
  - `read_count_access`: Protects the `readers` counter (binary semaphore acting as mutex)
- **read_lock():**
  1. Acquire `read_count_access` to safely modify `readers` count
  2. Increment `readers`
  3. **If first reader** (`readers == 1`), acquire `resource_access`
     - This blocks any writers from accessing the resource
     - Subsequent readers don't need to acquire resource_access
  4. Release `read_count_access`
- **read_unlock():**
  1. Acquire `read_count_access` to safely modify `readers` count
  2. Decrement `readers`
  3. **If last reader** (`readers == 0`), release `resource_access`
- Allows waiting writers to proceed
  4. Release `read_count_access`
- **write_lock():**
  - Simply acquire `resource_access` for exclusive access
  - Blocks if any readers or another writer is active
- **write_unlock():**
  - Release `resource_access`
  - Allows waiting readers or writers to proceed
- **Key insight:** First reader "locks out" writers, last reader "unlocks" for writers
- **Advantage:** Simpler than CV version, clearer semaphore semantics
- **Note:** This is a "readers-preference" solution - readers don't block each other, but writers must wait for all readers

---

### Comparison: CV vs Semaphores for Reader-Writer

| Aspect | Condition Variable | Semaphore |
|--------|-------------------|-----------|
| **Complexity** | More complex state logic | Simpler, more intuitive |
| **Flexibility** | Can add priority schemes easily | Less flexible |
| **Code clarity** | More verbose | More concise |
| **Typical use** | Complex coordination | Resource counting |
| **Writer starvation** | Can be addressed with queue | Requires additional logic |

Both solutions are valid - choose based on your needs and team familiarity!

---

## Quick Reference

### Synchronization Primitive Selection Guide

| Use Case | Best Choice | Why |
|----------|-------------|-----|
| Simple critical section | `std::lock_guard` | Simple, safe, automatic |
| Need to unlock early | `std::unique_lock` | Flexible control |
| With condition variable | `std::unique_lock` | Required for CV |
| Thread signaling | `std::binary_semaphore` | Designed for signaling |
| Resource pool | `std::counting_semaphore` | Counts available resources |
| Complex shared data | **Monitor pattern** | Clean encapsulation |
| Simple flags/counters | `std::atomic` | Lock-free, efficient |
| Multiple locks at once | `std::lock()` | Deadlock-safe |

---

### Common Pitfalls & Best Practices

#### ❌ Common Mistakes:

1. **Forgetting to unlock mutex**
   ```cpp
   mtx.lock();
   if (error) return; // FORGOT TO UNLOCK!
   mtx.unlock();
   ```
   ✅ **Fix:** Use `lock_guard` or `unique_lock`

2. **Deadlock from lock ordering**
   ```cpp
   // Thread 1
   lock_a.lock();
   lock_b.lock();
   
   // Thread 2
   lock_b.lock(); // Different order = DEADLOCK!
   lock_a.lock();
   ```
   ✅ **Fix:** Always lock in same order OR use `std::lock()`

3. **Lost notification**
   ```cpp
   cv.notify_one(); // No threads waiting yet
   // Later: thread calls wait() and misses notification!
   ```
   ✅ **Fix:** Always use predicate with `wait()`

4. **Spurious wakeup not handled**
   ```cpp
cv.wait(lock); // Might wake up even if condition not met!
   ```
   ✅ **Fix:** Always use predicate: `cv.wait(lock, []{return ready;})`

5. **Race condition with atomics**
   ```cpp
   if (atomic_flag) {     // Check
       atomic_flag = false; // Modify (not atomic together!)
   }
   ```
   ✅ **Fix:** Use `compare_exchange` or proper synchronization

---

#### ✅ Best Practices:

1. **Always use RAII for locks**
   - Prefer `lock_guard` or `unique_lock` over manual lock/unlock
   - Exception-safe and prevents forgot-to-unlock bugs

2. **Keep critical sections short**
   - Only protect what needs protection
   - Don't do I/O or expensive computation while holding lock

3. **Lock ordering**
   - Document required lock order
   - Use `std::lock()` for multiple locks
   - Consider using single lock if possible

4. **Condition variables**
   - Always use with `unique_lock`
   - Always use predicate
   - Hold lock when checking condition

5. **Prefer higher-level abstractions**
   - Use monitors/thread-safe classes
   - Encapsulate synchronization logic
   - Make threading invisible to users

6. **Avoid busy waiting**
   - Don't use `while` loops checking condition
   - Use condition variables or semaphores instead

7. **Test with thread sanitizers**
   - Use tools like ThreadSanitizer (TSan)
   - Test with multiple thread counts
   - Add artificial delays to expose race conditions

---

### Performance Considerations

| Primitive | Performance | Use When |
|-----------|-------------|----------|
| `std::atomic` | ⚡ Fastest | Simple operations, lock-free needed |
| `std::mutex` | ⚡⚡ Fast | General locking |
| `std::lock_guard` | ⚡⚡ Fast | Simple scoped locking |
| `std::unique_lock` | ⚡⚡ Fast | Flexible locking needed |
| `std::condition_variable` | ⚡⚡⚡ Moderate | Waiting for conditions |
| Semaphores | ⚡⚡⚡ Moderate | Signaling, resource counting |
| Monitors | ⚡⚡⚡ Moderate | Complex coordination |

**Note:** Performance impact is usually negligible compared to benefits of correct synchronization!

---

### Debugging Tips

1. **Enable Thread Sanitizer (TSan)**
   - Detects data races at runtime
   - Compile with `-fsanitize=thread`

2. **Add logging**
   ```cpp
   std::cout << "Thread " << std::this_thread::get_id() 
             << " acquiring lock\n";
   ```

3. **Use thread-safe logging**
   - Print statements can interleave!
   - Use mutex-protected logging or thread-safe logger

4. **Reduce thread count**
- Easier to debug with 2-3 threads
   - Increase once working

5. **Add artificial delays**
   ```cpp
   std::this_thread::sleep_for(std::chrono::milliseconds(100));
   ```
   - Helps expose race conditions
   - Makes timing bugs more reproducible

---

## Summary

### Key Takeaways:

1. **Race conditions** occur when threads access shared data simultaneously
2. **Mutexes** provide mutual exclusion but require careful management
3. **Lock guards** make mutex usage safe and exception-proof
4. **Unique locks** provide flexibility for complex scenarios
5. **Condition variables** coordinate thread execution based on conditions
6. **Semaphores** signal between threads and control resource access
7. **Monitors** encapsulate synchronization into clean, reusable classes
8. **Atomics** provide lock-free thread-safe operations
9. **Deadlock** requires all four conditions - break one to prevent it
10. **Always prefer higher-level abstractions** (monitors, thread-safe classes)

### Decision Tree:

```
Need thread synchronization?
│
├─ Simple critical section?
│  └─ Use std::lock_guard with std::mutex
│
├─ Need flexibility (unlock early, move, etc.)?
│  └─ Use std::unique_lock with std::mutex
│
├─ Wait for condition?
│  └─ Use std::condition_variable with std::unique_lock
│
├─ Signal between threads?
│  └─ Use std::binary_semaphore
│
├─ Limited resource pool?
│  └─ Use std::counting_semaphore
│
├─ Complex shared object?
│  └─ Create monitor class (mutex + CV + data)
│
└─ Simple counter/flag?
   └─ Use std::atomic
```

---

## Additional Resources

### C++20 Threading Headers:
```cpp
#include <thread>        // std::thread
#include <mutex>         // std::mutex, std::lock_guard, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <semaphore>     // std::binary_semaphore, std::counting_semaphore
#include <atomic>        // std::atomic
```

### Remember:
- 🔒 **Correctness first, performance second**
- 🧪 **Test thoroughly** with thread sanitizers
- 📚 **Use established patterns** (monitors, producer-consumer, etc.)
- 🎯 **Keep it simple** - use the right tool for the job
- 🛡️ **Prefer RAII** - let destructors handle cleanup

---

**Good luck with your multithreading journey! 🚀**

*Remember: The best code is code that's easy to understand and hard to get wrong.*
