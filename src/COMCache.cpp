/*
 * COMCache.cpp
 *
 * Async memoisation of per-frame centre-of-mass values.
 * See COMCache.hpp for design notes.
 */

// COMCache.hpp includes <glad/glad.h> before MassParams.hpp → settingsIO.hpp
// → particle.hpp, so no special ordering is needed here.
#include "COMCache.hpp"

#include <algorithm>
#include <span>

#include <glm/glm.hpp>

#include "COMCalculator.hpp"
#include "settingsIO.hpp"

COMCache::COMCache(SettingsIO& sio, MassParams mp, IExecutor& executor)
    : sio_(sio), mp_(std::move(mp)), executor_(executor)
{
}

std::optional<glm::vec3> COMCache::getCOM(long frame)
{
    {
        std::lock_guard<std::mutex> lock{mutex_};

        // Cache hit: return stored value immediately.
        if (auto it = cache_.find(frame); it != cache_.end()) {
            return it->second;
        }

        // Permanent I/O failure for this frame: do not re-enqueue.
        if (failed_.count(frame) > 0) {
            return std::nullopt;
        }

        // Already enqueued: don't double-enqueue.
        if (pending_.count(frame) > 0) {
            return std::nullopt;
        }

        // Mark as pending before releasing the lock.
        pending_.insert(frame);
    }
    // Mutex is NOT held here; the compute task may re-acquire it safely.
    // This prevents a deadlock when using SynchronousExecutor (which runs
    // the task inline on the calling thread before returning from enqueue).
    executor_.enqueue([this, frame]() {
        std::vector<glm::vec4> positions = sio_.readPosBuffer(frame);

        std::lock_guard<std::mutex> lock{mutex_};
        pending_.erase(frame);

        if (positions.empty()) {
            // I/O failure: record in failed_ to prevent infinite re-enqueue.
            // Do NOT cache (0,0,0) — callers keep their previous COM on miss.
            failed_.insert(frame);
            return;
        }

        glm::vec3 com = COMCalculator::computeMassWeightedCOM(std::span<const glm::vec4>(positions), mp_);
        cache_[frame] = com;
    });

    return std::nullopt;
}

void COMCache::prefetchAsync(long cur, long ahead, long total_frames)
{
    if (total_frames <= 0) {
        return;
    }
    const long last = std::min(cur + ahead, total_frames - 1);
    for (long f = cur + 1; f <= last; ++f) {
        getCOM(f); // side-effect: enqueues compute on miss; no-op on hit/pending
    }
}

void COMCache::clear()
{
    std::lock_guard<std::mutex> lock{mutex_};
    cache_.clear();
    pending_.clear();
    failed_.clear();
}

std::size_t COMCache::cachedCount() const
{
    std::lock_guard<std::mutex> lock{mutex_};
    return cache_.size();
}
