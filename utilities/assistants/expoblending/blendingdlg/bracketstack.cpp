/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2015      by Benjamin Girault, <benjamin dot girault at gmail dot com>
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

#include "bracketstack.h"

// Qt includes

#include <QHeaderView>
#include <QPainter>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

BracketStackItem::BracketStackItem(QTreeWidget* const parent)
    : QTreeWidgetItem(parent)
{
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    setCheckState(0, Qt::Unchecked);
    setThumbnail(QIcon::fromTheme(QLatin1String("view-preview")).pixmap(treeWidget()->iconSize().width(), QIcon::Disabled));
}

BracketStackItem::~BracketStackItem()
{
}

void BracketStackItem::setUrl(const QUrl& url)
{
    m_url = url;
    setText(1, m_url.fileName());
}

const QUrl& BracketStackItem::url() const
{
    return m_url;
}

void BracketStackItem::setThumbnail(const QPixmap& pix)
{
    int iconSize = qMax<int>(treeWidget()->iconSize().width(), treeWidget()->iconSize().height());
    QPixmap pixmap(iconSize+2, iconSize+2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width()/2) - (pix.width()/2), (pixmap.height()/2) - (pix.height()/2), pix);
    setIcon(0, QIcon(pixmap));
}

void BracketStackItem::setExposure(const QString& exp)
{
    setText(2, exp);
}

bool BracketStackItem::isOn() const
{
    return (checkState(0) == Qt::Checked ? true : false);
}

void BracketStackItem::setOn(bool b)
{
    setCheckState(0, b ? Qt::Checked : Qt::Unchecked);
}

bool BracketStackItem::operator< (const QTreeWidgetItem& other) const
{
    int column     = treeWidget()->sortColumn();
    double thisEv  = text(column).toDouble();
    double otherEv = other.text(column).toDouble();
    return (thisEv < otherEv);
}

// -------------------------------------------------------------------------

BracketStackList::BracketStackList(QWidget* const parent)
    : QTreeWidget(parent)
{
    setIconSize(QSize(64, 64));
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSortingEnabled(true);
    setAllColumnsShowFocus(true);
    setRootIsDecorated(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setColumnCount(3);
    setHeaderHidden(false);
    setDragEnabled(false);
    header()->setSectionResizeMode(QHeaderView::Stretch);

    QStringList labels;
    labels.append( i18nc("@title:column Processing checkbox", "Include for Enfuse") );
    labels.append( i18nc("@title:column Input file name", "File Name") );
    labels.append( i18nc("@title:column Input image exposure", "Exposure (EV)") );
    setHeaderLabels(labels);

    connect(ThumbnailLoadThread::defaultThread(), SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnail(LoadingDescription,QPixmap)));

    sortItems(2, Qt::DescendingOrder);
}

BracketStackList::~BracketStackList()
{
}

QList<QUrl> BracketStackList::urls()
{
    QList<QUrl> list;
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        BracketStackItem* const item = dynamic_cast<BracketStackItem*>(*it);

        if (item && item->isOn())
            list.append(item->url());

        ++it;
    }

    return list;
}

BracketStackItem* BracketStackList::findItem(const QUrl& url)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        BracketStackItem* const lvItem = dynamic_cast<BracketStackItem*>(*it);

        if (lvItem && lvItem->url() == url)
        {
            return lvItem;
        }

        ++it;
    }

    return 0;
}

void BracketStackList::addItems(const QList<QUrl>& list)
{
    if (list.count() == 0)
    {
        return;
    }

    QList<QUrl> urls;

    for (const QUrl& imageUrl: list)
    {
        // Check if the new item already exist in the list.
        bool found = false;

        QTreeWidgetItemIterator iter(this);

        while (*iter)
        {
            BracketStackItem* const item = dynamic_cast<BracketStackItem*>(*iter);

            if (item->url() == imageUrl)
            {
                found = true;
            }

            ++iter;
        }

        if (!found)
        {
            BracketStackItem* const item = new BracketStackItem(this);
            item->setUrl(imageUrl);
            item->setOn(true);
            urls.append(imageUrl);
        }
    }

    foreach(const QUrl& url, urls)
    {
        ThumbnailLoadThread::defaultThread()->find(ThumbnailIdentifier(url.toLocalFile()));
    }

    emit signalAddItems(urls);
}

void BracketStackList::slotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        BracketStackItem* const item = static_cast<BracketStackItem*>(*it);

        if (item->url() == QUrl::fromLocalFile(desc.filePath))
        {
            if (pix.isNull())
            {
                item->setThumbnail(QIcon::fromTheme(QLatin1String("view-preview")).pixmap(iconSize().width(), QIcon::Disabled));
            }
            else
            {
                item->setThumbnail(pix.scaled(iconSize().width(), iconSize().height(), Qt::KeepAspectRatio));
            }

            return;
        }

        ++it;
    }
}

}  // namespace Digikam
