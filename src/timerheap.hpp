#ifndef TIMERHEAP_HEADER
#define TIMERHEAP_HEADER

#include <stdlib.h>
#include <time.h>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

typedef boost::function<void(void*)> timeoutcallback;

struct timer : boost::noncopyable
{
    timer(int delay,timeoutcallback cb,void* ud)
    {
        expire_ = time(NULL) + delay;
        cb_ = cb;
        userdata_ = ud;
        oneshot_ = true;
        interval_ = 0;
    }
    timer(int delay, int interval, timeoutcallback cb, void* ud)
    {
        expire_ = time(NULL) + delay;
        cb_ = cb;
        userdata_ = ud;
        oneshot_ = false;
        interval_ = interval;
    }
    time_t expire_;
    timeoutcallback cb_;
    void* userdata_;
    bool oneshot_;
    int interval_;
};

class timerheap : boost::noncopyable
{
public:
    timerheap(int cap):capacity_(cap), cursize_(0)
    {
        array_ = new timer*[capacity_];
        if (!array_) {
            abort();
        }
        for (int i=0; i<capacity_; ++i) {
            array_[i] = NULL;
        }
    }
    
    timerheap(timer** initarray, int size, int capacity):
    cursize_(size), capacity_(capacity)
    {
        if (capacity < size) {
            abort();
        }
        array_ = new timer*[capacity];
        if (!array_) {
            abort();
        }
        for (int i=0; i<capacity; ++i) {
            array_[i] = NULL;
        }
        if (size != 0) {
            for (int i=0; i<size; ++i) {
                array_[i] = initarray[i];
            }
            for (int i=(cursize_-1)/2; i>=0; --i) {
                percolatedown(i);
            }
        }
    }
    
    ~timerheap()
    {
        for (int i=0; i<cursize_; ++i) {
            delete array_[i];
        }
        delete[] array_;
    }
    
    timer* addoneshottimer(int delay, timeoutcallback cb, void* userdata)
    {
        timer* t = new timer(delay,cb,userdata);
        addtimer(t);
        return t;
    }
    
    timer* addrepeattimer(int delay, int interval, timeoutcallback cb, void* ud)
    {
        timer* t = new timer(delay,interval,cb,ud);
        addtimer(t);
        return t;
    }
    
    void deltimer(timer* t)
    {
        if (!t) {
            return;
        }
        t->cb_ = NULL;
    }
    
    timer* top() const
    {
        if (empty()) {
            return NULL;
        }
        return array_[0];
    }
    
    void poptimer()
    {
        if (empty()) {
            return;
        }
        if (array_[0]) {
            timer* tmp = array_[0];
            array_[0] = array_[--cursize_];
            percolatedown(0);
            if (!array_[0]->oneshot_ && array_[0]->cb_) {
                tmp->expire_ = time(NULL) + tmp->interval;
                addtimer(tmp);
            } else {
                delete tmp;
            }
        }
    }
    
    void tick()
    {
        timer* tmp = array_[0];
        time_t cur = time(NULL);
        while (!empty()) {
            if (!tmp) {
                break;
            }
            if (tmp->expire_ > cur) {
                break;
            }
            if (array_[0]->cb_) {
                array_[0]->cb_(array_[0]->userdata_);
            }
            poptimer();
            tmp = array_[0];
        }
    }
    
    bool empty() const
    {
        return cursize_==0;
    }
    
private:
    void addtimer(timer* t)
    {
        if (!t) {
            return;
        }
        if (cursize_ >= capacity_) {
            resize();
        }
        int hole = cursize_++;
        int parent = 0;
        for (; hole > 0; hole = parent) {
            parent = (hole-1)/2;
            if (array_[parent]->expire_ <= t->expire_) {
                break;
            }
            array_[hole] = array_[parent];
        }
        array_[hole] = t;
    }
    
    void percolatedown(int hole)
    {
        timer* tmp = array_[hole];
        int child = 0;
        for (; ((hole*2+1) <= (cursize_-1)); hole=child) {
            chile = hole*2+1;
            if ((child < (cursize_-1)) &&
                (array_[child+1]->expire_ < array_[child]->expire_)) {
                ++child;
            }
            if (array_[child]->expire_ < tmp->expire_) {
                array_[hole] = array_[child];
            }
            else {
                break;
            }
        }
        array_[hole] = tmp;
    }
    
    void resize()
    {
        timer** tmp = new timer*[2*capacity_];
        for (int i=0; i<2*capacity_; ++i) {
            tmp[i] = NULL;
        }
        if (!tmp) {
            abort();
        }
        capacity_ = 2*capacity_;
        for (int i=0; i<cursize_; ++i) {
            tmp[i] = array_[i];
        }
        delete[] array_;
        array_ = tmp;
    }
    
private:
    timer** array_;
    int capacity_;
    int cursize_;
};

#endif



