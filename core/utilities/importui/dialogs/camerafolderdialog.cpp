/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-07-24
 * Description : a dialog to select a camera folders.
 *
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

#include "camerafolderdialog.h"

// Qt includes

#include <QLabel>
#include <QFrame>
#include <QGridLayout>
#include <QStandardPaths>
#include <QApplication>
#include <QStyle>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "camerafolderitem.h"
#include "camerafolderview.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class CameraFolderDialog::Private
{
public:

    Private() :
        buttons(0),
        folderView(0)
    {
    }

    QString           rootPath;
    QDialogButtonBox* buttons;

    CameraFolderView* folderView;
};

CameraFolderDialog::CameraFolderDialog(QWidget* const parent, const QMap<QString, int>& map,
                                       const QString& cameraName, const QString& rootPath)
    : QDialog(parent),
      d(new Private)
{
    setModal(true);
    setWindowTitle(i18nc("@title:window %1: name of the camera", "%1 - Select Camera Folder", cameraName));

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(false);

    d->rootPath        = rootPath;
    QFrame* const page = new QFrame(this);

    QGridLayout* const grid = new QGridLayout(page);
    d->folderView           = new CameraFolderView(page);
    QLabel* const logo      = new QLabel(page);
    QLabel* const message   = new QLabel(page);

    logo->setPixmap(QIcon::fromTheme(QLatin1String("digikam")).pixmap(QSize(48,48)));

    message->setText(i18n("<p>Please select the camera folder "
                          "where you want to upload the images.</p>"));
    message->setWordWrap(true);

    grid->addWidget(logo,          0, 0, 1, 1);
    grid->addWidget(message,       1, 0, 1, 1);
    grid->addWidget(d->folderView, 0, 1, 3, 1);
    grid->setRowStretch(2, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    d->folderView->addVirtualFolder(cameraName);
    d->folderView->addRootFolder(QLatin1String("/"));

    for (QMap<QString, int>::const_iterator it = map.constBegin(); it != map.constEnd(); ++it)
    {
        QString folder(it.key());

        if (folder != QLatin1String("/"))
        {
            if (folder.startsWith(rootPath) && rootPath != QLatin1String("/"))
            {
                folder.remove(0, rootPath.length());
            }

            QString path(QLatin1String("/"));

            foreach(const QString& sub, folder.split(QLatin1Char('/'), QString::SkipEmptyParts))
            {
                qCDebug(DIGIKAM_IMPORTUI_LOG) << "Camera folder:" << path << "Subfolder:" << sub;
                d->folderView->addFolder(path, sub, it.value());
                path += sub + QLatin1Char('/');
            }
        }
    }

    connect(d->folderView, SIGNAL(signalFolderChanged(CameraFolderItem*)),
            this, SLOT(slotFolderPathSelectionChanged(CameraFolderItem*)));

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    adjustSize();

    // make sure the ok button is properly set up
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(d->folderView->currentItem() != 0);
}

CameraFolderDialog::~CameraFolderDialog()
{
    delete d;
}

QString CameraFolderDialog::selectedFolderPath() const
{
    QTreeWidgetItem* const item = d->folderView->currentItem();

    if (!item)
    {
        return QString();
    }

    CameraFolderItem* const folderItem = dynamic_cast<CameraFolderItem*>(item);

    if (!folderItem)
    {
        return QString();
    }

    if (folderItem->isVirtualFolder())
    {
        return QString(d->rootPath);
    }

    // Case of Gphoto2 cameras. No need to duplicate root '/'.
    if (d->rootPath == QLatin1String("/"))
    {
        return(folderItem->folderPath());
    }

    return (d->rootPath + folderItem->folderPath());
}

void CameraFolderDialog::slotFolderPathSelectionChanged(CameraFolderItem* item)
{
    if (item)
    {
        d->buttons->button(QDialogButtonBox::Ok)->setEnabled(true);
        qCDebug(DIGIKAM_IMPORTUI_LOG) << "Camera folder path: " << selectedFolderPath();
    }
    else
    {
        d->buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
}

void CameraFolderDialog::slotHelp()
{
    DXmlGuiWindow::openHandbook();
}

}  // namespace Digikam
