/* ============================================================
 * Author:  Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2006-07-24
 * Description : a dialog to select a camera folders.
 * 
 * Copyright 2006 by Gilles Caulier
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

// Qt includes.

#include <qlabel.h>
#include <qlayout.h>
#include <qframe.h>

// KDE includes.

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kapplication.h>

// Local includes.

#include "cameraiconview.h"
#include "camerafolderitem.h"
#include "camerafolderview.h"
#include "camerafolderdialog.h"
#include "camerafolderdialog.moc"

namespace Digikam
{

CameraFolderDialog::CameraFolderDialog(QWidget *parent, CameraIconView *cameraView, 
                                       const QStringList& cameraFolderList,
                                       const QString& cameraName, const QString& rootPath)
                  : KDialogBase(parent, 0, true,
                                i18n("%1 - Select Camera Folder").arg(cameraName), 
                                Help|Ok|Cancel, Ok, true)
{
    setHelp("camerainterface.anchor", "digikam");
    enableButtonOK(false);

    m_rootPath = rootPath;

    QFrame *page      = makeMainWidget();
    QGridLayout* grid = new QGridLayout(page, 2, 1, 0, spacingHint());
    
    m_folderView    = new CameraFolderView(page);
    QLabel *logo    = new QLabel(page);
    QLabel *message = new QLabel(page);

    KIconLoader* iconLoader = KApplication::kApplication()->iconLoader();
    logo->setPixmap(iconLoader->loadIcon("digikam", KIcon::NoGroup, 128, KIcon::DefaultState, 0, true));    
    message->setText(i18n("<p>Please, select the right camera folder "
                          "where you want to upload the pictures.</p>"));

    grid->addMultiCellWidget(logo, 0, 0, 0, 0);
    grid->addMultiCellWidget(message, 1, 1, 0, 0);
    grid->addMultiCellWidget(m_folderView, 0, 2, 1, 1);
    grid->setRowStretch(2, 10);

    m_folderView->addVirtualFolder(cameraName);
    m_folderView->addRootFolder("/", cameraView->countItemsByFolder(rootPath));

    for (QStringList::const_iterator it = cameraFolderList.begin();
         it != cameraFolderList.end(); ++it)
    {
        QString folder(*it);
        if (folder.startsWith(rootPath) && rootPath != QString("/"))
            folder.remove(0, rootPath.length());

        if (folder != QString("/") && !folder.isEmpty())
        {
            QString root = folder.section( '/', 0, -2 );
            if (root.isEmpty()) root = QString("/");

            QString sub = folder.section( '/', -1 );
            m_folderView->addFolder(root, sub, cameraView->countItemsByFolder(*it));
            kdDebug() << "Camera folder: '" << folder << "' (root='" << root << "', sub='" <<sub <<"')" << endl;
        }
    }

    connect(m_folderView, SIGNAL(signalFolderChanged(CameraFolderItem*)),
            this, SLOT(slotFolderPathSelectionChanged(CameraFolderItem*)));

    resize(500, 500);
}

CameraFolderDialog::~CameraFolderDialog()
{
}

QString CameraFolderDialog::selectedFolderPath()
{
    QListViewItem *item = m_folderView->currentItem();
    if (!item) return QString();

    CameraFolderItem *folderItem = static_cast<CameraFolderItem *>(item);
    if (folderItem->isVirtualFolder())
        return QString(m_rootPath);

    // Case of Gphoto2 cameras. No need to duplicate root '/'.
    if (m_rootPath == QString("/"))
        return(folderItem->folderPath());

    return(m_rootPath + folderItem->folderPath());
}

void CameraFolderDialog::slotFolderPathSelectionChanged(CameraFolderItem* item)
{
    if (item) 
    {
        enableButtonOK(true);
        kdDebug() << "Camera folder path: " << selectedFolderPath() << endl;
    }
    else
    {
        enableButtonOK(false);
    }
}

}  // namespace Digikam

