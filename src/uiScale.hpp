#pragma once

/*
 * uiScale.hpp
 *
 * Pure free functions for UI scale selection and clamping.
 *
 * clampUiScale(v) — clamp a scale factor to the supported [1.0, 3.0] range.
 * selectUiScale(detected, persisted) — choose the effective UI scale:
 *   - if persisted >= 1.0 (a real saved preference), clamp and return it.
 *   - otherwise fall back to the OS-detected scale, enforcing a 1.5x minimum,
 *     and clamp the result. 0.0 is the sentinel meaning "no saved preference."
 */

#include <algorithm>

/*
 * Clamp v to [1.0f, 3.0f].
 */
inline float clampUiScale(float v)
{
    return std::clamp(v, 1.0f, 3.0f);
}

/*
 * Select the effective UI scale.
 *
 * Parameters:
 *   detected  — scale reported by the OS / display subsystem.
 *   persisted — scale loaded from config; 0.0f means "no saved preference".
 *
 * Returns:
 *   clampUiScale(persisted)                    if persisted >= 1.0f
 *   clampUiScale(std::max(detected, 1.5f))     otherwise (1.5x minimum enforced)
 */
inline float selectUiScale(float detected, float persisted)
{
    if (persisted >= 1.0f) {
        return clampUiScale(persisted);
    }
    return clampUiScale(std::max(detected, 1.5f));
}
