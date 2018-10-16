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

#ifndef DIGIKAM_IPFS_IMAGES_LIST_H
#define DIGIKAM_IPFS_IMAGES_LIST_H

// Qt includes

#include <QWidget>

// Local includes

#include "dimageslist.h"
#include "ipfstalker.h"

namespace Digikam
{

class IpfsImagesListViewItem;

class IpfsImagesList : public DImagesList
{
    Q_OBJECT

public:

    /* The different columns in a list.
     */
    enum FieldType
    {
        Title       = DImagesListView::User1,
        Description = DImagesListView::User2,
        Url         = DImagesListView::User3,
    };

    explicit IpfsImagesList(QWidget* const parent = 0);
    ~IpfsImagesList() Q_DECL_OVERRIDE {}

    QList<const IpfsImagesListViewItem*> getPendingItems();

public Q_SLOTS:

    void slotAddImages(const QList<QUrl>& list) override;
    void slotSuccess(const IpfsTalkerResult& result);
    void slotDoubleClick(QTreeWidgetItem* element, int i);
};

// -------------------------------------------------------------------------

class IpfsImagesListViewItem : public DImagesListViewItem
{
public:

    explicit IpfsImagesListViewItem(DImagesListView* const view, const QUrl& url);
    ~IpfsImagesListViewItem() Q_DECL_OVERRIDE {}

    void setTitle(const QString& str);
    QString Title() const;

    void setDescription(const QString& str);
    QString Description() const;

    void setIpfsUrl(const QString& str);
    QString IpfsUrl() const;
};

} // namespace Digikam

#endif // DIGIKAM_IPFS_IMAGES_LIST_H
