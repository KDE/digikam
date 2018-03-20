/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-28
 * Description : a tool to export image to a KIO accessible
 *               location
 *
 * Copyright (C) 2006-2009 by Johannes Wienke <languitar at semipol dot de>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "ftexportwidget.h"

// Qt includes

#include <QVBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QFileDialog>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>
#include <kcombobox.h>
#include <kurlrequester.h>

// Local includes

#include "digikam_debug.h"
#include "dimageslist.h"
#include "wstoolutils.h"
#include "dlayoutbox.h"

namespace Digikam
{

class FTExportWidget::Private
{
public:

    explicit Private()
    {
        targetLabel        = 0;
        targetDialog       = 0;
        targetSearchButton = 0;
        targetUrl          = 0;
        imageList          = 0;
    }

    KUrlComboRequester* targetLabel;
    QFileDialog*        targetDialog;
    QPushButton*        targetSearchButton;
    QUrl                targetUrl;
    DImagesList*        imageList;
};

FTExportWidget::FTExportWidget(DInfoInterface* const iface, QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    // setup remote target selection

    DHBox* const hbox   = new DHBox(this);
    QLabel* const label = new QLabel(hbox);
    d->targetLabel      = new KUrlComboRequester(hbox);

    if (d->targetLabel->button())
        d->targetLabel->button()->hide();

    d->targetLabel->comboBox()->setEditable(true);

    label->setText(i18n("Target location: "));
    d->targetLabel->setWhatsThis(i18n("Sets the target address to upload the images to. "
                                      "This can be any address as used in Dolphin or Konqueror, "
                                      "e.g. ftp://my.server.org/sub/folder."));

    d->targetSearchButton = new QPushButton(i18n("Select export location..."), this);
    d->targetSearchButton->setIcon(QIcon::fromTheme(QString::fromLatin1("folder-remote")));

    // setup image list
    d->imageList = new DImagesList(this);
    d->imageList->setIface(iface);
    d->imageList->loadImagesFromCurrentSelection();
    d->imageList->setAllowRAW(true);
    d->imageList->listView()->setWhatsThis(i18n("This is the list of images to upload "
                                                "to the specified target."));

    // layout dialog
    QVBoxLayout* const layout = new QVBoxLayout(this);

    layout->addWidget(hbox);
    layout->addWidget(d->targetSearchButton);
    layout->addWidget(d->imageList);
    layout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    layout->setContentsMargins(QMargins());

    // ------------------------------------------------------------------------

    connect(d->targetSearchButton, SIGNAL(clicked(bool)),
            this, SLOT(slotShowTargetDialogClicked(bool)));

    connect(d->targetLabel, SIGNAL(textChanged(QString)),
            this, SLOT(slotLabelUrlChanged()));

    // ------------------------------------------------------------------------

    updateTargetLabel();
}

FTExportWidget::~FTExportWidget()
{
    delete d;
}

QUrl FTExportWidget::targetUrl() const
{
    return d->targetUrl;
}

QList<QUrl> FTExportWidget::history() const
{
    QList<QUrl> urls;

    for (int i = 0 ; i <= d->targetLabel->comboBox()->count() ; i++)
        urls << QUrl(d->targetLabel->comboBox()->itemText(i));

    return urls;
}

void FTExportWidget::setHistory(const QList<QUrl>& urls)
{
    d->targetLabel->comboBox()->clear();

    foreach (QUrl url, urls)
        d->targetLabel->comboBox()->addUrl(url);
}

void FTExportWidget::setTargetUrl(const QUrl& url)
{
    d->targetUrl = url;
    updateTargetLabel();
}

void FTExportWidget::slotShowTargetDialogClicked(bool checked)
{
    Q_UNUSED(checked);

    d->targetDialog = new QFileDialog(this, i18n("Select target..."),
                                      d->targetUrl.toString(), i18n("All Files (*)"));
    d->targetDialog->setAcceptMode(QFileDialog::AcceptSave);
    d->targetDialog->setFileMode(QFileDialog::DirectoryOnly);

    if (d->targetDialog->exec() == QDialog::Accepted)
    {
        d->targetUrl = d->targetDialog->selectedUrls().isEmpty() ? QUrl() : d->targetDialog->selectedUrls().at(0);
        updateTargetLabel();
        emit signalTargetUrlChanged(d->targetUrl);
    }

    delete d->targetDialog;
}

void FTExportWidget::updateTargetLabel()
{
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Call for url "
                                 << d->targetUrl.toDisplayString()
                                 << ", valid = "
                                 << d->targetUrl.isValid();

    QString urlString = i18n("<not selected>");

    if (d->targetUrl.isValid())
    {
        urlString = d->targetUrl.toDisplayString();
        d->targetLabel->setUrl(QUrl(urlString));
    }
}

DImagesList* FTExportWidget::imagesList() const
{
    return d->imageList;
}

void FTExportWidget::slotLabelUrlChanged()
{
    d->targetUrl = d->targetLabel->url();
    emit signalTargetUrlChanged(d->targetUrl);
}

} // namespace Digikam
