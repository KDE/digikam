/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-06
 * Description : Multithreaded worker object
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

    WorkerObject();
    ~WorkerObject();

    State state() const;

    void wait();

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

    enum DeactivatingMode
    {
        /// Already sent signals are cleared
        FlushSignals,
        /// The thread is stopped, but already sent signals remain in the queue
        KeepSignals,
        /// The thread is stopped when all signals emitted until now have been processed
        PhaseOut
    };

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

    virtual bool event(QEvent* e);

    void run();
    void setEventLoop(QEventLoop* loop);
    void addRunnable(WorkerObjectRunnable* loop);
    void removeRunnable(WorkerObjectRunnable* loop);

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

private:

    friend class WorkerObjectRunnable;
    friend class ThreadManager;

    class WorkerObjectPriv;
    WorkerObjectPriv* const d;
};

} // namespace Digikam

#endif // WORKEROBJECT_H
