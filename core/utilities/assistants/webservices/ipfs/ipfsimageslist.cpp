/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-12
 * Description : a tool to export images to IPFS web service
 *
 * Copyright (C) 2018 by Amar Lakshya <amar dot lakshya  at xaviers dot edu dot in>
 * Copyright (C) 2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "ipfsimageslist.h"

// Qt includes

#include <QLabel>
#include <QPointer>
#include <QDesktopServices>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dmetadata.h"

namespace Digikam
{

IpfsImagesList::IpfsImagesList(QWidget* const parent)
    : DItemsList(parent)
{
    setControlButtonsPlacement(DItemsList::ControlButtonsBelow);
    setAllowDuplicate(false);
    setAllowRAW(false);

    auto* list = listView();

    list->setColumnLabel(DItemsListView::Thumbnail, i18n("Thumbnail"));

    list->setColumnLabel(static_cast<DItemsListView::ColumnType>(IpfsImagesList::Title),
                         i18n("Submission title"));

    list->setColumnLabel(static_cast<DItemsListView::ColumnType>(IpfsImagesList::Description),
                         i18n("Submission description"));

    list->setColumn(static_cast<DItemsListView::ColumnType>(IpfsImagesList::Url),
                    i18n("Ipfs Url"), true);

    connect(list, &DItemsListView::itemDoubleClicked,
            this, &IpfsImagesList::slotDoubleClick);
}

QList<const IpfsImagesListViewItem*> IpfsImagesList::getPendingItems()
{
    QList<const IpfsImagesListViewItem*> ret;

    for (unsigned int i = listView()->topLevelItemCount() ; --i ;)
    {
        const auto* item = dynamic_cast<const IpfsImagesListViewItem*>(listView()->topLevelItem(i));

        if (item && item->IpfsUrl().isEmpty())
            ret << item;
    }

    return ret;
}

void IpfsImagesList::slotAddImages(const QList<QUrl>& list)
{
    /* Replaces the KPImagesList::slotAddImages method, so that
     * IpfsImagesListViewItems can be added instead of ImagesListViewItems */

    DMetadata meta;

    for (QList<QUrl>::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        // Already in the list?
        if (listView()->findItem(*it) == nullptr)
        {
            // Load URLs from meta data, if possible
            if (meta.load((*it).toLocalFile()))
            {
                auto* const item = new IpfsImagesListViewItem(listView(), *it);
                item->setIpfsUrl(meta.getXmpTagString("Xmp.digiKam.IPFSId"));
            }
        }
    }

    emit signalImageListChanged();
    emit signalAddItems(list);
}

void IpfsImagesList::slotSuccess(const IpfsTalkerResult& result)
{
    QUrl ipfsl = QUrl::fromLocalFile(result.action->upload.imgpath);

    processed(ipfsl, true);

    DMetadata meta;

    // Save URLs to meta data, if possible
    if (meta.load(ipfsl.toLocalFile()))
    {
        meta.setXmpTagString("Xmp.digiKam.IPFSId", result.image.url);
        bool saved = meta.applyChanges();
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Metadata" << (saved ? "Saved" : "Not Saved") << "to" << ipfsl;
    }

    IpfsImagesListViewItem* const currItem = dynamic_cast<IpfsImagesListViewItem*>(listView()->findItem(ipfsl));

    if (!currItem)
        return;

    if (!result.image.url.isEmpty())
        currItem->setIpfsUrl(result.image.url);
}

void IpfsImagesList::slotDoubleClick(QTreeWidgetItem* element, int i)
{
    if (i == Url)
    {
        const QUrl url = QUrl(element->text(i));
        // The delete page asks for confirmation, so we don't need to do that here
        QDesktopServices::openUrl(url);
    }
}

// ------------------------------------------------------------------------------------------------

IpfsImagesListViewItem::IpfsImagesListViewItem(DItemsListView* const view, const QUrl& url)
    : DItemsListViewItem(view, url)
{
    const QColor blue(50, 50, 255);

    setTextColor(IpfsImagesList::Url, blue);
}

void IpfsImagesListViewItem::setTitle(const QString& str)
{
    setText(IpfsImagesList::Title, str);
}

QString IpfsImagesListViewItem::Title() const
{
    return text(IpfsImagesList::Title);
}

void IpfsImagesListViewItem::setDescription(const QString& str)
{
    setText(IpfsImagesList::Description, str);
}

QString IpfsImagesListViewItem::Description() const
{
    return text(IpfsImagesList::Description);
}

void IpfsImagesListViewItem::setIpfsUrl(const QString& str)
{
    setText(IpfsImagesList::Url, str);
}

QString IpfsImagesListViewItem::IpfsUrl() const
{
    return text(IpfsImagesList::Url);
}

} // namespace Digikam
