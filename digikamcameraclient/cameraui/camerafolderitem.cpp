/* ============================================================
 * File  : camerafolderitem.cpp
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

#include <kiconloader.h>

#include "camerafolderitem.h"


CameraFolderItem::CameraFolderItem(KListView* parent,
                                   const QString& name)
    : KListViewItem(parent, name)
{
    setPixmap(0, SmallIcon("folder"));
    virtualFolder_ = true;
    count_  = 0;
    name_   = name;
}

CameraFolderItem::CameraFolderItem(KListViewItem* parent,
                                   const QString& folderName,
                                   const QString& folderPath)
    : KListViewItem(parent, folderName)
{
    setPixmap(0, SmallIcon("folder"));
    folderName_ = folderName;
    folderPath_ = folderPath;
    virtualFolder_ = false;
    count_ = 0;
    name_  = folderName;
}

CameraFolderItem::~CameraFolderItem()
{
    
}

bool CameraFolderItem::isVirtualFolder()
{
    return virtualFolder_;    
}

QString CameraFolderItem::folderName()
{
    return folderName_;
}

QString CameraFolderItem::folderPath()
{
    return folderPath_;
}

// void CameraFolderItem::setCount(int count)
// {
    
//     setText(folderName_ + " (" + QString::number(count) + ")");    
// }

void CameraFolderItem::changeCount(int val)
{
    count_ += val;
    setText(0, name_ + " (" + QString::number(count_) + ")");    
}

void CameraFolderItem::setCount(int val)
{
    count_ = val;    
    setText(0, name_ + " (" + QString::number(count_) + ")");
}

int CameraFolderItem::count()
{
    return count_;    
}
