/* ============================================================
 * File  : camerafolderitem.h
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

#ifndef CAMERAFOLDERITEM_H
#define CAMERAFOLDERITEM_H

#include <klistview.h>
#include <qstring.h>

class CameraFolderItem : public KListViewItem
{
public:

    CameraFolderItem(KListView* parent,
                     const QString& name);

    CameraFolderItem(KListViewItem* parent,
                     const QString& folderName,
                     const QString& folderPath);

    ~CameraFolderItem();

    QString folderName();
    QString folderPath();
    bool    isVirtualFolder();
    void    changeCount(int val);
    void    setCount(int val);
    int     count();
    
private:

    QString folderName_;
    QString folderPath_;
    QString name_;
    bool    virtualFolder_;
    int     count_;

    
};

#endif /* CAMERAFOLDERITEM_H */
