#ifndef LISTMT_H
#define LISTMT_H


#include <qvaluelist.h>
#include <qmutex.h>


template<class Type> class MTList {

public:

    typedef typename QValueList<Type>::const_iterator const_iterator_type;

    MTList() {
    }

    ~MTList() {

        flush();
    }

    MTList(const MTList<Type>& origList) {

        mutex_.lock();

        list_.clear();
        MTList<Type> &ref = const_cast<MTList<Type>&> (origList); // get rid of const

        const_iterator_type iter;
        for (iter = ref.begin(); iter != ref.end(); ++iter)
            list_.push_back(*iter);

        mutex_.unlock();

    }

    MTList(const QValueList<Type>& origList) {

        mutex_.lock();
        list_.clear();

        const_iterator_type iter;
        for (iter = origList.begin(); iter != origList.end(); ++iter)
            list_.push_back(*iter);

        mutex_.unlock();

    }


    MTList& operator=(const MTList<Type>& origList) {

        mutex_.lock();

        if (this != &origList) {

            list_.clear();
            MTList<Type> &ref = const_cast<MTList<Type>&> (origList); // get rid of const

            const_iterator_type iter;
            for (iter = ref.begin(); iter != ref.end(); ++iter)
                list_.push_back(*iter);

        }

        mutex_.unlock();

        return *this;

    }

    bool isEmpty() {

        mutex_.lock();
        bool empty = list_.empty();
        mutex_.unlock();
        return empty;
    }

    int count() {

        mutex_.lock();
        int num = list_.count();
        mutex_.unlock();
        return num;
    }



    void flush() {

        mutex_.lock();
        list_.clear();
        mutex_.unlock();
    }

    void push_front(const Type& t) {

        mutex_.lock();
        list_.push_front(t);
        mutex_.unlock();
    }

    void push_back(const Type& t) {

        mutex_.lock();
        list_.push_back(t);
        mutex_.unlock();
    }

    void pop_front() {

        mutex_.lock();
        list_.pop_front();
        mutex_.unlock();
    }

    void pop_back() {

        mutex_.lock();
        list_.pop_back();
        mutex_.unlock();
    }

    void append(const Type& t) {

        mutex_.lock();
        list_.append(t);
        mutex_.unlock();

    }

    Type first() {

        mutex_.lock();
        Type t(list_.first());
        mutex_.unlock();
        return t;
    }

    Type last() {

        mutex_.lock();
        Type t(list_.last());
        mutex_.unlock();
        return t;
    }

    Type operator[](int i) {

        mutex_.lock();
        Type t(list_[i]);
        mutex_.unlock();
        return t;
    }

    const_iterator_type begin() {

        mutex_.lock();
        const_iterator_type iter;
        iter = list_.begin();
        mutex_.unlock();
        return iter;

    }

    const_iterator_type end() {

        mutex_.lock();
        const_iterator_type iter;
        iter = list_.end();
        mutex_.unlock();
        return iter;

    }


private:

    QValueList<Type> list_;
    QMutex mutex_;

};

#endif

