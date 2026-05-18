/*
 * FileDialogContractTests.cpp
 *
 * Contract tests for the IFileDialog interface and MockFileDialog test double.
 * Verifies behavioural contracts that all IFileDialog implementations must satisfy.
 */

#include <gtest/gtest.h>

#include "IFileDialog.hpp"
#include "MockFileDialog.hpp"

// ---- Polymorphism ----

static_assert(std::is_base_of_v<IFileDialog, MockFileDialog>, "MockFileDialog must derive from IFileDialog");

// ---- Confirm path ----

TEST(FileDialogContractTest, SelectFolder_WithPresetPath_ReturnsPresetPath)
{
    MockFileDialog mock("/some/path");
    const std::string result = mock.selectFolder("Select Folder");
    EXPECT_EQ(result, "/some/path");
}

TEST(FileDialogContractTest, SelectFolder_AfterFirstCall_IsOpenIsFalse)
{
    MockFileDialog mock("/some/path");
    mock.selectFolder("Select Folder");
    EXPECT_FALSE(mock.isOpen());
}

// ---- Cancel path ----

TEST(FileDialogContractTest, SelectFolder_WithEmptyPreset_ReturnsEmptyString)
{
    MockFileDialog mock("");
    const std::string result = mock.selectFolder("Select Folder");
    EXPECT_TRUE(result.empty());
}

TEST(FileDialogContractTest, SelectFolder_WithEmptyPreset_IsOpenIsFalse)
{
    MockFileDialog mock("");
    mock.selectFolder("Select Folder");
    EXPECT_FALSE(mock.isOpen());
}

// ---- Reset / reuse ----

TEST(FileDialogContractTest, Reset_AfterCall_IsOpenTrue)
{
    MockFileDialog mock("");
    mock.selectFolder("Select Folder"); // first dialog cycle: cancel
    mock.reset("/new/path");
    EXPECT_TRUE(mock.isOpen());
}

TEST(FileDialogContractTest, Reset_SelectFolder_ReturnsNewPreset)
{
    MockFileDialog mock("");
    mock.selectFolder("Select Folder"); // first dialog cycle: cancel
    mock.reset("/new/path");
    const std::string result = mock.selectFolder("Select Folder");
    EXPECT_EQ(result, "/new/path");
}

TEST(FileDialogContractTest, Reset_AfterSelectFolder_IsOpenFalse)
{
    MockFileDialog mock("");
    mock.selectFolder("Select Folder"); // first dialog cycle: cancel
    mock.reset("/new/path");
    mock.selectFolder("Select Folder"); // second dialog cycle: confirm
    EXPECT_FALSE(mock.isOpen());
}
