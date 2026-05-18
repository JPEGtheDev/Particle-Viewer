/*
 * ImGuiFolderBrowserTests.cpp
 *
 * Structural contract and state-machine tests for ImGuiFolderBrowser.
 *
 * selectFolder() requires an active ImGui context; AC9 (keyboard nav) is verified manually.
 */

#include <gtest/gtest.h>
// glad.h before project includes (project convention)
#include <glad/glad.h>

#include "ui/imgui_file_dialog.hpp"

// Structural contract: ImGuiFolderBrowser IS-A IFileDialog
static_assert(std::is_base_of_v<IFileDialog, ImGuiFolderBrowser>, "ImGuiFolderBrowser must inherit from IFileDialog");

TEST(ImGuiFolderBrowser, Default_IsOpenFalse)
{
    ImGuiFolderBrowser browser;
    EXPECT_FALSE(browser.isOpen());
}

TEST(ImGuiFolderBrowser, SetLastConfirmedFolder_ValidPath_DoesNotCrash)
{
    ImGuiFolderBrowser browser;
    EXPECT_NO_THROW(browser.setLastConfirmedFolder("/some/path"));
}

TEST(ImGuiFolderBrowser, SetLastConfirmedFolder_EmptyString_DoesNotCrash)
{
    ImGuiFolderBrowser browser;
    EXPECT_NO_THROW(browser.setLastConfirmedFolder(""));
}
