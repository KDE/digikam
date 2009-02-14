/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-13
 * Description : tabbed queue items list.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef QUEUE_TAB_H
#define QUEUE_TAB_H

// Qt includes.

#include <QWidget>

// KDE includes.

#include <ktabwidget.h>

namespace Digikam
{

class ImageInfo;
class ImageInfoList;
class AssignedBatchTools;
class QueueListView;
class QueueTabPriv;

class QueueTab : public KTabWidget
{
    Q_OBJECT

public:

    QueueTab(QWidget *parent);
    ~QueueTab();

    void addQueue();
    QueueListView* currentQueue() const;

signals:

    void signalImageListChanged();
    void signalItemSelected(const AssignedBatchTools&);

public slots:

    void slotClearList();
    void slotRemoveSelectedItems();
    void slotRemoveItemsDone();
    void slotAddItems(const ImageInfoList& list, const ImageInfo &current);
    void slotAssignedToolsChanged(const AssignedBatchTools& tools4Item);

private:

    QueueTabPriv* const d;
};

}  // namespace Digikam

#endif // QUEUE_TAB_H
