/*
 * FrameCache.cpp
 *
 * Implementation of the LRU sliding-window frame cache.
 */

// FrameCache.hpp includes <glad/glad.h> before settingsIO.hpp → particle.hpp,
// so no special ordering is needed here.
#include "FrameCache.hpp"

#include <algorithm>

// ──────────────────────────────────────────────────────────────────────────────
// Constructor
// ──────────────────────────────────────────────────────────────────────────────

FrameCache::FrameCache(SettingsIO& sio, std::size_t max_frame_bytes, IExecutor& executor)
    : sio_(sio), max_frames_(0), executor_(executor)
{
    const std::size_t frame_size_bytes = (sio_.N > 0) ? static_cast<std::size_t>(sio_.N) * sizeof(glm::vec4) : 0;

    if (frame_size_bytes > 0) {
        max_frames_ = max_frame_bytes / frame_size_bytes;
    }
    // If frame_size_bytes == 0: max_frames_ stays 0 → getFrame always returns nullptr
}

// ──────────────────────────────────────────────────────────────────────────────
// getFrame
// ──────────────────────────────────────────────────────────────────────────────

std::shared_ptr<std::vector<glm::vec4>> FrameCache::getFrame(long frame)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = cache_map_.find(frame);
    if (it == cache_map_.end()) {
        return nullptr; // cache miss
    }

    // Move hit entry to front of LRU list (most-recently-used)
    lru_list_.erase(it->second.second);
    lru_list_.push_front(frame);
    it->second.second = lru_list_.begin();

    return it->second.first;
}

// ──────────────────────────────────────────────────────────────────────────────
// prefetch
// ──────────────────────────────────────────────────────────────────────────────

void FrameCache::prefetch(long current, long window, long total_frames)
{
    if (total_frames <= 0) {
        return;
    }

    const long last = std::min(current + window, total_frames - 1);

    for (long f = current + 1; f <= last; ++f) {
        std::unique_lock<std::mutex> lock(mutex_);

        // Skip if already cached or already pending
        if (cache_map_.count(f) > 0 || pending_.count(f) > 0) {
            continue;
        }

        pending_.insert(f);
        lock.unlock();

        executor_.enqueue([this, f]() {
            auto data = sio_.readPosBuffer(f);
            if (data.empty()) {
                // I/O failed: remove from pending, nothing to insert
                std::lock_guard<std::mutex> guard(mutex_);
                pending_.erase(f);
                return;
            }

            auto shared = std::make_shared<std::vector<glm::vec4>>(std::move(data));

            std::lock_guard<std::mutex> guard(mutex_);
            pending_.erase(f);
            insertLocked(f, std::move(shared));
        });
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// clear
// ──────────────────────────────────────────────────────────────────────────────

void FrameCache::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    cache_map_.clear();
    lru_list_.clear();
    pending_.clear();
}

// ──────────────────────────────────────────────────────────────────────────────
// insertLocked (private) — must be called with mutex_ held
// ──────────────────────────────────────────────────────────────────────────────

void FrameCache::insertLocked(long frame, std::shared_ptr<std::vector<glm::vec4>> data)
{
    // Zero-capacity: budget rounds to zero frames; discard all inserts.
    if (max_frames_ == 0) {
        return;
    }

    // If frame is already in cache (race: two tasks for same frame), drop duplicate
    if (cache_map_.count(frame) > 0) {
        return;
    }

    // Insert at front of LRU list (most-recently-used)
    lru_list_.push_front(frame);
    cache_map_.emplace(frame, std::make_pair(std::move(data), lru_list_.begin()));

    // Enforce capacity: evict from back (least-recently-used)
    while (max_frames_ > 0 && lru_list_.size() > max_frames_) {
        const long lru = lru_list_.back();
        lru_list_.pop_back();
        cache_map_.erase(lru);
    }
}
