# pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

class Algorithm {
public:
	Algorithm() = default;
	~Algorithm() = default;
	void execute();
private:
	void createWorker();
};