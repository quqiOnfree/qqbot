#ifndef QUQICORO_HPP
#define QUQICORO_HPP

#define QUQICORO_NAMESPACE_START namespace qcoro {
#define QUQICORO_NAMESPACE_END }

#include <coroutine>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <chrono>
#include <functional>
#include <vector>
#include <condition_variable>
#include <stdexcept>

QUQICORO_NAMESPACE_START

// base executor
class executor
{
public:
    executor() = default;
    executor(const executor&) = delete;
    executor(executor&&) = delete;
    virtual ~executor() = default;

    executor& operator=(const executor&) = delete;
    executor& operator=(executor&&) = delete;

    // post a task
    template<typename Func, typename... Args>
    void post(Func&& func, Args&&... args)
    {
        _post(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    }

protected:
    virtual void _post(std::function<void()> func)
    {
        func();
    }
};

// thread pool inherit from executor
class thread_pool_executor : public executor
{
public:
    explicit thread_pool_executor(int num = 12) :
        can_join_(true),
        is_running_(true),
        working_thread_num_(0)
    {
        // work function of single thread
        auto work_func = [this]()
            {
                while (true)
                {
                    // get the lock to get a task
                    std::unique_lock<std::mutex> lock(mutex_);
                    cv_.wait(lock,
                        [&]() {
                            return !funcs_.empty() || !(is_running_ || working_thread_num_);
                        }
                    );

                    // keep running until is_running_ is false and funcs_ is empty
                    if (!is_running_ && funcs_.empty() && !working_thread_num_)
                        return;
                    else if (funcs_.empty())
                        continue;

                    working_thread_num_++;
                    auto task = std::move(funcs_.front());
                    funcs_.pop();

                    lock.unlock();

                    // execute the task
                    task();
                    if (!--working_thread_num_)
                        cv_.notify_all();
                }
            };

        // add threads
        for (int i = 0; i < num; i++)
        {
            threads_.emplace_back(work_func);
        }
    }
    virtual ~thread_pool_executor() = default;

    // if threads can be joined
    bool joinable() const
    {
        return can_join_;
    }

    // join all threads
    void join()
    {
        if (!can_join_)
            throw std::runtime_error("can't join");
        can_join_ = false;
        is_running_ = false;
        cv_.notify_all();
        for (auto i = threads_.begin(); i != threads_.end(); i++)
        {
            if (i->joinable())
                i->join();
        }
    }

    // post a task
    template<typename Func, typename... Args>
    void post(Func&& func, Args&&... args)
    {
        _post(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    }

protected:
    virtual void _post(std::function<void()> func)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        funcs_.push(func);
        cv_.notify_one();
    }

private:
    std::atomic<bool>                   is_running_;
    std::atomic<int>                    working_thread_num_;
    std::queue<std::function<void()>>   funcs_;
    std::vector<std::thread>            threads_;
    std::condition_variable             cv_;
    std::mutex                          mutex_;
    std::atomic<bool>                   can_join_;
};

class thread_executor : public executor
{
public:
    thread_executor() = default;
    ~thread_executor() = default;

    // post a task
    template<typename Func, typename... Args>
    void post(Func&& func, Args&&... args)
    {
        _post(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    }

protected:
    virtual void _post(std::function<void()> func)
    {
        std::thread([func]() {
            func();
            }).detach();
    }
};

// 非 void 协程
template<class T, class RT = T>
class generator final
{
public:
    class promise_type;

    class iterator
    {
    public:
        T& operator*()
        {
            return h.promise().result_;
        }

        const T& operator*() const
        {
            return h.promise().result_;
        }

        iterator operator++()
        {
            if (!h.done())
                h.resume();
            return *this;
        }

        bool operator==(const iterator&) const
        {
            return h.done();
        }

        bool operator!=(const iterator&) const
        {
            return !h.done();
        }

        std::coroutine_handle<promise_type>& h;
    };

    class promise_type
    {
    public:
        T result_;
        RT revalue_;

        generator get_return_object() {
            return { std::coroutine_handle<promise_type>::from_promise(*this) };
        }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(T value)
        {
            result_ = value;
            return {};
        }
        void unhandled_exception() { std::terminate(); }
        void return_value(RT value)
        {
            revalue_ = value;
        }
    };

    T& operator ()()
    {
        if (!handle_.done())
        {
            handle_.resume();
            return handle_.promise().result_;
        }
        else
        {
            return handle_.promise().revalue_;
        }
    }

    const T& operator ()() const
    {
        if (!handle_.done())
        {
            handle_.resume();
            return handle_.promise().result_;
        }
        else
        {
            return handle_.promise().revalue_;
        }
    }

    iterator begin() { return { handle_ }; }
    iterator end() { return { handle_ }; }

    std::coroutine_handle<promise_type> handle_;
};

// void协程
template<class T>
class generator<T, void> final
{
public:
    class promise_type;

    class iterator
    {
    public:
        T& operator*()
        {
            return handle_.promise().result_;
        }

        const T& operator*() const
        {
            return handle_.promise().result_;
        }

        iterator operator++()
        {
            if (!handle_.done())
                handle_.resume();
            return *this;
        }

        bool operator==(const iterator&) const
        {
            return handle_.done();
        }

        bool operator!=(const iterator&) const
        {
            return !handle_.done();
        }

        std::coroutine_handle<promise_type>& handle_;
    };

    class promise_type
    {
    public:
        T result_;

        generator get_return_object()
        {
            return { std::coroutine_handle<promise_type>::from_promise(*this) };
        }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(T value)
        {
            result_ = value;
            return {};
        }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
    };

    iterator begin() { return { handle_ }; }
    iterator end() { return { handle_ }; }

    T& operator ()()
    {
        if (!handle_.done())
        {
            handle_.resume();
            return handle_.promise().result_;
        }
        else
        {
            throw std::runtime_error("end");
        }
    }

    const T& operator ()() const
    {
        if (!handle_.done())
        {
            handle_.resume();
            return handle_.promise().result_;
        }
        else
        {
            throw std::runtime_error("end");
        }
    }

    std::coroutine_handle<promise_type> handle_;
};

// 双 void 协程
template<>
class generator<void, void> final
{
public:
    class promise_type;

    class iterator
    {
    public:
        iterator operator++()
        {
            if (!handle_.done())
                handle_.resume();
            return *this;
        }

        bool operator==(const iterator&) const
        {
            return handle_.done();
        }

        bool operator!=(const iterator&) const
        {
            return !handle_.done();
        }

        std::coroutine_handle<promise_type>& handle_;
    };

    class promise_type
    {
    public:
        generator get_return_object()
        {
            return { std::coroutine_handle<promise_type>::from_promise(*this) };
        }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { /*std::terminate();*/ }
        void return_void() {}
    };

    iterator begin() { return { handle_ }; }
    iterator end() { return { handle_ }; }

    void operator ()()
    {
        if (!handle_.done())
        {
            handle_.resume();
        }
        else
        {
            throw std::runtime_error("end");
        }
    }

    std::coroutine_handle<promise_type> handle_;
};

template<class T>
class awaiter final
{
public:
    class promise_type;
    using function_type = std::function<void(std::function<void(T)>, executor&)>;

    awaiter(std::coroutine_handle<promise_type> h)
        : executor_(local_executor_)
    {
        handle_ = h;
    }
    awaiter(function_type func, executor& e)
        : executor_(e)
    {
        func_ = func;
    }
    ~awaiter() = default;

    class promise_type
    {
    public:
        T result_;

        awaiter get_return_object()
        {
            return { std::coroutine_handle<promise_type>::from_promise(*this) };
        }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { std::terminate(); }
        void return_value(T value)
        {
            result_ = value;
        }
    };

    bool    await_ready() const { return ready_; }
    T       await_resume() { return result_; }
    void    await_suspend(std::coroutine_handle<> handle)
    {
        // callback function
        auto f = [handle, this](T value) {
            result_ = std::move(value);
            ready_ = true;
            handle.resume();
            };
        func_(f, executor_);
    }
    void unhandled_exception() { std::terminate(); }

    std::coroutine_handle<promise_type> handle_;
    T               result_;
    function_type   func_;
    executor        &executor_;
    executor        local_executor_;
    bool            ready_ = false;
};

template<>
class awaiter<void> final
{
public:
    class promise_type;
    using function_type = std::function<void(std::function<void()>, executor&)>;

    awaiter(std::coroutine_handle<promise_type> h)
        : executor_(local_executor_)
    {
        handle_ = h;
    }
    awaiter(function_type func, executor& e)
        : executor_(e)
    {
        func_ = func;
    }
    ~awaiter() = default;

    class promise_type
    {
    public:
        awaiter get_return_object()
        {
            return { std::coroutine_handle<promise_type>::from_promise(*this) };
        }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}
    };

    bool    await_ready() const { return ready_; }
    void    await_resume() {}
    void    await_suspend(std::coroutine_handle<> handle)
    {
        // callback function
        auto f = [handle, this]() {
            ready_ = true;
            handle.resume();
            };
        func_(f, executor_);
    }
    void unhandled_exception() { std::terminate(); }

    std::coroutine_handle<promise_type> handle_;
    function_type   func_;
    executor&       executor_;
    executor        local_executor_;
    bool            ready_ = false;
};

// timer
class timer
{
public:
    timer() = default;
    ~timer() = default;

    template<class _Rep, class _Period, typename Func, typename... Args>
    void async_wait(const std::chrono::duration<_Rep, _Period>& d, executor& e, Func&& func, Args&&... args)
    {
        e.post([d, &e](Func&& func, Args&&... args) {
            std::this_thread::sleep_for(d);
            func(args...);
            }, std::forward<Func>(func), std::forward<Args>(args)...);
    }

    template<class _Rep, class _Period>
    awaiter<void> async_wait(const std::chrono::duration<_Rep, _Period>& d, executor& e)
    {
        return { [d](std::function<void()> func, executor& e) {
            e.post([func, d]() {
                std::this_thread::sleep_for(d);
                func();
                });
            }, e };
    }
};

// spawn a coroutine of awaiter
void co_spawn(std::function<qcoro::awaiter<void>()> func, qcoro::executor& e)
{
    e.post([func, &e]() {
        auto f = [func, &e]() -> generator<void> {
            co_await func();
            co_return;
            };
        f();
        });
}

executor        use_await_t;
thread_executor detach_t;

QUQICORO_NAMESPACE_END

#endif // !QUQICORO_HPP
