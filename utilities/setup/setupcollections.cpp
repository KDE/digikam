/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-01
 * Description : collections setup tab
 *
 * Copyright (C) 2005-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupcollections.h"

// Qt includes

#include <QGroupBox>
#include <QLabel>
#include <QDir>
#include <QList>
#include <QFileInfo>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QIntValidator>
#include <QSpinBox>
#include <QFormLayout>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <klineedit.h>
#include <kpagedialog.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <ktemporaryfile.h>

// Local includes

#include "applicationsettings.h"
#include "setupcollectionview.h"

namespace Digikam
{

class SetupCollections::Private
{
public:

    Private() :
        rootsPathChanged(false),
        collectionView(0),
        collectionModel(0),
        mainDialog(0)
    {
    }

    bool                     rootsPathChanged;

    SetupCollectionTreeView* collectionView;
    SetupCollectionModel*    collectionModel;

    KPageDialog*             mainDialog;
};

SetupCollections::SetupCollections(KPageDialog* const dialog, QWidget* const parent)
    : QScrollArea(parent), d(new Private)
{
    d->mainDialog       = dialog;
    QWidget* panel      = new QWidget;

    QVBoxLayout* layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox* albumPathBox = new QGroupBox(i18n("Root Album Folders"), panel);

#ifndef _WIN32
    QLabel* albumPathLabel  = new QLabel(i18n("<p>Below are the locations of your root albums used to store "
                                              "your images. Write access is necessary to be able "
                                              "to edit images in these albums.</p>"
                                              "<p>Note: Removable media (such as USB drives or DVDs) and remote file systems "
                                              "(such as NFS, or Samba mounted with cifs/smbfs) are supported.</p><p></p>"),
                                         albumPathBox);
#else
    QLabel* albumPathLabel  = new QLabel(i18n("<p>Below are the locations of your root albums used to store "
                                              "your images. Write access is necessary to be able "
                                              "to edit images in these albums.</p><p></p>"),
                                         albumPathBox);
#endif
    albumPathLabel->setWordWrap(true);
    
    d->collectionView  = new SetupCollectionTreeView(albumPathBox);
    d->collectionModel = new SetupCollectionModel(panel);
    d->collectionView->setModel(d->collectionModel);

    QVBoxLayout* albumPathBoxLayout = new QVBoxLayout;
    albumPathBoxLayout->addWidget(albumPathLabel);
    albumPathBoxLayout->addWidget(d->collectionView);
    albumPathBox->setLayout(albumPathBoxLayout);
    albumPathBoxLayout->setSpacing(0);
    albumPathBoxLayout->setMargin(KDialog::spacingHint());

    // --------------------------------------------------------

    layout->addWidget(albumPathBox);

    setWidget(panel);
    setWidgetResizable(true);

    // --------------------------------------------------------

    readSettings();
    adjustSize();
}

SetupCollections::~SetupCollections()
{
    delete d;
}

void SetupCollections::applySettings()
{
    d->collectionModel->apply();
}

void SetupCollections::readSettings()
{
    d->collectionModel->loadCollections();
}

}  // namespace Digikam
