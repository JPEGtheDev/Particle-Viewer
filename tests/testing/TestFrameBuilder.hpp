#pragma once
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <glm/glm.hpp>

/// Builds a synthetic PosAndVel binary buffer for testing.
///
/// Format matches readPosVelFile (settingsIO.hpp): each frame consists of
/// N position glm::vec4 entries (with .w encoding the particle type) followed
/// by N velocity glm::vec4 entries. Frames are laid out sequentially.
///
/// Particle type encoding (.w field):
///   0 = Fe body1, 1 = Si body1, 2 = Fe body2, 3 = Si body2, 500 = default cube
class TestFrameBuilder
{
  public:
    explicit TestFrameBuilder(long n) : n_(n)
    {
    }

    /// Appends one frame. positions.size() must equal n_.
    /// Velocities are written as zero.
    void addFrame(const std::vector<glm::vec4>& positions)
    {
        if (positions.size() != static_cast<std::size_t>(n_)) {
            throw std::invalid_argument("TestFrameBuilder::addFrame: positions.size() != n_");
        }
        for (const auto& p : positions) {
            append(p);
        }
        const glm::vec4 zero{0.f, 0.f, 0.f, 0.f};
        for (long i = 0; i < n_; ++i) {
            append(zero);
        }
        ++frameCount_;
    }

    /// Writes the buffer to a temporary file and returns the file path.
    std::string writeToTempFile(const std::string& suffix = ".bin") const
    {
        std::string path =
            std::string("/tmp/TestFrameBuilder_") + std::to_string(reinterpret_cast<uintptr_t>(this)) + suffix;
        std::ofstream f(path, std::ios::binary);
        if (!f.is_open()) {
            throw std::runtime_error("TestFrameBuilder: failed to open temp file: " + path);
        }
        f.write(reinterpret_cast<const char*>(buffer_.data()), static_cast<std::streamsize>(buffer_.size()));
        if (f.fail()) {
            throw std::runtime_error("TestFrameBuilder: write failed: " + path);
        }
        return path;
    }

    const std::vector<uint8_t>& buffer() const
    {
        return buffer_;
    }
    long frameCount() const
    {
        return frameCount_;
    }
    long n() const
    {
        return n_;
    }

  private:
    long n_;
    long frameCount_ = 0;
    std::vector<uint8_t> buffer_;

    void append(const glm::vec4& v)
    {
        const auto* bytes = reinterpret_cast<const uint8_t*>(&v);
        buffer_.insert(buffer_.end(), bytes, bytes + sizeof(glm::vec4));
    }
};
