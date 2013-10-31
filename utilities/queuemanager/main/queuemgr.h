/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager GUI
 *
 * Copyright (C) 2008-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef QUEUEMGR_H
#define QUEUEMGR_H

// Qt includes

#include <QObject>

// KDE includes


// Local includes


namespace Digikam
{

class QueueSettings;
class AssignedBatchTools;
class ActionData;


class QueueMgr : public QObject
{
    Q_OBJECT

public:
    
    QueueMgr(QObject* parent);
    ~QueueMgr();

    bool setup(const QueueSettings& settings, const QList<AssignedBatchTools>& tools);
    
    void cancel();
    
Q_SIGNALS:
    void started();
    void action(const Digikam::ActionData& ad);
    void finished();

private Q_SLOTS:
    void slotStartAction(const Digikam::ActionData& ad);
    void slotFinishedAction(const Digikam::ActionData& ad);
    void slotQueueProcessed();
    
    void runnerFinished();
    void runnerError(const QString& title, const QString& text);

private:
    class Private;
    QScopedPointer<Private> d;
};

}  // namespace Digikam

#endif /* QUEUEMGR_H */
