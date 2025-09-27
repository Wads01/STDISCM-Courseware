#pragma once

#include <vector>
#include <cstddef>

class IWorker {
public:
    virtual void operator()() = 0;

    virtual ~IWorker() = default;
};