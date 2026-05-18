/*
 * file_dialog_helpers.hpp
 *
 * Cancel-safety free functions for dialog result processing.
 * These functions are pure — no side effects beyond the out-parameter.
 */
#ifndef PARTICLE_VIEWER_FILE_DIALOG_HELPERS_H
#define PARTICLE_VIEWER_FILE_DIALOG_HELPERS_H

#include <string>

#include "recording_state.hpp"

class SettingsIO;

/*
 * Apply a recording folder selection result to RecordingState.
 * Returns true if folder was applied (non-empty), false if cancelled.
 * If folder is empty, state is left unchanged.
 */
[[nodiscard]] inline bool applyRecordingFolderResult(const std::string& folder, RecordingState& state)
{
    if (folder.empty()) {
        return false;
    }
    state.folder = folder;
    state.is_active = true;
    return true;
}

/*
 * Returns true if result represents a new file selection (different from current).
 * Safe to call when result is null.
 */
[[nodiscard]] inline bool isNewFileSelected(const SettingsIO* result, const SettingsIO* current) noexcept
{
    return result != nullptr && result != current;
}

#endif // PARTICLE_VIEWER_FILE_DIALOG_HELPERS_H
