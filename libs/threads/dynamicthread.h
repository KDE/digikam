/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-13
 * Description : Dynamically active thread
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DYNAMICTHREAD_H
#define DYNAMICTHREAD_H

// Qt includes

#include <QObject>
#include <QRunnable>
#include <QThread>

// Local includes

#include "digikam_export.h"

class QMutex;
class QMutexLocker;

namespace Digikam
{

class DIGIKAM_EXPORT DynamicThread : public QObject, public QRunnable
{
    Q_OBJECT

public:

    enum State
    {
        Inactive,
        Scheduled,
        Running,
        Deactivating
    };

public:

    /** This class extends QRunnable, so you have to reimplement
     *  virtual void run(). In all aspects the class will act similar to a QThread.
     */
    explicit DynamicThread(QObject* const parent = 0);

    /** The destructor calls stop() and wait(), but if you, in your destructor,
     *  delete any data that is accessed by your run() method,
     *  you must call stop() and wait() before yourself. */
    virtual ~DynamicThread();

    State state() const;
    bool  isRunning() const;
    bool  isFinished() const;

    void setEmitSignals(bool emitThem);

    /** Sets the priority for this dynamic thread.
     *  Can be set anytime. If the thread is currently not running,
     *  the priority will be set when it is run next time.
     *  When you set QThread::InheritPriority (default), the
     *  priority is not changed but inherited from the thread pool.
     */
    void setPriority(QThread::Priority priority);
    QThread::Priority priority() const;

public Q_SLOTS:

    void start();

    /** Stop computation, sets the running flag to false. */
    void stop();

    /** Waits until the thread finishes. Typically, call stop() before. */
    void wait();

Q_SIGNALS:

    /// Emitted if emitSignals is enabled
    void starting();
    void finished();

protected:

    /**
     * If you are deleting data in your destructor which is accessed from the thread,
     * do one of the following from your destructor to guarantee a safe shutdown:
     * 1) Call this method
     * 2) Call stop() and wait(), knowing that nothing will
     *    call start() anymore after this
     * 3) Be sure the thread will never be running at destruction.
     * Note: This irrevocably stops this object.
     * Note: It is not sufficient that your parent class does this.
     * Calling this method, or providing one of the above mentioned
     * equivalent guarantees, must be done by every
     * single last class in the hierarchy with an implemented destructor deleting data.
     * (the base class destructor is always called after the derived class)
     */
    void shutDown();

    /** In you run() method, you shall regularly check for runningFlag()
     *  and cleanup and return if false. */
    bool runningFlag() const volatile;

    /**
     * This is the non-recursive mutex used to protect state variables
     * and waiting in this class. You can use it if you want to protect
     * your memory in the same scope as calling start, stop or wait,
     * then using the QMutexLocker variants below. Note that when you have locked this mutex,
     * you must use these variants, as the mutex is non-recursive.
     */
    QMutex* threadMutex() const;

    /**
     * Doing the same as start(), stop() and wait above, provide it
     * with a locked QMutexLocker on mutex().
     * Note the start() will unlock and relock for scheduling once, after state change.
     */
    void start(QMutexLocker& locker);
    void stop(QMutexLocker& locker);
    void wait(QMutexLocker& locker);

private:

    friend class DynamicThreadPriv;

    class DynamicThreadPriv;
    DynamicThreadPriv* const d;
};

} // namespace Digikam

#endif // DYNAMICTHREAD_H
