/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-02-01
 * Description : collections setup tab
 *
 * Copyright (C) 2005-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QLabel>
#include <QStyle>
#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "applicationsettings.h"
#include "setupcollectionview.h"

namespace Digikam
{

class Q_DECL_HIDDEN SetupCollections::Private
{
public:

    explicit Private()
      : rootsPathChanged(false),
        collectionView(nullptr),
        collectionModel(nullptr),
        monitoringBox(nullptr)
    {
    }

    bool                     rootsPathChanged;

    SetupCollectionTreeView* collectionView;
    SetupCollectionModel*    collectionModel;

    QCheckBox*               monitoringBox;
};

SetupCollections::SetupCollections(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
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
                                              "(such as NFS, or Samba mounted with cifs/smbfs) are supported.</p>"
                                              "<p>Important: You need to mount the share natively on your system before to setup a remote collection.</p>"
                                              "<p></p>"),
                                              albumPathBox);
#else
    QLabel* const albumPathLabel  = new QLabel(i18n("<p>Below are the locations of your root albums used to store "
                                              "your images. Write access is necessary to be able "
                                              "to edit images in these albums.</p>"
                                              "<p></p>"),
                                              albumPathBox);
#endif
    albumPathLabel->setWordWrap(true);

    d->collectionView  = new SetupCollectionTreeView(albumPathBox);
    d->collectionModel = new SetupCollectionModel(panel);
    d->collectionView->setModel(d->collectionModel);

    d->monitoringBox   = new QCheckBox(i18n("Monitor the albums for external changes (requires restart)"), panel);

    QVBoxLayout* const albumPathBoxLayout = new QVBoxLayout;
    albumPathBoxLayout->addWidget(albumPathLabel);
    albumPathBoxLayout->addWidget(d->collectionView);
    albumPathBoxLayout->addWidget(d->monitoringBox);
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

    ApplicationSettings* const settings = ApplicationSettings::instance();
    settings->setAlbumMonitoring(d->monitoringBox->isChecked());
}

void SetupCollections::readSettings()
{
    d->collectionModel->loadCollections();

    ApplicationSettings* const settings = ApplicationSettings::instance();
    d->monitoringBox->setChecked(settings->getAlbumMonitoring());
}

} // namespace Digikam
