/*
 * COMCacheTests.cpp
 *
 * Unit tests for COMCache — async frame-COM memoization.
 * Uses SynchronousExecutor so tasks run inline (no threading uncertainty).
 * Uses TestFrameBuilder to write synthetic binary frame files for SettingsIO.
 */

#include <cstdio>
#include <functional>
#include <optional>
#include <vector>

// Include glad before other OpenGL-related headers to avoid conflicts
#include <glad/glad.h>
#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "COMCache.hpp"
#include "COMCalculator.hpp"
#include "IExecutor.hpp"
#include "MassParams.hpp"
#include "MockOpenGL.hpp"
#include "settingsIO.hpp"
#include "testing/SynchronousExecutor.hpp"
#include "testing/TestFrameBuilder.hpp"

// ─── Sentinel MassParams ─────────────────────────────────────────────────────

/// All fractions = 1.0 so that a COM is always computable for any nonzero span.
constexpr MassParams kSimpleMassParams{
    1.0, // fraction_earth_mass_of_body1
    1.0, // fraction_earth_mass_of_body2
    1.0, // fraction_fe_body1
    1.0, // fraction_si_body1
    1.0, // fraction_fe_body2
    1.0, // fraction_si_body2
    1.0  // mass_of_earth
};

// ─── DeferredExecutor ────────────────────────────────────────────────────────

/// Stores enqueued tasks without running them. Lets tests verify that the
/// pending-set guard prevents double-enqueue before any task executes.
class DeferredExecutor : public IExecutor
{
  public:
    void enqueue(std::function<void()> task) override
    {
        ++enqueue_count;
        tasks_.push_back(std::move(task));
    }

    void runAll()
    {
        for (auto& t : tasks_) {
            t();
        }
        tasks_.clear();
    }

    int enqueue_count{0};

  private:
    std::vector<std::function<void()>> tasks_;
};

// ─── Helpers ─────────────────────────────────────────────────────────────────

/// Checks that two vec3 values are component-wise within tolerance.
static void expectNearVec3(const glm::vec3& actual, const glm::vec3& expected, double tol = 1e-5)
{
    EXPECT_NEAR(static_cast<double>(actual.x), static_cast<double>(expected.x), tol);
    EXPECT_NEAR(static_cast<double>(actual.y), static_cast<double>(expected.y), tol);
    EXPECT_NEAR(static_cast<double>(actual.z), static_cast<double>(expected.z), tol);
}

/// Builds a SettingsIO pointing at a 1-frame binary file containing @p positions.
/// The caller is responsible for removing the returned path after the test.
static std::pair<SettingsIO, std::string> makeOneFSio(const std::vector<glm::vec4>& positions)
{
    const long n = static_cast<long>(positions.size());
    TestFrameBuilder builder(n);
    builder.addFrame(positions);
    std::string path = builder.writeToTempFile("_comcache.bin");

    SettingsIO sio;
    sio.posName = path;
    sio.N = n;
    sio.frames = 1;
    return {sio, path};
}

// ─── Fixture ─────────────────────────────────────────────────────────────────

class COMCacheTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
        MockOpenGL::initGLAD();
    }

    SynchronousExecutor executor_;
};

// ─── Test 1: CacheMiss ───────────────────────────────────────────────────────

TEST_F(COMCacheTest, COMCache_CacheMiss_ReturnsNullopt)
{
    // Arrange — 1 Fe1 particle at (4, 0, 0)
    auto [sio, path] = makeOneFSio({{4.f, 0.f, 0.f, 0.f}});
    COMCache cache(sio, kSimpleMassParams, executor_);

    // Act — first call: cache is empty; task enqueued (runs inline but returns
    // nullopt before the map is re-checked)
    std::optional<glm::vec3> result = cache.getCOM(0);

    // Assert
    EXPECT_FALSE(result.has_value());

    std::remove(path.c_str());
}

// ─── Test 2: CacheHit ────────────────────────────────────────────────────────

TEST_F(COMCacheTest, COMCache_CacheHit_ReturnsValue)
{
    // Arrange — 1 Fe1 particle; SynchronousExecutor runs the compute task inline
    // so after the first getCOM the cache is already populated.
    auto [sio, path] = makeOneFSio({{4.f, 0.f, 0.f, 0.f}});
    COMCache cache(sio, kSimpleMassParams, executor_);
    cache.getCOM(0); // first call — miss, but task runs inline → cache filled

    // Act — second call: cache hit
    std::optional<glm::vec3> result = cache.getCOM(0);

    // Assert
    EXPECT_TRUE(result.has_value());

    std::remove(path.c_str());
}

// ─── Test 3: ComputedValue ───────────────────────────────────────────────────

TEST_F(COMCacheTest, COMCache_ComputedValue_MatchesCOMCalculator)
{
    // Arrange — 1 Fe1 particle at (4, 0, 0).
    // Expected COM = (4,0,0) * kSimToDisplayScale = (1,0,0).
    const glm::vec4 particle{4.f, 0.f, 0.f, 0.f};
    auto [sio, path] = makeOneFSio({particle});
    COMCache cache(sio, kSimpleMassParams, executor_);

    // Compute reference value directly via COMCalculator.
    std::span<const glm::vec4> span{&particle, 1};
    const glm::vec3 expected = COMCalculator::computeMassWeightedCOM(span, kSimpleMassParams);

    cache.getCOM(0);                                   // populate cache
    std::optional<glm::vec3> result = cache.getCOM(0); // hit

    // Assert
    ASSERT_TRUE(result.has_value());
    expectNearVec3(*result, expected);

    std::remove(path.c_str());
}

// ─── Test 4: NoPendingDuplicate ──────────────────────────────────────────────

TEST_F(COMCacheTest, COMCache_NoPendingDuplicate_DoesNotDoubleEnqueue)
{
    // Arrange — DeferredExecutor: tasks are stored but NOT executed, so the
    // pending set still contains frame 0 during the second getCOM call.
    auto [sio, path] = makeOneFSio({{1.f, 0.f, 0.f, 0.f}});
    DeferredExecutor deferred;
    COMCache cache(sio, kSimpleMassParams, deferred);

    // Act — first call enqueues; second call finds frame in pending and skips.
    cache.getCOM(0);
    cache.getCOM(0);

    // Assert — executor was called exactly once
    EXPECT_EQ(deferred.enqueue_count, 1);

    std::remove(path.c_str());
}

// ─── Test 5: Clear ───────────────────────────────────────────────────────────

TEST_F(COMCacheTest, COMCache_Clear_ErasesCache)
{
    // Arrange — populate cache via SynchronousExecutor
    auto [sio, path] = makeOneFSio({{2.f, 0.f, 0.f, 0.f}});
    COMCache cache(sio, kSimpleMassParams, executor_);
    cache.getCOM(0);                          // miss → task runs inline → cache filled
    ASSERT_TRUE(cache.getCOM(0).has_value()); // sanity: hit before clear

    // Act
    cache.clear();

    // Assert — after clear, getCOM returns nullopt (cache miss)
    std::optional<glm::vec3> result = cache.getCOM(0);
    EXPECT_FALSE(result.has_value());

    std::remove(path.c_str());
}

// ─── Helpers (multi-frame) ────────────────────────────────────────────────────

/// Builds a SettingsIO pointing at a file with @p frame_count identical frames,
/// each containing one particle at (1, 0, 0, 0).
/// The caller is responsible for removing the returned path after the test.
static std::pair<SettingsIO, std::string> makeMultiFrameFSio(int frame_count = 5)
{
    const long n = 1;
    TestFrameBuilder builder(n);
    for (int i = 0; i < frame_count; ++i) {
        builder.addFrame({{1.f, 0.f, 0.f, 0.f}});
    }
    std::string path = builder.writeToTempFile("_comcache_multi.bin");

    SettingsIO sio;
    sio.posName = path;
    sio.N = n;
    sio.frames = static_cast<long>(frame_count);
    return {sio, path};
}

// ─── Test 6: PrefetchAsync_PopulatesCache ─────────────────────────────────────

TEST_F(COMCacheTest, COMCache_PrefetchAsync_PopulatesCache)
{
    // Arrange — 5 frames (indices 0-4), SynchronousExecutor runs tasks inline.
    auto [sio, path] = makeMultiFrameFSio(5);
    COMCache cache(sio, kSimpleMassParams, executor_);

    // Act — prefetch window: cur=0, ahead=3, total=5 → enqueues frames 1, 2, 3
    cache.prefetchAsync(0, 3, 5);

    // Assert — frames 1, 2, 3 are populated (cache hit)
    EXPECT_TRUE(cache.getCOM(1).has_value());
    EXPECT_TRUE(cache.getCOM(2).has_value());
    EXPECT_TRUE(cache.getCOM(3).has_value());
    // Frame 4 was not prefetched (outside window) — should still miss
    EXPECT_FALSE(cache.getCOM(4).has_value());

    std::remove(path.c_str());
}

// ─── Test 7: PrefetchAsync_ZeroTotalFrames_DoesNothing ───────────────────────

TEST_F(COMCacheTest, COMCache_PrefetchAsync_ZeroTotalFrames_DoesNothing)
{
    // Arrange — SettingsIO with no frames; DeferredExecutor to count enqueues.
    SettingsIO sio;
    sio.N = 0;
    sio.frames = 0;
    DeferredExecutor deferred;
    COMCache cache(sio, kSimpleMassParams, deferred);

    // Act — must not crash; total_frames=0 is a guard condition
    cache.prefetchAsync(0, 64, 0);

    // Assert — nothing was enqueued
    EXPECT_EQ(deferred.enqueue_count, 0);
}

// ─── Test 8: PrefetchAsync_NoDuplicateEnqueue ────────────────────────────────

TEST_F(COMCacheTest, COMCache_PrefetchAsync_NoDuplicateEnqueue)
{
    // Arrange — DeferredExecutor: tasks are stored but NOT executed, so the
    // pending set still contains frames 1, 2, 3 during the second prefetch call.
    auto [sio, path] = makeMultiFrameFSio(5);
    DeferredExecutor deferred;
    COMCache cache(sio, kSimpleMassParams, deferred);

    // Act — two identical prefetch calls; second must not re-enqueue pending frames
    cache.prefetchAsync(0, 3, 5); // enqueues frames 1, 2, 3 → count = 3
    cache.prefetchAsync(0, 3, 5); // all frames already pending → count stays at 3

    // Assert
    EXPECT_EQ(deferred.enqueue_count, 3);

    std::remove(path.c_str());
}

// ─── Test 9: IOFailure ───────────────────────────────────────────────────────
// When readPosBuffer() returns empty (e.g. N=0 or disk error), getCOM() must:
//   - Return nullopt on every call (never return a cached (0,0,0))
//   - Not re-enqueue the compute task after the first failure (failed_ set)

TEST_F(COMCacheTest, COMCache_IOFailure_ReturnsNulloptOnFirstCall)
{
    // Arrange — N=0 so readPosBuffer always returns {}
    SettingsIO sio;
    sio.N = 0;
    sio.frames = 0;
    COMCache cache(sio, kSimpleMassParams, executor_);

    // Act — first call: miss, task runs inline, positions empty → NOT cached
    std::optional<glm::vec3> result = cache.getCOM(0);

    // Assert — nullopt: I/O failure must not be memoised as (0,0,0)
    EXPECT_FALSE(result.has_value());
}

TEST_F(COMCacheTest, COMCache_IOFailure_ReturnsNulloptPersistently)
{
    // Arrange — N=0 so readPosBuffer always returns {}
    SettingsIO sio;
    sio.N = 0;
    sio.frames = 0;
    COMCache cache(sio, kSimpleMassParams, executor_);

    // Act — first call runs inline task (empty → goes to failed_ set)
    cache.getCOM(0);
    // second call — frame is in failed_; must return nullopt without re-enqueuing
    std::optional<glm::vec3> second = cache.getCOM(0);

    // Assert — still nullopt; no COM=(0,0,0) stored
    EXPECT_FALSE(second.has_value());
}

TEST_F(COMCacheTest, COMCache_IOFailure_DoesNotReenqueue)
{
    // Arrange — DeferredExecutor so we can count enqueue calls
    SettingsIO sio;
    sio.N = 0;
    sio.frames = 0;
    DeferredExecutor deferred;
    COMCache cache(sio, kSimpleMassParams, deferred);

    // Act — first call enqueues; run tasks manually so failed_ is populated
    cache.getCOM(0);
    deferred.runAll(); // executes the lambda → positions empty → inserts to failed_
    // second call — frame already failed; must NOT enqueue again
    cache.getCOM(0);

    // Assert — only 1 enqueue total
    EXPECT_EQ(deferred.enqueue_count, 1);
}

// ─── Test 11: ClearResetsFailedSet ──────────────────────────────────────────
// clear() must clear failed_ so that failed frames can be retried after a
// folder switch (i.e., after new cache infrastructure is built via clear()).

TEST_F(COMCacheTest, COMCache_Clear_AllowsFailedFrameToBeReenqueued)
{
    // Arrange — N=0 so positions are empty; DeferredExecutor to control timing
    SettingsIO sio;
    sio.N = 0;
    sio.frames = 0;
    DeferredExecutor deferred;
    COMCache cache(sio, kSimpleMassParams, deferred);

    // Act — first call enqueues; run task so frame enters failed_
    cache.getCOM(0);
    deferred.runAll();
    // Confirm: frame is now in failed_; second call should NOT re-enqueue
    cache.getCOM(0);
    const int enqueue_before_clear = deferred.enqueue_count;

    // Act — clear() must also clear failed_
    cache.clear();
    cache.getCOM(0);
    const int enqueue_after_clear = deferred.enqueue_count;

    // Assert
    EXPECT_EQ(enqueue_before_clear, 1); // failed_ blocked re-enqueue
    EXPECT_EQ(enqueue_after_clear, 2);  // clear() allowed re-enqueue
}

TEST_F(COMCacheTest, COMCache_CachedCount_AfterGetCOM_IsOne)
{
    // Arrange — 1 Fe1 particle; SynchronousExecutor runs compute inline
    auto [sio, path] = makeOneFSio({{4.f, 0.f, 0.f, 0.f}});
    COMCache cache(sio, kSimpleMassParams, executor_);

    // Assert: starts at 0
    EXPECT_EQ(cache.cachedCount(), std::size_t{0});

    // Act: first getCOM enqueues + runs inline; second call is a cache hit
    cache.getCOM(0);
    cache.getCOM(0);

    // Assert: exactly one frame in cache
    EXPECT_EQ(cache.cachedCount(), std::size_t{1});

    std::remove(path.c_str());
}
