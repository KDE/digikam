/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "databasepage.h"

// Qt includes

#include <QApplication>
#include <QStyle>
#include <QLabel>
#include <QVBoxLayout>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "dbengineparameters.h"
#include "dbsettingswidget.h"

namespace Digikam
{

class DatabasePage::Private
{
public:

    explicit Private()
      : dbsettingswidget(0)
    {
    }

    DatabaseSettingsWidget* dbsettingswidget;
};

DatabasePage::DatabasePage(QWizard* const dlg)
    : DWizardPage(dlg, i18n("<b>Configure where you will store databases</b>")),
      d(new Private)
{
    d->dbsettingswidget = new DatabaseSettingsWidget(this);

    setPageWidget(d->dbsettingswidget);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("network-server-database")));
}

DatabasePage::~DatabasePage()
{
    delete d;
}

void DatabasePage::setDatabasePath(const QString& path)
{
    d->dbsettingswidget->setDatabasePath(path);
}

DbEngineParameters DatabasePage::getDbEngineParameters() const
{
    return d->dbsettingswidget->getDbEngineParameters();
}

void DatabasePage::saveSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    DbEngineParameters params = d->dbsettingswidget->getDbEngineParameters();
    params.writeToConfig(config);

    config->sync();
}

bool DatabasePage::checkSettings()
{
    // TODO : add checks for Mysql Server.
    return d->dbsettingswidget->checkDatabaseSettings();
}

} // namespace Digikam
