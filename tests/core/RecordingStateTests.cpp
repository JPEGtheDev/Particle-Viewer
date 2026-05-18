/*
 * RecordingStateTests.cpp
 *
 * Unit tests for the RecordingState struct.
 * Verifies struct compiles and members have correct default values.
 */

#include <gtest/gtest.h>

#include "recording_state.hpp"

TEST(RecordingStateTest, IsActive_DefaultValue_IsFalse)
{
    RecordingState state;
    EXPECT_FALSE(state.is_active);
}

TEST(RecordingStateTest, Folder_DefaultValue_IsEmpty)
{
    RecordingState state;
    EXPECT_TRUE(state.folder.empty());
}

TEST(RecordingStateTest, ErrorCount_DefaultValue_IsZero)
{
    RecordingState state;
    EXPECT_EQ(state.error_count, 0);
}

TEST(RecordingStateTest, ErrorMax_DefaultValue_IsFive)
{
    RecordingState state;
    EXPECT_EQ(state.error_max, 5);
}
