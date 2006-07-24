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

#ifndef CAMERAFOLDERVIEW_H
#define CAMERAFOLDERVIEW_H

// Qt includes.

#include <qstring.h>

// KDE includes.

#include <klistview.h>

namespace Digikam
{

class CameraFolderItem;

class CameraFolderView : public KListView
{
    Q_OBJECT

public:

    CameraFolderView(QWidget* parent);
    ~CameraFolderView();

    void addVirtualFolder(const QString& name);
    void addRootFolder(const QString& folder);
    CameraFolderItem* addFolder(const QString& folder,
                                const QString& subFolder);

    CameraFolderItem* findFolder(const QString& folderPath);

    CameraFolderItem* virtualFolder();
    CameraFolderItem* rootFolder();

    virtual void clear();

signals:

    void signalFolderChanged(CameraFolderItem*);
    void signalCleared();
    
    
private slots:

    void slotSelectionChanged(QListViewItem* item);

private:

    void setupConnections();

private:

    QString           cameraName_;
    CameraFolderItem *virtualFolder_;
    CameraFolderItem *rootFolder_;
};

} // namespace Digikam

#endif /* CAMERAFOLDERVIEW_H */
