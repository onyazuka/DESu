#pragma once
#include <thread>
#include <vector>
#include <atomic>
#include <future>
#include <functional>
#include "ThreadsafeQueue.h"

/*-----------------------------------------------------------------------------*/

/*
Îáðåòêà äëÿ ôóíêöèé.
Îòëè÷èå îò std::function<void()> - íå êîïèðóåìàÿ, ïîýòîìó ìîæåò èñïîëüçîâàòüñÿ
â std::packaged_task
*/
class function_wrapper
{
    struct impl_base
    {
        virtual void call() = 0;
        virtual ~impl_base() {};
    };
    std::unique_ptr<impl_base> impl;
    template<typename F>
    struct impl_type : impl_base
    {
        F f;
        impl_type(F&& f_) : f(std::move(f_)) {}
        void call() { f(); }
    };
public:
    template<typename F>
    function_wrapper(F&& f) :
        impl{ new impl_type<F>(std::move(f)) }
    {}
    void operator() () { impl->call(); }
    // êîíñòðóêòîð ïî óìîë÷àíèþ - ïî óìîë÷àíèþ
    function_wrapper() = default;
    // ðàçðåøàåì ïåðåìåùàþùèå êîíñòðóêòîðû
    function_wrapper(function_wrapper&& other) :
        impl{ std::move(other.impl) }
    {}
    function_wrapper& operator=(function_wrapper&& other)
    {
        impl = std::move(other.impl);
        return *this;
    }
    // çàïðåùàåì êîïèðóþùèå êîíñòðóêòîðû
    function_wrapper(const function_wrapper&) = delete;
    function_wrapper(function_wrapper&) = delete;
    function_wrapper& operator=(const function_wrapper&) = delete;
};

/*-----------------------------------------------------------------------------*/


class ThreadPoolMy;

/*-------------------------------------------------------------------------------------------------------*/

/*
    Ðîáî÷èé.
    Ñîäåðæèò ïîòîê, êîòîðûé ïðèíèìàåò çàäà÷è è ðàáî÷åé î÷åðåäè êëàññà ThreadPoolMy.
        run çàïóñêàåò çàäà÷ó,
        terminate òîëüêî óñòàíàâëèâàåò ôëàã çàïðîñà îñòàíîâêè(stop_flag), ïðè âîçìîæíîñòè ïîòîê áóäåò êîððåêòíî çàâåðøåí
        parent - thread pool
*/
class Worker
{
public:
    Worker(ThreadPoolMy* _parent);
    void run();
    ThreadPoolMy* pool() const { return parent_; }
    inline bool free() const { return free_; }
private:
    ThreadPoolMy* parent_;
    std::atomic<bool> free_;
};

/*-------------------------------------------------------------------------------------------------------*/

/*
    Òðåä ïóë, êîòîðûé ñîáñòâåííî è óïðàâëÿåò âñåìè ðàáî÷èìè ïîòîêàìè.
    Äîáàâëåíèå çàäà÷ îñóùåñòâëÿåòñÿ ïðè ïîìîùè ôóíêöèé try_do_task è wait_do_task:
        try_do_task ÏÛÒÀÅÒÑß äîáàâèòü çàäà÷ó è åñëè íåò ñâîáîäíûõ ðàáî÷èõ, òåðïèò íåóäà÷ó;
        wait_do_task ÆÄÅÒ ïîêà ïîÿâèòñÿ ñâîáîäíûé ðàáî÷èé è äîáàâëÿåò çàäà÷ó.
    Äëÿ òîãî, ÷òîáû äî âûõîäà èç ïðîãðàììû çàâåðøèëèñü âñå çàäà÷è, ÍÅÎÁÕÎÄÈÌÎ âûçâàòü
    ìåòîä wait_all_tasks().
    ÂÍÈÌÀÍÈÅ: íåò âîçìîæíîñòè âîçâðàòà çíà÷åíèé èç ôóíêöèé, ïåðåäàííûõ â ïóë.
        Èçâðàùàéòåñü ñî ññûëêàìè èëè óêàçàòåëÿìè.
*/
class ThreadPoolMy
{
public:
    typedef int size_type;
    typedef std::vector<std::shared_ptr<Worker>> pWorkers;
    typedef std::vector<std::thread> Threads;
    ThreadPoolMy(size_type n = std::thread::hardware_concurrency());
    ~ThreadPoolMy();
    inline size_type size() const { return _size; }
    inline int free_workers() const { return _size - busy_workers_count; }
    bool has_tasks() { return !working_queue.empty(); }
    inline bool is_terminated() const { return terminated_; }
    template<typename F>
    bool try_do_task(F f, std::future<typename std::result_of<F()>::type>& fut);
    template<typename F>
    bool try_do_task(F f);
    template<typename F>
    std::future<typename std::result_of<F()>::type> wait_do_task(F f);
    void wait_all_tasks();
    inline int tasks_left() const { return not_done_tasks; }
private:
    friend class Worker;
    void join_threads();
    void terminate_all();
    Threads threads;
    pWorkers workers;
    std::atomic<size_type> _size;
    bool terminated_;
    // function_wrapper èñïîëüçóåòñÿ êàê ìåòàêëàññ äëÿ îáåñïå÷åíèÿ âîçìîæíîñòè
    // âîçâðàòà çíà÷åíèé
    threadsafe_queue_fg<function_wrapper> working_queue;
    std::atomic<int> not_done_tasks;
    std::atomic<int> busy_workers_count;
    // áëîêèðîâêà äîáàâëåíèÿ çàäà÷ â î÷åðåäü
    std::mutex add_mtx;

};

/*-------------------------------------------------------------------------------------------------------*/

/*
Ïûòàåòñÿ âñóíóòü çàäà÷ó â ïóë.
Âîçâðàùàåò true, åñëè êàêîé-íèáóäü ïîòîê âçÿëñÿ çà åå âûïîëíåíèå,
èíà÷å false - íåò ìåñòà.
Ðåçóëüòàò âûçîâà ôóíêöèè áóäåò çàïèñàí â ïåðåäàííûé ïî ññûëêå future.
*/
template<typename F>
bool ThreadPoolMy::try_do_task(F f, std::future<typename std::result_of<F()>::type>& fut)
{
    if (busy_workers_count.load() == _size)
    {
        return false;
    }
    typedef typename std::result_of<F()>::type result_type;
    std::packaged_task<result_type()> task(std::move(f));
    fut = task.get_future();
    std::lock_guard<std::mutex> lck{ add_mtx };
    // Çäåñü èñïîëüçóåì move ïîòîìó, ÷òî function_wrapper ïðèíèìàåò ññûëêó íà rvalue
    working_queue.push(std::move(f));
    ++not_done_tasks;
    return true;
}

/*-------------------------------------------------------------------------------------------------------*/

/*
    Âàðèàíò ôóíêöèè ñ îäíèì àðãóìåíòîì, åñëè âîçâðàùàåìîå çíà÷åíèå íå íóæíî.
*/
template<typename F>
bool ThreadPoolMy::try_do_task(F f)
{
    if (busy_workers_count.load() == _size)
    {
        return false;
    }
    typedef typename std::result_of<F()>::type result_type;
    std::packaged_task<result_type()> task(std::move(f));
    std::lock_guard<std::mutex> lck{ add_mtx };
    // Çäåñü èñïîëüçóåì move ïîòîìó, ÷òî function_wrapper ïðèíèìàåò ññûëêó íà rvalue
    working_queue.push(std::move(f));
    ++not_done_tasks;
    return true;
}

/*-------------------------------------------------------------------------------------------------------*/

/*
Ïî-ëþáîìó çàïèõíåò çàäà÷ó â ïóë. Áóäåò æäàòü, ñêîëüêî ïîòðåáóåòñÿ.
Âîçâðàùàåò future, èç êîòîðîãî ìû ïîòîì ñìîæåì ïîëó÷èòü ðåçóëüòàò
*/
template<typename F>
std::future<typename std::result_of<F()>::type> ThreadPoolMy::wait_do_task(F f)
{
    typedef typename std::result_of<F()>::type result_type;
    std::packaged_task<result_type()> task(std::move(f));
    std::future<result_type> res(task.get_future());
    std::lock_guard<std::mutex> lck{ add_mtx };
    working_queue.push(std::move(task));
    ++not_done_tasks;
    return res;
}

/*-------------------------------------------------------------------------------------------------------*/
