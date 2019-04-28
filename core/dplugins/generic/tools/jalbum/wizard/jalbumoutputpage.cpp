/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate jAlbum image galleries
 *
 * Copyright (C) 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
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

#include "jalbumoutputpage.h"

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

#include "jalbumwizard.h"
#include "jalbumsettings.h"
#include "dfileselector.h"

namespace DigikamGenericJAlbumPlugin
{

class Q_DECL_HIDDEN JAlbumOutputPage::Private
{
public:

    explicit Private()
      : destUrl(nullptr),
        titleLabel(nullptr),
        imageSelectionTitle(nullptr)
    {
    }

    DFileSelector* destUrl;
    QLabel*        titleLabel;
    QLineEdit*     imageSelectionTitle;
};

JAlbumOutputPage::JAlbumOutputPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private)
{
    setObjectName(QLatin1String("OutputPage"));

    QWidget* const main      = new QWidget(this);

    // --------------------

    d->titleLabel = new QLabel(main);
    d->titleLabel->setWordWrap(false);
    d->titleLabel->setText(i18n("Project Title:"));

    d->imageSelectionTitle   = new QLineEdit(main);
    d->titleLabel->setBuddy(d->imageSelectionTitle);

    // --------------------

    QLabel* const textLabel1 = new QLabel(main);
    textLabel1->setWordWrap(false);
    textLabel1->setText(i18n("Projects Folder:"));

    d->destUrl = new DFileSelector(main);
    d->destUrl->setFileDlgTitle(i18n("Projects Folder"));
    d->destUrl->setFileDlgMode(QFileDialog::Directory);
    textLabel1->setBuddy(d->destUrl);

    // --------------------

    QGridLayout* const grid = new QGridLayout(main);
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid->addWidget(d->titleLabel,          0, 0, 1, 1);
    grid->addWidget(d->imageSelectionTitle, 0, 1, 1, 1);
    grid->addWidget(textLabel1,             1, 0, 1, 1);
    grid->addWidget(d->destUrl,             1, 1, 1, 1);
    grid->setRowStretch(2, 10);

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

JAlbumOutputPage::~JAlbumOutputPage()
{
    delete d;
}

void JAlbumOutputPage::initializePage()
{
    JAlbumWizard* const wizard = dynamic_cast<JAlbumWizard*>(assistant());

    if (!wizard)
        return;

    JAlbumSettings* const info  = wizard->settings();

    d->destUrl->setFileDlgPath(info->m_destPath);
    d->imageSelectionTitle->setText(info->m_imageSelectionTitle);
}

bool JAlbumOutputPage::validatePage()
{
    if (d->destUrl->fileDlgPath().isEmpty())
        return false;

    if (d->imageSelectionTitle->text().isEmpty())
        return false;

    JAlbumWizard* const wizard  = dynamic_cast<JAlbumWizard*>(assistant());

    if (!wizard)
        return false;

    JAlbumSettings* const settings  = wizard->settings();
    settings->m_destPath            = d->destUrl->fileDlgPath();
    settings->m_imageSelectionTitle = d->imageSelectionTitle->text();

    return true;
}

bool JAlbumOutputPage::isComplete() const
{
    JAlbumWizard* const wizard = dynamic_cast<JAlbumWizard*>(assistant());

    if (!wizard)
        return false;

    bool b = !d->destUrl->fileDlgPath().isEmpty();
    b      = b && !d->imageSelectionTitle->text().isEmpty();

    return b;
}

} // namespace DigikamGenericJAlbumPlugin
