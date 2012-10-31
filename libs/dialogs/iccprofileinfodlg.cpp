/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-16
 * Description : a dialog to display ICC profile information.
 *
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "iccprofileinfodlg.h"

// KDE includes

#include <klocale.h>

// Local includes

#include "iccprofilewidget.h"

namespace Digikam
{

ICCProfileInfoDlg::ICCProfileInfoDlg(QWidget* const parent, const QString& profilePath, const IccProfile& profile)
    : KDialog(parent)
{
    setCaption(i18n("Color Profile Info - %1", profilePath));
    setButtons(Help|Ok);
    setDefaultButton(Ok);
    setModal(true);
    setHelp("iccprofile.anchor", "digikam");

    ICCProfileWidget* const profileWidget = new ICCProfileWidget(this, 340, 256);

    if (profile.isNull())
    {
        profileWidget->loadFromURL(KUrl(profilePath));
    }
    else
    {
        profileWidget->loadProfile(profilePath, profile);
    }

    setMainWidget(profileWidget);
}

ICCProfileInfoDlg::~ICCProfileInfoDlg()
{
}

}  // namespace Digikam
