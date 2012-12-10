/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : Thread actions manager.
 *
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Pankaj Kumar <me at panks dot me>
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

#ifndef ACTIONTHREAD_H
#define ACTIONTHREAD_H

// Qt includes

#include <QThread>

// KDE includes

#include <kurl.h>
#include <threadweaver/Job.h>

// Libkdcraw includes

#include <libkdcraw/ractionthreadbase.h>

// Local includes

#include "batchtool.h"
#include "actions.h"

using namespace KDcrawIface;

namespace Digikam
{

class TaskSettings;
class QueueSettings;

class ActionThread : public RActionThreadBase
{
    Q_OBJECT

public:

    explicit ActionThread(QObject* const parent);
    ~ActionThread();

    void setSettings(const QueueSettings& queuePrm);
    void setResetExifOrientationAllowed(bool b);

    void processQueueItems(const QList<AssignedBatchTools>& items);

    void cancel();

Q_SIGNALS:

    /** Emit when an item from a queue start to be processed.
     */
    void signalStarting(const Digikam::ActionData& ad);

    /** Emit when an item from a queue have been processed.
     */
    void signalFinished(const Digikam::ActionData& ad);

    /** Emit when a queue have been fully processed (all items from queue are finished).
     */
    void signalQueueProcessed();

    /** Signal to emit to sub-tasks to cancel processing.
     */
    void signalCancelTask();

private Q_SLOTS:

    void slotThreadFinished();

private:

    class Private;
    Private* const d;
};

// ---------------------------------------------------------------------------------------------------------

class Task : public Job
{
    Q_OBJECT

public:

    Task();
    ~Task();

    void setSettings(const TaskSettings& settings);
    void setItem(const AssignedBatchTools& item);

Q_SIGNALS:

    void signalStarting(const Digikam::ActionData& ad);
    void signalFinished(const Digikam::ActionData& ad);

public Q_SLOTS:

    void slotCancel();

protected:

    void run();

private:

    void emitActionData(ActionData::ActionStatus st, const QString& mess=QString(), const KUrl& dest=KUrl());

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* ACTIONTHREAD_H */
