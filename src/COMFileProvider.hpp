#pragma once

// glad must be included before settingsIO.hpp, which transitively includes
// particle.hpp that uses inline GL methods.
#include <glad/glad.h> // NOLINT(llvm-include-order)

#include "ICOMProvider.hpp"
#include "settingsIO.hpp"

/// COMFileProvider: reads per-frame center-of-mass values from a pre-computed
/// COMFile on disk via SettingsIO.
///
/// getCOM() delegates to SettingsIO::checkCOM() and SettingsIO::getCOM().
/// The scale factor (kSimToDisplayScale) is applied inside SettingsIO::getCOM()
/// — this class must NOT apply it again.
///
/// Non-copyable (stores a reference to SettingsIO).
class COMFileProvider : public ICOMProvider
{
  public:
    /// @param sio  SettingsIO instance whose comName points at the COMFile.
    explicit COMFileProvider(SettingsIO& sio) : sio_(sio)
    {
    }

    COMFileProvider(const COMFileProvider&) = delete;
    COMFileProvider& operator=(const COMFileProvider&) = delete;

    /// Returns true and writes the scaled COM into @p out if the COMFile is
    /// present and the record for @p frame is valid.
    /// Returns false (leaving @p out unchanged) if checkCOM() reports no file.
    bool getCOM(long frame, glm::vec3& out) override
    {
        if (!sio_.checkCOM()) {
            return false;
        }
        sio_.getCOM(frame, out);
        return true;
    }

  private:
    SettingsIO& sio_;
};
