/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-26
 * Description : Camera log view.
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

#include "logview.h"
#include "logview.moc"

// Qt includes

#include <QHeaderView>
#include <QPixmap>
#include <QStringList>
#include <QMouseEvent>

// KDE includes

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

namespace Digikam
{

class LogViewItem : public QTreeWidgetItem
{
public:

    LogViewItem(QTreeWidget *parent, const QString& folder, const QString& file,
                const QString& text, LogView::LogEntryType entryType)
        : QTreeWidgetItem(parent, QStringList())
    {
        m_folder = folder;
        m_file   = file;

        switch(entryType)
        {
            case LogView::StartingEntry:
                setIcon(0, SmallIcon("system-run"));
                break;
            case LogView::SuccessEntry:
                setIcon(0, SmallIcon("dialog-ok"));
                break;
            case LogView::WarningEntry:
                setIcon(0, SmallIcon("dialog-warning"));
                setTextColor(2, Qt::darkYellow);
                break;
            case LogView::ErrorEntry:
                setIcon(0, SmallIcon("dialog-error"));
                setTextColor(2, Qt::red);
                break;
            case LogView::ProgressEntry:
                setIcon(0, SmallIcon("dialog-information"));
                break;
            default:
                setIcon(0, SmallIcon("dialog-information"));
        }

        setText(1, text);
    }

    QString folder() const
    {
        return m_folder;
    }

    QString file() const
    {
        return m_file;
    }

private:

    QString m_folder;
    QString m_file;
};

// ---------------------------------------------------------------------------

LogView::LogView(QWidget *parent)
       : QTreeWidget(parent)
{
    setIconSize(QSize(22, 22));
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSortingEnabled(false);
    setAllColumnsShowFocus(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setColumnCount(2);
    setHeaderHidden(true);
    setRootIsDecorated(false);
    setDragEnabled(true);
    viewport()->setMouseTracking(true);
    header()->setResizeMode(0, QHeaderView::ResizeToContents);
    header()->setResizeMode(1, QHeaderView::Stretch);

    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
            this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*)));
}

LogView::~LogView()
{
}

void LogView::addedLogEntry(const QString& folder, const QString& file, const QString& text, LogEntryType type)
{
    LogViewItem *item = new LogViewItem(this, folder, file, text, type);
    setCurrentItem(item);
}

void LogView::slotItemDoubleClicked(QTreeWidgetItem* item)
{
    LogViewItem* lvi = dynamic_cast<LogViewItem*>(item);
    if (lvi)
    {
        if (!lvi->folder().isEmpty() && !lvi->file().isEmpty())
            emit signalEntryClicked(lvi->folder(), lvi->file());
    }
}

void LogView::mouseMoveEvent(QMouseEvent* e)
{
    LogViewItem* lvi = dynamic_cast<LogViewItem*>(itemAt(e->pos()));
    if (lvi)
    {
        if (!lvi->folder().isEmpty() && !lvi->file().isEmpty())
            setCursor(Qt::PointingHandCursor);
        else
            unsetCursor();
    }
    QTreeWidget::mouseMoveEvent(e);
}

}  // namespace Digikam
