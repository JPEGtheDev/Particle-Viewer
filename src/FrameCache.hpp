/*
 * FrameCache.hpp
 *
 * Thread-safe LRU (least-recently-used) frame cache for particle positions.
 *
 * Stores raw particle position buffers (std::vector<glm::vec4>) keyed by frame
 * index. Bounds memory usage by evicting the least-recently-used frame when the
 * cache exceeds its capacity. All operations are thread-safe.
 *
 * Note: This cache reduces disk I/O for position data. The GL buffer upload
 * (glBufferData via Particle::changeTranslations) still occurs on every frame
 * change, as it requires the GL context on the main thread.
 */

#ifndef PARTICLE_VIEWER_FRAMECACHE_HPP
#define PARTICLE_VIEWER_FRAMECACHE_HPP

#include <list>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <vector>

#include "glm/glm.hpp"

/*
 * Thread-safe LRU cache mapping frame index -> particle position buffers.
 *
 * Capacity is specified in number of frames. When capacity is exceeded, the
 * least-recently-used frame is evicted.
 */
class FrameCache
{
  public:
    static constexpr long DEFAULT_MAX_FRAMES = 32;

    explicit FrameCache(long max_frames = DEFAULT_MAX_FRAMES) : max_frames_(max_frames > 0 ? max_frames : 1)
    {
    }

    ~FrameCache() = default;

    FrameCache(const FrameCache&) = delete;
    FrameCache& operator=(const FrameCache&) = delete;

    /*
     * Look up positions for a frame. Returns nullopt on cache miss.
     * A hit promotes the frame to the front of the LRU order.
     */
    std::optional<std::vector<glm::vec4>> get(long frame)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = data_.find(frame);
        if (it == data_.end()) {
            misses_++;
            return std::nullopt;
        }
        hits_++;
        // Promote to front of LRU list
        lru_order_.erase(it->second.lru_it);
        lru_order_.push_front(frame);
        it->second.lru_it = lru_order_.begin();
        return it->second.positions;
    }

    /*
     * Store positions for a frame. Evicts the LRU entry if at capacity.
     * If the frame is already cached, its positions are updated and it is promoted.
     */
    void put(long frame, std::vector<glm::vec4> positions)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = data_.find(frame);
        if (it != data_.end()) {
            // Update existing entry and promote
            lru_order_.erase(it->second.lru_it);
            lru_order_.push_front(frame);
            it->second.lru_it = lru_order_.begin();
            it->second.positions = std::move(positions);
            return;
        }

        // Evict LRU if at capacity
        if (static_cast<long>(data_.size()) >= max_frames_) {
            evict();
        }

        lru_order_.push_front(frame);
        data_.emplace(frame, Entry{std::move(positions), lru_order_.begin()});
    }

    /*
     * Remove all cached entries and reset statistics.
     */
    void clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        data_.clear();
        lru_order_.clear();
        hits_ = 0;
        misses_ = 0;
    }

    /*
     * Statistics for UI display.
     */
    struct Stats
    {
        long cached_frames{0};
        long max_frames{0};
        long hits{0};
        long misses{0};
    };

    Stats getStats() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        Stats s;
        s.cached_frames = static_cast<long>(data_.size());
        s.max_frames = max_frames_;
        s.hits = hits_;
        s.misses = misses_;
        return s;
    }

  private:
    void evict()
    {
        // Called under lock
        if (lru_order_.empty()) {
            return;
        }
        long lru_frame = lru_order_.back();
        lru_order_.pop_back();
        data_.erase(lru_frame);
    }

    struct Entry
    {
        std::vector<glm::vec4> positions;
        std::list<long>::iterator lru_it;
    };

    long max_frames_;
    mutable std::mutex mutex_;
    std::unordered_map<long, Entry> data_;
    std::list<long> lru_order_; // Front = most recently used, back = LRU
    long hits_{0};
    long misses_{0};
};

#endif // PARTICLE_VIEWER_FRAMECACHE_HPP
