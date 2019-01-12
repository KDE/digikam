/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : Setup view panel for dplugins.
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "showfotosetupplugins.h"

// Qt includes

#include <QTabWidget>

// Local includes

#include "dpluginsetup.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN SetupPlugins::Private
{
public:

    explicit Private()
      : tab(0),
        setupGeneric(0),
        setupEditor(0)
    {
    }

    QTabWidget*            tab;

    Digikam::DPluginSetup* setupGeneric;
    Digikam::DPluginSetup* setupEditor;
};

SetupPlugins::SetupPlugins(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    d->tab = new QTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    // --------------------

    d->setupGeneric = new Digikam::DPluginSetup(Digikam::DPluginAction::Generic, d->tab);
    d->tab->insertTab(Generic, d->setupGeneric, i18nc("@title:tab", "Generic"));

    // --------------------

    d->setupEditor = new Digikam::DPluginSetup(Digikam::DPluginAction::Editor, d->tab);
    d->tab->insertTab(Editor, d->setupEditor, i18nc("@title:tab", "Image Editor"));
}

SetupPlugins::~SetupPlugins()
{
    delete d;
}

void SetupPlugins::applySettings()
{
    d->setupGeneric->applySettings();
    d->setupEditor->applySettings();
}

}  // namespace ShowFoto
