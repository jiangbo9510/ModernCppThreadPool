/*
** EPITECH PROJECT, 2019
** THREAD_POOL
** File description:
** thread pool
*/

#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include<thread>
#include<queue>
#include<future>
#include<mutex>
#include<condition_variable>

//task class
class task
{
private:
    /* data */
public:
    task(/* args */);
    ~task();
};

task::task(/* args */)
{
}

task::~task()
{
}


//thread pool class
class thread_pool
{
private:
    std::mutex _mtx;
    std::condition_variable _cond;
    typedef std::function<void()> task_type;
    std::queue<task_type> _tasks;
    std::atomic<bool> _stop;
    std::vector<std::thread> _threads;
    int threadCount;
    
    /* data */
public:
    explicit thread_pool(int size);
    ~thread_pool();

    template<class Function,class ... Args>
    std::future<typename std::result_of<Function(Args ...)>::type>  addTask(Function&&,Args&& ...);

    void stop();
};

thread_pool::thread_pool(int size):_stop(false),threadCount(size)
{
    if(size <= 0)
        threadCount = 2;

    for(int i = 0; i != threadCount; i++)
    {
        _threads.push_back(std::thread([this]{

            while(this->_stop.load() == false)
            {
                task_type task;
                {
                    std::unique_lock<std::mutex> upt(this->_mtx);
                    this->_cond.wait(upt,[this]{ return this->_stop.load() ||  this->_tasks.size() != 0; });
                    if(this->_stop.load() == true)
                    {
                        return;
                    }

                    task = std::move(this->_tasks.front());
                    this->_tasks.pop();
                }

                task();
            }


        }));
    }

}

thread_pool::~thread_pool()
{
    stop();
    _cond.notify_all();

    for(auto &th : _threads)
    {
        if(th.joinable())
            th.join();
    }
}

    template<class Function,class ... Args>
std::future<typename std::result_of<Function(Args ...)>::type> 
    thread_pool::addTask(Function&& fun,Args&& ...args)
{
    typedef typename std::result_of<Function(Args...)>::type return_type;
    typedef std::packaged_task<return_type()> task;

    auto theTask = std::make_shared<task>( std::bind(std::forward<Function>(fun),std::forward<Args>(args)...) );
    auto ret = theTask->get_future();
    {
        std::lock_guard<std::mutex> lk(_mtx);

        //check the thread pool status
        if(_stop.load() == true)
        {
            throw std::runtime_error("thread pool has stopped");
        }
        _tasks.emplace([theTask]{(*theTask)();});

    }

    _cond.notify_one();
    return ret;
}

void thread_pool::stop()
{
    _stop.store(true);
}



#endif /* !THREAD_POOL_H_ */
