#ifndef THREAD_POOL
#define THREAD_POOL

#include <functional>
#include <memory>
#include <atomic>

#include "sync_queue.h"

const int MaxTaskCount = 100;
class ThreadPool
{
public:
    using Task = std::function<void()>;
    ThreadPool()
        :m_queue(MaxTaskCount)
    {
    }

    ~ThreadPool()
    {
        Stop();
    }

    void Start(int numThreads = std::thread::hardware_concurrency())
    {
        m_running = true;
        for(int i = 0; i < numThreads; ++i)
        {
            m_threadGroup.push_back(std::make_shared<std::thread>(&ThreadPool::RunInThread, this));
        }
    }


    void Stop()
    {
        std::call_once(m_flag, [this]{StopThreadGroup();});
    }

    void AddTask(Task&& task)
    {
        m_queue.Put(std::forward<Task>(task));
    }

    void AddTask(const Task &task)
    {
        m_queue.Put(task);
    }

private:

    void RunInThread()
    {
        while(m_running)
        {
            std::list<Task> list;
            m_queue.Take(list);
            for(auto & task: list)
            {
                if( !m_running) return;
                task();
            }
        }
    }

    void StopThreadGroup()
    {
        m_queue.Stop();
        m_running = false;

        for(auto& thread: m_threadGroup)
        {
            if( thread )
                thread->join();
        }
        m_threadGroup.clear();
    }

private:
    std::list<std::shared_ptr<std::thread>> m_threadGroup;
    SyncQueue<Task> m_queue;
    std::atomic_bool    m_running;
    std::once_flag  m_flag;
};

#endif // THREAD_POOL

