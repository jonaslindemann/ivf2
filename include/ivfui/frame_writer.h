#pragma once

/**
 * @file frame_writer.h
 * @brief Writes captured frames to PNG files on background threads.
 */

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace ivfui {

/**
 * @class FrameWriter
 * @brief Thread pool that encodes RGB frames to PNG files.
 *
 * PNG encoding is much slower than rendering, so frames are queued and written
 * by worker threads. The queue is bounded: write() blocks while it is full,
 * which keeps memory use limited when rendering outpaces encoding.
 */
class FrameWriter {
public:
    /**
     * @brief Constructor.
     * @param threads Number of worker threads, 0 picks one based on the CPU count.
     * @param maxQueued Maximum number of frames waiting to be written.
     */
    explicit FrameWriter(unsigned threads = 0, size_t maxQueued = 8);
    ~FrameWriter();

    FrameWriter(const FrameWriter &) = delete;
    FrameWriter &operator=(const FrameWriter &) = delete;

    /**
     * @brief Queue a frame for writing.
     * @param path Output PNG path.
     * @param width Frame width in pixels.
     * @param height Frame height in pixels.
     * @param rgb Tightly packed RGB pixels, bottom row first (as read by glReadPixels).
     */
    void write(std::string path, int width, int height, std::vector<unsigned char> rgb);

    /**
     * @brief Block until all queued frames are written.
     */
    void finish();

    /**
     * @brief Number of frames that failed to write since construction.
     */
    int failedWrites() const;

private:
    struct Job {
        std::string path;
        int width;
        int height;
        std::vector<unsigned char> rgb;
    };

    void workerLoop();

    std::vector<std::thread> m_workers;
    std::deque<Job> m_queue;
    size_t m_maxQueued;
    int m_active{0};
    int m_failed{0};
    bool m_stop{false};

    mutable std::mutex m_mutex;
    std::condition_variable m_jobAvailable;
    std::condition_variable m_spaceAvailable;
    std::condition_variable m_idle;
};

using FrameWriterPtr = std::unique_ptr<FrameWriter>;

} // namespace ivfui
