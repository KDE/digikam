/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-01-02
 * Description : setup Kipi plugins tab.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupplugins.h"
#include "setupplugins.moc"

// Qt includes.

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QString>
#include <QLabel>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>

// LibKIPI includes.

#include <libkipi/pluginloader.h>
#include <libkipi/version.h>

namespace Digikam
{

class SetupPluginsPriv
{
public:

    SetupPluginsPriv()
    {
        pluginsNumber = 0;
        kipiConfig    = 0;
    }

    QLabel*             pluginsNumber;

    KIPI::ConfigWidget* kipiConfig;
};

SetupPlugins::SetupPlugins(QWidget* parent)
            : QWidget(parent), d(new SetupPluginsPriv)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    d->pluginsNumber    = new QLabel(this);
    d->kipiConfig       = KIPI::PluginLoader::instance()->configWidget(this);
    d->kipiConfig->setWhatsThis(i18n("A list of available Kipi plugins appears below."));

    layout->addWidget(d->pluginsNumber);
    layout->addWidget(d->kipiConfig);
    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
}

SetupPlugins::~SetupPlugins()
{
    delete d;
}

void SetupPlugins::initPlugins(int kipiPluginsNumber)
{
    d->pluginsNumber->setText(i18np("1 Kipi plugin found",
                                    "%1 Kipi plugins found",
                                    kipiPluginsNumber));
}

void SetupPlugins::applyPlugins()
{
    d->kipiConfig->apply();
}

}  // namespace Digikam
