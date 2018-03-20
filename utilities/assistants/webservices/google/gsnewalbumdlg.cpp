/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-01
 * Description : a tool to export items to Google web services
 *
 * Copyright (C) 2010 by Jens Mueller <tschenser at gmx dot de>
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

#include "gsnewalbumdlg.h"

// Qt includes

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QIcon>
#include <QApplication>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "gsitem.h"

namespace Digikam
{

class GSNewAlbumDlg::Private
{
public:

    explicit Private()
    {
        publicRBtn    = 0;
        unlistedRBtn  = 0;
        protectedRBtn = 0;
    }

    QString        serviceName;
    QRadioButton*  publicRBtn;
    QRadioButton*  unlistedRBtn;
    QRadioButton*  protectedRBtn;
};

GSNewAlbumDlg::GSNewAlbumDlg(QWidget* const parent,
                             const QString& serviceName,
                             const QString& toolName)
    : WSNewAlbumDialog(parent, toolName),
      d(new Private)
{
    d->serviceName           = serviceName;
    const int spacing        = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGroupBox* const privBox = new QGroupBox(i18n("Access Level"), getMainWidget());
    privBox->setWhatsThis(i18n("These are security and privacy settings for the new Google Photos/PicasaWeb album."));

    d->publicRBtn        = new QRadioButton(i18nc("google photos/picasaweb album privacy", "Public"));
    d->publicRBtn->setChecked(true);
    d->publicRBtn->setWhatsThis(i18n("Public album is listed on your public Google Photos/PicasaWeb page."));
    d->unlistedRBtn      = new QRadioButton(i18nc("google photos/picasaweb album privacy", "Unlisted / Private"));
    d->unlistedRBtn->setWhatsThis(i18n("Unlisted album is only accessible via URL."));
    d->protectedRBtn     = new QRadioButton(i18nc("google photos/picasaweb album privacy", "Sign-In Required to View"));
    d->protectedRBtn->setWhatsThis(i18n("Unlisted album require Sign-In to View"));

    QVBoxLayout* const radioLayout = new QVBoxLayout;
    radioLayout->addWidget(d->publicRBtn);
    radioLayout->addWidget(d->unlistedRBtn);
    radioLayout->addWidget(d->protectedRBtn);

    QFormLayout* const privBoxLayout = new QFormLayout;
    privBoxLayout->addRow(i18n("Privacy:"), radioLayout);
    privBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    privBoxLayout->setSpacing(spacing);
    privBox->setLayout(privBoxLayout);

    if (!(QString::compare(d->serviceName,
                           QString::fromLatin1("googledriveexport"),
                           Qt::CaseInsensitive) == 0))
    {
        addToMainLayout(privBox);
    }
    else
    {
        privBox->hide();
        hideDateTime();
        hideDesc();
        hideLocation();
        getMainWidget()->setMinimumSize(300,0);
    }
}

GSNewAlbumDlg::~GSNewAlbumDlg()
{
    delete d;
}

void GSNewAlbumDlg::getAlbumProperties(GSFolder& album)
{
    if (QString::compare(d->serviceName,
                         QString::fromLatin1("googledriveexport"),
                         Qt::CaseInsensitive) == 0)
    {
        album.title = getTitleEdit()->text();
        return;
    }

    album.title       = getTitleEdit()->text();
    album.description = getDescEdit()->toPlainText();
    album.location    = getLocEdit()->text();

    if (d->publicRBtn->isChecked())
        album.access = QString::fromLatin1("public");
    else if (d->unlistedRBtn->isChecked())
        album.access = QString::fromLatin1("private");
    else
        album.access = QString::fromLatin1("protected");

    long long timestamp = getDateTimeEdit()->dateTime().toTime_t();
    album.timestamp     = QString::number(timestamp * 1000);
}

} // namespace Digikam
