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

NewAlbumDlg::NewAlbumDlg(QWidget* const parent,
                         const QString& serviceName,
                         const QString& pluginName)
    : NewAlbumDialog(parent, pluginName)
{
    m_serviceName            = serviceName;
    const int spacing        = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGroupBox* const privBox = new QGroupBox(i18n("Access Level"), getMainWidget());
    privBox->setWhatsThis(i18n("These are security and privacy settings for the new Google Photos/PicasaWeb album."));

    m_publicRBtn        = new QRadioButton(i18nc("google photos/picasaweb album privacy", "Public"));
    m_publicRBtn->setChecked(true);
    m_publicRBtn->setWhatsThis(i18n("Public album is listed on your public Google Photos/PicasaWeb page."));
    m_unlistedRBtn      = new QRadioButton(i18nc("google photos/picasaweb album privacy", "Unlisted / Private"));
    m_unlistedRBtn->setWhatsThis(i18n("Unlisted album is only accessible via URL."));
    m_protectedRBtn     = new QRadioButton(i18nc("google photos/picasaweb album privacy", "Sign-In Required to View"));
    m_protectedRBtn->setWhatsThis(i18n("Unlisted album require Sign-In to View"));

    QVBoxLayout* const radioLayout = new QVBoxLayout;
    radioLayout->addWidget(m_publicRBtn);
    radioLayout->addWidget(m_unlistedRBtn);
    radioLayout->addWidget(m_protectedRBtn);

    QFormLayout* const privBoxLayout = new QFormLayout;
    privBoxLayout->addRow(i18n("Privacy:"), radioLayout);
    privBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    privBoxLayout->setSpacing(spacing);
    privBox->setLayout(privBoxLayout);

    if (!(QString::compare(m_serviceName,
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

NewAlbumDlg::~NewAlbumDlg()
{
}

void NewAlbumDlg::getAlbumProperties(GSFolder& album)
{
    if (QString::compare(m_serviceName,
                         QString::fromLatin1("googledriveexport"),
                         Qt::CaseInsensitive) == 0)
    {
        album.title = getTitleEdit()->text();
        return;
    }

    album.title       = getTitleEdit()->text();
    album.description = getDescEdit()->toPlainText();
    album.location    = getLocEdit()->text();

    if (m_publicRBtn->isChecked())
        album.access = QString::fromLatin1("public");
    else if (m_unlistedRBtn->isChecked())
        album.access = QString::fromLatin1("private");
    else
        album.access = QString::fromLatin1("protected");

    long long timestamp = getDateTimeEdit()->dateTime().toTime_t();
    album.timestamp     = QString::number(timestamp * 1000);
}

} // namespace Digikam
