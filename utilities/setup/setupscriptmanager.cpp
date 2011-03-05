/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 19-07-2010
 * Description : script manager for digiKam
 *
 * Copyright (C) 2010 Created By: Kunal Ghosh <kunal dot t2 at gmail dot com>
 * Copyright (C) 2010 Created By: Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupscriptmanager.moc"

//Qt Includes

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QString>

//KDE includes

#include <KDialog>
#include <KLocale>

//local includes

#include "setupscriptmanager.h"
#include "scriptplugintype.h"

namespace Digikam
{

class SetupScriptManager::SetupScriptManagerPriv
{
public:

    SetupScriptManagerPriv() :
        configGroupName("ScriptManager Settings")

    {
        group        = 0;
        pluginGroup  = 0;
        mainLayout   = 0;
        count        = 0;
    }

    const QString configGroupName;

    QGroupBox*    group;
    QGroupBox*    pluginGroup;
    QVBoxLayout*  mainLayout;
    int*          count;//count of the number of plugins loaded
};

SetupScriptManager::SetupScriptManager(QWidget* parent)
    : QScrollArea(parent), d(new SetupScriptManagerPriv)
{
    //first create a GroupBox
    d->group = new QGroupBox(viewport());
    setWidget(d->group);
    setWidgetResizable(true);

    //create a vbox layout and set the layout to the GroupBox
    d->mainLayout = new QVBoxLayout;
    d->group->setLayout(d->mainLayout);
    d->count = new int;

    //read settings from a xml file
    readSettings();
}

SetupScriptManager::~SetupScriptManager()
{
    delete d;
}

void SetupScriptManager::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    // TODO
    config->sync();
}

void SetupScriptManager::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    // TODO
    // Every time we read a setting value it must call the addEntry method to
    //add the entry for the individual script plugin

    //The first entry into the scriptManager
    ScriptPluginType* plugin = new ScriptPluginType();
    //plugin->createPlugin(name,path,mod);
    plugin->createPlugin("Name","",false);
    addEntry(plugin);

    //Second entry into the scriptManager
    ScriptPluginType* plugin2 = new ScriptPluginType();
    plugin2->createPlugin("Plugin2","",false);
    addEntry(plugin2);

    ScriptPluginType* plugin3 = new ScriptPluginType();
    plugin3->createPlugin("Plugin3","",false);
    addEntry(plugin3);

    d->mainLayout->setMargin(0);
    d->mainLayout->setSpacing(KDialog::spacingHint());
    d->mainLayout->addStretch();
}

void SetupScriptManager::addEntry(ScriptPluginType* plugin)
{
    //create a set of widgets
    QCheckBox* box             = new QCheckBox(plugin->name());
    QPushButton* btnDebug      = new QPushButton(i18n("Debug"));
    QPushButton* btnInfo       = new QPushButton(i18n("Info"));
    //add the widgets to a Horizontal layout whose parent is a GroupBox
    QHBoxLayout* pluginHLayout = new QHBoxLayout;
    //add the individual plugins to the horizontal layout
    pluginHLayout->addWidget(box,Qt::AlignLeft);//the second argument is stretch
    pluginHLayout->addWidget(btnDebug,0,Qt::AlignRight);
    pluginHLayout->addWidget(btnInfo,0,Qt::AlignRight);
    pluginHLayout->setSpacing(0);

    d->mainLayout->insertLayout(-1,pluginHLayout);
    d->count++;
    //the index is negative(-1) to add the layout at the end
}

} // namespace Digikam
