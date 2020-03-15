/*!
 *  @brief     c++线程池
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.15
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef THREADPOOL_H
#define THREADPOOL_H


#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <queue>
#include <vector>
#include <future>
#include <stdexcept>

class ThreadPool
{
public:
    ThreadPool(int threadNum = 5);
    ~ThreadPool();

    template<class F,class...Args>
    auto enqueue(F&& f ,Args&&... args)->std::future<typename std::result_of<F(Args...)>::type>;

private:
    std::mutex m_taskMutex;
    std::vector<std::thread> m_threads;
    std::queue<std::function<void()>> m_taskQueue;
    std::condition_variable condition;

    bool m_bStopped;        /*!< 是否结束 */
};

template<class F,class...Args>
auto ThreadPool::enqueue(F && f,Args&&... args)->std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(std::bind(std::forward<F>(f),std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::lock_guard<std::mutex> lg(m_taskMutex);
        if(m_bStopped)
            throw std::runtime_error("enqueue on stopped threadpool!");

        m_taskQueue.emplace([task](){ (*task)(); });
    }

    condition.notify_one();

    return res;
}

#endif // THREADPOOL_H
