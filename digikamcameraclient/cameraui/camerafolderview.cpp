/* ============================================================
 * File  : camerafolderview.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-23
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>

#include "camerafolderitem.h"
#include "camerafolderview.h"


CameraFolderView::CameraFolderView(QWidget* parent)
    : KListView(parent)
{
    addColumn(i18n("Camera Folders"));
    setFullWidth(true);
    setDragEnabled(false);
    setDropVisualizer(false);
    setDropHighlighter(false);
    setAcceptDrops(true);

    cameraName_ = "Camera";
    virtualFolder_ = 0;
    rootFolder_    = 0;

    setupConnections();

}

CameraFolderView::~CameraFolderView()
{
    
}


void CameraFolderView::setupConnections()
{
    connect(this, SIGNAL(selectionChanged(QListViewItem*)),
            this, SLOT(slotSelectionChanged(QListViewItem*)));
}

void CameraFolderView::addVirtualFolder(const QString& name)
{
    cameraName_ = name;
    virtualFolder_ = new CameraFolderItem(this, cameraName_);
    virtualFolder_->setOpen(true);
}

void CameraFolderView::addRootFolder(const QString& folder)
{
    rootFolder_ = new CameraFolderItem(virtualFolder_,
                                       folder, folder);
    rootFolder_->setOpen(true);
}

CameraFolderItem* CameraFolderView::addFolder(const QString& folder,
                                              const QString& subFolder)
{
    CameraFolderItem *parentItem = findFolder(folder);

    kdDebug() << "CameraFolderView: Adding Subfolder " << subFolder
              << " of folder " << folder << endl;
    
    if (parentItem) {
        QString path(folder);
        if (!folder.endsWith("/"))
            path += "/";
        path += subFolder;
        CameraFolderItem* item = new CameraFolderItem(parentItem, subFolder,
                                                      path);
        kdDebug() << "CameraFolderView: Added ViewItem with path "
                  << item->folderPath() << endl;
        item->setOpen(true);
        return item;
    }
    else {
        kdWarning() << "CameraFolderView: Couldn't find parent for subFolder "
                    << subFolder << " of folder " << folder << endl;
        return 0;
    }
}

CameraFolderItem* CameraFolderView::findFolder(const QString& folderPath)
{

    QListViewItemIterator it(this);
    for ( ; it.current(); ++it) {

        CameraFolderItem* item =
            static_cast<CameraFolderItem*>(it.current());

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
    return virtualFolder_;    
}

CameraFolderItem* CameraFolderView::rootFolder()
{
    return rootFolder_;
}

void CameraFolderView::clear()
{
    KListView::clear();
    virtualFolder_ = 0;
    rootFolder_    = 0;
    emit signalCleared();
}
