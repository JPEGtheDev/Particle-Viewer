#pragma once
#include <functional>

/// IExecutor: abstract task runner interface.
/// Production code uses ThreadedExecutor; tests use SynchronousExecutor.
class IExecutor
{
  public:
    IExecutor() = default;
    virtual ~IExecutor() = default;
    IExecutor(const IExecutor&) = delete;
    IExecutor& operator=(const IExecutor&) = delete;

    virtual void enqueue(std::function<void()> task) = 0;
};
