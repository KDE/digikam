/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-07-24
 * Description : a dialog to select a camera folders.
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "camerafolderdialog.h"
#include "camerafolderdialog.moc"

// Qt includes

#include <QLabel>
#include <QFrame>
#include <QGridLayout>

// KDE includes

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "cameraiconview.h"
#include "camerafolderitem.h"
#include "camerafolderview.h"
#include "debug.h"

namespace Digikam
{

CameraFolderDialog::CameraFolderDialog(QWidget *parent, CameraIconView *cameraView,
                                       const QStringList& cameraFolderList,
                                       const QString& cameraName, const QString& rootPath)
                  : KDialog(parent)
{
    setHelp("camerainterface.anchor", "digikam");
    enableButtonOk(false);
    setCaption(i18n("%1 - Select Camera Folder",cameraName));
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    setModal(true);

    m_rootPath = rootPath;

    QFrame *page = new QFrame(this);
    setMainWidget(page);

    QGridLayout* grid = new QGridLayout(page);
    m_folderView      = new CameraFolderView(page);
    QLabel *logo      = new QLabel(page);
    QLabel *message   = new QLabel(page);

    logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                            .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    message->setText(i18n("<p>Please select the camera folder "
                          "where you want to upload the images.</p>"));
    message->setWordWrap(true);

    grid->addWidget(logo,           0, 0, 1, 1);
    grid->addWidget(message,        1, 0, 1, 1);
    grid->addWidget(m_folderView,   0, 1, 3, 1);
    grid->setRowStretch(2, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    m_folderView->addVirtualFolder(cameraName);
    m_folderView->addRootFolder("/", cameraView->countItemsByFolder(rootPath));

    for (QStringList::const_iterator it = cameraFolderList.constBegin();
         it != cameraFolderList.constEnd(); ++it)
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
            kDebug(digiKamAreaCode) << "Camera folder: '" << folder << "' (root='" << root << "', sub='" <<sub <<"')";
        }
    }

    connect(m_folderView, SIGNAL(signalFolderChanged(CameraFolderItem*)),
            this, SLOT(slotFolderPathSelectionChanged(CameraFolderItem*)));

    resize(500, 500);
}

CameraFolderDialog::~CameraFolderDialog()
{
}

QString CameraFolderDialog::selectedFolderPath() const
{
    QTreeWidgetItem *item = m_folderView->currentItem();
    if (!item) return QString();

    CameraFolderItem *folderItem = dynamic_cast<CameraFolderItem *>(item);
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
        enableButtonOk(true);
        kDebug(digiKamAreaCode) << "Camera folder path: " << selectedFolderPath();
    }
    else
    {
        enableButtonOk(false);
    }
}

}  // namespace Digikam
