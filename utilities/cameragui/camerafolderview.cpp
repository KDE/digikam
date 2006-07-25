/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2003-01-23
 * Description : A widget to display a list of camera folders.
 * 
 * Copyright 2003-2005 by Renchi Raju
 * Copyright 2006 by Gilles Caulier
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

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>

// Local includes.

#include "camerafolderitem.h"
#include "camerafolderview.h"

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

    d->cameraName    = "Camera";
    d->virtualFolder = 0;
    d->rootFolder    = 0;

    setupConnections();
}

CameraFolderView::~CameraFolderView()
{
    delete d;
}

void CameraFolderView::setupConnections()
{
    connect(this, SIGNAL(selectionChanged(QListViewItem*)),
            this, SLOT(slotSelectionChanged(QListViewItem*)));
}

void CameraFolderView::addVirtualFolder(const QString& name)
{
    d->cameraName    = name;
    d->virtualFolder = new CameraFolderItem(this, d->cameraName);
    d->virtualFolder->setOpen(true);
}

void CameraFolderView::addRootFolder(const QString& folder)
{
    d->rootFolder = new CameraFolderItem(d->virtualFolder, folder, folder);
    d->rootFolder->setOpen(true);
}

CameraFolderItem* CameraFolderView::addFolder(const QString& folder, const QString& subFolder)
{
    CameraFolderItem *parentItem = findFolder(folder);

    kdDebug() << "CameraFolderView: Adding Subfolder " << subFolder
              << " of folder " << folder << endl;
    
    if (parentItem) 
    {
        QString path(folder);
        if (!folder.endsWith("/"))
            path += "/";
        path += subFolder;
        CameraFolderItem* item = new CameraFolderItem(parentItem, subFolder, path);
        kdDebug() << "CameraFolderView: Added ViewItem with path "
                  << item->folderPath() << endl;
        item->setOpen(true);
        return item;
    }
    else 
    {
        kdWarning() << "CameraFolderView: Couldn't find parent for subFolder "
                    << subFolder << " of folder " << folder << endl;
        return 0;
    }
}

CameraFolderItem* CameraFolderView::findFolder(const QString& folderPath)
{

    QListViewItemIterator it(this);
    for ( ; it.current(); ++it) 
    {
        CameraFolderItem* item = static_cast<CameraFolderItem*>(it.current());

        if (item->folderPath() == folderPath)
            return item;
    }

    return 0;
}

void CameraFolderView::slotSelectionChanged(QListViewItem* item)
{
    if (!item) return;
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

#include "camerafolderview.h"
