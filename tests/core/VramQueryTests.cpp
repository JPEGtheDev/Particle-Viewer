// tests/core/VramQueryTests.cpp
// Tests for IVramQuery interface and resolveGridResolution helper.
// NOTE: Do NOT include GladVramQuery.hpp here -- it requires GLAD and a live GL context.

#include <gtest/gtest.h>

#include "IVramQuery.hpp"

// 1. 256^3 requested with insufficient VRAM -> downgraded to 128
TEST(VramQueryTest, ResolveGridResolution_BelowThreshold_DowngradesTo128)
{
    bool was_downgraded = false;
    MockVramQuery vram(200);
    int result = resolveGridResolution(256, vram, was_downgraded);
    EXPECT_EQ(result, 128);
    EXPECT_TRUE(was_downgraded);
}

// 2. 256^3 requested with ample VRAM -> no downgrade
TEST(VramQueryTest, ResolveGridResolution_AboveThreshold_Returns256)
{
    bool was_downgraded = false;
    MockVramQuery vram(512);
    int result = resolveGridResolution(256, vram, was_downgraded);
    EXPECT_EQ(result, 256);
    EXPECT_FALSE(was_downgraded);
}

// 3. 256^3 requested with VRAM exactly at threshold -> NOT downgraded
TEST(VramQueryTest, ResolveGridResolution_ExactlyAtThreshold_Returns256)
{
    bool was_downgraded = false;
    MockVramQuery vram(280);
    int result = resolveGridResolution(256, vram, was_downgraded);
    EXPECT_EQ(result, 256);
    EXPECT_FALSE(was_downgraded);
}

// 4. 128^3 requested with very low VRAM -> no downgrade (only 256 is guarded)
TEST(VramQueryTest, ResolveGridResolution_128RequestedLowVram_Returns128)
{
    bool was_downgraded = false;
    MockVramQuery vram(50);
    int result = resolveGridResolution(128, vram, was_downgraded);
    EXPECT_EQ(result, 128);
    EXPECT_FALSE(was_downgraded);
}

// 5. 64^3 requested with minimal VRAM -> no downgrade
TEST(VramQueryTest, ResolveGridResolution_64RequestedMinimalVram_Returns64)
{
    bool was_downgraded = false;
    MockVramQuery vram(10);
    int result = resolveGridResolution(64, vram, was_downgraded);
    EXPECT_EQ(result, 64);
    EXPECT_FALSE(was_downgraded);
}
