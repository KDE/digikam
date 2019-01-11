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

// Local includes

#include "dplugingenericsetup.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN SetupPlugins::Private
{
public:

    explicit Private()
      : setupView(0)
    {
    }

    Digikam::DPluginGenericSetup* setupView;
};

SetupPlugins::SetupPlugins(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    d->setupView = new Digikam::DPluginGenericSetup(viewport());

    setWidget(d->setupView);
    setWidgetResizable(true);
}

SetupPlugins::~SetupPlugins()
{
    delete d;
}

void SetupPlugins::applySettings()
{
    d->setupView->applySettings();
}

}  // namespace ShowFoto
