/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-01-02
 * Description : setup Kipi plugins tab.
 *
 * Copyright (C) 2004-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupplugins.moc"

// Qt includes

#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QString>
#include <QVBoxLayout>
#include <QGridLayout>

// KDE includes

#include <kdialog.h>
#include <klocale.h>

// Libkipi includes

#include <libkipi/pluginloader.h>
#include <libkipi/version.h>

namespace Digikam
{

class SetupPlugins::SetupPluginsPriv
{
public:

    SetupPluginsPriv() :
        pluginsNumber(0),
        pluginsNumberActivated(0),
        kipiConfig(0)
    {
    }

    QLabel*             pluginsNumber;
    QLabel*             pluginsNumberActivated;

    KIPI::ConfigWidget* kipiConfig;
};

SetupPlugins::SetupPlugins(QWidget* parent)
    : QScrollArea(parent), d(new SetupPluginsPriv)
{
    QWidget* panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QGridLayout* mainLayout   = new QGridLayout;
    d->pluginsNumber          = new QLabel;
    d->pluginsNumberActivated = new QLabel;

    if (KIPI::PluginLoader::instance())
    {
        d->kipiConfig = KIPI::PluginLoader::instance()->configWidget(panel);
        d->kipiConfig->setWhatsThis(i18n("A list of available Kipi plugins."));
    }

    mainLayout->addWidget(d->pluginsNumber,             0, 0, 1, 1);
    mainLayout->addWidget(d->pluginsNumberActivated,    0, 1, 1, 1);
    mainLayout->addWidget(d->kipiConfig,                1, 0, 1, -1);
    mainLayout->setColumnStretch(2, 10);
    mainLayout->setMargin(KDialog::spacingHint());
    mainLayout->setSpacing(KDialog::spacingHint());

    panel->setLayout(mainLayout);

    initPlugins();

    // --------------------------------------------------------

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    panel->setAutoFillBackground(false);
}

SetupPlugins::~SetupPlugins()
{
    delete d;
}

void SetupPlugins::initPlugins()
{
    if (KIPI::PluginLoader::instance())
    {
        KIPI::PluginLoader::PluginList list = KIPI::PluginLoader::instance()->pluginList();
        d->pluginsNumber->setText(i18np("1 Kipi plugin found",
                                        "%1 Kipi plugins found",
                                        list.count()));

        int activated = 0;
        KIPI::PluginLoader::PluginList::const_iterator it = list.constBegin();
        for (; it != list.constEnd(); ++it)
        {
            if ((*it)->shouldLoad())
            {
                ++activated;
            }
        }

        d->pluginsNumberActivated->setText(i18nc("%1: number of plugins activated",
                                                 "(%1 activated)", activated));
    }
}

void SetupPlugins::applyPlugins()
{
    if (KIPI::PluginLoader::instance())
    {
        d->kipiConfig->apply();
    }
}

}  // namespace Digikam
