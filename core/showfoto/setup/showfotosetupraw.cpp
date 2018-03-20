/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : setup RAW decoding settings.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "showfotosetupraw.h"

// Qt includes

#include <QTabWidget>

// Local includes

#include "setupraw.h"

namespace ShowFoto
{

class SetupRaw::Private
{
public:


    Private() :
        tab(0),
        raw(0)
    {
    }

    QTabWidget*        tab;
    Digikam::SetupRaw* raw;
};

SetupRaw::SetupRaw(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    d->tab = new QTabWidget;
    d->raw = new Digikam::SetupRaw(d->tab);

    setWidget(d->tab);
    setWidgetResizable(true);

    readSettings();
}

SetupRaw::~SetupRaw()
{
    delete d;
}

void SetupRaw::applySettings()
{
    d->raw->applySettings();
}

void SetupRaw::readSettings()
{
    d->raw->readSettings();
}

}  // namespace ShowFoto
