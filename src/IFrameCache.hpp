/*
 * IFrameCache.hpp
 *
 * Interface for an LRU sliding-window frame cache.
 * Decouples the main render loop from the concrete implementation.
 *
 * Thread safety: implementations must be thread-safe for prefetch() calls
 * from a background executor, but getFrame() and clear() are called from
 * the main thread only.
 */

#pragma once
#include <memory>
#include <vector>

#include <glm/glm.hpp>

/// Interface for LRU frame cache. Decouples main loop from implementation.
class IFrameCache
{
  public:
    virtual ~IFrameCache() = default;
    IFrameCache(const IFrameCache&) = delete;
    IFrameCache& operator=(const IFrameCache&) = delete;

    /// Returns cached frame data, or nullptr on cache miss.
    /// Does NOT enqueue I/O — only prefetch() does background work.
    virtual std::shared_ptr<std::vector<glm::vec4>> getFrame(long frame) = 0;

    /// Enqueues background I/O for frames [current+1, min(current+window, total_frames-1)].
    /// Frames already cached or already pending are skipped.
    virtual void prefetch(long current, long window, long total_frames) = 0;

    /// Clears all cached entries and pending-set. Call before folder reload.
    virtual void clear() = 0;

    /// Returns the size in bytes of one cached frame (N particles × sizeof glm::vec4).
    virtual std::size_t frameSizeBytes() const = 0;

  protected:
    IFrameCache() = default;
};
