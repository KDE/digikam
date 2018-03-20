/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-01
 * Description : collections setup tab
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QApplication>
#include <QStyle>
#include <QUrl>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
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
        collectionModel(0)
    {
    }

    bool                     rootsPathChanged;

    SetupCollectionTreeView* collectionView;
    SetupCollectionModel*    collectionModel;
};

SetupCollections::SetupCollections(QWidget* const parent)
    : QScrollArea(parent), d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QWidget* const panel = new QWidget;

    QVBoxLayout* const layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox* const albumPathBox = new QGroupBox(i18n("Root Album Folders"), panel);

#ifndef Q_OS_WIN
    QLabel* const albumPathLabel  = new QLabel(i18n("<p>Below are the locations of your root albums used to store "
                                              "your images. Write access is necessary to be able "
                                              "to edit images in these albums.</p>"
                                              "<p>Note: Removable media (such as USB drives or DVDs) and remote file systems "
                                              "(such as NFS, or Samba mounted with cifs/smbfs) are supported.</p><p></p>"),
                                              albumPathBox);
#else
    QLabel* const albumPathLabel  = new QLabel(i18n("<p>Below are the locations of your root albums used to store "
                                              "your images. Write access is necessary to be able "
                                              "to edit images in these albums.</p><p></p>"),
                                              albumPathBox);
#endif
    albumPathLabel->setWordWrap(true);

    d->collectionView  = new SetupCollectionTreeView(albumPathBox);
    d->collectionModel = new SetupCollectionModel(panel);
    d->collectionView->setModel(d->collectionModel);

    QVBoxLayout* const albumPathBoxLayout = new QVBoxLayout;
    albumPathBoxLayout->addWidget(albumPathLabel);
    albumPathBoxLayout->addWidget(d->collectionView);
    albumPathBox->setLayout(albumPathBoxLayout);
    albumPathBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    albumPathBoxLayout->setSpacing(0);

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
