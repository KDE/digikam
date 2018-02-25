/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 04-10-2009
 * Description : main widget of the import dialog
 *
 * Copyright (C) 2009      by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "ftimportwidget.h"

// Qt includes

#include <QBoxLayout>
#include <QApplication>
#include <QFileDialog>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimageslist.h"

namespace Digikam
{

class FTImportWidget::Private
{
public:

    explicit Private()
    {
        imageList       = 0;
        uploadWidget    = 0;
        importDlg       = 0;
        importSearchBtn = 0;
    }

    DImagesList* imageList;
    QWidget*     uploadWidget;
    QFileDialog* importDlg;
    QPushButton* importSearchBtn;
};

FTImportWidget::FTImportWidget(QWidget* const parent, DInfoInterface* const iface)
    : QWidget(parent),
      d(new Private)
{
    d->importSearchBtn = new QPushButton(i18n("Select import location..."), this);
    d->importSearchBtn->setIcon(QIcon::fromTheme(QString::fromLatin1("folder-remote")));

    // setup image list
    d->imageList = new DImagesList(this);
    d->imageList->setAllowRAW(true);
    d->imageList->setIface(iface);
    d->imageList->listView()->setColumnEnabled(DImagesListView::Thumbnail, false);
    d->imageList->setControlButtons(DImagesList::Remove | DImagesList::MoveUp | DImagesList::MoveDown | DImagesList::Clear);
    d->imageList->listView()->setWhatsThis(i18n("This is the list of images to import "
                                                "into the current album."));

    // setup upload widget
    d->uploadWidget = iface->uploadWidget(this);

    // layout dialog
    QVBoxLayout* const layout = new QVBoxLayout(this);
    layout->addWidget(d->importSearchBtn);
    layout->addWidget(d->imageList);
    layout->addWidget(d->uploadWidget);
    layout->setContentsMargins(QMargins());
    layout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    connect(d->importSearchBtn, SIGNAL(clicked(bool)),
            this, SLOT(slotShowImportDialogClicked(bool)));
}

FTImportWidget::~FTImportWidget()
{
    delete d;
}

void FTImportWidget::slotShowImportDialogClicked(bool checked)
{
    Q_UNUSED(checked);

    d->importDlg = new QFileDialog(this, i18n("Select items to import..."),
                                   QString(),  // TODO : store and restore previous session url from rc file
                                   i18n("All Files (*)"));
    d->importDlg->setAcceptMode(QFileDialog::AcceptOpen);
    d->importDlg->setFileMode(QFileDialog::ExistingFiles);

    if (d->importDlg->exec() == QDialog::Accepted)
    {
        d->imageList->slotAddImages(d->importDlg->selectedUrls());
    }

    delete d->importDlg;
}

DImagesList* FTImportWidget::imagesList() const
{
    return d->imageList;
}

QWidget* FTImportWidget::uploadWidget() const
{
    return d->uploadWidget;
}

QList<QUrl> FTImportWidget::sourceUrls() const
{
    return d->imageList->imageUrls();
}

} // namespace Digikam
