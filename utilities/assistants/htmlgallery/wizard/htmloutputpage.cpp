/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "htmloutputpage.h"

// Qt includes

#include <QIcon>
#include <QLabel>
#include <QUrl>
#include <QWidget>
#include <QApplication>
#include <QStyle>
#include <QCheckBox>
#include <QLineEdit>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "htmlwizard.h"
#include "galleryinfo.h"
#include "dfileselector.h"
#include "dlayoutbox.h"

namespace Digikam
{

class HTMLOutputPage::Private
{
public:

    Private()
      : destUrl(0),
        titleBox(0),
        openInBrowser(0),
        imageSelectionTitle(0)
    {
    }

    DFileSelector* destUrl;
    DVBox*         titleBox;
    QCheckBox*     openInBrowser;
    QLineEdit*     imageSelectionTitle;
};

HTMLOutputPage::HTMLOutputPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private)
{
    setObjectName(QLatin1String("OutputPage"));

    DVBox* const vbox        = new DVBox(this);
    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------

    d->titleBox              = new DVBox(vbox);
    d->titleBox->setContentsMargins(QMargins());
    d->titleBox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    QLabel* const textLabel2 = new QLabel(d->titleBox);
    textLabel2->setObjectName(QLatin1String("textLabel2"));
    textLabel2->setWordWrap(false);
    textLabel2->setText(i18n("Gallery Title:"));

    d->imageSelectionTitle   = new QLineEdit(d->titleBox);
    textLabel2->setBuddy(d->imageSelectionTitle);

    // --------------------

    DVBox* const hbox1       = new DVBox(vbox);
    hbox1->setContentsMargins(QMargins());
    hbox1->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    QLabel* const textLabel1 = new QLabel(hbox1);
    textLabel1->setWordWrap(false);
    textLabel1->setText(i18n("Destination Folder:"));

    d->destUrl = new DFileSelector(hbox1);
    d->destUrl->setFileDlgTitle(i18n("Destination Folder"));
    d->destUrl->setFileDlgMode(QFileDialog::Directory);
    textLabel1->setBuddy(d->destUrl);

    // --------------------

    d->openInBrowser         = new QCheckBox(vbox);
    d->openInBrowser->setText(i18n("Open in Browser"));

    // --------------------

    QWidget* const spacer    = new QWidget(vbox);
    vbox->setStretchFactor(spacer, 10);

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-html")));

    connect(d->destUrl->lineEdit(), SIGNAL(textEdited(QString)),
            this, SIGNAL(completeChanged()));

    connect(d->destUrl, SIGNAL(signalUrlSelected(QUrl)),
            this, SIGNAL(completeChanged()));

    connect(d->imageSelectionTitle, SIGNAL(textEdited(QString)),
            this, SIGNAL(completeChanged()));
}

HTMLOutputPage::~HTMLOutputPage()
{
    delete d;
}

void HTMLOutputPage::initializePage()
{
    HTMLWizard* const wizard = dynamic_cast<HTMLWizard*>(assistant());

    if (!wizard)
        return;

    GalleryInfo* const info  = wizard->galleryInfo();

    d->destUrl->setFileDlgPath(info->destUrl().toLocalFile());
    d->openInBrowser->setChecked(info->openInBrowser());
    d->imageSelectionTitle->setText(info->imageSelectionTitle());

    d->titleBox->setVisible(info->m_getOption == GalleryInfo::IMAGES);
}

bool HTMLOutputPage::validatePage()
{
    if (d->destUrl->fileDlgPath().isEmpty())
        return false;

    HTMLWizard* const wizard = dynamic_cast<HTMLWizard*>(assistant());

    if (!wizard)
        return false;

    GalleryInfo* const info  = wizard->galleryInfo();

    if (info->m_getOption == GalleryInfo::IMAGES && d->imageSelectionTitle->text().isEmpty())
        return false;

    info->setDestUrl(QUrl::fromLocalFile(d->destUrl->fileDlgPath()));
    info->setOpenInBrowser(d->openInBrowser->isChecked());
    info->setImageSelectionTitle(d->imageSelectionTitle->text());

    return true;
}

bool HTMLOutputPage::isComplete() const
{
    HTMLWizard* const wizard = dynamic_cast<HTMLWizard*>(assistant());

    if (!wizard)
        return false;

    GalleryInfo* const info  = wizard->galleryInfo();

    bool b                   = !d->destUrl->fileDlgPath().isEmpty();

    if (info->m_getOption == GalleryInfo::IMAGES)
        b = b & !d->imageSelectionTitle->text().isEmpty();

    return b;
}

} // namespace Digikam
