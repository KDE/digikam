/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
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
#include "jalbuminfo.h"
#include "dfileselector.h"

namespace Digikam
{

class Q_DECL_HIDDEN JAlbumOutputPage::Private
{
public:

    explicit Private()
      : destUrl(0),
        jarUrl(0),
        titleLabel(0),
        imageSelectionTitle(0)
    {
    }

    DFileSelector* destUrl;
    DFileSelector* jarUrl;
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

    QLabel* const textLabel2 = new QLabel(main);
    textLabel2->setWordWrap(false);
    textLabel2->setText(i18n("Path to jAlbum jar file:"));

    d->jarUrl = new DFileSelector(main);
    d->jarUrl->setFileDlgTitle(i18n("jAlbum jar path"));
    d->jarUrl->setFileDlgMode(QFileDialog::Directory);
    textLabel2->setBuddy(d->jarUrl);

    // --------------------

    QGridLayout* const grid = new QGridLayout(main);
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid->addWidget(d->titleLabel,          0, 0, 1, 1);
    grid->addWidget(d->imageSelectionTitle, 0, 1, 1, 1);
    grid->addWidget(textLabel1,             1, 0, 1, 1);
    grid->addWidget(d->destUrl,             1, 1, 1, 1);
    grid->addWidget(textLabel2,             2, 0, 1, 1);
    grid->addWidget(d->jarUrl,              2, 1, 1, 1);
    grid->setRowStretch(3, 10);

    // --------------------

    setPageWidget(main);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-html")));

    connect(d->destUrl->lineEdit(), SIGNAL(textEdited(QString)),
            this, SIGNAL(completeChanged()));

    connect(d->destUrl, SIGNAL(signalUrlSelected(QUrl)),
            this, SIGNAL(completeChanged()));

    connect(d->jarUrl->lineEdit(), SIGNAL(textEdited(QString)),
            this, SIGNAL(completeChanged()));

    connect(d->jarUrl, SIGNAL(signalUrlSelected(QUrl)),
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

    JAlbumInfo* const info  = wizard->jalbumInfo();

    d->destUrl->setFileDlgPath(info->destUrl().toLocalFile());
    d->jarUrl->setFileDlgPath(info->jarUrl().toLocalFile());
    d->imageSelectionTitle->setText(info->imageSelectionTitle());
}

bool JAlbumOutputPage::validatePage()
{
    if (d->destUrl->fileDlgPath().isEmpty())
        return false;

    if (d->jarUrl->fileDlgPath().isEmpty())
        return false;

    if (d->imageSelectionTitle->text().isEmpty())
        return false;

    JAlbumWizard* const wizard = dynamic_cast<JAlbumWizard*>(assistant());

    if (!wizard)
        return false;

    JAlbumInfo* const info  = wizard->jalbumInfo();

    info->setDestUrl(QUrl::fromLocalFile(d->destUrl->fileDlgPath()));
    info->setJarUrl(QUrl::fromLocalFile(d->jarUrl->fileDlgPath()));
    info->setImageSelectionTitle(d->imageSelectionTitle->text());

    return true;
}

bool JAlbumOutputPage::isComplete() const
{
    JAlbumWizard* const wizard = dynamic_cast<JAlbumWizard*>(assistant());

    if (!wizard)
        return false;

    bool b                   = !d->destUrl->fileDlgPath().isEmpty();
    b                        = b && !d->jarUrl->fileDlgPath().isEmpty();
    b                        = b && !d->imageSelectionTitle->text().isEmpty();

    return b;
}

} // namespace Digikam
