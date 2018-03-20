/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-12-18
 * Description : Customized Workflow Settings list.
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

#ifndef WORKFLOW_LIST_H
#define WORKFLOW_LIST_H

// Qt includes

#include <QTreeWidget>
#include <QWidget>
#include <QPixmap>
#include <QMap>
#include <QString>
#include <QList>

// Local includes

#include "batchtool.h"
#include "batchtoolsmanager.h"

namespace Digikam
{

class WorkflowList;

class WorkflowItem : public QTreeWidgetItem
{

public:

    WorkflowItem(WorkflowList* const parent, const QString& name);
    virtual ~WorkflowItem();

    QString title() const;
    int     count() const;
};

// -------------------------------------------------------------------------

class WorkflowList : public QTreeWidget
{
    Q_OBJECT

public:

    explicit WorkflowList(QWidget* const parent);
    virtual ~WorkflowList();

Q_SIGNALS:

    void signalAssignQueueSettings(const QString&);

public Q_SLOTS:

    void slotRemoveQueueSettings(const QString& title);
    void slotsAddQueueSettings(const QString& title);

private Q_SLOTS:

    void slotContextMenu();
    void slotAssignQueueSettings();

private:

    WorkflowItem* findByTitle(const QString& title);

    void        startDrag(Qt::DropActions supportedActions);
    QStringList mimeTypes() const;
    QMimeData*  mimeData(const QList<QTreeWidgetItem*> items) const;

    void mouseDoubleClickEvent(QMouseEvent*);
};

}  // namespace Digikam

#endif // WORKFLOW_LIST_H
