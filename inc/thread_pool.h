#pragma once
#include <condition_variable>
#include <mutex>
#include <thread>
#include <functional>
#include <vector>
#include <queue>
#include <system_error>


namespace vv{

class thread_pool{
public:
    typedef typename std::vector<std::thread>::size_type size_type;
private:
    typedef std::function<void()> _task_type;

    std::condition_variable _cond;
    std::mutex _mutex;

    //线程数量
    std::vector<std::thread> _threads;
    //任务队列最大长度
    const size_type _max_wait;
    //任务队列
    std::queue<_task_type> _task;
    //启动线程标志位
    bool _start;
public:
    thread_pool(size_type thread_num = 1,size_type max_wait = 10,bool start = true)
        :_threads(thread_num),_max_wait(max_wait),_start(start)
    {
        if(_start && !_threads.empty()){
            this->start();
        }
    }
    //no-copyable
    thread_pool(const thread_pool&) = delete;
    //no-moveable
    thread_pool(thread_pool&& other) = delete;
    thread_pool& operator=(const thread_pool&) = delete;

    //添加任务
    template<class _Callable,class..._Args>
    bool add_task(_Callable&& Fn,_Args...args)
    {
        _task_type f = std::bind(std::forward<_Callable>(Fn),std::forward<_Args>(args)...);
        return add_task(std::move(f));
    }
private:
    bool add_task(_task_type& task)
    { return add_task(_task_type(task)); }
    bool add_task(_task_type&& task)
    {
        if(_task.size() == _max_wait) 
            return false;
        {
            std::unique_lock<std::mutex> _lk(_mutex);
            _task.emplace(std::move(task));
            _cond.notify_one();
        }
        return true;
    }
public:
    //启动线程池里所有线程
    bool start()
    {
        for(auto& thr:_threads){
            thr = std::thread([this](){
                _task_type task;
                for(;;){
                    {
                        std::unique_lock<std::mutex> _lk(_mutex);
                        if(_start && _task.empty()){
                                _cond.wait(_lk,[=](){
                                    return !_start || !_task.empty();
                            });
                        }
                        if(!_start) return;
                        task = std::move(this->_task.front());
                        this->_task.pop();
                    }
                    task();
                }            
            });
        }
        return _start = _threads.empty()?false:true;
    }
    //停止所有线程
    void stop()
    {
        {
            std::unique_lock<std::mutex> _lk(_mutex);
            _start = false;
            _cond.notify_all();
        }
        for(auto& _thr:_threads){
            if(_thr.joinable())
                _thr.join();
        }
    }

    ~thread_pool()
    {
        stop();
    }
};
}//end namespace vv
