/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-23
 * Description : A widget to display a camera folder.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "camerafolderitem.h"

namespace Digikam
{

class CameraFolderItemPriv
{
public:

    CameraFolderItemPriv() :
        virtualFolder(true),
        count(0)
    {
    }

    bool    virtualFolder;
    int     count;

    QString folderName;
    QString folderPath;
    QString name;
};

CameraFolderItem::CameraFolderItem(QTreeWidget* parent, const QString& name, const QPixmap& pixmap)
    : QTreeWidgetItem(parent), d(new CameraFolderItemPriv)
{
    d->name = name;
    setIcon(0, pixmap);
    setText(0, d->name);
}

CameraFolderItem::CameraFolderItem(QTreeWidgetItem* parent, const QString& folderName,
                                   const QString& folderPath, const QPixmap& pixmap)
    : QTreeWidgetItem(parent), d(new CameraFolderItemPriv)
{
    d->folderName    = folderName;
    d->folderPath    = folderPath;
    d->virtualFolder = false;
    d->name          = folderName;
    setIcon(0, pixmap);
    setText(0, d->name);
}

CameraFolderItem::~CameraFolderItem()
{
    delete d;
}

bool CameraFolderItem::isVirtualFolder()
{
    return d->virtualFolder;
}

QString CameraFolderItem::folderName()
{
    return d->folderName;
}

QString CameraFolderItem::folderPath()
{
    return d->folderPath;
}

void CameraFolderItem::changeCount(int val)
{
    d->count += val;
    setText(0, QString("%1 (%2)").arg(d->name).arg(QString::number(d->count)));
}

void CameraFolderItem::setCount(int val)
{
    d->count = val;
    setText(0, QString("%1 (%2)").arg(d->name).arg(QString::number(d->count)));
}

int CameraFolderItem::count()
{
    return d->count;
}

} // namespace Digikam
