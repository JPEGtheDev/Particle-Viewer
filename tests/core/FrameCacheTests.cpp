/*
 * FrameCacheTests.cpp
 *
 * Unit tests for FrameCache — LRU sliding-window prefetch cache.
 * Uses SynchronousExecutor so all enqueued tasks run inline.
 * No GL calls are made by FrameCache; MockOpenGL is reset for isolation only.
 */

#include <cstdio>
#include <functional>
#include <memory>
#include <vector>

// Include glad first to avoid OpenGL header conflicts
#include <glad/glad.h>
#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "FrameCache.hpp"
#include "IFrameCache.hpp"
#include "MockOpenGL.hpp"
#include "settingsIO.hpp"
#include "testing/SynchronousExecutor.hpp"
#include "testing/TestFrameBuilder.hpp"

// ──────────────────────────────────────────────────────────────────────────────
// CountingExecutor: records enqueue calls without running tasks.
// Used to verify that duplicate enqueues are suppressed.
// ──────────────────────────────────────────────────────────────────────────────

class CountingExecutor : public IExecutor
{
  public:
    void enqueue(std::function<void()> /*task*/) override
    {
        ++callCount;
    }

    int callCount = 0;
};

// ──────────────────────────────────────────────────────────────────────────────
// Test fixture
// ──────────────────────────────────────────────────────────────────────────────

class FrameCacheTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
        MockOpenGL::initGLAD();
    }

    SynchronousExecutor executor_;
};

// ──────────────────────────────────────────────────────────────────────────────
// 1. getFrame on a fresh cache returns nullptr (cache miss)
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(FrameCacheTest, FrameCache_GetFrame_MissReturnNullptr)
{
    // Arrange
    SettingsIO sio;
    sio.N = 2;
    sio.frames = 3;
    constexpr std::size_t maxBytes = 10 * sizeof(glm::vec4);
    FrameCache cache(sio, maxBytes, executor_);

    // Act
    auto result = cache.getFrame(0);

    // Assert
    EXPECT_EQ(result, nullptr);
}

// ──────────────────────────────────────────────────────────────────────────────
// 2. prefetch populates cache; getFrame returns correct data
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(FrameCacheTest, FrameCache_Prefetch_PopulatesCache)
{
    // Arrange
    constexpr long N = 2;
    TestFrameBuilder builder(N);
    builder.addFrame({{1.f, 2.f, 3.f, 0.f}, {4.f, 5.f, 6.f, 1.f}});
    builder.addFrame({{7.f, 8.f, 9.f, 0.f}, {10.f, 11.f, 12.f, 1.f}});
    builder.addFrame({{13.f, 14.f, 15.f, 0.f}, {16.f, 17.f, 18.f, 1.f}});
    std::string path = builder.writeToTempFile("_fc_prefetch.bin");

    SettingsIO sio;
    sio.posName = path;
    sio.N = N;
    sio.frames = 3;

    constexpr std::size_t maxBytes = 10 * sizeof(glm::vec4);
    FrameCache cache(sio, maxBytes, executor_);

    // Act — SynchronousExecutor runs tasks inline, so cache is populated immediately
    cache.prefetch(0, 2, 3);

    // Assert
    auto frame1 = cache.getFrame(1);
    auto frame2 = cache.getFrame(2);

    ASSERT_NE(frame1, nullptr);
    ASSERT_NE(frame2, nullptr);
    EXPECT_EQ(static_cast<long>(frame1->size()), N);
    EXPECT_EQ(static_cast<long>(frame2->size()), N);

    std::remove(path.c_str());
}

// ──────────────────────────────────────────────────────────────────────────────
// 3. getFrame hit returns the same shared_ptr (pointer identity)
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(FrameCacheTest, FrameCache_GetFrame_HitReturnsSamePointer)
{
    // Arrange
    constexpr long N = 2;
    TestFrameBuilder builder(N);
    builder.addFrame({{1.f, 2.f, 3.f, 0.f}, {4.f, 5.f, 6.f, 1.f}});
    builder.addFrame({{7.f, 8.f, 9.f, 0.f}, {10.f, 11.f, 12.f, 1.f}});
    std::string path = builder.writeToTempFile("_fc_hit.bin");

    SettingsIO sio;
    sio.posName = path;
    sio.N = N;
    sio.frames = 2;

    constexpr std::size_t maxBytes = 10 * sizeof(glm::vec4);
    FrameCache cache(sio, maxBytes, executor_);
    cache.prefetch(0, 1, 2);

    // Act
    auto first = cache.getFrame(1);
    auto second = cache.getFrame(1);

    // Assert — same object in memory
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first.get(), second.get());

    std::remove(path.c_str());
}

// ──────────────────────────────────────────────────────────────────────────────
// 4. LRU eviction evicts the least-recently-used entry
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(FrameCacheTest, FrameCache_LRUEviction_EvictsLeastRecentlyUsed)
{
    // Arrange: 4-frame file, N=1, cap = 2 frames
    constexpr long N = 1;
    TestFrameBuilder builder(N);
    builder.addFrame({{0.f, 0.f, 0.f, 0.f}});
    builder.addFrame({{1.f, 1.f, 1.f, 0.f}});
    builder.addFrame({{2.f, 2.f, 2.f, 0.f}});
    builder.addFrame({{3.f, 3.f, 3.f, 0.f}});
    std::string path = builder.writeToTempFile("_fc_lru.bin");

    SettingsIO sio;
    sio.posName = path;
    sio.N = N;
    sio.frames = 4;

    // max = 2 * sizeof(glm::vec4) / sizeof(glm::vec4) = 2 frames
    const std::size_t maxBytes = 2 * sizeof(glm::vec4);
    FrameCache cache(sio, maxBytes, executor_);

    // Act: prefetch frames 1 and 2 (fills cap)
    cache.prefetch(0, 2, 4);
    ASSERT_NE(cache.getFrame(1), nullptr);
    ASSERT_NE(cache.getFrame(2), nullptr);

    // Access frame 1 to make it MRU (frame 2 becomes LRU)
    cache.getFrame(1);

    // Prefetch frame 3 — should evict frame 2 (LRU)
    cache.prefetch(2, 1, 4);

    // Assert
    EXPECT_NE(cache.getFrame(1), nullptr) << "frame 1 (MRU) should still be cached";
    EXPECT_EQ(cache.getFrame(2), nullptr) << "frame 2 (LRU) should have been evicted";
    EXPECT_NE(cache.getFrame(3), nullptr) << "frame 3 (just loaded) should be cached";

    std::remove(path.c_str());
}

// ──────────────────────────────────────────────────────────────────────────────
// 5. Cache never exceeds max_frames count
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(FrameCacheTest, FrameCache_LRUEviction_MaxFramesEnforced)
{
    // Arrange: 6-frame file, N=1, cap = 3 frames
    constexpr long N = 1;
    constexpr long totalFrames = 6;
    TestFrameBuilder builder(N);
    for (long i = 0; i < totalFrames; ++i) {
        builder.addFrame({{static_cast<float>(i), 0.f, 0.f, 0.f}});
    }
    std::string path = builder.writeToTempFile("_fc_maxframes.bin");

    SettingsIO sio;
    sio.posName = path;
    sio.N = N;
    sio.frames = totalFrames;

    const std::size_t maxBytes = 3 * sizeof(glm::vec4);
    FrameCache cache(sio, maxBytes, executor_);

    // Act: prefetch with window larger than cap
    cache.prefetch(0, 5, totalFrames);

    // Assert: count how many frames are cached (at most 3)
    int cachedCount = 0;
    for (long f = 1; f < totalFrames; ++f) {
        if (cache.getFrame(f) != nullptr) {
            ++cachedCount;
        }
    }
    EXPECT_EQ(cachedCount, 3) << "cache must hold exactly max_frames entries";

    std::remove(path.c_str());
}

// ──────────────────────────────────────────────────────────────────────────────
// 6. clear() erases all cached entries
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(FrameCacheTest, FrameCache_Clear_ErasesCache)
{
    // Arrange
    constexpr long N = 2;
    TestFrameBuilder builder(N);
    builder.addFrame({{1.f, 2.f, 3.f, 0.f}, {4.f, 5.f, 6.f, 1.f}});
    builder.addFrame({{7.f, 8.f, 9.f, 0.f}, {10.f, 11.f, 12.f, 1.f}});
    std::string path = builder.writeToTempFile("_fc_clear.bin");

    SettingsIO sio;
    sio.posName = path;
    sio.N = N;
    sio.frames = 2;

    constexpr std::size_t maxBytes = 10 * sizeof(glm::vec4);
    FrameCache cache(sio, maxBytes, executor_);
    cache.prefetch(0, 1, 2);
    ASSERT_NE(cache.getFrame(1), nullptr) << "precondition: frame 1 should be cached";

    // Act
    cache.clear();

    // Assert
    EXPECT_EQ(cache.getFrame(1), nullptr) << "after clear(), frame 1 must not be in cache";

    std::remove(path.c_str());
}

// ──────────────────────────────────────────────────────────────────────────────
// 7. prefetch with zero total_frames must not crash
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(FrameCacheTest, FrameCache_Prefetch_ZeroTotalFrames_DoesNothing)
{
    // Arrange
    SettingsIO sio;
    sio.N = 2;
    sio.frames = 0;

    constexpr std::size_t maxBytes = 10 * sizeof(glm::vec4);
    FrameCache cache(sio, maxBytes, executor_);

    // Act + Assert: must not crash
    EXPECT_NO_FATAL_FAILURE(cache.prefetch(0, 64, 0));
}

// ──────────────────────────────────────────────────────────────────────────────
// 8. pending set prevents duplicate enqueue
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(FrameCacheTest, FrameCache_Prefetch_NoDuplicateEnqueue)
{
    // Arrange: CountingExecutor does NOT run tasks, so pending set stays populated
    constexpr long N = 2;
    TestFrameBuilder builder(N);
    builder.addFrame({{1.f, 2.f, 3.f, 0.f}, {4.f, 5.f, 6.f, 1.f}});
    builder.addFrame({{7.f, 8.f, 9.f, 0.f}, {10.f, 11.f, 12.f, 1.f}});
    builder.addFrame({{13.f, 14.f, 15.f, 0.f}, {16.f, 17.f, 18.f, 1.f}});
    std::string path = builder.writeToTempFile("_fc_nodup.bin");

    SettingsIO sio;
    sio.posName = path;
    sio.N = N;
    sio.frames = 3;

    constexpr std::size_t maxBytes = 10 * sizeof(glm::vec4);
    CountingExecutor counter;
    FrameCache cache(sio, maxBytes, counter);

    // Act: prefetch twice — second call should not re-enqueue already-pending frames
    cache.prefetch(0, 2, 3); // enqueues frames 1 and 2 → count = 2
    cache.prefetch(0, 2, 3); // frames 1 and 2 still pending → count stays 2

    // Assert
    EXPECT_EQ(counter.callCount, 2) << "pending set must prevent duplicate enqueue";

    std::remove(path.c_str());
}

// ──────────────────────────────────────────────────────────────────────────────
// 9. zero-capacity cache never stores any frame
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(FrameCacheTest, FrameCache_ZeroCapacity_NeverCaches)
{
    // Arrange: max_frame_bytes < one frame → max_frames_ rounds to 0
    constexpr long N = 2;
    TestFrameBuilder builder(N);
    builder.addFrame({{1.f, 2.f, 3.f, 0.f}, {4.f, 5.f, 6.f, 1.f}});
    builder.addFrame({{7.f, 8.f, 9.f, 0.f}, {10.f, 11.f, 12.f, 1.f}});
    std::string path = builder.writeToTempFile("_fc_zerocap.bin");

    SettingsIO sio;
    sio.posName = path;
    sio.N = N;
    sio.frames = 2;

    // Budget: 1 byte — far smaller than one frame (2 × 16 = 32 bytes)
    constexpr std::size_t maxBytes = 1;
    FrameCache cache(sio, maxBytes, executor_);

    // Act: attempt to prefetch
    cache.prefetch(0, 1, 2);

    // Assert: nothing should be cached
    EXPECT_EQ(cache.getFrame(1), nullptr) << "zero-capacity cache must never store frames";

    std::remove(path.c_str());
}

// ──────────────────────────────────────────────────────────────────────────────
// 10. cachedCount() reflects the number of frames currently in the cache
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(FrameCacheTest, FrameCache_CachedCount_ReflectsInsertedFrames)
{
    // Arrange
    constexpr long N = 2;
    TestFrameBuilder builder(N);
    builder.addFrame({{1.f, 2.f, 3.f, 0.f}, {4.f, 5.f, 6.f, 1.f}});
    builder.addFrame({{7.f, 8.f, 9.f, 0.f}, {10.f, 11.f, 12.f, 1.f}});
    builder.addFrame({{13.f, 14.f, 15.f, 0.f}, {16.f, 17.f, 18.f, 1.f}});
    std::string path = builder.writeToTempFile("_fc_count.bin");

    SettingsIO sio;
    sio.posName = path;
    sio.N = N;
    sio.frames = 3;
    constexpr std::size_t maxBytes = 10 * sizeof(glm::vec4);
    FrameCache cache(sio, maxBytes, executor_);

    // Assert: starts empty
    EXPECT_EQ(cache.cachedCount(), std::size_t{0});

    // Act — SynchronousExecutor runs tasks inline; frames 1 and 2 are inserted
    cache.prefetch(0, 2, 3);

    // Assert: two frames now in cache
    EXPECT_EQ(cache.cachedCount(), std::size_t{2});

    std::remove(path.c_str());
}
