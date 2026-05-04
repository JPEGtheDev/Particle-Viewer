#pragma once

#include <mutex>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#include <glm/glm.hpp>

// glad must be included before MassParams.hpp, which transitively pulls in
// settingsIO.hpp → particle.hpp (inline GL methods).  COMCache itself makes
// no GL calls.
#include <glad/glad.h> // NOLINT(llvm-include-order)

#include "IExecutor.hpp"
#include "MassParams.hpp"

// Forward declaration to avoid including the heavyweight settingsIO.hpp header.
class SettingsIO;

/// COMCache: on-demand, async memoisation of per-frame centre-of-mass values.
///
/// Each call to getCOM() either:
///   - Returns a cached glm::vec3 immediately (cache hit), or
///   - Returns std::nullopt and enqueues a background compute task (cache miss).
///
/// The compute task calls SettingsIO::readPosBuffer() + COMCalculator on the
/// executor thread. Results are stored under mutex when the task completes.
///
/// Thread-safety notes:
///   - getCOM() and clear() are individually thread-safe.
///   - Callers must ensure no tasks are in-flight when calling clear()
///     (e.g. on folder reload, drain the executor before calling clear()).
///
/// No GL calls are made anywhere in this class.
///
/// Non-copyable, non-movable.
class COMCache
{
  public:
    /// @param sio       Simulation I/O source; must outlive this object.
    /// @param mp        Mass parameters used for COM computation.
    /// @param executor  Task runner (ThreadedExecutor in production,
    ///                  SynchronousExecutor in tests); must outlive this object.
    COMCache(SettingsIO& sio, MassParams mp, IExecutor& executor);

    ~COMCache() = default;
    COMCache(const COMCache&) = delete;
    COMCache& operator=(const COMCache&) = delete;
    COMCache(COMCache&&) = delete;
    COMCache& operator=(COMCache&&) = delete;

    /// Returns the cached COM for @p frame if available, or std::nullopt while
    /// a compute task is pending. Callers should poll on subsequent frames.
    std::optional<glm::vec3> getCOM(long frame);

    /// Enqueues background COM computation for frames [cur+1, min(cur+ahead, total_frames-1)].
    /// Frames already cached or pending are silently skipped.
    /// Guards:
    ///   - If total_frames <= 0: returns immediately (no-op).
    ///   - If cur+1 > total_frames-1: no frames to prefetch (no-op).
    /// Caller (main loop) should pass ahead=64 (hard-coded default).
    void prefetchAsync(long cur, long ahead, long total_frames);

    /// Clears the cache and the pending set.
    /// Callers must ensure no compute tasks are in-flight when this is called
    /// (e.g. flush the executor before calling clear() on folder reload).
    void clear();

    /// Returns the number of frames whose COM is currently cached.
    /// Thread-safe.
    std::size_t cachedCount() const;

  private:
    SettingsIO& sio_;
    MassParams mp_;
    IExecutor& executor_;

    std::unordered_map<long, glm::vec3> cache_;
    std::unordered_set<long> pending_;
    mutable std::mutex mutex_;
};
