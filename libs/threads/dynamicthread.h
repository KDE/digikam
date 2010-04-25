/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-13
 * Description : Dynamically active thread
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

#ifndef DYNAMICTHREAD_H
#define DYNAMICTHREAD_H

// Qt includes

#include <QObject>
#include <QRunnable>

// Local includes

#include "digikam_export.h"

class QMutex;
class QMutexLocker;

namespace Digikam
{

class DynamicThreadPriv;

class DynamicThread : public QObject, public QRunnable
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

    /** This class extends QRunnable, so you have to reimplement
     *  virtual void run(). In all aspects the class will act similar to a QThread.
     */

    DynamicThread(QObject *parent = 0);

    /** The destructor calls stop() and wait(), but if you, in your destructor,
     *  delete any data that is accessed by your run() method,
     *  you must call stop() and wait() before yourself. */
    virtual ~DynamicThread();

    State state() const;
    bool  isRunning() const;
    bool  isFinished() const;

    void setEmitSignals(bool emitThem);

public Q_SLOTS:

    void start();
    /** Stop computation, sets the running flag to false. */
    void stop();
    /** Waits until the thread finishes. Typically, call stop() before. */
    void wait();

Q_SIGNALS:

    /// Emitted if emitSignals is enabled
    void started();
    void finished();

protected:

    /** In you run() method, you shall regularly check for runningFlag()
     *  and cleanup and return if false. */
    bool runningFlag() const;

    /**
     * This is the non-recursive mutex used to protect state variables
     * and waiting in this class. You can use it if you want to protect
     * your memory in the same scope as calling start, stop or wait,
     * then using the QMutexLocker variants below. Note that when you have locked this mutex,
     * you must use these variants, as the mutex is non-recursive.
     */
    QMutex *threadMutex() const;

    /**
     * Doing the same as start(), stop() and wait above, provide it
     * with a locked QMutexLocker on mutex().
     * Note the start() will unlock and relock for scheduling once, after state change.
     */
    void start(QMutexLocker &locker);
    void stop(QMutexLocker &locker);
    void wait(QMutexLocker &locker);

private:

    friend class DynamicThreadPriv;
    DynamicThreadPriv* const d;
};

} // namespace Digikam

#endif // DYNAMICTHREAD_H


