/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * Copyright (C) 2011      by Lukas Krejci <krejci.l at centrum dot cz>
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

#include "rajcenewalbumdlg.h"

// Qt includes

#include <QCheckBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

RajceNewAlbumDlg::RajceNewAlbumDlg(QWidget* const parent)
    : WSNewAlbumDialog(parent,
      QString::fromLatin1("Rajce.net"))
{
    getLocEdit()->hide();
    getDateTimeEdit()->hide();

    QGroupBox* const privBox = new QGroupBox(i18n("Visibility"), getMainWidget());
    privBox->setWhatsThis(i18n("Set the visibility of the album"));

    QLabel* const lbl   = new QLabel(i18n("Public"), privBox);

    m_albumVisible = new QCheckBox;
    m_albumVisible->setChecked(true);

    QGridLayout* const layout = new QGridLayout(privBox);

    layout->addWidget(lbl,             0, 0, 1, 1);
    layout->addWidget(m_albumVisible,  0, 1, 1, 1);

    addToMainLayout(privBox);
}

RajceNewAlbumDlg::~RajceNewAlbumDlg()
{
}

QString RajceNewAlbumDlg::albumDescription() const
{
    return getDescEdit()->toPlainText();
}

QString RajceNewAlbumDlg::albumName() const
{
    return getTitleEdit()->text();
}

bool RajceNewAlbumDlg::albumVisible() const
{
    return m_albumVisible->isChecked();
}

} // namespace Digikam
