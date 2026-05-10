#pragma once

/*
 * MockCOMProvider.hpp
 *
 * Mock implementation of ICOMProvider for use in unit tests.
 * Preset per-frame COM values with setCOM(); getCOM() returns true and
 * writes the preset value on a hit, or returns false leaving out unchanged
 * on a miss.
 *
 * Usage:
 *   MockCOMProvider provider;
 *   provider.setCOM(5, glm::vec3{1.0f, 2.0f, 3.0f});
 *   glm::vec3 out;
 *   bool ok = provider.getCOM(5, out);  // true; out == {1,2,3}
 */

#include <unordered_map>

#include <glm/glm.hpp>

#include "ICOMProvider.hpp"

class MockCOMProvider : public ICOMProvider
{
  public:
    /// Presets the COM value returned for @p frame.
    void setCOM(long frame, const glm::vec3& value)
    {
        m_presets[frame] = value;
    }

    /// Returns true and writes the preset value into @p out if @p frame was
    /// preset via setCOM(). Returns false and leaves @p out unchanged otherwise.
    bool getCOM(long frame, glm::vec3& out) override
    {
        auto it = m_presets.find(frame);
        if (it == m_presets.end()) {
            return false;
        }
        out = it->second;
        return true;
    }

  private:
    std::unordered_map<long, glm::vec3> m_presets;
};
