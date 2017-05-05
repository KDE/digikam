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
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QLabel>
#include <QUrl>
#include <QWidget>
#include <QApplication>
#include <QStyle>
#include <QCheckBox>

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

    Private()
      : kcfg_destUrl(0),
        kcfg_openInBrowser(0)
    {
    }

    DFileSelector* kcfg_destUrl;
    QCheckBox*     kcfg_openInBrowser;
};

HTMLOutputPage::HTMLOutputPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private)
{
    setObjectName(QLatin1String("OutputPage"));

    QWidget* const box       = new QWidget(this);

    QLabel* const textLabel1 = new QLabel(this);
    textLabel1->setObjectName(QLatin1String("textLabel1"));
    textLabel1->setWordWrap(false);
    textLabel1->setText(i18n("Destination folder:"));

    d->kcfg_destUrl = new DFileSelector(this);
    d->kcfg_destUrl->setObjectName(QLatin1String("d->kcfg_destUrl"));
    d->kcfg_destUrl->setFileDlgMode(QFileDialog::Directory);
    textLabel1->setBuddy(d->kcfg_destUrl);

    QHBoxLayout* const hboxLayout = new QHBoxLayout();
    hboxLayout->setContentsMargins(QMargins());
    hboxLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    hboxLayout->setObjectName(QLatin1String("hboxLayout"));
    hboxLayout->addWidget(textLabel1);
    hboxLayout->addWidget(d->kcfg_destUrl);

    d->kcfg_openInBrowser         = new QCheckBox(this);
    d->kcfg_openInBrowser->setObjectName(QLatin1String("d->kcfg_openInBrowser"));
    d->kcfg_openInBrowser->setText(i18n("Open in browser"));

    QSpacerItem* const spacer1    = new QSpacerItem(20, 51, QSizePolicy::Minimum,
                                                    QSizePolicy::Expanding);

    QVBoxLayout* const vboxLayout = new QVBoxLayout(box);
    vboxLayout->setContentsMargins(QMargins());
    vboxLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    vboxLayout->setObjectName(QLatin1String("vboxLayout"));
    vboxLayout->addLayout(hboxLayout);
    vboxLayout->addWidget(d->kcfg_openInBrowser);
    vboxLayout->addItem(spacer1);

    setPageWidget(box);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-html")));

    connect(d->kcfg_destUrl, SIGNAL(signalUrlSelected(QUrl)),
            this, SIGNAL(completeChanged()));
}

HTMLOutputPage::~HTMLOutputPage()
{
    delete d;
}

void HTMLOutputPage::initializePage()
{
    HTMLWizard* const wizard = dynamic_cast<HTMLWizard*>(assistant());
    GalleryInfo* const info  = wizard->galleryInfo();

    d->kcfg_destUrl->setFileDlgPath(info->destUrl().toLocalFile());
    d->kcfg_openInBrowser->setChecked(info->openInBrowser());
}

bool HTMLOutputPage::validatePage()
{
    if (d->kcfg_destUrl->fileDlgPath().isEmpty())
        return false;

    HTMLWizard* const wizard = dynamic_cast<HTMLWizard*>(assistant());
    GalleryInfo* const info  = wizard->galleryInfo();

    info->setDestUrl(QUrl::fromLocalFile(d->kcfg_destUrl->fileDlgPath()));
    info->setOpenInBrowser(d->kcfg_openInBrowser->isChecked());

    return true;
}

} // namespace Digikam
