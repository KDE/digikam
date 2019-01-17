/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : digiKam image editor - Configure
 *
 * Copyright (C) 2004-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagewindow.h"
#include "imagewindow_p.h"

namespace Digikam
{

void ImageWindow::slotSetup()
{
    Setup::execDialog(this);
}

void ImageWindow::slotSetupICC()
{
    Setup::execSinglePage(this, Setup::ICCPage);
}

void ImageWindow::slotSetupChanged()
{
    applyStandardSettings();

    VersionManagerSettings versionSettings = ApplicationSettings::instance()->getVersionManagerSettings();
    d->versionManager.setSettings(versionSettings);
    m_nonDestructive                       = versionSettings.enabled;
    toggleNonDestructiveActions();

    d->imageFilterModel->setStringTypeNatural(ApplicationSettings::instance()->isStringTypeNatural());
    d->imageFilterModel->setSortRole((ItemSortSettings::SortRole)ApplicationSettings::instance()->getImageSortOrder());
    d->imageFilterModel->setSortOrder((ItemSortSettings::SortOrder)ApplicationSettings::instance()->getImageSorting());
    d->rightSideBar->setStyle(ApplicationSettings::instance()->getSidebarTitleStyle());
}

} // namespace Digikam
