/*
 * FrameCache.hpp
 *
 * LRU sliding-window prefetch cache for particle frame data.
 *
 * Memory is bounded by max_frame_bytes: at most floor(max_frame_bytes /
 * frame_size_bytes) frames are kept in memory at once.  When the cache is
 * full the least-recently-used entry is evicted before the new one is
 * inserted.
 *
 * Background I/O is injected through IExecutor so tests can substitute a
 * synchronous implementation.  No OpenGL calls are made here; the caller is
 * responsible for calling particle.stageTranslations() + particle.pushVBO()
 * after receiving a cache hit.
 */

#pragma once
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <glm/glm.hpp>

// glad must be included before settingsIO.hpp, which transitively includes
// particle.hpp (GL types).  FrameCache itself makes no GL calls.
#include <glad/glad.h> // NOLINT(llvm-include-order)

#include "IExecutor.hpp"
#include "IFrameCache.hpp"
#include "settingsIO.hpp"

/// LRU frame cache backed by an injected IExecutor for background I/O.
class FrameCache : public IFrameCache
{
  public:
    /// @param sio            Source of particle position data (readPosBuffer).
    /// @param max_frame_bytes Maximum total bytes allowed for cached frames.
    /// @param executor        Executes background prefetch tasks.
    FrameCache(SettingsIO& sio, std::size_t max_frame_bytes, IExecutor& executor);

    ~FrameCache() override = default;
    FrameCache(const FrameCache&) = delete;
    FrameCache& operator=(const FrameCache&) = delete;
    FrameCache(FrameCache&&) = delete;
    FrameCache& operator=(FrameCache&&) = delete;

    /// Returns cached frame, or nullptr on miss. Moves hit entry to MRU.
    std::shared_ptr<std::vector<glm::vec4>> getFrame(long frame) override;

    /// Enqueues I/O for frames [current+1, min(current+window, total_frames-1)].
    void prefetch(long current, long window, long total_frames) override;

    /// Clears cache, LRU list, and pending set.
    void clear() override;

    /// Returns the number of frames currently held in the cache.
    /// Thread-safe.
    std::size_t cachedCount() const;

    /// Returns the per-frame memory footprint (N × sizeof(glm::vec4)).
    std::size_t frameSizeBytes() const override { return frame_size_bytes_; }

  private:
    /// Inserts a frame into the cache and enforces LRU capacity.
    /// Must be called with mutex_ held.
    void insertLocked(long frame, std::shared_ptr<std::vector<glm::vec4>> data);

    SettingsIO& sio_;
    std::size_t max_frames_; ///< floor(max_frame_bytes / frame_size_bytes), 0 if uncalculable
    std::size_t frame_size_bytes_; ///< bytes per frame; 0 if N is 0
    IExecutor& executor_;

    /// Value: {data, iterator into lru_list_}
    std::unordered_map<long, std::pair<std::shared_ptr<std::vector<glm::vec4>>, std::list<long>::iterator>> cache_map_;
    std::list<long> lru_list_;
    std::unordered_set<long> pending_;
    mutable std::mutex mutex_;
};
