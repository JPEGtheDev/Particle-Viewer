#pragma once

/// Pure virtual interface for VRAM availability queries.
class IVramQuery
{
  public:
    virtual ~IVramQuery() = default;

    /// Returns the currently available VRAM in megabytes.
    virtual int availableMB() const = 0;
};

/// Test double -- returns a fixed VRAM amount.
class MockVramQuery : public IVramQuery
{
  public:
    explicit MockVramQuery(int mb) : mb_(mb)
    {
    }
    int availableMB() const override
    {
        return mb_;
    }

  private:
    int mb_;
};

/// VRAM threshold required to safely run a 256^3 marching cubes pass:
/// - GL_R32F density field: 256 * 256 * 256 * 4 bytes = 64 MB
/// - SSBO (2M triangles * 3 vertices * 36 bytes per vertex): 216 MB
/// - Total: 280 MB
inline constexpr int kVramThreshold256Mb = 280;

/// Resolves the effective grid resolution given available VRAM.
///
/// @param requested_res  One of 64, 128, or 256.
/// @param vram           VRAM query provider.
/// @param was_downgraded Set to true if resolution was reduced, false otherwise.
/// @return Effective grid resolution (64, 128, or 256).
inline int resolveGridResolution(int requested_res, const IVramQuery& vram, bool& was_downgraded)
{
    if (requested_res == 256 && vram.availableMB() < kVramThreshold256Mb) {
        was_downgraded = true;
        return 128;
    }
    was_downgraded = false;
    return requested_res;
}
