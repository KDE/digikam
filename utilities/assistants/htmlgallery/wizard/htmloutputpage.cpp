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

#include <QHBoxLayout>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QUrl>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

HTMLOutputPage::HTMLOutputPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title)
{
    setObjectName(QLatin1String("OutputPage"));

    QWidget* const box = new QWidget(this);

    textLabel1         = new QLabel(this);
    textLabel1->setObjectName(QLatin1String("textLabel1"));
    textLabel1->setWordWrap(false);
    textLabel1->setText(i18n("Destination folder:"));

    kcfg_destUrl = new DFileSelector(this);
    kcfg_destUrl->setObjectName(QLatin1String("kcfg_destUrl"));
    kcfg_destUrl->setFileDlgMode(QFileDialog::Directory);
    textLabel1->setBuddy(kcfg_destUrl);

    QHBoxLayout* const hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setObjectName(QLatin1String("hboxLayout"));
    hboxLayout->addWidget(textLabel1);
    hboxLayout->addWidget(kcfg_destUrl);

    kcfg_openInBrowser = new QCheckBox(this);
    kcfg_openInBrowser->setObjectName(QLatin1String("kcfg_openInBrowser"));
    kcfg_openInBrowser->setText(i18n("Open in browser"));

    QSpacerItem* const spacer1    = new QSpacerItem(20, 51, QSizePolicy::Minimum, QSizePolicy::Expanding);

    QVBoxLayout* const vboxLayout = new QVBoxLayout(box);
    vboxLayout->setContentsMargins(QMargins());
    vboxLayout->setObjectName(QLatin1String("vboxLayout"));
    vboxLayout->addLayout(hboxLayout);
    vboxLayout->addWidget(kcfg_openInBrowser);
    vboxLayout->addItem(spacer1);

    setPageWidget(box);

    connect(kcfg_destUrl, SIGNAL(signalUrlSelected(QUrl)),
            this, SIGNAL(completeChanged()));
}

HTMLOutputPage::~HTMLOutputPage()
{
}

bool HTMLOutputPage::validatePage()
{
    return (!kcfg_destUrl->fileDlgPath().isEmpty());
}

} // namespace Digikam
