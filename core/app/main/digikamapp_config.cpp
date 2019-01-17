/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation - Configure
 *
 * Copyright (C) 2002-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "digikamapp.h"
#include "digikamapp_p.h"

namespace Digikam
{

bool DigikamApp::setup()
{
    return Setup::execDialog(this, Setup::LastPageUsed);
}

bool DigikamApp::setupICC()
{
    return Setup::execSinglePage(this, Setup::ICCPage);
}

void DigikamApp::slotSetup()
{
    setup();
}

void DigikamApp::slotSetupChanged()
{
    // raw loading options might have changed
    LoadingCacheInterface::cleanCache();

    // TODO: clear history when location changed
    //if(ApplicationSettings::instance()->getAlbumLibraryPath() != AlbumManager::instance()->getLibraryPath())
    //  d->view->clearHistory();

    const DbEngineParameters prm = ApplicationSettings::instance()->getDbEngineParameters();

    if (!AlbumManager::instance()->databaseEqual(prm))
    {
        AlbumManager::instance()->changeDatabase(ApplicationSettings::instance()->getDbEngineParameters());
    }

    if (ApplicationSettings::instance()->getShowFolderTreeViewItemsCount())
    {
        AlbumManager::instance()->prepareItemCounts();
    }

    // Load full-screen options
    KConfigGroup group = KSharedConfig::openConfig()->group(configGroupName());
    readFullScreenSettings(group);

    d->view->applySettings();

    AlbumThumbnailLoader::instance()->setThumbnailSize(ApplicationSettings::instance()->getTreeViewIconSize());

    if (LightTableWindow::lightTableWindowCreated())
    {
        LightTableWindow::lightTableWindow()->applySettings();
    }

    if (QueueMgrWindow::queueManagerWindowCreated())
    {
        QueueMgrWindow::queueManagerWindow()->applySettings();
    }

    d->config->sync();
}

void DigikamApp::slotEditKeys()
{
    editKeyboardShortcuts();
}

void DigikamApp::slotThemeChanged()
{
    ApplicationSettings::instance()->setCurrentTheme(ThemeManager::instance()->currentThemeName());
}

} // namespace Digikam
