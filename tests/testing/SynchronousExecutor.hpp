#pragma once
#include <functional>

#include "IExecutor.hpp"

/// SynchronousExecutor: runs enqueued tasks inline on the calling thread.
/// Use in tests to avoid real thread scheduling and timing dependencies.
class SynchronousExecutor : public IExecutor
{
  public:
    void enqueue(std::function<void()> task) override
    {
        task();
    }
};
