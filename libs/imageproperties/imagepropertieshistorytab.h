/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-23
 * Description : a tab to display image editing history
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef IMAGEPROPERTIESHISTORYTAB_H
#define IMAGEPROPERTIESHISTORYTAB_H

// Qt includes

#include <QWidget>
#include <QAction>
#include <QModelIndex>

// KDE includes

#include <kurl.h>

// Local includes

#include "digikam_export.h"
#include "dmetadata.h"

namespace Digikam
{

class DIGIKAM_EXPORT RemoveFilterAction : public QAction
{
    Q_OBJECT

public:

    RemoveFilterAction(const QString& label, const QModelIndex& index, QObject* const parent = 0);
    ~RemoveFilterAction()
    {
    }

    void setIndex(QModelIndex& index)
    {
        m_index = index;
    }

public Q_SLOTS:

    void triggerSlot()
    {
        emit actionTriggered(m_index);
    }

Q_SIGNALS:

    void actionTriggered(QModelIndex index);

private:

    QModelIndex m_index;
};

// -------------------------------------------------------------------------------------

class DIGIKAM_EXPORT ImagePropertiesHistoryTab : public QWidget
{
    Q_OBJECT

public:

    explicit ImagePropertiesHistoryTab(QWidget* const parent);
    ~ImagePropertiesHistoryTab();

    void setCurrentURL(const KUrl& url = KUrl());

public Q_SLOTS:

    void showCustomContextMenu(const QPoint& position);
    void setModelData(const QList<DImageHistory::Entry>& entries);
    void disableEntry(bool disable);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // IMAGEPROPERTIESHISTORYTAB_H
