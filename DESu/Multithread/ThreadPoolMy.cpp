// ThreadPoolMy.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "ThreadPoolMy.h"


/*-------------------------------------------WORKER------------------------------------------------------*/

// каждый поток рождается свободным(и не остановленным)...
Worker::Worker(ThreadPoolMy* _parent)
    : parent_{ _parent }, free_{ true }
{
}

/*-------------------------------------------------------------------------------------------------------*/

/*
    Когда запущен, ожидает задачи из очереди и выполняет их.
    Ожидаемые события: пополнение очереди задач.
    Для синхронизации выхода используется костыль в виде добавления фейковых задач при
        вызове деструктора.
*/
void Worker::run()
{
    while (true)
    {
        function_wrapper task;
        parent_->working_queue.wait_and_pop(task);
        if (pool()->is_terminated())
        {
            parent_->_size--;
            return;
        }

        free_.store(false);
        parent_->busy_workers_count.fetch_add(1);
        // если будут исключения, мы их перехватим future'ами
        task();
        --parent_->not_done_tasks;
        parent_->busy_workers_count.fetch_sub(1);
        free_.store(true);
    }
}

/*-----------------------------------------ThreadPool---------------------------------------------------*/

ThreadPoolMy::ThreadPoolMy(ThreadPoolMy::size_type n)
    :_size{ n }, working_queue{}, busy_workers_count{ 0 }, terminated_{ false }, not_done_tasks{0}
{
    try
    {
        for (size_type i = 0; i < n; ++i)
        {
            std::shared_ptr<Worker> uptrw{ new Worker(this) };
            workers.push_back(uptrw);
            threads.push_back(std::thread(&Worker::run, uptrw.get()));
        }
    }
    catch (...)
    {
        terminate_all();
    }
}

/*-------------------------------------------------------------------------------------------------------*/

ThreadPoolMy::~ThreadPoolMy()
{
    terminate_all();
}

/*-------------------------------------------------------------------------------------------------------*/

void ThreadPoolMy::join_threads()
{
    for (int i = 0; i < threads.size(); ++i)
    {
        threads[i].join();
    }
}

/*-------------------------------------------------------------------------------------------------------*/

/*
    Эту функцию используем, чтобы перед выходом выполнились все задачи.
*/
void ThreadPoolMy::wait_all_tasks()
{
    // блокируем добавление новых задач и ждем окончания старых
    std::lock_guard<std::mutex> add_lck{ add_mtx };
    // используем активное ожидание - все равно вызов блокирующий и поток простаивает пока все задачи не будут выполены
    while (not_done_tasks.load())
    {
        std::this_thread::yield();
    }
}

/*-------------------------------------------------------------------------------------------------------*/

// завершаем все потоки рабочих
void ThreadPoolMy::terminate_all()
{
    terminated_ = true;
    // пхаем фейковые задачи, чтобы завершились потоки - так и живем-с, с костылями:(
    // Зато не используем активное ожидание!

    while(_size)
    {
        for (int i = 0; i < _size; ++i)
        {
            working_queue.push(function_wrapper());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    join_threads();
    //working_queue.clear();
}
