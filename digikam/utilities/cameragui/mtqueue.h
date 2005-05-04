#ifndef COMMAND_QUEUE_H
#define COMMAND_QUEUE_H

#include <qptrqueue.h>
#include <qmutex.h>

template<class Type> class MTQueue
{

public:

  MTQueue()
  {
      queue_.setAutoDelete(true);
  }

  ~MTQueue()
  {
    flush();
  }

  bool isEmpty()
  {
    mutex_.lock();
    bool empty = queue_.isEmpty();
    mutex_.unlock();
    return empty;
  }

  void flush()
  {
    mutex_.lock();
    queue_.clear();
    mutex_.unlock();
  }

  void enqueue(Type * t)
  {
    mutex_.lock();
    queue_.enqueue(t);
    mutex_.unlock();
  }

  Type * dequeue()
  {
    mutex_.lock();
    Type * i = queue_.dequeue();
    mutex_.unlock();
    return i;
  }

  Type * head(bool lock=true)
  {
    if (lock)
       mutex_.lock();
    Type * i = queue_.head();
    if (lock)
        mutex_.unlock();
    return i;
  }

  int count()
  {
    mutex_.lock();
    int c = queue_.count();
    mutex_.unlock();
    return c;
  }

  void lock()
  {
    mutex_.lock();
  }

  void unlock()
  {
    mutex_.unlock();
  }
    
private:

  QPtrQueue<Type> queue_;
  QMutex mutex_;

};

#endif
