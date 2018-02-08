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

#ifndef IMGUR_IMAGES_LIST_H
#define IMGUR_IMAGES_LIST_H

// Qt includes

#include <QWidget>

// Local includes

#include "dimageslist.h"
#include "imgurtalker.h"

namespace Digikam
{

class ImgurImageListViewItem;

class ImgurImagesList : public DImagesList
{
    Q_OBJECT

public:

    // The different columns in a list.
    enum FieldType
    {
        Title           = DImagesListView::User1,
        Description     = DImagesListView::User2,
        URL             = DImagesListView::User3,
        DeleteURL       = DImagesListView::User4
    };

public:

    explicit ImgurImagesList(QWidget* const parent = 0);
    ~ImgurImagesList() override {}

    QList<const ImgurImageListViewItem*> getPendingItems();

public Q_SLOTS:

    void slotAddImages(const QList<QUrl>& list) override;
    void slotSuccess(const ImgurTalkerResult& result);
    void slotDoubleClick(QTreeWidgetItem* element, int i);
};

// -------------------------------------------------------------------------

class ImgurImageListViewItem : public DImagesListViewItem
{
public:

    explicit ImgurImageListViewItem(DImagesListView* const view, const QUrl& url);
    ~ImgurImageListViewItem() override {}

    void setTitle(const QString& str);
    QString Title() const;

    void setDescription(const QString& str);
    QString Description() const;

    void setImgurUrl(const QString& str);
    QString ImgurUrl() const;

    void setImgurDeleteUrl(const QString& str);
    QString ImgurDeleteUrl() const;
};

} // namespace Digikam

#endif // IMGUR_IMAGES_LIST_H
