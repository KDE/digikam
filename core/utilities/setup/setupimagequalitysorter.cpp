/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : Image Quality setup page
 *
 * Copyright (C) 2013-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
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

#include "setupimagequalitysorter.h"

// Local includes

#include "imagequalitysettings.h"

namespace Digikam
{

class Q_DECL_HIDDEN SetupImageQualitySorter::Private
{
public:

    explicit Private()
      : settingsWidget(0)
    {
    }

    ImageQualitySettings* settingsWidget;
};

// --------------------------------------------------------

SetupImageQualitySorter::SetupImageQualitySorter(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    d->settingsWidget = new ImageQualitySettings(viewport());
    setWidget(d->settingsWidget);
    setWidgetResizable(true);

    d->settingsWidget->readSettings();
}

SetupImageQualitySorter::~SetupImageQualitySorter()
{
    delete d;
}

void SetupImageQualitySorter::applySettings()
{
    d->settingsWidget->applySettings();
}

void SetupImageQualitySorter::readSettings()
{
    d->settingsWidget->readSettings();
}

} // namespace Digikam
