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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes.

#include <qlayout.h>
#include <qstring.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>

// libkipi includes.

#include <libkipi/pluginloader.h>
#include <libkipi/version.h>

// Local includes.

#include "setupplugins.h"
#include "setupplugins.moc"

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

SetupPlugins::SetupPlugins(QWidget* parent )
            : QWidget(parent)
{
    d = new SetupPluginsPriv;
    QVBoxLayout *layout = new QVBoxLayout(parent);
    d->pluginsNumber    = new QLabel(parent);
    d->pluginsNumber->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->kipiConfig = KIPI::PluginLoader::instance()->configWidget( parent );
    QString pluginsListHelp = i18n("<p>A list of available Kipi plugins appears below.");
    QWhatsThis::add(d->kipiConfig, pluginsListHelp);

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
    d->pluginsNumber->setText(i18n("1 Kipi plugin found",
                                   "%n Kipi plugins found",
                                   kipiPluginsNumber));
}

void SetupPlugins::applyPlugins()
{
    d->kipiConfig->apply();
}

}  // namespace Digikam
