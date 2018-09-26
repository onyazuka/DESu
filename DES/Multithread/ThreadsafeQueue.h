#pragma once
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <iostream>

/*-------------------------------------------------------------------------------------------------------------*/

/*
	Simple threadsafe queue.
*/
template<typename T>
class threadsafe_queue
{
private:
	mutable std::mutex mut;
	std::queue<T> data_queue;
	std::condition_variable data_cond;			
public:
	threadsafe_queue()
	{}
	threadsafe_queue(threadsafe_queue const& other)
	{
		//somewhere other queue can be used as non-conts, so we need mutex here
		std::lock_guard<std::mutex> lk(other.mut);
		data_queue = other.data_queue;
	}
	void push(T new_value)
	{
		std::lock_guard<std::mutex> lk(mut);
		data_queue.push(std::move(new_value));
		data_cond.notify_one();
	}
	/*
		waits for value in queue, then pops
	*/
	void wait_and_pop(T& value)
	{
		// unique_lock is needed for condition_variable
		std::unique_lock<std::mutex> lk(mut);
		data_cond.wait(lk, [this] {return !data_queue.empty(); });	// if queue is free - wait
		value = std::move(data_queue.front());
		data_queue.pop();
	}
	std::shared_ptr<T> wait_and_pop()
	{
		std::unique_lock<std::mutex> lk(mut);
		data_cond.wait(lk, [this] {return !data_queue.empty(); });
		std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
		data_queue.pop();
		return res;
	}
	/*
		Sometimes can be useful only to try to push task, and
		if we can not, do something other.
	*/
	bool try_pop(T& value)
	{
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty())
			return false;
		value = data_queue.front();
		data_queue.pop();
		return true;
	}
	std::shared_ptr<T> try_pop()
	{
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty())
			return std::shared_ptr<T>();
		std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
		data_queue.pop();
		return res;
	}
	bool empty() const
	{
		std::lock_guard<std::mutex> lk(mut);
		return data_queue.empty();
	}
};

/*-------------------------------------------------------------------------------------------------------------*/

/*
	Threadsafe queue with fine grained locks.
	(fg - fine grained)
	Head and tail have its own locks.
	Queue is modulated on unidirectional list.
	In tale we have dummy-node.
		This is needed for push(so it only checks tail and not checks absence of head.
*/
template<typename T>
class threadsafe_queue_fg
{
private:

	/*
		Data is shared_ptr for preventing memory leaks.
	*/
	struct node
	{
		std::shared_ptr<T> data;
		std::unique_ptr<node> next;
	};
	std::mutex head_mutex;
	std::unique_ptr<node> head;
	std::mutex tail_mutex;
	node* tail;
	std::condition_variable data_cond;		// used in waip_and_pop

	/*
		safe tail getter
	*/
	node* get_tail()
	{
		std::lock_guard<std::mutex> tail_lock(tail_mutex);
		return tail;
	}

	/*
		safe head getter
	*/
	std::unique_ptr<node> pop_head()
	{
		std::lock_guard<std::mutex> head_lock(head_mutex);
		
		if (head.get() == get_tail())		// queue is empty
		{
			return std::unique_ptr<node>();
		}

		// head extraction with using of std::move
		std::unique_ptr<node> old_head = std::move(head);
		head = std::move(old_head->next);
		return old_head;
	}

	/*
		safe head getter with waiting
	*/
	std::unique_ptr<node> pop_head_wait()
	{
		std::unique_lock<std::mutex> lk(head_mutex);
		// here we are not using empty because than it will lock head again
		data_cond.wait(lk, [&] { return head.get() != get_tail(); });

		std::unique_ptr<node> old_head = std::move(head);
		head = std::move(old_head->next);
		return old_head;
	}

public:
	// on creation list is empty and tail and head are the same
	threadsafe_queue_fg() :
		head(new node), tail(head.get())
	{}
	// forbidding copying
	threadsafe_queue_fg(const threadsafe_queue_fg<T>& other) = delete;
	threadsafe_queue_fg<T>& operator=(const threadsafe_queue_fg<T>& other) = delete;

	std::shared_ptr<T> try_pop();
	bool try_pop(T& value);
	std::shared_ptr<T> wait_and_pop();
	void wait_and_pop(T& value);
	void push(T new_value);
	void clear();
	bool empty();
};

/*-------------------------------------------------------------------------------------------------------------*/

template<typename T>
void threadsafe_queue_fg<T>::push(T new_value)
{
	std::shared_ptr<T> new_data(
		std::make_shared<T>(std::move(new_value)));
	std::unique_ptr<node> p(new node);		// new dummy node
	{	// scoped lock_guard
		node* const new_tail = p.get();
		std::lock_guard<std::mutex> tail_lock(tail_mutex);
		tail->data = new_data;
		tail->next = std::move(p);
		tail = new_tail;
	}
	data_cond.notify_one();
}

/*-------------------------------------------------------------------------------------------------------------*/

/*
	On fail return empty shared_ptr, on success - shared_ptr with data
*/
template<typename T>
std::shared_ptr<T> threadsafe_queue_fg<T>::try_pop()
{
	std::unique_ptr<node> old_head = pop_head();
	return old_head ? old_head->data : std::shared_ptr<T>();
}

/*-------------------------------------------------------------------------------------------------------------*/

/*
	Variant that returns success flag and stores result in passed reference
*/
template<typename T>
bool threadsafe_queue_fg<T>::try_pop(T& val)
{
	std::unique_ptr<node> old_head = pop_head();
	if(old_head)
	{ 
		val = std::move(*(old_head.get()->data.get()));
	}
	return old_head ? true : false;
}

/*-------------------------------------------------------------------------------------------------------------*/

/*
	Waits and pops
*/
template<typename T>
std::shared_ptr<T> threadsafe_queue_fg<T>::wait_and_pop()
{
	return pop_head_wait().get()->data;
}

/*-------------------------------------------------------------------------------------------------------------*/

template<typename T>
void threadsafe_queue_fg<T>::wait_and_pop(T& val)
{
	val = std::move(*(pop_head_wait().get()->data.get()));
}

/*-------------------------------------------------------------------------------------------------------------*/

template<typename T>
bool threadsafe_queue_fg<T>::empty()
{
	std::lock_guard<std::mutex> head_lock(head_mutex);
	return get_tail() == head.get();
	
}
/*-------------------------------------------------------------------------------------------------------------*/

template<typename T>
void threadsafe_queue_fg<T>::clear()
{
	std::lock<std::mutex, std::mutex> (head_mutex, tail_mutex);
	std::lock_guard<std::mutex> lg1{ head_mutex, std::adopt_lock };
	std::lock_guard<std::mutex> lg2{ tail_mutex, std::adopt_lock };

	std::unique_ptr<node> deleting_head = std::move(head);
	head = std::unique_ptr<node>(new node());
}