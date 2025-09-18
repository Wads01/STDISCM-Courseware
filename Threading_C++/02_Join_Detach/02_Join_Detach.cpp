#include "main.hpp"

// JOIN
// 1. A thread can be joined using the join() method.
// 2. Joining a thread means that the calling thread will wait for the joined thread to finish its execution.
// 3. Once a thread is joined, it cannot be joined again or detached.
// 4. A joined thread will be cleaned up automatically once it finishes its execution.

// DETACH
// 1. A thread can be detached using the detach() method.
// 2. Detaching a thread means that the thread will run independently from the calling thread.
// 3. Once a thread is detached, it cannot be joined.
// 4. A detached thread will continue to run even if the main thread finishes its execution.
// 5. Detached threads are not automatically cleaned up, so it's important to ensure they complete before the program exits.

// JOINABLE
// 1. A thread is joinable if it represents a thread of execution.
// 2. A thread is joinable if it has been started and has not been joined or detached.
// 3. The joinable() method can be used to check if a thread is joinable.

void run(int x) {
    while (x-- > 0) {
		std::cout << "Thread ID: " << std::this_thread::get_id() << " - Value: " << x << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void check_joinable(std::thread& t, const std::string& name) {
    if (t.joinable()) {
        std::cout << "Thread " << name << " (ID: " << t.get_id() << ") is joinable." << std::endl;
        t.join();

		return;
    }

    std::cout << "Thread " << name << " is not joinable." << std::endl;
}

int main()
{
    std::cout << " ========================= PROGRAM START ========================= " << std::endl;

    std::thread t1(run, 5);
    std::thread t2(run, 10);

	t1.join();

    check_joinable(t1, "t1");
    check_joinable(t2, "t2");

	std::cout << "Threads t1 and t2 has been joined." << std::endl; 

    std::cout << "\n=====================================\n";

	std::cout << "Thread t3 will be detached and run independently." << std::endl;
    std::thread t3(run, 20);
    
	if (t3.joinable()) {
        t3.detach();
	}
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << " ========================== PROGRAM END ========================== " << std::endl;

    return 0;
}