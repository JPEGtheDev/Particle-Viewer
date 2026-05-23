/*
 * IFileDialog.hpp
 *
 * Abstract interface for folder selection dialogs.
 * Separates dialog UI from application logic, enabling test injection.
 *
 * For ImGui implementations: selectFolder() is called each frame while the
 * dialog is open. It renders one frame of UI and returns:
 *   - "" while the user is still browsing
 *   - a non-empty path string when the user confirms a selection
 * isOpen() returns false once the dialog closes (cancelled or confirmed).
 *
 * For mock implementations: selectFolder() returns the preset result immediately
 * and isOpen() returns false after the first call.
 */
#ifndef PARTICLE_VIEWER_IFILEDIALOG_H
#define PARTICLE_VIEWER_IFILEDIALOG_H

#include <string>

class IFileDialog
{
  public:
    virtual ~IFileDialog() = default;

    // Returns selected path on confirm, empty string while in progress or on cancel.
    [[nodiscard]] virtual std::string selectFolder(const std::string& title) = 0;

    // Returns true while dialog is visible. Becomes false on cancel or confirm.
    // Initial state (before the first selectFolder() call in a cycle) is
    // implementation-defined; callers must not rely on isOpen() until after
    // the first selectFolder() call of that cycle.
    [[nodiscard]] virtual bool isOpen() const noexcept = 0;
};

#endif // PARTICLE_VIEWER_IFILEDIALOG_H
