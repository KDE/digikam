/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QComboBox>
#include <QLineEdit>
#include <QGridLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "htmlwizard.h"
#include "galleryinfo.h"
#include "dfileselector.h"

namespace Digikam
{

class HTMLOutputPage::Private
{
public:

    explicit Private()
      : destUrl(0),
        openInBrowser(0),
        titleLabel(0),
        imageSelectionTitle(0)
    {
    }

    DFileSelector* destUrl;
    QComboBox*     openInBrowser;
    QLabel*        titleLabel;
    QLineEdit*     imageSelectionTitle;
};

HTMLOutputPage::HTMLOutputPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private)
{
    setObjectName(QLatin1String("OutputPage"));

    QWidget* const main      = new QWidget(this);

    // --------------------

    d->titleLabel = new QLabel(main);
    d->titleLabel->setWordWrap(false);
    d->titleLabel->setText(i18n("Gallery Title:"));

    d->imageSelectionTitle   = new QLineEdit(main);
    d->titleLabel->setBuddy(d->imageSelectionTitle);

    // --------------------

    QLabel* const textLabel1 = new QLabel(main);
    textLabel1->setWordWrap(false);
    textLabel1->setText(i18n("Destination Folder:"));

    d->destUrl = new DFileSelector(main);
    d->destUrl->setFileDlgTitle(i18n("Destination Folder"));
    d->destUrl->setFileDlgMode(DFileDialog::Directory);
    textLabel1->setBuddy(d->destUrl);

    // --------------------

    QLabel* const browserLabel = new QLabel(main);
    browserLabel->setWordWrap(false);
    browserLabel->setText(i18n("Open in Browser:"));
    d->openInBrowser           = new QComboBox(main);
    d->openInBrowser->addItem(i18n("None"),                 GalleryConfig::NOBROWSER);
    d->openInBrowser->addItem(i18n("Internal"),             GalleryConfig::INTERNAL);
    d->openInBrowser->addItem(i18n("Default from Desktop"), GalleryConfig::DESKTOP);
    d->openInBrowser->setEditable(false);
    browserLabel->setBuddy(d->openInBrowser);

    // --------------------

    QGridLayout* const grid = new QGridLayout(main);
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid->addWidget(d->titleLabel,          0, 0, 1, 1);
    grid->addWidget(d->imageSelectionTitle, 0, 1, 1, 1);
    grid->addWidget(textLabel1,             1, 0, 1, 1);
    grid->addWidget(d->destUrl,             1, 1, 1, 1);
    grid->addWidget(browserLabel,           2, 0, 1, 1);
    grid->addWidget(d->openInBrowser,       2, 1, 1, 1);
    grid->setRowStretch(3, 10);

    // --------------------

    setPageWidget(main);
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
    d->openInBrowser->setCurrentIndex(info->openInBrowser());
    d->imageSelectionTitle->setText(info->imageSelectionTitle());

    d->titleLabel->setVisible(info->m_getOption == GalleryInfo::IMAGES);
    d->imageSelectionTitle->setVisible(info->m_getOption == GalleryInfo::IMAGES);
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
    info->setOpenInBrowser(d->openInBrowser->currentIndex());
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
