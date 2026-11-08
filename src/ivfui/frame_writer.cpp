#include <ivfui/frame_writer.h>

#include <ivf/logger.h>

#include <stb_image_write.h>

#include <algorithm>

using namespace ivfui;

FrameWriter::FrameWriter(unsigned threads, size_t maxQueued) : m_maxQueued(std::max<size_t>(1, maxQueued))
{
    if (threads == 0)
    {
        // Leave a core for rendering
        unsigned hw = std::thread::hardware_concurrency();
        threads = std::clamp(hw > 1 ? hw - 1 : 1u, 1u, 6u);
    }

    for (unsigned i = 0; i < threads; ++i)
        m_workers.emplace_back(&FrameWriter::workerLoop, this);
}

FrameWriter::~FrameWriter()
{
    finish();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stop = true;
    }
    m_jobAvailable.notify_all();
    for (auto &worker : m_workers)
        worker.join();
}

void FrameWriter::write(std::string path, int width, int height, std::vector<unsigned char> rgb)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_spaceAvailable.wait(lock, [this] { return m_queue.size() < m_maxQueued; });
    m_queue.push_back(Job{std::move(path), width, height, std::move(rgb)});
    lock.unlock();
    m_jobAvailable.notify_one();
}

void FrameWriter::finish()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_idle.wait(lock, [this] { return m_queue.empty() && m_active == 0; });
}

int FrameWriter::failedWrites() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_failed;
}

void FrameWriter::workerLoop()
{
    for (;;)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_jobAvailable.wait(lock, [this] { return m_stop || !m_queue.empty(); });
        if (m_stop && m_queue.empty())
            return;

        Job job = std::move(m_queue.front());
        m_queue.pop_front();
        ++m_active;
        lock.unlock();
        m_spaceAvailable.notify_one();

        // glReadPixels delivers the bottom row first; a negative stride flips it
        const int stride = job.width * 3;
        bool ok = stbi_write_png(job.path.c_str(), job.width, job.height, 3,
                                 job.rgb.data() + (job.height - 1) * stride, -stride) != 0;
        if (!ok)
            ivf::logError("Could not write frame: " + job.path, "FrameWriter");

        lock.lock();
        --m_active;
        if (!ok)
            ++m_failed;
        if (m_queue.empty() && m_active == 0)
            m_idle.notify_all();
    }
}
