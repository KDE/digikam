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
#include <QMimeData>
#include <QClipboard>
#include <QApplication>
#include <QTime>

// KDE includes

#include <kaction.h>
#include <kmenu.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

namespace Digikam
{

class LogViewItem : public QTreeWidgetItem
{
public:

    LogViewItem(QTreeWidget *parent, const QString& folder, LogView::LogEntryType entryType,
                const QString& file, const QString& text)
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

        setText(1, QTime::currentTime().toString(Qt::ISODate));
        setText(2, text);
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
    setContextMenuPolicy(Qt::CustomContextMenu);
    setIconSize(QSize(22, 22));
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSortingEnabled(false);
    setAllColumnsShowFocus(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setColumnCount(3);
    setHeaderHidden(true);
    setRootIsDecorated(false);
    setDragEnabled(true);
    viewport()->setMouseTracking(true);
    header()->setResizeMode(0, QHeaderView::ResizeToContents);  // Icon
    header()->setResizeMode(1, QHeaderView::ResizeToContents);  // Time
    header()->setResizeMode(2, QHeaderView::Stretch);           // Message

    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
            this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*)));

    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slotContextMenu()));
}

LogView::~LogView()
{
}

void LogView::addedLogEntry(const QString& folder, LogEntryType type, const QString& file, const QString& text)
{
    LogViewItem *item = new LogViewItem(this, folder, type, file, text);
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

void LogView::slotContextMenu()
{
    KMenu popmenu(this);
    KAction *action = new KAction(KIcon("edit-copy"), i18n("Copy to Clipboard"), this);
    connect(action, SIGNAL(triggered(bool) ),
            this, SLOT(slotCopy2ClipBoard()));

    popmenu.addAction(action);
    popmenu.exec(QCursor::pos());
}

void LogView::slotCopy2ClipBoard()
{
    QString textInfo;

    QTreeWidgetItemIterator it(this);
    while (*it)
    {
        textInfo.append((*it)->text(1));
        textInfo.append(" :: ");
        textInfo.append((*it)->text(2));
        textInfo.append("\n");
        ++it;
    }

    QMimeData *mimeData = new QMimeData();
    mimeData->setText(textInfo);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

}  // namespace Digikam
