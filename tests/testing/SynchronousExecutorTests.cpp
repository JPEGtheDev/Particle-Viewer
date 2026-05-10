#include <vector>

#include <gtest/gtest.h>

#include "IExecutor.hpp"
#include "testing/SynchronousExecutor.hpp"

TEST(SynchronousExecutorTests, SynchronousExecutor_Enqueue_RunsTaskImmediately)
{
    SynchronousExecutor executor;
    bool flag = false;
    executor.enqueue([&flag]() { flag = true; });
    ASSERT_TRUE(flag);
}

TEST(SynchronousExecutorTests, SynchronousExecutor_EnqueueMultiple_RunsAllInOrder)
{
    SynchronousExecutor executor;
    std::vector<int> order;
    executor.enqueue([&order]() { order.push_back(1); });
    executor.enqueue([&order]() { order.push_back(2); });
    executor.enqueue([&order]() { order.push_back(3); });
    ASSERT_EQ(order.size(), 3u);
    ASSERT_EQ(order[0], 1);
    ASSERT_EQ(order[1], 2);
    ASSERT_EQ(order[2], 3);
}

TEST(SynchronousExecutorTests, SynchronousExecutor_DispatchesThroughBasePointer)
{
    SynchronousExecutor sync;
    bool called = false;
    IExecutor* base = &sync;
    base->enqueue([&called]() { called = true; });
    ASSERT_TRUE(called);
}
