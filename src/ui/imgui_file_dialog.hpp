/*
 * imgui_file_dialog.hpp
 *
 * ImGui-based folder browser implementing IFileDialog.
 *
 * ImGuiFolderBrowser renders an interactive folder-selection dialog using
 * Dear ImGui widgets. selectFolder() is called each frame while the dialog
 * is open; it renders one frame of UI and returns:
 *   - "" while the user is still browsing
 *   - a non-empty validated path string when the user confirms a selection
 * isOpen() returns false once the dialog closes (cancelled or confirmed).
 *
 * Validation is controlled by the ValidationMode constructor parameter:
 *   - kSimulationFolder (default): folder must contain PosAndVel and RunSetup
 *   - kAnyDirectory: any existing directory is accepted (used for recording output)
 */

#ifndef PARTICLE_VIEWER_IMGUI_FILE_DIALOG_H
#define PARTICLE_VIEWER_IMGUI_FILE_DIALOG_H

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "IFileDialog.hpp"
#include "imgui.h"

class ImGuiFolderBrowser : public IFileDialog
{
  public:
    enum class ValidationMode
    {
        kSimulationFolder, // requires PosAndVel and RunSetup (default)
        kAnyDirectory      // accepts any existing directory (recording output)
    };

    explicit ImGuiFolderBrowser(ValidationMode mode = ValidationMode::kSimulationFolder);

    // Called each frame while the dialog is open.
    // Returns "" while browsing, non-empty validated path on confirm.
    [[nodiscard]] std::string selectFolder(const std::string& title) override;

    // True while dialog is visible; false after cancel or confirm.
    [[nodiscard]] bool isOpen() const noexcept override;

    // Set the folder from the last confirmed simulation load.
    // The dialog will open in the PARENT of this folder and pre-select it
    // if it still exists on disk.
    void setLastConfirmedFolder(const std::string& folder);

  private:
    bool m_isOpen = false;
    bool m_needsInit = true; // true on first call after construction/reset
    ValidationMode m_validationMode;

    std::filesystem::path m_currentDir;          // directory currently being listed
    std::filesystem::path m_selectedEntry;       // highlighted entry in the list
    std::filesystem::path m_lastConfirmedFolder; // from setLastConfirmedFolder()

    char m_inputBuf[4096] = {};    // for ImGui::InputText manual path entry
    char m_newFolderBuf[256] = {}; // for the "New Folder" popup InputText
    std::string m_errorMsg;        // shown inline when validation fails

    // Enumerated directories (non-dot) and files in m_currentDir.
    std::vector<std::filesystem::directory_entry> m_entries;

    // Set m_currentDir from m_lastConfirmedFolder or $HOME; calls refreshEntries().
    void initDirectory();

    // Rebuild m_entries from m_currentDir (directories first, then files, both
    // sorted alphabetically). Catches filesystem exceptions gracefully.
    void refreshEntries();

    // Validate m_selectedEntry; returns the canonical path string on success or
    // sets m_errorMsg and returns "" on failure.
    std::string tryConfirm();
};

// ---------------------------------------------------------------------------
// Inline implementation
// ---------------------------------------------------------------------------

inline ImGuiFolderBrowser::ImGuiFolderBrowser(ValidationMode mode) : m_validationMode(mode)
{
}

inline bool ImGuiFolderBrowser::isOpen() const noexcept
{
    return m_isOpen;
}

inline void ImGuiFolderBrowser::setLastConfirmedFolder(const std::string& folder)
{
    m_lastConfirmedFolder = folder;
}

inline void ImGuiFolderBrowser::initDirectory()
{
    if (!m_lastConfirmedFolder.empty() && std::filesystem::exists(m_lastConfirmedFolder)) {
        m_currentDir = m_lastConfirmedFolder.parent_path();
        m_selectedEntry = m_lastConfirmedFolder;
    } else {
        const char* home = std::getenv("HOME");
        m_currentDir = home ? std::filesystem::path(home) : std::filesystem::path("/");
        m_selectedEntry = std::filesystem::path{};
    }

    refreshEntries();

    // Pre-fill the manual path input buffer
    const std::string cur = m_currentDir.string();
    const std::size_t len = std::min(cur.size(), sizeof(m_inputBuf) - 1);
    cur.copy(m_inputBuf, len);
    m_inputBuf[len] = '\0';
}

inline void ImGuiFolderBrowser::refreshEntries()
{
    m_entries.clear();

    try {
        for (const auto& entry : std::filesystem::directory_iterator(
                 m_currentDir, std::filesystem::directory_options::skip_permission_denied)) {
            const std::string fname = entry.path().filename().string();
            if (fname.empty() || fname[0] == '.') {
                continue; // hide dot-dirs and dot-files
            }
            if (entry.is_directory() || entry.is_regular_file()) {
                m_entries.push_back(entry);
            }
        }

        // Sort: directories first, then files; each group alphabetically.
        std::sort(m_entries.begin(), m_entries.end(),
                  [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b) {
                      const bool aDir = a.is_directory();
                      const bool bDir = b.is_directory();
                      if (aDir != bDir) {
                          return aDir > bDir; // directories first
                      }
                      return a.path().filename() < b.path().filename();
                  });
    } catch (...) {
        m_entries.clear(); // Permission denied or other I/O error — discard partial entries.
    }
}

inline std::string ImGuiFolderBrowser::tryConfirm()
{
    if (m_selectedEntry.empty()) {
        m_errorMsg = "No folder selected";
        return {};
    }

    try {
        const std::filesystem::path path = std::filesystem::weakly_canonical(m_selectedEntry);

        if (m_validationMode == ValidationMode::kSimulationFolder) {
            const bool valid =
                std::filesystem::exists(path / "PosAndVel") && std::filesystem::exists(path / "RunSetup");
            if (!valid) {
                m_errorMsg = "Not a valid simulation folder (PosAndVel/RunSetup not found)";
                return {};
            }
        } else {
            if (!std::filesystem::is_directory(path)) {
                m_errorMsg = "Please select an existing directory";
                return {};
            }
        }

        m_errorMsg.clear();
        return path.string();
    } catch (const std::filesystem::filesystem_error& e) {
        m_errorMsg = std::string("Filesystem error: ") + e.what();
        return {};
    }
}

inline std::string ImGuiFolderBrowser::selectFolder(const std::string& title)
{
    if (m_needsInit) {
        initDirectory();
        m_isOpen = true;
        m_needsInit = false;
    }

    constexpr ImGuiWindowFlags kWindowFlags =
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600.0f, 450.0f), ImGuiCond_FirstUseEver);
    bool windowOpen = true;
    if (!ImGui::Begin(title.c_str(), &windowOpen, kWindowFlags)) {
        ImGui::End();
        return {};
    }

    // If the user closed the window via the [x] button
    if (!windowOpen) {
        m_isOpen = false;
        m_needsInit = true;
        m_errorMsg.clear();
        ImGui::End();
        return {};
    }

    // --- Current path display ---
    ImGui::TextUnformatted(m_currentDir.string().c_str());

    // --- "Up" button ---
    if (ImGui::Button("^##up")) {
        const std::filesystem::path parent = m_currentDir.parent_path();
        if (parent != m_currentDir) {
            m_currentDir = parent;
            refreshEntries();

            const std::string cur = m_currentDir.string();
            const std::size_t len = std::min(cur.size(), sizeof(m_inputBuf) - 1);
            cur.copy(m_inputBuf, len);
            m_inputBuf[len] = '\0';
        }
    }
    ImGui::SameLine();

    // --- "New Folder" button ---
    if (ImGui::Button("New Folder")) {
        m_newFolderBuf[0] = '\0';
        ImGui::OpenPopup("New Folder##popup");
    }
    ImGui::SameLine();

    // --- New Folder modal popup ---
    if (ImGui::BeginPopupModal("New Folder##popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Folder name:");
        ImGui::SetNextItemWidth(300.0f);
        if (ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere();
        }
        const bool confirmed =
            ImGui::InputText("##newname", m_newFolderBuf, sizeof(m_newFolderBuf), ImGuiInputTextFlags_EnterReturnsTrue);
        if ((ImGui::Button("Create") || confirmed) && m_newFolderBuf[0] != '\0') {
            std::error_code ec;
            const std::filesystem::path newPath = m_currentDir / m_newFolderBuf;
            if (std::filesystem::create_directories(newPath, ec)) {
                m_selectedEntry = newPath;
                refreshEntries();
                const std::string sel = newPath.string();
                const std::size_t slen = std::min(sel.size(), sizeof(m_inputBuf) - 1);
                sel.copy(m_inputBuf, slen);
                m_inputBuf[slen] = '\0';
            } else {
                m_errorMsg = "Could not create folder: " + ec.message();
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // --- Manual path InputText ---
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::InputText("##path", m_inputBuf, sizeof(m_inputBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
        std::filesystem::path typed(m_inputBuf);
        std::error_code ec;
        if (std::filesystem::is_directory(typed, ec)) {
            // Navigate to the parent of the typed path and select the typed
            // directory itself, so the user can immediately click OK to confirm.
            const std::filesystem::path parent = typed.parent_path();
            m_currentDir = (parent != typed) ? parent : typed;
            m_selectedEntry = typed;
            refreshEntries();
        }
    }

    // --- Scrollable directory listing ---
    ImGui::BeginChild("##listing", ImVec2(0.0f, 300.0f), true);

    for (std::size_t i = 0; i < m_entries.size(); ++i) {
        const auto& entry = m_entries[i];
        const std::string name = entry.path().filename().string();
        const bool selected = (entry.path() == m_selectedEntry);

        if (entry.is_directory()) {
            if (ImGui::Selectable(name.c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick)) {
                if (ImGui::IsMouseDoubleClicked(0)) {
                    m_currentDir = entry.path();
                    m_selectedEntry = std::filesystem::path{};
                    refreshEntries();

                    const std::string cur = m_currentDir.string();
                    const std::size_t len = std::min(cur.size(), sizeof(m_inputBuf) - 1);
                    cur.copy(m_inputBuf, len);
                    m_inputBuf[len] = '\0';
                } else {
                    m_selectedEntry = entry.path();
                }
            }

            // Arrow-key navigation when this item is focused
            if (ImGui::IsItemFocused()) {
                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && i > 0) {
                    m_selectedEntry = m_entries[i - 1].path();
                } else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && i + 1 < m_entries.size()) {
                    m_selectedEntry = m_entries[i + 1].path();
                }
            }
        } else if (entry.is_regular_file()) {
            // Files are shown greyed-out and non-selectable
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            ImGui::TextUnformatted(name.c_str());
            ImGui::PopStyleColor();
        }
    }

    ImGui::EndChild();

    // --- Inline error message ---
    if (!m_errorMsg.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextUnformatted(m_errorMsg.c_str());
        ImGui::PopStyleColor();
    }

    // --- OK / Cancel buttons ---
    const bool anyPopupOpen = ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId);
    if (ImGui::Button("OK") ||
        (!m_selectedEntry.empty() && !anyPopupOpen && ImGui::IsKeyPressed(ImGuiKey_Enter, false))) {
        std::string result = tryConfirm();
        if (!result.empty()) {
            m_isOpen = false;
            m_needsInit = true;
            ImGui::End();
            return result;
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
        m_isOpen = false;
        m_needsInit = true;
        m_errorMsg.clear();
        ImGui::End();
        return {};
    }

    ImGui::End();
    return {}; // still browsing
}

#endif // PARTICLE_VIEWER_IMGUI_FILE_DIALOG_H
