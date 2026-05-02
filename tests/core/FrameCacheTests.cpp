/*
 * FrameCacheTests.cpp
 *
 * Unit tests for FrameCache following AAA pattern.
 *
 * Tests verify:
 *   - Cache miss on empty cache
 *   - Cache hit after put
 *   - LRU eviction when capacity is exceeded
 *   - LRU ordering is updated on access
 *   - Stats tracking (hits, misses, cached count)
 *   - clear() resets state
 */

#include <vector>

#include <glad/glad.h>
#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "FrameCache.hpp"
#include "MockOpenGL.hpp"

class FrameCacheTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
        MockOpenGL::initGLAD();
    }

    // Helper: create a simple position vector for testing
    static std::vector<glm::vec4> makePositions(int particle_count, float x_start = 0.0f)
    {
        std::vector<glm::vec4> positions(static_cast<size_t>(particle_count));
        for (int i = 0; i < particle_count; ++i) {
            positions[static_cast<size_t>(i)] = glm::vec4(x_start + static_cast<float>(i), 0.0f, 0.0f, 0.0f);
        }
        return positions;
    }
};

// ============================================
// Basic Cache Operations
// ============================================

TEST_F(FrameCacheTest, Get_EmptyCache_ReturnsMiss)
{
    // Arrange
    FrameCache cache(4);

    // Act
    auto result = cache.get(0);

    // Assert
    EXPECT_FALSE(result.has_value());
}

TEST_F(FrameCacheTest, Put_ThenGet_ReturnsPositions)
{
    // Arrange
    FrameCache cache(4);
    auto positions = makePositions(3, 10.0f);

    // Act
    cache.put(0, positions);
    auto result = cache.get(0);

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), static_cast<size_t>(3));
    EXPECT_FLOAT_EQ((*result)[0].x, 10.0f);
}

TEST_F(FrameCacheTest, Put_MultipleFrames_AllRetrievable)
{
    // Arrange
    FrameCache cache(4);

    // Act
    cache.put(0, makePositions(2, 0.0f));
    cache.put(1, makePositions(2, 10.0f));
    cache.put(2, makePositions(2, 20.0f));

    // Assert
    EXPECT_TRUE(cache.get(0).has_value());
    EXPECT_TRUE(cache.get(1).has_value());
    EXPECT_TRUE(cache.get(2).has_value());
    EXPECT_FLOAT_EQ(cache.get(2)->at(0).x, 20.0f);
}

TEST_F(FrameCacheTest, Get_AbsentFrame_ReturnsMiss)
{
    // Arrange
    FrameCache cache(4);
    cache.put(0, makePositions(2));

    // Act
    auto result = cache.get(99);

    // Assert
    EXPECT_FALSE(result.has_value());
}

// ============================================
// LRU Eviction
// ============================================

TEST_F(FrameCacheTest, LRU_WhenAtCapacity_EvictsLeastRecentlyUsed)
{
    // Arrange: cache with capacity 2
    FrameCache cache(2);
    cache.put(0, makePositions(2, 0.0f));  // inserted first = LRU
    cache.put(1, makePositions(2, 10.0f)); // inserted second

    // Act: add a third entry, which should evict frame 0 (LRU)
    cache.put(2, makePositions(2, 20.0f));

    // Assert: frame 0 evicted, frames 1 and 2 remain
    EXPECT_FALSE(cache.get(0).has_value());
    EXPECT_TRUE(cache.get(1).has_value());
    EXPECT_TRUE(cache.get(2).has_value());
}

TEST_F(FrameCacheTest, LRU_GetPromotesFrame_PreventsEviction)
{
    // Arrange: cache with capacity 2, frame 0 inserted first
    FrameCache cache(2);
    cache.put(0, makePositions(2, 0.0f));
    cache.put(1, makePositions(2, 10.0f));

    // Act: access frame 0 to promote it, then add frame 2
    cache.get(0);                          // promotes frame 0, frame 1 becomes LRU
    cache.put(2, makePositions(2, 20.0f)); // should evict frame 1

    // Assert: frame 1 evicted; frames 0 and 2 remain
    EXPECT_TRUE(cache.get(0).has_value());
    EXPECT_FALSE(cache.get(1).has_value());
    EXPECT_TRUE(cache.get(2).has_value());
}

TEST_F(FrameCacheTest, LRU_CapacityOne_SingleEntryOnly)
{
    // Arrange
    FrameCache cache(1);

    // Act
    cache.put(0, makePositions(2, 0.0f));
    cache.put(1, makePositions(2, 10.0f)); // evicts frame 0

    // Assert
    EXPECT_FALSE(cache.get(0).has_value());
    EXPECT_TRUE(cache.get(1).has_value());
}

TEST_F(FrameCacheTest, Put_SameFrameTwice_UpdatesPositionsAndPromotes)
{
    // Arrange
    FrameCache cache(3);
    cache.put(0, makePositions(2, 0.0f));
    cache.put(1, makePositions(2, 10.0f));
    cache.put(2, makePositions(2, 20.0f));
    // Frame 0 was LRU

    // Act: update frame 0 (should promote it, making frame 1 the new LRU)
    cache.put(0, makePositions(2, 99.0f));
    cache.put(3, makePositions(2, 30.0f)); // should evict frame 1

    // Assert: frame 0 updated and kept, frame 1 evicted
    ASSERT_TRUE(cache.get(0).has_value());
    EXPECT_FLOAT_EQ(cache.get(0)->at(0).x, 99.0f); // updated value
    EXPECT_FALSE(cache.get(1).has_value());
}

// ============================================
// Statistics
// ============================================

TEST_F(FrameCacheTest, Stats_InitialState_AllZero)
{
    // Arrange
    FrameCache cache(4);

    // Act
    auto stats = cache.getStats();

    // Assert
    EXPECT_EQ(stats.cached_frames, 0);
    EXPECT_EQ(stats.hits, 0);
    EXPECT_EQ(stats.misses, 0);
    EXPECT_EQ(stats.max_frames, 4);
}

TEST_F(FrameCacheTest, Stats_TracksMisses)
{
    // Arrange
    FrameCache cache(4);

    // Act
    cache.get(0); // miss
    cache.get(1); // miss

    // Assert
    EXPECT_EQ(cache.getStats().misses, 2);
    EXPECT_EQ(cache.getStats().hits, 0);
}

TEST_F(FrameCacheTest, Stats_TracksHits)
{
    // Arrange
    FrameCache cache(4);
    cache.put(0, makePositions(2));
    cache.put(1, makePositions(2));

    // Act
    cache.get(0); // hit
    cache.get(1); // hit
    cache.get(2); // miss

    // Assert
    EXPECT_EQ(cache.getStats().hits, 2);
    EXPECT_EQ(cache.getStats().misses, 1);
}

TEST_F(FrameCacheTest, Stats_CachedFramesCount_AfterEviction)
{
    // Arrange
    FrameCache cache(2);
    cache.put(0, makePositions(2));
    cache.put(1, makePositions(2));

    // Act: add a third entry (evicts one)
    cache.put(2, makePositions(2));

    // Assert: still 2 frames cached (capacity)
    EXPECT_EQ(cache.getStats().cached_frames, 2);
}

// ============================================
// Clear
// ============================================

TEST_F(FrameCacheTest, Clear_RemovesAllEntries)
{
    // Arrange
    FrameCache cache(4);
    cache.put(0, makePositions(2));
    cache.put(1, makePositions(2));

    // Act
    cache.clear();

    // Assert
    EXPECT_EQ(cache.getStats().cached_frames, 0);
    EXPECT_FALSE(cache.get(0).has_value());
    EXPECT_FALSE(cache.get(1).has_value());
}

TEST_F(FrameCacheTest, Clear_ResetsStats)
{
    // Arrange
    FrameCache cache(4);
    cache.put(0, makePositions(2));
    cache.get(0);  // hit
    cache.get(99); // miss

    // Act
    cache.clear();

    // Assert: stats reset
    auto stats = cache.getStats();
    EXPECT_EQ(stats.hits, 0);
    EXPECT_EQ(stats.misses, 0);
}

TEST_F(FrameCacheTest, AfterClear_NewEntries_Accepted)
{
    // Arrange
    FrameCache cache(2);
    cache.put(0, makePositions(2));
    cache.put(1, makePositions(2));
    cache.clear();

    // Act
    cache.put(5, makePositions(2, 50.0f));
    auto result = cache.get(5);

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(result->at(0).x, 50.0f);
}
