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

#ifndef CAMERAFOLDERITEM_H
#define CAMERAFOLDERITEM_H

// Qt includes

#include <QString>
#include <QPixmap>
#include <QTreeWidgetItem>
#include <QIcon>

namespace Digikam
{

class CameraFolderItem : public QTreeWidgetItem
{

public:

    CameraFolderItem(QTreeWidget* const parent, const QString& name,
                     const QIcon &icon = QIcon::fromTheme(QLatin1String("folder")));

    CameraFolderItem(QTreeWidgetItem* const parent, const QString& folderName,
                     const QString& folderPath,
                     const QIcon& icon = QIcon::fromTheme(QLatin1String("folder")));

    ~CameraFolderItem();

    QString folderName() const;
    QString folderPath() const;
    bool    isVirtualFolder() const;
    void    changeCount(int val);
    void    setCount(int val);
    int     count() const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* CAMERAFOLDERITEM_H */
