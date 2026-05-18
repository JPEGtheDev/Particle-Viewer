/*
 * MockFileDialog.hpp
 *
 * Test double for IFileDialog. Returns a preset result on the first
 * selectFolder() call and closes immediately (isOpen() == false after first call).
 */
#ifndef PARTICLE_VIEWER_MOCK_FILE_DIALOG_H
#define PARTICLE_VIEWER_MOCK_FILE_DIALOG_H

#include <string>

#include "IFileDialog.hpp"

class MockFileDialog : public IFileDialog
{
  public:
    explicit MockFileDialog(std::string preset_result = "")
        : m_preset_result_(std::move(preset_result)), m_is_open_(true)
    {
    }

    std::string selectFolder(const std::string& /*title*/) override
    {
        m_is_open_ = false;
        return m_preset_result_;
    }

    [[nodiscard]] bool isOpen() const noexcept override
    {
        return m_is_open_;
    }

    // Reset so the mock can be reused across multiple dialog opens.
    void reset(std::string new_result)
    {
        m_preset_result_ = std::move(new_result);
        m_is_open_ = true;
    }

  private:
    std::string m_preset_result_;
    bool m_is_open_;
};

#endif // PARTICLE_VIEWER_MOCK_FILE_DIALOG_H
