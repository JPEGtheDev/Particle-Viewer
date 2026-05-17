/*
 * UiScaleTests.cpp
 *
 * Unit tests for clampUiScale() and selectUiScale() pure free functions.
 */

#include <gtest/gtest.h>

#include "uiScale.hpp"

// ---------------------------------------------------------------------------
// clampUiScale tests
// ---------------------------------------------------------------------------

TEST(ClampUiScale, BelowMin_ReturnsMin)
{
    EXPECT_FLOAT_EQ(clampUiScale(0.5f), 1.0f);
}

TEST(ClampUiScale, AboveMax_ReturnsMax)
{
    EXPECT_FLOAT_EQ(clampUiScale(4.0f), 3.0f);
}

TEST(ClampUiScale, InRange_ReturnsInput)
{
    EXPECT_FLOAT_EQ(clampUiScale(1.75f), 1.75f);
}

TEST(ClampUiScale, AtMinBoundary_ReturnsMin)
{
    EXPECT_FLOAT_EQ(clampUiScale(1.0f), 1.0f);
}

TEST(ClampUiScale, AtMaxBoundary_ReturnsMax)
{
    EXPECT_FLOAT_EQ(clampUiScale(3.0f), 3.0f);
}

// ---------------------------------------------------------------------------
// selectUiScale tests
// ---------------------------------------------------------------------------

// persisted >= 1.0f: saved preference wins
TEST(SelectUiScale, PersistedAboveOne_PersistedWins)
{
    // persisted 1.5 wins over detected 2.0
    EXPECT_FLOAT_EQ(selectUiScale(2.0f, 1.5f), 1.5f);
}

TEST(SelectUiScale, PersistedEqualsOne_PersistedWins)
{
    // persisted 1.0 wins over detected 2.0
    EXPECT_FLOAT_EQ(selectUiScale(2.0f, 1.0f), 1.0f);
}

// persisted == 0.0f (sentinel): fall back to OS-detected scale
TEST(SelectUiScale, PersistedZeroDetectedTwo_ReturnsDetected)
{
    EXPECT_FLOAT_EQ(selectUiScale(2.0f, 0.0f), 2.0f);
}

TEST(SelectUiScale, PersistedZeroDetectedOne_ReturnsMinimum)
{
    // 1.5x minimum enforced when detected is below it
    EXPECT_FLOAT_EQ(selectUiScale(1.0f, 0.0f), 1.5f);
}

TEST(SelectUiScale, PersistedZeroDetectedZero_ReturnsMinimum)
{
    EXPECT_FLOAT_EQ(selectUiScale(0.0f, 0.0f), 1.5f);
}

TEST(SelectUiScale, PersistedAboveMax_Clamped)
{
    // persisted 4.0 is clamped to 3.0
    EXPECT_FLOAT_EQ(selectUiScale(1.0f, 4.0f), 3.0f);
}

TEST(SelectUiScale, PersistedBelowOne_FallsBackToDetected)
{
    // 0.5 < 1.0, so OS-detected 2.5 is used
    EXPECT_FLOAT_EQ(selectUiScale(2.5f, 0.5f), 2.5f);
}
