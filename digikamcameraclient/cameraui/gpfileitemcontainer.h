/* ============================================================
 * File  : gpfileitemcontainer.h
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

#ifndef GPFILEITEMCONTAINER_H
#define GPFILEITEMCONTAINER_H

#include <qobject.h>
#include <qdict.h>
#include <qptrlist.h>

#include "gpfileiteminfo.h"


class QString;
class CameraFolderItem;
class CameraFolderView;
class CameraIconView;
class CameraIconItem;

class GPFileItemContainer : public QObject
{
    Q_OBJECT

public:

    GPFileItemContainer(QObject *parent,
                        CameraFolderView *folderView,
                        CameraIconView   *iconView);
    ~GPFileItemContainer();

    void addVirtualFolder(const QString& title);
    void addRootFolder(const QString& folder);
    void addFolder(const QString& folder,
                   const QString& subfolder);

    void addFiles(const QString& folder,
                  const GPFileItemInfoList& infoList);
    void addFiles(const GPFileItemInfoList& infoList);
    void addFile(const QString& folder,
                 const GPFileItemInfo& info);

    void delFile(const QString& folder,
                 const QString& name);
    void delAllFiles();

    CameraIconItem* findItem(const QString& folder,
                             const QString& name);

    QPtrList<GPFileItemInfo> allFiles();
    
    
private:

    typedef QDict<GPFileItemInfo>  GPFileDict;
    typedef QDictIterator<GPFileItemInfo> GPFileDictIterator;
    
    class GPFolder
    {
    public:
        GPFolder() {
            viewItem = 0;
            fileDict = new GPFileDict(307);
            fileDict->setAutoDelete(true);
        }

        ~GPFolder() {
            if (fileDict)
                delete fileDict;
        }
        
        GPFileDict        *fileDict;
        CameraFolderItem  *viewItem;
    };
    
    typedef QDict<GPFolder>  GPFolderDict;
    typedef QDictIterator<GPFolder> GPFolderDictIterator;
    
    GPFolderDict      folderDict_;
    CameraFolderView *folderView_;
    CameraIconView   *iconView_;

private slots:

    void slotFolderViewCleared();
    void slotIconViewCleared();
    
};

#endif /* GPFILEITEMCONTAINER_H */
