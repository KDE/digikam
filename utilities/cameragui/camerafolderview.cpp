/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-23
 * Description : A widget to display a list of camera folders.
 * 
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

// KDE includes.

#include <klocale.h>
#include <kiconloader.h>

// Local includes.

#include "ddebug.h"
#include "camerafolderitem.h"
#include "camerafolderview.h"
#include "camerafolderview.moc"
//Added by qt3to4:
#include <QPixmap>

namespace Digikam
{

class CameraFolderViewPriv
{
public:

    CameraFolderViewPriv()
    {
        virtualFolder = 0;
        rootFolder    = 0;
    }

    QString           cameraName;

    CameraFolderItem *virtualFolder;
    CameraFolderItem *rootFolder;
};

CameraFolderView::CameraFolderView(QWidget* parent)
                : KListView(parent)
{
    d = new CameraFolderViewPriv;
    addColumn(i18n("Camera Folders"));
    setFullWidth(true);
    setDragEnabled(false);
    setDropVisualizer(false);
    setDropHighlighter(false);
    setAcceptDrops(true);
    setSelectionMode(Q3ListView::Single);

    d->cameraName    = "Camera";
    d->virtualFolder = 0;
    d->rootFolder    = 0;

    connect(this, SIGNAL(currentChanged(Q3ListViewItem*)),
            this, SLOT(slotCurrentChanged(Q3ListViewItem*)));

    connect(this, SIGNAL(clicked(Q3ListViewItem*)),
            this, SLOT(slotCurrentChanged(Q3ListViewItem*)));
}

CameraFolderView::~CameraFolderView()
{
    delete d;
}

void CameraFolderView::addVirtualFolder(const QString& name, const QPixmap& pixmap)
{
    d->cameraName    = name;
    d->virtualFolder = new CameraFolderItem(this, d->cameraName, pixmap);
    d->virtualFolder->setOpen(true);
    d->virtualFolder->setSelected(false);
    d->virtualFolder->setSelectable(false);
}

void CameraFolderView::addRootFolder(const QString& folder, int nbItems, const QPixmap& pixmap)
{
    d->rootFolder = new CameraFolderItem(d->virtualFolder, folder, folder, pixmap);
    d->rootFolder->setOpen(true);
    d->rootFolder->setCount(nbItems);
}

CameraFolderItem* CameraFolderView::addFolder(const QString& folder, const QString& subFolder,
                                              int nbItems, const QPixmap& pixmap)
{
    CameraFolderItem *parentItem = findFolder(folder);

    DDebug() << "CameraFolderView: Adding Subfolder " << subFolder
              << " of folder " << folder << endl;
    
    if (parentItem) 
    {
        QString path(folder);

        if (!folder.endsWith("/"))
            path += '/';

        path += subFolder;
        CameraFolderItem* item = new CameraFolderItem(parentItem, subFolder, path, pixmap);
        
        DDebug() << "CameraFolderView: Added ViewItem with path "
                  << item->folderPath() << endl;

        item->setCount(nbItems);
        item->setOpen(true);
        return item;
    }
    else 
    {
        DWarning() << "CameraFolderView: Couldn't find parent for subFolder "
                    << subFolder << " of folder " << folder << endl;
        return 0;
    }
}

CameraFolderItem* CameraFolderView::findFolder(const QString& folderPath)
{

    Q3ListViewItemIterator it(this);
    for ( ; it.current(); ++it) 
    {
        CameraFolderItem* item = static_cast<CameraFolderItem*>(it.current());

        if (item->folderPath() == folderPath)
            return item;
    }

    return 0;
}

void CameraFolderView::slotCurrentChanged(Q3ListViewItem* item)
{
    if (!item) 
        emit signalFolderChanged(0);
    else
        emit signalFolderChanged(static_cast<CameraFolderItem *>(item));
}

CameraFolderItem* CameraFolderView::virtualFolder()
{
    return d->virtualFolder;    
}

CameraFolderItem* CameraFolderView::rootFolder()
{
    return d->rootFolder;
}

void CameraFolderView::clear()
{
    KListView::clear();
    d->virtualFolder = 0;
    d->rootFolder    = 0;
    emit signalCleared();
}

} // namespace Digikam

