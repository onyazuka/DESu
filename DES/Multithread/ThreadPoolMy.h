#pragma once
#include <thread>
#include <vector>
#include <atomic>
#include <future>
#include "ThreadsafeQueue.h"

/*-----------------------------------------------------------------------------*/

/*
Functions abstaction.
We can't copy it, so can use with std::packaged_task
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
	// default default constructor)
	function_wrapper() = default;
	// allowing moving constructors
	function_wrapper(function_wrapper&& other) :
		impl{ std::move(other.impl) }
	{}
	function_wrapper& operator=(function_wrapper&& other)
	{
		impl = std::move(other.impl);
		return *this;
	}
	// forbidding copying constructors
	function_wrapper(const function_wrapper&) = delete;
	function_wrapper(function_wrapper&) = delete;
	function_wrapper& operator=(const function_wrapper&) = delete;
};

/*-----------------------------------------------------------------------------*/


class ThreadPoolMy;

/*-------------------------------------------------------------------------------------------------------*/

/*
	Worker.
	Contents thread that takes tasks from thread pool(parent) and does them.
		to start worker use run
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
	Owns workers
	To append task use try_do_task or wait_do_task:
		try_do_task only TRIES to append task, return success flag;
		wait_do_task WAITS for free worker and only after that appends task.
	If you want to terminate all tasks before exit, use
	method wait_all_tasks().
	wait_do_task and try_do_task can return values with futures.
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
	// function_wrapper is used as abstract class for returning values
	threadsafe_queue_fg<function_wrapper> working_queue;
	std::atomic<int> not_done_tasks;
	std::atomic<int> busy_workers_count;
	// mutex for blocking tasks addition in queue
	std::mutex add_mtx;
};

/*-------------------------------------------------------------------------------------------------------*/

/*
Tries to push task into working queue.
Returns success flag(true - task was pushed into queue, false - wasn't).
Result will be written into future passed by reference.
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
	// using move because function wrapper only accepts references on rvalues
	working_queue.push(std::move(f));
	++not_done_tasks;
	return true;
}

/*-------------------------------------------------------------------------------------------------------*/

/*
	Use this if you don't need return value
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
	// «десь используем move потому, что function_wrapper принимает ссылку на rvalue
	working_queue.push(std::move(f));
	++not_done_tasks;
	return true;
}

/*-------------------------------------------------------------------------------------------------------*/

/*
Waits for free worker and pushes task in working queue.
Returns future by which you can retrieve returned value later.
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