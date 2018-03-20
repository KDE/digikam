/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : digiKam image editor GUI
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEWINDOW_P_H
#define IMAGEWINDOW_P_H

// Local includes

#include "versionmanager.h"

namespace Digikam
{

class DatabaseVersionManager : public VersionManager
{
public:

    virtual QString toplevelDirectory(const QString& path)
    {
        CollectionLocation loc = CollectionManager::instance()->locationForPath(path);

        if (!loc.isNull())
        {
            return loc.albumRootPath();
        }

        return QLatin1String("/");
    }
};

// -----------------------------------------------------------------------------------------

class ImageWindow::Private
{

public:

    Private() :
        viewContainer(0),
        toMainWindowAction(0),
        fileDeletePermanentlyAction(0),
        fileDeletePermanentlyDirectlyAction(0),
        fileTrashDirectlyAction(0),
        imageInfoModel(0),
        imageFilterModel(0),
        dragDropHandler(0),
        thumbBar(0),
        thumbBarDock(0),
        rightSideBar(0)
    {
    }

    QModelIndex currentIndex() const
    {
        return imageFilterModel->indexForImageInfo(currentImageInfo);
    }

    QModelIndex currentSourceIndex() const
    {
        return imageInfoModel->indexForImageInfo(currentImageInfo);
    }

    bool currentIsValid() const
    {
        return !currentImageInfo.isNull();
    }

    QUrl currentUrl() const
    {
        return currentImageInfo.fileUrl();
    }

    QModelIndex nextIndex() const
    {
        return imageFilterModel->index(currentIndex().row() + 1, 0);
    }

    QModelIndex previousIndex() const
    {
        return imageFilterModel->index(currentIndex().row() - 1, 0);
    }

    QModelIndex firstIndex() const
    {
        return imageFilterModel->index(0, 0);
    }

    QModelIndex lastIndex() const
    {
        return imageFilterModel->index(imageFilterModel->rowCount() - 1, 0);
    }

    ImageInfo imageInfo(const QModelIndex& index) const
    {
        return imageFilterModel->imageInfo(index);
    }

    void setThumbBarToCurrent()
    {
        QModelIndex index = imageFilterModel->indexForImageInfo(currentImageInfo);

        if (index.isValid())
        {
            thumbBar->setCurrentIndex(index);
        }
        else
        {
            thumbBar->setCurrentWhenAvailable(currentImageInfo.id());
        }
    }

    void ensureModelContains(const ImageInfo& info)
    {
        if (!imageInfoModel->hasImage(info))
        {
            imageInfoModel->addImageInfoSynchronously(info);
            imageFilterModel->sort(imageFilterModel->sortColumn());
        }
    }

public:

    static const QString         configShowThumbbarEntry;
    static const QString         configHorizontalThumbbarEntry;

    KMainWindow*                 viewContainer;

    QAction*                     toMainWindowAction;

    // Delete actions
    QAction*                     fileDeletePermanentlyAction;
    QAction*                     fileDeletePermanentlyDirectlyAction;
    QAction*                     fileTrashDirectlyAction;

    ImageInfo                    currentImageInfo;
    ImageListModel*              imageInfoModel;
    ImageFilterModel*            imageFilterModel;
    ImageDragDropHandler*        dragDropHandler;

    ImageThumbnailBar*           thumbBar;
    ThumbBarDock*                thumbBarDock;

    ImagePropertiesSideBarDB*    rightSideBar;

    DatabaseVersionManager       versionManager;

    QMultiMap<QString, QVariant> newFaceTags;
};

const QString ImageWindow::Private::configShowThumbbarEntry(QLatin1String("Show Thumbbar"));
const QString ImageWindow::Private::configHorizontalThumbbarEntry(QLatin1String("HorizontalThumbbar"));

}  // namespace Digikam

#endif // IMAGEWINDOW_P_H
