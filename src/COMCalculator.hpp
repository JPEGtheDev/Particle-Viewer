#pragma once

#include <span>

#include <glm/glm.hpp>

#include "MassParams.hpp"
#include "constants.hpp"

/// Pure static utility for computing a mass-weighted centre of mass (COM)
/// from a span of particle positions.
///
/// Particle type is encoded in the w-component of each glm::vec4:
///   0 → Fe particle, body 1
///   1 → Si particle, body 1
///   2 → Fe particle, body 2
///   3 → Si particle, body 2
///   any other value → ignored
///
/// The result is expressed in display-space coordinates
/// (i.e. already multiplied by kSimToDisplayScale).
class COMCalculator
{
  public:
    /// Compute the mass-weighted COM of @p positions using @p mp mass fractions.
    ///
    /// @param positions  Span of (x, y, z, type) particle data.
    /// @param mp         Mass parameters (fractions, earth mass).
    /// @return           COM in display space, or (0,0,0) if total mass is zero.
    static glm::vec3 computeMassWeightedCOM(std::span<const glm::vec4> positions, const MassParams& mp)
    {
        // ── Step 1: count particles by type ───────────────────────────────────
        std::size_t nFe1 = 0;
        std::size_t nSi1 = 0;
        std::size_t nFe2 = 0;
        std::size_t nSi2 = 0;

        for (const auto& p : positions) {
            switch (static_cast<int>(p.w)) {
                case 0:
                    ++nFe1;
                    break;
                case 1:
                    ++nSi1;
                    break;
                case 2:
                    ++nFe2;
                    break;
                case 3:
                    ++nSi2;
                    break;
                default:
                    break;
            }
        }

        // ── Step 2: per-particle masses (guard: 0 when no particles of that type) ──
        const double mFe1 = (nFe1 > 0) ? (mp.fraction_fe_body1 * mp.fraction_earth_mass_of_body1 * mp.mass_of_earth) /
                                             static_cast<double>(nFe1)
                                       : 0.0;
        const double mSi1 = (nSi1 > 0) ? (mp.fraction_si_body1 * mp.fraction_earth_mass_of_body1 * mp.mass_of_earth) /
                                             static_cast<double>(nSi1)
                                       : 0.0;
        const double mFe2 = (nFe2 > 0) ? (mp.fraction_fe_body2 * mp.fraction_earth_mass_of_body2 * mp.mass_of_earth) /
                                             static_cast<double>(nFe2)
                                       : 0.0;
        const double mSi2 = (nSi2 > 0) ? (mp.fraction_si_body2 * mp.fraction_earth_mass_of_body2 * mp.mass_of_earth) /
                                             static_cast<double>(nSi2)
                                       : 0.0;

        // ── Step 3: accumulate weighted positions in double precision ─────────
        double sumMx = 0.0;
        double sumMy = 0.0;
        double sumMz = 0.0;
        double totalMass = 0.0;

        for (const auto& p : positions) {
            double mass = 0.0;
            switch (static_cast<int>(p.w)) {
                case 0:
                    mass = mFe1;
                    break;
                case 1:
                    mass = mSi1;
                    break;
                case 2:
                    mass = mFe2;
                    break;
                case 3:
                    mass = mSi2;
                    break;
                default:
                    continue; // unknown type: skip
            }

            sumMx += mass * static_cast<double>(p.x);
            sumMy += mass * static_cast<double>(p.y);
            sumMz += mass * static_cast<double>(p.z);
            totalMass += mass;
        }

        // ── Step 4: guard zero total mass ─────────────────────────────────────
        if (totalMass == 0.0) {
            return glm::vec3(0.0f);
        }

        // ── Step 5: divide and scale to display space ─────────────────────────
        const auto comX = static_cast<float>(sumMx / totalMass) * kSimToDisplayScale;
        const auto comY = static_cast<float>(sumMy / totalMass) * kSimToDisplayScale;
        const auto comZ = static_cast<float>(sumMz / totalMass) * kSimToDisplayScale;

        return glm::vec3(comX, comY, comZ);
    }
};
