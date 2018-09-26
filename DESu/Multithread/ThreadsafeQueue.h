#pragma once
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <iostream>

/*-------------------------------------------------------------------------------------------------------------*/

/*
    Ïðîñòàÿ ìóëüòèïîòî÷íàÿ î÷åðåäü.
*/
template<typename T>
class threadsafe_queue
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;			// óñëîâíàÿ ïåðåìåííàÿ
public:
    threadsafe_queue()
    {}
    threadsafe_queue(threadsafe_queue const& other)
    {
        // çäåñü ìû èìååì êîíñòàíòíóþ ññûëêó íà äðóãóþ î÷åðåäü,
        // îäíàêî ãäå-òî â äðóãîì ìåñòå êàêîé-òî îáúåêò ìîæåò èìåòü òó æå î÷åðåäü íåêîíñòàíòíî,
        // ïîýòîìó íóæíî çàïåðåòü ìüþòåêñ.
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
        ÏÎï êîòîðûé æäåò, ïîêà ÷òî-òî ïîÿâèòñÿ â î÷åðåäè.
    */
    void wait_and_pop(T& value)
    {
        // unique_lock ïîòîìó, ÷òî çäåñü ìû æäåì ïîêà î÷åðåäü ïóñòà, è ýòîò lock íå îòíîñèòñÿ ê îáùåìó
        std::unique_lock<std::mutex> lk(mut);
        data_cond.wait(lk, [this] {return !data_queue.empty(); });	// æäåò, ïîêà î÷åðåäü ïóñòà
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
        Ïîï, êîòîðûé íå æäåò, åñëè î÷åðåäü ïóñòà,
        è ñðàçó âîçâðàùàåò çíà÷åíèå íåóäà÷è.
        Èñïîëüçóåòñÿ äëÿ ýêîíîìèè âðåìåíè ïî ñðàâíåíèþ ñî æäóùèì ïîïîì,
        â ïîäõîäÿùèõ äëÿ ýòîãî ñèòóàöèÿõ.
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
    Ìóëüòèïîòî÷íàÿ î÷åðåäü ñ õîðîøî ñêîíóñòðóèðîâàííûì ìåõàíèçìîì çàìêîâ(îíè äåðæàòñÿ ìàëîå âðåìÿ)
    (fg - fine grained)
    Head è tail èìåþò ñâîè ñîáñòâåííûå çàìêè.
    Î÷åðåäü ñêîíñòðóèðîâàíà íà îäíîñâÿçíîì ñïèñêå.
    Â íà÷àëå ñîäåðæèòñÿ îäèí dummy node(ò.å. ïóñòàÿ î÷åðåäü ñîäåðæèò îäèí ôåéêîâûé ýëåìåíò).
        Ýòî íóæíî äëÿ òîãî, ÷òîáû push íå îáðàùàëñÿ ê ãîëîâå(íå ïðîâåðÿë åå îòñóòñòâèå).
        dummy node íàõîäèòñÿ â õâîñòå.
*/
template<typename T>
class threadsafe_queue_fg
{
private:

    /*
        Óçåë îäíîñâÿçíîãî ñïèñêà.
        data ïîìå÷åí êàê shared_ptr, ÷òîáû èçáåæàòü ïðîáëåì ñ âûäåëåíèåì ïàìÿòè.
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
    std::condition_variable data_cond;		// èñïîëüçóåòñÿ â waip_and_pop

    /*
        Çàùèùåííîå ïîëó÷åíèå õâîñòà
    */
    node* get_tail()
    {
        std::lock_guard<std::mutex> tail_lock(tail_mutex);
        return tail;
    }

    /*
        Çàùèùåííîå èçâëå÷åíèå ãîëîâû
    */
    std::unique_ptr<node> pop_head()
    {
        std::lock_guard<std::mutex> head_lock(head_mutex);

        if (head.get() == get_tail())		// íåò ýëåìåíòîâ
        {
            return std::unique_ptr<node>();
        }

        // ýëåãàíòíîå èçâëå÷åíèå ãîëîâû ïðè ïîìîùè move
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }

    /*
        Çàùèùåííîå èçâëå÷åíèå ãîëîâû ñ îæèäàíèåì
    */
    std::unique_ptr<node> pop_head_wait()
    {
        std::unique_lock<std::mutex> lk(head_mutex);
        // çäåñü íå èñïîëüçóåì empty èç-çà ïîâòîðíîãî lock'a
        data_cond.wait(lk, [&] { return head.get() != get_tail(); });

        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }

public:
    // ïðè ñîçäàíèè ãîëîâà è õâîñò ñîâïàäàþò
    threadsafe_queue_fg() :
        head(new node), tail(head.get())
    {}
    // çàïðåùàåì êîïèðîâàíèå
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

/*
    Çàùèùåííîå äîáàâëåíèå ýëåìåíòà â î÷åðåäü
*/
template<typename T>
void threadsafe_queue_fg<T>::push(T new_value)
{
    std::shared_ptr<T> new_data(
        std::make_shared<T>(std::move(new_value)));
    std::unique_ptr<node> p(new node);		// ýòî áóäåò íîâûé dummy node
    {	// çäåñü ïðîñòî îãðàíè÷èâàåòñÿ îáëàñòü lock_guard'a
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
    Ïûòàåòñÿ èçâëå÷ü ýëåìåíò èç î÷åðåäè, â ñëó÷àå íåóäà÷è âîçâðàùàåò ïóñòîé shared_ptr,
    â ñëó÷àå óñïåõà - shared_ptr ñ èçâëå÷åííûìè äàííûìè.
*/
template<typename T>
std::shared_ptr<T> threadsafe_queue_fg<T>::try_pop()
{
    std::unique_ptr<node> old_head = pop_head();
    return old_head ? old_head->data : std::shared_ptr<T>();
}

/*-------------------------------------------------------------------------------------------------------------*/

/*
    Âåðñèÿ, êîòîðàÿ âîçâðàùàåò ôëàã óñïåõà èëè íåóäà÷è, à ýëåìåíò çàïèñûâàåò â ïåðåäàííóþ ññûëêó.
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
    Âåðñèÿ, êîòîðàÿ æäåò, ïîêà ìîæíî èçâëå÷ü ýëåìåíò.
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

/*
    Âîçâðàùàåò ôëàã ïóñòîòû î÷åðåäè.
*/
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
