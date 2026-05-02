/*
 * COMCache.hpp
 *
 * Thread-safe per-frame center-of-mass cache with async background prefetch.
 *
 * computeWeightedCOM() computes a mass-weighted COM from particle positions using
 * body mass fractions from RunSetup. Particle type IDs come from the .w component:
 *   0 = Body1 Fe (core1, heaviest)
 *   1 = Body1 Si (sil1)
 *   2 = Body2 Fe (core2)
 *   3 = Body2 Si (sil2)
 * All other .w values (e.g. 500 for the default cube) fall back to equal-weight mean.
 *
 * COMCache owns a background worker thread that reads the position file and fills
 * the cache ahead of the current playback frame. The background thread never calls GL.
 */

#ifndef PARTICLE_VIEWER_COMCACHE_HPP
#define PARTICLE_VIEWER_COMCACHE_HPP

#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <numeric>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "glm/glm.hpp"

/*
 * Mass model derived from RunSetup for per-body, per-material mass weighting.
 * All fields are in SI units (kg).
 */
struct MassModel
{
    double mass_body1{0.0}; // FractionEarthMassOfBody1 * MassOfEarth
    double mass_body2{0.0}; // FractionEarthMassOfBody2 * MassOfEarth
    double frac_fe1{0.0};   // FractionFeBody1
    double frac_si1{0.0};   // FractionSiBody1
    double frac_fe2{0.0};   // FractionFeBody2
    double frac_si2{0.0};   // FractionSiBody2
};

/*
 * Compute mass-weighted center-of-mass from particle positions.
 *
 * Particle type IDs are read from positions[i].w (cast to int):
 *   0 = Body1 Fe, 1 = Body1 Si, 2 = Body2 Fe, 3 = Body2 Si.
 *
 * Per-particle mass is (total body material mass) / (number of particles of that type).
 * Falls back to equal-weight arithmetic mean when no particles have a recognized type.
 *
 * Returns glm::vec3(0) when positions is empty.
 */
inline glm::vec3 computeWeightedCOM(const std::vector<glm::vec4>& positions, const MassModel& model)
{
    if (positions.empty()) {
        return glm::vec3(0.0f);
    }

    // Count particles per recognized type (0-3)
    long n_type[4] = {0, 0, 0, 0};
    for (const auto& p : positions) {
        int t = static_cast<int>(p.w);
        if (t >= 0 && t <= 3) {
            n_type[t]++;
        }
    }

    // Check if any recognized type IDs are present
    long n_recognized = std::accumulate(std::begin(n_type), std::end(n_type), 0L);
    if (n_recognized == 0) {
        // Equal-weight fallback
        glm::dvec3 sum(0.0);
        for (const auto& p : positions) {
            sum.x += static_cast<double>(p.x);
            sum.y += static_cast<double>(p.y);
            sum.z += static_cast<double>(p.z);
        }
        double inv_n = 1.0 / static_cast<double>(positions.size());
        return glm::vec3(static_cast<float>(sum.x * inv_n), static_cast<float>(sum.y * inv_n),
                         static_cast<float>(sum.z * inv_n));
    }

    // Compute per-particle mass for each type
    // type 0 = Body1 Fe:  (mass_body1 * frac_fe1) / n_type[0]
    // type 1 = Body1 Si:  (mass_body1 * frac_si1) / n_type[1]
    // type 2 = Body2 Fe:  (mass_body2 * frac_fe2) / n_type[2]
    // type 3 = Body2 Si:  (mass_body2 * frac_si2) / n_type[3]
    double mass_per_type[4] = {0.0, 0.0, 0.0, 0.0};
    if (n_type[0] > 0) {
        mass_per_type[0] = (model.mass_body1 * model.frac_fe1) / static_cast<double>(n_type[0]);
    }
    if (n_type[1] > 0) {
        mass_per_type[1] = (model.mass_body1 * model.frac_si1) / static_cast<double>(n_type[1]);
    }
    if (n_type[2] > 0) {
        mass_per_type[2] = (model.mass_body2 * model.frac_fe2) / static_cast<double>(n_type[2]);
    }
    if (n_type[3] > 0) {
        mass_per_type[3] = (model.mass_body2 * model.frac_si2) / static_cast<double>(n_type[3]);
    }

    // Compute weighted sum
    glm::dvec3 weighted_sum(0.0);
    double total_mass = 0.0;
    for (const auto& p : positions) {
        int t = static_cast<int>(p.w);
        if (t >= 0 && t <= 3) {
            double m = mass_per_type[t];
            weighted_sum.x += m * static_cast<double>(p.x);
            weighted_sum.y += m * static_cast<double>(p.y);
            weighted_sum.z += m * static_cast<double>(p.z);
            total_mass += m;
        }
        // Unrecognized particles are excluded from the weighted COM
    }

    if (total_mass <= 0.0) {
        return glm::vec3(0.0f);
    }

    return glm::vec3(static_cast<float>(weighted_sum.x / total_mass), static_cast<float>(weighted_sum.y / total_mass),
                     static_cast<float>(weighted_sum.z / total_mass));
}

/*
 * Thread-safe per-frame COM cache with async background prefetch.
 *
 * Call start() after loading a new simulation file to configure the worker.
 * The worker reads particle positions from the binary position file and calls
 * computeWeightedCOM() for frames in [current_frame, current_frame + lookahead].
 * The background thread never calls GL.
 *
 * Call setCurrentFrame() each render loop iteration to update the prefetch target.
 * Call get() to retrieve a cached COM. Returns nullopt on cache miss.
 * Call stop() (or let the destructor run) to shut down the worker.
 */
class COMCache
{
  public:
    static constexpr long DEFAULT_LOOKAHEAD = 50;
    // Idle polling interval: how long the worker waits when all frames in the
    // current window are already cached before re-checking for new work.
    static constexpr auto WORKER_POLL_INTERVAL = std::chrono::milliseconds(50);

    COMCache() = default;
    ~COMCache()
    {
        stop();
    }

    COMCache(const COMCache&) = delete;
    COMCache& operator=(const COMCache&) = delete;

    /*
     * Configure and start the background prefetch worker.
     * If a worker is already running, it is stopped (joined) before the new one starts.
     *
     * pos_path:     path to the binary position file (opened in "rb" mode)
     * N:            number of particles per frame
     * total_frames: total number of frames in the file
     * model:        mass model for COM weighting
     * lookahead:    frames to prefetch ahead of the current frame
     */
    void start(std::string pos_path, long N, long total_frames, const MassModel& model,
               long lookahead = DEFAULT_LOOKAHEAD)
    {
        stop(); // join any existing worker before reconfiguring

        {
            std::lock_guard<std::mutex> lock(mutex_);
            cache_.clear();
            pos_path_ = std::move(pos_path);
            N_ = N;
            total_frames_ = total_frames;
            model_ = model;
            lookahead_ = lookahead;
        }

        stop_.store(false);
        worker_ = std::thread(&COMCache::workerLoop, this);
    }

    /*
     * Stop and join the background worker. Safe to call multiple times.
     */
    void stop()
    {
        stop_.store(true);
        cv_.notify_all();
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    /*
     * Look up a cached COM value for the given frame.
     * Returns nullopt if the frame has not been computed yet.
     */
    std::optional<glm::vec3> get(long frame) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = cache_.find(frame);
        if (it != cache_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    /*
     * Notify the worker that playback is at `frame`.
     * The worker will prefetch COM values for [frame, frame + lookahead].
     */
    void setCurrentFrame(long frame)
    {
        current_frame_.store(frame);
        cv_.notify_one();
    }

    /*
     * Manually insert a COM for a frame (e.g., read from COMFile).
     */
    void insert(long frame, glm::vec3 com)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_[frame] = com;
    }

    /*
     * Statistics for UI display.
     */
    struct Stats
    {
        long cached_frames{0};
        long current_target{0};
        bool running{false};
    };

    Stats getStats() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        Stats s;
        s.cached_frames = static_cast<long>(cache_.size());
        s.current_target = current_frame_.load();
        s.running = worker_.joinable() && !stop_.load();
        return s;
    }

  private:
    /*
     * Read the positions for a single frame from the binary position file.
     * Returns an empty vector on I/O error or if N_ == 0.
     * Never calls GL.
     */
    std::vector<glm::vec4> readFrame(long frame) const
    {
        if (N_ <= 0 || pos_path_.empty()) {
            return {};
        }
        FILE* f = fopen(pos_path_.c_str(), "rb");
        if (!f) {
            return {};
        }
        // Each frame stores N_ positions (vec4) followed by N_ velocities (vec4).
        // VECS_PER_PARTICLE = 2 accounts for both records per particle.
        static constexpr long VECS_PER_PARTICLE = 2;
        long offset = frame * static_cast<long>(sizeof(glm::vec4)) * VECS_PER_PARTICLE * N_;
        if (fseek(f, offset, SEEK_SET) != 0) {
            fclose(f);
            return {};
        }
        std::vector<glm::vec4> positions(static_cast<size_t>(N_));
        size_t items_read = fread(positions.data(), sizeof(glm::vec4), static_cast<size_t>(N_), f);
        fclose(f);
        if (static_cast<long>(items_read) != N_) {
            return {};
        }
        return positions;
    }

    void workerLoop()
    {
        while (!stop_.load()) {
            long current = current_frame_.load();
            long end = std::min(current + lookahead_, total_frames_ - 1);

            // Find the next frame that needs computation
            long next_frame = -1;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                for (long f = current; f <= end; ++f) {
                    if (cache_.find(f) == cache_.end()) {
                        next_frame = f;
                        break;
                    }
                }
            }

            if (next_frame >= 0) {
                std::vector<glm::vec4> positions = readFrame(next_frame);
                if (!positions.empty()) {
                    glm::vec3 com = computeWeightedCOM(positions, model_);
                    std::lock_guard<std::mutex> lock(mutex_);
                    cache_[next_frame] = com;
                }
                // Don't sleep - immediately check for the next uncached frame
                continue;
            }

            // All frames in window are cached; wait for current frame to advance
            std::unique_lock<std::mutex> lock(cv_mutex_);
            cv_.wait_for(lock, WORKER_POLL_INTERVAL, [this] { return stop_.load(); });
        }
    }

    std::string pos_path_;
    long N_{0};
    long total_frames_{0};
    MassModel model_;
    long lookahead_{DEFAULT_LOOKAHEAD};

    mutable std::mutex mutex_;
    std::unordered_map<long, glm::vec3> cache_;

    std::atomic<long> current_frame_{0};
    std::atomic<bool> stop_{true};
    std::thread worker_;

    std::mutex cv_mutex_;
    std::condition_variable cv_;
};

#endif // PARTICLE_VIEWER_COMCACHE_HPP
