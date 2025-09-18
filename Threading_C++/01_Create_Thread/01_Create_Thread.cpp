#include "main.hpp"

// 1. Function Pointer
void threadFunction(int x) {
    while (x-- > 0) {
        std::cout << x << std::endl;
	}
}

int main()
{
    std::cout << " ========================= PROGRAM START ========================= " << std::endl;

	std::thread t1(threadFunction, 10); // Create a thread that runs threadFunction with argument 10
	std::thread t2(threadFunction, 20); // Create a thread that runs threadFunction with argument 20

	t1.join(); // Wait for t1 to finish
	t2.join(); // Wait for t2 to finish

    std::cout << " ========================== PROGRAM END ========================== " << std::endl;

    return 0;
}

//// 2. Lambda Function
//int main()
//{
//	std::cout << " ========================= PROGRAM START ========================= " << std::endl;
//
//	std::thread t1([](int x) {
//		while (x-- > 0) {
//			std::cout << x << std::endl;
//		}
//	}, 10);
//
//	t1.join();
//
//	std::cout << " ========================== PROGRAM END ========================== " << std::endl;
//
//	return 0;
//}

//// 3. Function Pointer
//class ThreadClass {
//public:
//	void operator()(int x) {
//		while (x-- > 0) {
//			std::cout << x << std::endl;
//		}
//	}
//};
//
//int main()
//{
//	std::cout << " ========================= PROGRAM START ========================= " << std::endl;
//
//	std::thread t1(ThreadClass(), 15);
//
//	t1.join();
//
//	std::cout << " ========================== PROGRAM END ========================== " << std::endl;
//
//	return 0;
//}

//// 4. Non-static Member Function
//class ThreadClass {
//public:
//	void run(int x) {
//		while (x-- > 0) {
//			std::cout << x << std::endl;
//		}
//	}
//};
//
//int main()
//{
//	std::cout << " ========================= PROGRAM START ========================= " << std::endl;
//
//	ThreadClass obj;
//
//	std::thread t1(&ThreadClass::run, &obj, 15);
//
//	t1.join();
//
//	std::cout << " ========================== PROGRAM END ========================== " << std::endl;
//
//	return 0;
//}

//// 5. Static Member Function
//class ThreadClass {
//public:
//	static void run(int x) {
//		while (x-- > 0) {
//			std::cout << x << std::endl;
//		}
//	}
//};
//
//int main()
//{
//	std::cout << " ========================= PROGRAM START ========================= " << std::endl;
//
//	std::thread t1(&ThreadClass::run, 15);
//
//	t1.join();
//
//	std::cout << " ========================== PROGRAM END ========================== " << std::endl;
//
//	return 0;
//}