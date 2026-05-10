#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

#include "IExecutor.hpp"

/// ThreadedExecutor: runs enqueued tasks on a single background worker thread.
///
/// Ownership model:
///   - The background thread is started in the constructor and joined in the
///     destructor (RAII). Destruction is safe even if the queue is non-empty:
///     all queued tasks complete before the join returns.
///   - Non-copyable, non-movable.
class ThreadedExecutor : public IExecutor
{
  public:
    ThreadedExecutor() : stop_(false), worker_([this]() { run(); })
    {
    }

    ~ThreadedExecutor() override
    {
        {
            std::lock_guard<std::mutex> lock{mutex_};
            stop_ = true;
        }
        cv_.notify_all();
        worker_.join();
    }

    ThreadedExecutor(const ThreadedExecutor&) = delete;
    ThreadedExecutor& operator=(const ThreadedExecutor&) = delete;
    ThreadedExecutor(ThreadedExecutor&&) = delete;
    ThreadedExecutor& operator=(ThreadedExecutor&&) = delete;

    /// Enqueues @p task for execution on the background worker thread.
    void enqueue(std::function<void()> task) override
    {
        {
            std::lock_guard<std::mutex> lock{mutex_};
            queue_.push(std::move(task));
        }
        cv_.notify_one();
    }

  private:
    void run()
    {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock{mutex_};
                cv_.wait(lock, [this] { return stop_ || !queue_.empty(); });
                if (stop_ && queue_.empty()) {
                    break;
                }
                task = std::move(queue_.front());
                queue_.pop();
            }
            task();
        }
    }

    bool stop_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::function<void()>> queue_;
    std::thread worker_;
};
