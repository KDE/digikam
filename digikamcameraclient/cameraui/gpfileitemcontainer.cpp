/* ============================================================
 * File  : gpfileitemcontainer.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-21
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
#include <qstring.h>

#include "camerafolderview.h"
#include "camerafolderitem.h"
#include "cameraiconview.h"
#include "cameraiconitem.h"
#include "gpfileitemcontainer.h"


GPFileItemContainer::GPFileItemContainer(QObject *parent,
                                         CameraFolderView *folderView,
                                         CameraIconView   *iconView)
    : QObject(parent)
{
    folderView_ = folderView;
    iconView_   = iconView;
    folderDict_.setAutoDelete(true);

    connect(folderView_, SIGNAL(signalCleared()),
            this, SLOT(slotFolderViewCleared()));
    connect(iconView_, SIGNAL(signalCleared()),
            this, SLOT(slotIconViewCleared()));
}

GPFileItemContainer::~GPFileItemContainer()
{
    
}

void GPFileItemContainer::addVirtualFolder(const QString& title)
{
    folderView_->addVirtualFolder(title);    
}

void GPFileItemContainer::addRootFolder(const QString& folder)
{
    folderView_->addRootFolder(folder);

    GPFolder *item = new GPFolder;
    item->viewItem = folderView_->rootFolder();
    item->viewItem->setCount(0);
    folderDict_.insert(folder, item);
}

void GPFileItemContainer::addFolder(const QString& folder,
                                    const QString& subfolder)
{
    QString path(folder);
    if (!path.endsWith("/"))
        path += "/";
    path += subfolder;

    kdDebug() << "GPFileItemContainer: Adding folder " << path << endl;
    
    if (!folderDict_.find(path)) {
        GPFolder *item = new GPFolder;
        folderDict_.insert(path, item);
        item->viewItem = folderView_->addFolder(folder,
                                                subfolder);
	if (item->viewItem)
        	item->viewItem->setCount(0);
    }
}

void GPFileItemContainer::addFiles(const QString& folder,
                                   const GPFileItemInfoList& infoList)
{
    GPFolder *folderItem = folderDict_.find(folder);
    if (!folderItem) {
        kdWarning() << "GPFileItemContainer: "
                    << "Couldn't find Folder in Dict: " << folder
                    << endl;
        return;
    }

    GPFileDict* fileDict = folderItem->fileDict;

    GPFileItemInfoList::const_iterator it;
    for (it = infoList.begin(); it != infoList.end(); it++) {

        GPFileItemInfo *fileInfo = fileDict->find((*it).name);
        
        if (!fileInfo) {
            // Hmm... Totally New file
            fileInfo = new GPFileItemInfo((*it));
            fileDict->insert((*it).name, fileInfo);
            // Update the count for the correspong folderviewitem
            if (folderItem->viewItem) 
                folderItem->viewItem->changeCount(1);
            // Also update the count of the virtual folder
            if (folderView_->virtualFolder()) 
                folderView_->virtualFolder()->changeCount(1);
        }

        // Have to check here: as when changing thumbnailsize,
        // we cannot cancel the controller and so might end up
        // having two items of the same type in the iconview
        if (!fileInfo->viewItem) {
            CameraIconItem *iconItem = iconView_->addItem(fileInfo);
            fileInfo->viewItem = iconItem;
        }
            
    }
    
}

// Files to be added to the virtual folder

void GPFileItemContainer::addFiles(const GPFileItemInfoList& infoList)
{
    if (!folderView_->virtualFolder()) {
        kdWarning() << "GPFileItemContainer: "
                    << "Virtual Folder not created yet"
                    << endl;
        return;
    }

    GPFileItemInfoList::const_iterator it;
    for (it = infoList.begin(); it != infoList.end(); it++) {
        GPFileItemInfo info(*it);

        GPFolder *folderItem = folderDict_.find(info.folder);
        if (!folderItem) {
            kdWarning() << "GPFileItemContainer: "
                        << "Couldn't find Folder in Dict: " << info.folder
                        << endl;
            continue;
        }
        
        GPFileDict* fileDict = folderItem->fileDict;
        GPFileItemInfo *fileInfo = fileDict->find((*it).name);
        
        if (!fileInfo) {
            // Hmm... Totally New file
            fileInfo = new GPFileItemInfo(info);
            fileDict->insert((*it).name, fileInfo);
            // Update the count for the correspong folderviewitem
            if (folderItem->viewItem) 
                folderItem->viewItem->changeCount(1);
            // Also update the count of the virtual folder
            if (folderView_->virtualFolder()) 
                folderView_->virtualFolder()->changeCount(1);
        }

        if (!fileInfo->viewItem) {
            CameraIconItem *iconItem = iconView_->addItem(fileInfo);
            fileInfo->viewItem = iconItem;
        }
            
    }
    
}

void GPFileItemContainer::addFile(const QString& folder,
                                  const GPFileItemInfo& info)
{
    GPFolder *folderItem = folderDict_.find(folder);
    if (!folderItem) {
        kdWarning() << "GPFileItemContainer: "
                    << "Couldn't find Folder in Dict: " << folder
                    << endl;
        return;
    }

    GPFileDict* fileDict = folderItem->fileDict;
    GPFileItemInfo *fileInfo = fileDict->find(info.name);
    
    if (!fileInfo) {
        // Hmm... Totally New file
        fileInfo = new GPFileItemInfo(info);
        fileDict->insert(info.name, fileInfo);
        // Update the count for the correspong folderviewitem
        if (folderItem->viewItem) 
            folderItem->viewItem->changeCount(1);
        // Also update the count of the virtual folder
        if (folderView_->virtualFolder()) 
            folderView_->virtualFolder()->changeCount(1);
    }

    if (!fileInfo->viewItem) {
        CameraIconItem *iconItem = iconView_->addItem(fileInfo);
        fileInfo->viewItem = iconItem;
    }
    
}

void GPFileItemContainer::delFile(const QString& folder,
                                  const QString& name)
{
    GPFolder *folderItem = folderDict_.find(folder);
    if (!folderItem) {
        kdWarning() << "GPFileItemContainer: "
                    << "Couldn't find Folder in Dict: " << folder
                    << endl;
        return;
    }

    GPFileDict* fileDict = folderItem->fileDict;
    GPFileItemInfo* fileInfo = fileDict->find(name);
    if (!fileInfo) {
        kdWarning() << "GPFileItemContainer: "
                    << "Couldn't File Item to Delete in Dict: " << name
                    << endl;
        return;
    }

    if (fileInfo->viewItem) {
        CameraIconItem *iconItem = (CameraIconItem*) fileInfo->viewItem;
        delete iconItem;
    }
    
    fileDict->remove(name);
    // Update the count for the correspong folderviewitem
    if (folderItem->viewItem) 
        folderItem->viewItem->changeCount(-1);
    // Also update the count of the virtual folder
    if (folderView_->virtualFolder()) 
        folderView_->virtualFolder()->changeCount(-1);
    
}

void GPFileItemContainer::delAllFiles()
{
    iconView_->clear();
    
    GPFolderDictIterator it(folderDict_);
    for (; it.current(); ++it) {

        GPFolder* folderItem = it.current();
        folderItem->fileDict->clear();
        if (folderItem->viewItem)
            folderItem->viewItem->setCount(0);
    }

    if (folderView_->virtualFolder())
        folderView_->virtualFolder()->setCount(0);
}


CameraIconItem* GPFileItemContainer::findItem(const QString& folder,
                                              const QString& name)
{
    GPFolder *folderItem = folderDict_.find(folder);
    if (!folderItem) {
        kdWarning() << "GPFileItemContainer: "
                    << "Couldn't find Folder in Dict: " << folder
                    << endl;
        return 0;
    }

    GPFileDict* fileDict = folderItem->fileDict;
    GPFileItemInfo* fileInfo = fileDict->find(name);
    if (!fileInfo) {
        kdWarning() << "GPFileItemContainer: "
                    << "Couldn't File Item to Delete in Dict: " << name
                    << endl;
        return 0;
    }
    
    return (CameraIconItem*) fileInfo->viewItem;
}

QPtrList<GPFileItemInfo> GPFileItemContainer::allFiles()
{
    QPtrList<GPFileItemInfo> ptrList;
    
    GPFolderDictIterator it(folderDict_);
    for (; it.current(); ++it) {
        GPFolder* folderItem = it.current();

        GPFileDictIterator iter(*(folderItem->fileDict));
        for (; iter.current(); ++iter) {
            ptrList.append(iter.current());
        }
    }

    return ptrList;
}

void GPFileItemContainer::slotFolderViewCleared()
{
    folderDict_.clear();    
}

void GPFileItemContainer::slotIconViewCleared()
{
    // Uh Oh... expensive process

    // Zero out all the view items 
    
    GPFolderDictIterator it(folderDict_);
    for (; it.current(); ++it) {
        GPFolder* folderItem = it.current();

        GPFileDictIterator iter(*(folderItem->fileDict));
        for (; iter.current(); ++iter) {
            GPFileItemInfo *fileInfo = iter.current();
            fileInfo->viewItem = 0;
        }
    }
    
}
