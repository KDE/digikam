/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-23
 * Description : A widget to display a list of camera folders.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "camerafolderview.moc"

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "camerafolderitem.h"

namespace Digikam
{

class CameraFolderView::Private
{
public:

    Private() :
        cameraName("Camera"),
        virtualFolder(0),
        rootFolder(0)
    {
    }

    QString           cameraName;

    CameraFolderItem* virtualFolder;
    CameraFolderItem* rootFolder;
};

CameraFolderView::CameraFolderView(QWidget* const parent)
    : QTreeWidget(parent), d(new Private)
{
    setColumnCount(1);
    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setDragEnabled(false);
    setDropIndicatorShown(false);
    setAcceptDrops(false);
    setHeaderLabels(QStringList() << i18n("Camera Folders"));

    connect(this, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(slotCurrentChanged(QTreeWidgetItem*,int)));

    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotCurrentChanged(QTreeWidgetItem*,int)));
}

CameraFolderView::~CameraFolderView()
{
    delete d;
}

void CameraFolderView::addVirtualFolder(const QString& name, const QPixmap& pixmap)
{
    d->cameraName    = name;
    d->virtualFolder = new CameraFolderItem(this, d->cameraName, pixmap);
    d->virtualFolder->setExpanded(true);
    d->virtualFolder->setSelected(false);
    // item is not selectable.
    d->virtualFolder->setFlags(d->virtualFolder->flags() & (int)!Qt::ItemIsSelectable);
    d->virtualFolder->setDisabled(false);
}

void CameraFolderView::addRootFolder(const QString& folder, int nbItems, const QPixmap& pixmap)
{
    d->rootFolder = new CameraFolderItem(d->virtualFolder, folder, folder, pixmap);
    d->rootFolder->setExpanded(true);

    if (nbItems != -1)
    {
        d->rootFolder->setCount(nbItems);
    }
}

CameraFolderItem* CameraFolderView::addFolder(const QString& folder, const QString& subFolder,
                                              int nbItems, const QPixmap& pixmap)
{
    CameraFolderItem* parentItem = findFolder(folder);

    kDebug() << "Adding Subfolder " << subFolder
             << " of folder " << folder;

    if (parentItem)
    {
        QString path(folder);

        if (!folder.endsWith('/'))
        {
            path += '/';
        }

        path += subFolder;
        CameraFolderItem* item = new CameraFolderItem(parentItem, subFolder, path, pixmap);

        kDebug() << "Added ViewItem with path "
                 << item->folderPath();

        item->setCount(nbItems);
        item->setExpanded(true);
        return item;
    }
    else
    {
        kWarning() << "Could not find parent for subFolder "
                   << subFolder << " of folder " << folder;
        return 0;
    }
}

CameraFolderItem* CameraFolderView::findFolder(const QString& folderPath)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        CameraFolderItem* lvItem = dynamic_cast<CameraFolderItem*>(*it);

        if (lvItem && lvItem->folderPath() == folderPath)
        {
            return lvItem;
        }

        ++it;
    }

    return 0;
}

void CameraFolderView::slotCurrentChanged(QTreeWidgetItem* item, int)
{
    if (!item)
    {
        emit signalFolderChanged(0);
    }
    else
    {
        emit signalFolderChanged(dynamic_cast<CameraFolderItem*>(item));
    }
}

CameraFolderItem* CameraFolderView::virtualFolder() const
{
    return d->virtualFolder;
}

CameraFolderItem* CameraFolderView::rootFolder() const
{
    return d->rootFolder;
}

void CameraFolderView::clear()
{
    QTreeWidget::clear();
    d->virtualFolder = 0;
    d->rootFolder    = 0;
    emit signalCleared();
}

} // namespace Digikam
