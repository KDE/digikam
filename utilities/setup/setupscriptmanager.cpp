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

//KDE includes

#include <KDialog>
#include <KLocale>

//local includes

#include "setupscriptmanager.h"

namespace Digikam
{

class SetupScriptManagerPriv
{
public:

    SetupScriptManagerPriv() :
        configGroupName("ScriptManager Settings")

    {
        scriptLoaded = 0;
        scriptDebug  = 0;
        scriptInfo   = 0;
        group        = 0;
    }

    const QString configGroupName;

    QCheckBox*    scriptLoaded;
    QPushButton*  scriptDebug;
    QPushButton*  scriptInfo;
    QGroupBox*    group;
};

SetupScriptManager::SetupScriptManager(QWidget* parent)
                  : QScrollArea(parent), d(new SetupScriptManagerPriv)
{
    //The Script Manager Window is arranged as follows
    //initially there is a QGroupBox to which Vertical Layout is assigned
    //every time a new Script needs to be added , another Group Box containing
    //a horizontal group of widgets ( a checkbox and a debug button and an info button
    //is added to the Vertical Group Box.

    //I have looked at the code of setupmetadata.cpp for GUI arrangement
    d->group = new QGroupBox(viewport());
    setWidget(d->group);
    setWidgetResizable(true);

    //QWidget* panel          = new QWidget(d->group);
    //QVBoxLayout* mainLayout = new QVBoxLayout(panel);
    QVBoxLayout* mainLayout = new QVBoxLayout(d->group);

    //-----------------End of Main Layout-------------

    //create a new function which takes in the plugin name etc
    //and returns the widget which can be added to the GroupBox.
    QGroupBox* individualPluginGroup     = new QGroupBox(d->group);//panel);
    individualPluginGroup->setTitle(i18n("Single Plugin"));
    QHBoxLayout* individualPluginHLayout = new QHBoxLayout(individualPluginGroup);

    d->scriptLoaded = new QCheckBox(i18n("Name of the Script"));
    d->scriptDebug  = new QPushButton(i18n("Debug"));
    d->scriptInfo   = new QPushButton(i18n("Info"));

    individualPluginHLayout->addWidget(d->scriptLoaded);
    individualPluginHLayout->addWidget(d->scriptDebug);
    individualPluginHLayout->addWidget(d->scriptInfo);
    individualPluginHLayout->setSpacing(0);

    //-----------end of widget set for a single Script---------

    mainLayout->addWidget(individualPluginGroup);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(KDialog::spacingHint());
    mainLayout->addStretch();

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
}

} // namespace Digikam