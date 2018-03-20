/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-12-17
 * Description : workflow manager.
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef WORKFLOW_MANAGER_H
#define WORKFLOW_MANAGER_H

// Qt includes

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

// Local includes

#include "queuesettings.h"
#include "batchtoolutils.h"

namespace Digikam
{

/** This container group all queue common settings plus all assigned batch tools.
 */
class Workflow
{
public:

    explicit Workflow() {};
    ~Workflow()         {};

public:

    QString       title;
    QString       desc;

    QueueSettings qSettings;
    BatchSetList  aTools;
};

// -----------------------------------------------------------------------------------------------------------

class WorkflowManager : public QObject
{
    Q_OBJECT

public:

    static WorkflowManager* instance();

public:

    /** Load all Workflow from XML settings file. Fill 'failed' list with incompatible Workflow
     *  title/description not loaded.
     */
    bool load(QStringList& failed);

    /** Save all Workflow to  XML settings file.
     */
    bool save();
    void clear();

    void insert(const Workflow& q);
    void remove(const Workflow& q);

    Workflow findByTitle(const QString& title) const;
    QList<Workflow> queueSettingsList()        const;

Q_SIGNALS:

    void signalQueueSettingsAdded(const QString&);
    void signalQueueSettingsRemoved(const QString&);

private:

    void insertPrivate(const Workflow& q);
    void removePrivate(const Workflow& q);

private:

    WorkflowManager();
    ~WorkflowManager();

private:

    friend class WorkflowManagerCreator;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // WORKFLOW_MANAGER_H
