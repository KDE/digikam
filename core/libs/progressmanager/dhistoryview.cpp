/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-26
 * Description : History view.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dhistoryview.h"

// Qt includes

#include <QHeaderView>
#include <QPixmap>
#include <QStringList>
#include <QMouseEvent>
#include <QMimeData>
#include <QClipboard>
#include <QTime>
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class DHistoryViewItem : public QTreeWidgetItem
{
public:

    DHistoryViewItem(QTreeWidget* const parent, const QString& msg, DHistoryView::EntryType type, const QVariant& metadata)
        : QTreeWidgetItem(parent, QStringList())
    {
        m_metadata = metadata;

        switch (type)
        {
            case DHistoryView::StartingEntry:
                setIcon(0, QIcon::fromTheme(QLatin1String("system-run")));
                break;
            case DHistoryView::SuccessEntry:
                setIcon(0, QIcon::fromTheme(QLatin1String("dialog-ok-apply")));
                break;
            case DHistoryView::WarningEntry:
                setIcon(0, QIcon::fromTheme(QLatin1String("dialog-warning")));
                setTextColor(2, Qt::darkYellow);
                break;
            case DHistoryView::ErrorEntry:
                setIcon(0, QIcon::fromTheme(QLatin1String("dialog-error")));
                setTextColor(2, Qt::red);
                break;
            case DHistoryView::ProgressEntry:
                setIcon(0, QIcon::fromTheme(QLatin1String("dialog-information")));
                break;
            case DHistoryView::CancelEntry:
                setIcon(0, QIcon::fromTheme(QLatin1String("dialog-cancel")));
                setTextColor(2, Qt::darkBlue);
                break;
            default:
                setIcon(0, QIcon::fromTheme(QLatin1String("dialog-information")));
                break;
        }

        setText(1, QTime::currentTime().toString(Qt::ISODate));
        setText(2, msg);
    }

    QVariant metadata() const
    {
        return m_metadata;
    }

private:

    QVariant m_metadata;
};

// ---------------------------------------------------------------------------

DHistoryView::DHistoryView(QWidget* const parent)
    : QTreeWidget(parent)
{
    qRegisterMetaType<EntryType>("DHistoryView::EntryType");

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
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);  // Icon
    header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);  // Time
    header()->setSectionResizeMode(2, QHeaderView::Stretch);           // Message

    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotItemDoubleClicked(QTreeWidgetItem*)));

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotContextMenu()));
}

DHistoryView::~DHistoryView()
{
}

void DHistoryView::slotContextMenu()
{
    QMenu popmenu(this);
    QAction* const action = new QAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("Copy to Clipboard"), this);

    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotCopy2ClipBoard()));

    popmenu.addAction(action);
    popmenu.exec(QCursor::pos());
}

void DHistoryView::slotCopy2ClipBoard()
{
    QString textInfo;

    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        textInfo.append((*it)->text(1));
        textInfo.append(QLatin1String(" :: "));
        textInfo.append((*it)->text(2));
        textInfo.append(QLatin1String("\n"));
        ++it;
    }

    QMimeData* const mimeData = new QMimeData();
    mimeData->setText(textInfo);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void DHistoryView::addEntry(const QString& msg, EntryType type, const QVariant& metadata)
{
    DHistoryViewItem* const item = new DHistoryViewItem(this, msg, type, metadata);
    // Dispatch events to Qt loop in case of bombarding of messages. See bug #338629
    qApp->processEvents();
    setCurrentItem(item);
}

void DHistoryView::slotItemDoubleClicked(QTreeWidgetItem* item)
{
    DHistoryViewItem* const lvi = dynamic_cast<DHistoryViewItem*>(item);

    if (lvi)
    {
        if (!lvi->metadata().isNull())
        {
            emit signalEntryClicked(lvi->metadata());
        }
    }
}

void DHistoryView::mouseMoveEvent(QMouseEvent* e)
{
    DHistoryViewItem* const lvi = dynamic_cast<DHistoryViewItem*>(itemAt(e->pos()));

    if (lvi)
    {
        if (!lvi->metadata().isNull())
        {
            setCursor(Qt::PointingHandCursor);
        }
        else
        {
            unsetCursor();
        }
    }
    else
    {
        unsetCursor();
    }

    QTreeWidget::mouseMoveEvent(e);
}

}  // namespace Digikam
