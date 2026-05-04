#pragma once
#include <glm/glm.hpp>

/// ICOMProvider: supplies per-frame center-of-mass positions.
///
/// Implementations:
///   - COMFileProvider  (reads from a pre-computed COMFile on disk)
///   - COMCacheProvider (computes from particle positions via COMCache) [future]
///
/// Caller contract: getCOM() may be called from the main thread only.
class ICOMProvider
{
  public:
    virtual ~ICOMProvider() = default;

    /// Writes the COM for @p frame into @p out.
    /// @return true if COM is available, false if unavailable (file absent, not yet computed, etc.)
    virtual bool getCOM(long frame, glm::vec3& out) = 0;
};
