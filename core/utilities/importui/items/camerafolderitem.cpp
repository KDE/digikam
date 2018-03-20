/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-23
 * Description : A widget to display a camera folder.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class CameraFolderItem::Private
{
public:

    Private() :
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

CameraFolderItem::CameraFolderItem(QTreeWidget* const parent, const QString& name, const QIcon& icon)
    : QTreeWidgetItem(parent),
      d(new Private)
{
    d->name = name;
    setIcon(0, icon);
    setText(0, d->name);
}

CameraFolderItem::CameraFolderItem(QTreeWidgetItem* const parent, const QString& folderName,
                                   const QString& folderPath, const QIcon &icon)
    : QTreeWidgetItem(parent),
      d(new Private)
{
    d->folderName    = folderName;
    d->folderPath    = folderPath;
    d->virtualFolder = false;
    d->name          = folderName;
    setIcon(0, icon);
    setText(0, d->name);
}

CameraFolderItem::~CameraFolderItem()
{
    delete d;
}

bool CameraFolderItem::isVirtualFolder() const
{
    return d->virtualFolder;
}

QString CameraFolderItem::folderName() const
{
    return d->folderName;
}

QString CameraFolderItem::folderPath() const
{
    return d->folderPath;
}

void CameraFolderItem::changeCount(int val)
{
    d->count += val;
    setText(0, QString::fromUtf8("%1 (%2)").arg(d->name).arg(QString::number(d->count)));
}

void CameraFolderItem::setCount(int val)
{
    d->count = val;
    setText(0, QString::fromUtf8("%1 (%2)").arg(d->name).arg(QString::number(d->count)));
}

int CameraFolderItem::count() const
{
    return d->count;
}

} // namespace Digikam
