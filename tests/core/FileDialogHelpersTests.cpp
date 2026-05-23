/*
 * FileDialogHelpersTests.cpp
 *
 * Unit tests for cancel-safety free functions in file_dialog_helpers.hpp.
 * Tests cover: applyRecordingFolderResult() and isNewFileSelected().
 */

// glad must come first to avoid OpenGL header conflicts
#include <glad/glad.h>
#include <gtest/gtest.h>

#include "MockOpenGL.hpp"
#include "file_dialog_helpers.hpp"
#include "recording_state.hpp"
#include "settingsIO.hpp"

// ---- applyRecordingFolderResult() ----

TEST(FileDialogHelpersTest, ApplyRecordingFolderResult_EmptyFolder_ReturnsFalse)
{
    RecordingState state;
    const bool result = applyRecordingFolderResult("", state);
    EXPECT_FALSE(result);
}

TEST(FileDialogHelpersTest, ApplyRecordingFolderResult_EmptyFolder_DoesNotMutateState)
{
    RecordingState state;
    const bool original_active = state.is_active;
    const std::string original_folder = state.folder;

    (void)applyRecordingFolderResult("", state);

    EXPECT_EQ(state.is_active, original_active);
    EXPECT_EQ(state.folder, original_folder);
}

TEST(FileDialogHelpersTest, ApplyRecordingFolderResult_ValidFolder_ReturnsTrue)
{
    RecordingState state;
    const bool result = applyRecordingFolderResult("/some/path", state);
    EXPECT_TRUE(result);
}

TEST(FileDialogHelpersTest, ApplyRecordingFolderResult_ValidFolder_SetsFolder)
{
    RecordingState state;
    (void)applyRecordingFolderResult("/some/path", state);
    EXPECT_EQ(state.folder, "/some/path");
}

TEST(FileDialogHelpersTest, ApplyRecordingFolderResult_ValidFolder_SetsIsActiveTrue)
{
    RecordingState state;
    (void)applyRecordingFolderResult("/some/path", state);
    EXPECT_TRUE(state.is_active);
}

// ---- isNewFileSelected() ----

TEST(FileDialogHelpersTest, IsNewFileSelected_NullResult_ReturnsFalse)
{
    // Arrange
    SettingsIO current;

    // Act / Assert
    EXPECT_FALSE(isNewFileSelected(nullptr, &current));
}

TEST(FileDialogHelpersTest, IsNewFileSelected_SamePointer_ReturnsFalse)
{
    SettingsIO settings;
    EXPECT_FALSE(isNewFileSelected(&settings, &settings));
}

TEST(FileDialogHelpersTest, IsNewFileSelected_DifferentNonNullPointers_ReturnsTrue)
{
    SettingsIO a;
    SettingsIO b;
    EXPECT_TRUE(isNewFileSelected(&a, &b));
}

TEST(FileDialogHelpersTest, IsNewFileSelected_NullCurrentAndNonNullResult_ReturnsTrue)
{
    SettingsIO result;
    EXPECT_TRUE(isNewFileSelected(&result, nullptr));
}

TEST(FileDialogHelpersTest, IsNewFileSelected_BothNull_ReturnsFalse)
{
    // Act / Assert
    EXPECT_FALSE(isNewFileSelected(nullptr, nullptr));
}
