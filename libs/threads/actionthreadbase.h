/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2011-12-28
 * @brief  Low level threads management for batch processing on multi-core
 *
 * @author Copyright (C) 2011-2016 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2014 by Veaceslav Munteanu
 *         <a href="mailto:veaceslav dot munteanu90 at gmail dot com">veaceslav dot munteanu90 at gmail dot com</a>
 * @author Copyright (C) 2011-2012 by A Janardhan Reddy
 *         <a href="annapareddyjanardhanreddy at gmail dot com">annapareddyjanardhanreddy at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef ACTION_THREAD_BASE_H
#define ACTION_THREAD_BASE_H

// Qt includes

#include <QThread>
#include <QRunnable>
#include <QObject>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT ActionJob : public QObject,
                                 public QRunnable
{
    Q_OBJECT

public:

    /** Constructor which delegate deletion of QRunnable instance to ActionThreadBase, not QThreadPool.
     */
    ActionJob();

    /** Re-implement destructor in you implementation. Don't forget to cancel job.
     */
    virtual ~ActionJob();

Q_SIGNALS:

    /** Use this signal in your implementation to inform ActionThreadBase manager that job is started
     */
    void signalStarted();

    /** Use this signal in your implementation to inform ActionThreadBase manager the job progress
     */
    void signalProgress(int);

    /** Use this signal in your implementation to inform ActionThreadBase manager the job is done.
     */
    void signalDone();

public Q_SLOTS:

    /** Call this method to cancel job.
     */
    void cancel();

protected:

    /** You can use this boolean in your implementation to know if job must be canceled.
     */
    bool m_cancel;
};

/** Define a map of job/priority to process by ActionThreadBase manager.
 *  Priority value can be used to control the run queue's order of execution.
 *  Zero priority want mean to process job with higher priority.
 */
typedef QMap<ActionJob*, int> ActionJobCollection;

// -------------------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT ActionThreadBase : public QThread
{
    Q_OBJECT

public:

    ActionThreadBase(QObject* const parent=0);
    virtual ~ActionThreadBase();

    /** Adjust maximum number of threads used to parallelize collection of job processing.
     */
    void setMaximumNumberOfThreads(int n);

    /** Return the maximum number of threads used to parallelize collection of job processing.
     */
    int  maximumNumberOfThreads() const;

    /** Reset maximum number of threads used to parallelize collection of job processing to max core detected on computer.
     *  This method is called in contructor.
     */
    void defaultMaximumNumberOfThreads();

    /** Cancel processing of current jobs under progress.
     */
    void cancel();

protected:

    /** Main thread loop used to process jobs in todo list.
     */
    void run();

    /** Append a collection of jobs to process into QThreadPool.
     *  Jobs are add to pending lists and will be deleted by ActionThreadBase, not QThreadPool.
     */
    void appendJobs(const ActionJobCollection& jobs);

    /** Return true if list of pending jobs to process is empty.
     */
    bool isEmpty() const;

protected Q_SLOTS:

    void slotJobFinished();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // ACTION_THREAD_BASE_H
