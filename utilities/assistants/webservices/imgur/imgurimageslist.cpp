/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-04
 * Description : a tool to export images to Imgur web service
 *
 * Copyright (C) 2010-2012 by Marius Orcsik <marius at habarnam dot ro>
 * Copyright (C) 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "imgurimageslist.h"

// C++ includes

#include <memory>

// Qt includes

#include <QLabel>
#include <QPointer>
#include <QDesktopServices>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dmetadata.h"
#include "dinfointerface.h"
#include "digikam_debug.h"

namespace Digikam
{

ImgurImagesList::ImgurImagesList(QWidget* const parent)
    : DImagesList(parent)
{
    setControlButtonsPlacement(DImagesList::ControlButtonsBelow);
    setAllowDuplicate(false);
    setAllowRAW(false);

    auto* const list = listView();

    list->setColumnLabel(DImagesListView::Thumbnail, i18n("Thumbnail"));

    list->setColumnLabel(static_cast<DImagesListView::ColumnType>(ImgurImagesList::Title),
                         i18n("Submission title"));

    list->setColumnLabel(static_cast<DImagesListView::ColumnType>(ImgurImagesList::Description),
                         i18n("Submission description"));

    list->setColumn(static_cast<DImagesListView::ColumnType>(ImgurImagesList::URL),
                    i18n("Imgur URL"), true);

    list->setColumn(static_cast<DImagesListView::ColumnType>(ImgurImagesList::DeleteURL),
                    i18n("Imgur Delete URL"), true);

    connect(list, &DImagesListView::itemDoubleClicked,
            this, &ImgurImagesList::slotDoubleClick);
}

QList<const ImgurImageListViewItem*> ImgurImagesList::getPendingItems()
{
    QList<const ImgurImageListViewItem*> ret;

    for (unsigned int i = listView()->topLevelItemCount(); i--;)
    {
        const auto* item = dynamic_cast<const ImgurImageListViewItem*>(listView()->topLevelItem(i));

        if (item && item->ImgurUrl().isEmpty())
            ret << item;
    }

    return ret;
}

void ImgurImagesList::slotAddImages(const QList<QUrl>& list)
{
    /* Replaces the DImagesList::slotAddImages method, so that
     * ImgurImageListViewItems can be added instead of ImagesListViewItems */

    DMetadata meta;

    for (QList<QUrl>::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
    {
        // Already in the list?
        if (listView()->findItem(*it) == nullptr)
        {
            auto* const item = new ImgurImageListViewItem(listView(), *it);

            // Load URLs from meta data, if possible
            if (meta.load((*it).toLocalFile()))
            {
                item->setImgurUrl(meta.getXmpTagString("Xmp.digiKam.ImgurId"));
                item->setImgurDeleteUrl(meta.getXmpTagString("Xmp.digiKam.ImgurDeleteHash"));
            }
        }
    }

    emit signalImageListChanged();
    emit signalAddItems(list);
}

void ImgurImagesList::slotSuccess(const ImgurTalkerResult& result)
{
    QUrl imgurl = QUrl::fromLocalFile(result.action->upload.imgpath);

    processed(imgurl, true);

    DMetadata meta;

    // Save URLs to meta data, if possible
    if (meta.load(imgurl.toLocalFile()))
    {
        meta.setXmpTagString("Xmp.digiKam.ImgurId",
                             result.image.url);
        meta.setXmpTagString("Xmp.digiKam.ImgurDeleteHash",
                             ImgurTalker::urlForDeletehash(result.image.deletehash).toString());
        meta.setMetadataWritingMode((int)DMetadata::WRITETOIMAGEONLY);
        bool saved = meta.applyChanges();

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Metadata"
                                         << (saved ? "Saved" : "Not Saved")
                                         << "to" << imgurl;
    }

    ImgurImageListViewItem* const currItem = dynamic_cast<ImgurImageListViewItem*>(listView()->findItem(imgurl));

    if (!currItem)
        return;

    if (!result.image.url.isEmpty())
        currItem->setImgurUrl(result.image.url);

    if (!result.image.deletehash.isEmpty())
        currItem->setImgurDeleteUrl(ImgurTalker::urlForDeletehash(result.image.deletehash).toString());
}

void ImgurImagesList::slotDoubleClick(QTreeWidgetItem* element, int i)
{
    if (i == URL || i == DeleteURL)
    {
        const QUrl url = QUrl(element->text(i));
        // The delete page asks for confirmation, so we don't need to do that here
        QDesktopServices::openUrl(url);
    }
}

// ------------------------------------------------------------------------------------------------

ImgurImageListViewItem::ImgurImageListViewItem(DImagesListView* const view, const QUrl& url)
    : DImagesListViewItem(view, url)
{
    const QColor blue(50, 50, 255);

    setTextColor(ImgurImagesList::URL,       blue);
    setTextColor(ImgurImagesList::DeleteURL, blue);
}

void ImgurImageListViewItem::setTitle(const QString& str)
{
    setText(ImgurImagesList::Title, str);
}

QString ImgurImageListViewItem::Title() const
{
    return text(ImgurImagesList::Title);
}

void ImgurImageListViewItem::setDescription(const QString& str)
{
    setText(ImgurImagesList::Description, str);
}

QString ImgurImageListViewItem::Description() const
{
    return text(ImgurImagesList::Description);
}

void ImgurImageListViewItem::setImgurUrl(const QString& str)
{
    setText(ImgurImagesList::URL, str);
}

QString ImgurImageListViewItem::ImgurUrl() const
{
    return text(ImgurImagesList::URL);
}

void ImgurImageListViewItem::setImgurDeleteUrl(const QString& str)
{
    setText(ImgurImagesList::DeleteURL, str);
}

QString ImgurImageListViewItem::ImgurDeleteUrl() const
{
    return text(ImgurImagesList::DeleteURL);
}

} // namespace Digikam
