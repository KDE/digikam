/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-06
 * Description : Multithreaded worker object
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

#ifndef WORKEROBJECT_H
#define WORKEROBJECT_H

// Qt includes

#include <QObject>
#include <QThread>

// Local includes

#include "digikam_export.h"

class QEventLoop;

namespace Digikam
{

class WorkerObjectRunnable;

class DIGIKAM_EXPORT WorkerObject : public QObject
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

    enum DeactivatingMode
    {
        /// Already sent signals are cleared
        FlushSignals,
        /// The thread is stopped, but already sent signals remain in the queue
        KeepSignals,
        /// The thread is stopped when all signals emitted until now have been processed
        PhaseOut
    };

public:

    /**
     * Deriving from a worker object allows you to execute your slots in a thread.
     * Implement any slots and connect signals just as usual.
     * Call schedule() before or when signals are emitted. The object
     * will have moved to a thread when the signals are received by the slots.
     * Call deactivate() to stop computation.
     * Note that without calling schedule(), no signal will ever be processed.
     * You can use the connectAndSchedule convenience connection to avoid
     * having to call schedule() directly.
     * Note that you cannot make this QObject the child of another QObject.
     * Please check if you need to call shutDown from your destructor (see below).
     */

    WorkerObject();
    ~WorkerObject();

    State state() const;

    void wait();

    /** Sets the priority for this dynamic thread.
     *  Can be set anytime. If the thread is currently not running,
     *  the priority will be set when it is run next time.
     *  When you set QThread::InheritPriority (default), the
     *  priority is not changed but inherited from the thread pool.
     */
    void setPriority(QThread::Priority priority);
    QThread::Priority priority() const;

    /** You must normally call schedule() to ensure that the object is active when you send
     *  a signal with work data. Instead, you can use these connect() methods
     *  when connecting your signal to this object, the signal that carries work data.
     *  Then the object will be scheduled each time you emit the signal.
     */
    bool connectAndSchedule(const QObject* sender, const char* signal, const char* method,
                            Qt::ConnectionType type = Qt::AutoConnection) const;

    static bool connectAndSchedule(const QObject* sender, const char* signal,
                                   const WorkerObject* receiver, const char* method,
                                   Qt::ConnectionType type = Qt::AutoConnection);

    static bool disconnectAndSchedule(const QObject* sender, const char* signal,
                                      const WorkerObject* receiver, const char* method);

public Q_SLOTS:

    /**
     * Starts execution of this worker object:
     * The object is moved to a thread and an event loop started,
     * so that queued signals will be received.
     */
    void schedule();

    /**
     * Quits execution of this worker object.
     * If mode is FlushSignals, all already emitted signals will be cleared.
     * If mode is KeepSignals, already emitted signals are not cleared and
     * will be kept in the event queue until destruction or schedule() is called.
     * If mode is PhaseOut, already emitted signals will be processed
     * and the thread quit immediately afterwards.
     */
    void deactivate(DeactivatingMode mode = FlushSignals);

Q_SIGNALS:

    void started();
    void finished();

protected:

    bool transitionToRunning();
    void transitionToInactive();

    void run();
    void setEventLoop(QEventLoop* loop);
    void addRunnable(WorkerObjectRunnable* loop);
    void removeRunnable(WorkerObjectRunnable* loop);

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

    /**
     * Called from within thread's event loop to quit processing.
     * Quit any blocking operation.
     * Immediately afterwards, the event loop will be quit.
     */
    virtual void aboutToQuitLoop();

    /**
     * Called from deactivate(), typically from a different
     * thread than the worker thread, possibly the UI thread.
     * You can stop any extra controlled threads here.
     * Immediately afterwards, an event will be sent to the working
     * thread which will cause the event loop to quit. (aboutToQuitLoop())
     */
    virtual void aboutToDeactivate();

    virtual bool event(QEvent* e);

private:

    friend class WorkerObjectRunnable;
    friend class ThreadManager;

    class WorkerObjectPriv;
    WorkerObjectPriv* const d;
};

} // namespace Digikam

#endif // WORKEROBJECT_H
