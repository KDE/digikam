/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-08-27
 * @brief  Setup widget for geo correlator.
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "gpssetupgeneral.h"

// Qt includes

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>

// local includes

#include "gpscommon.h"

namespace Digikam
{

class SetupGeneral::Private
{
public:

    Private()
    {
        cbMapLayout = 0;
    }

    QComboBox* cbMapLayout;
};

SetupGeneral::SetupGeneral(QWidget* const parent)
    : GPSSetupTemplate(parent),
      d(new Private())
{
    QVBoxLayout* const mainLayout  = new QVBoxLayout(this);

    QHBoxLayout* const hboxLayout1 = new QHBoxLayout();
    d->cbMapLayout                 = new QComboBox(this);
    d->cbMapLayout->addItem(i18n("One map"), QVariant::fromValue(MapLayoutOne));
    d->cbMapLayout->addItem(i18n("Two maps - horizontal"), QVariant::fromValue(MapLayoutHorizontal));
    d->cbMapLayout->addItem(i18n("Two maps - vertical"), QVariant::fromValue(MapLayoutVertical));

    QLabel* const labelMapLayout   = new QLabel(i18n("Layout:"), this);
    labelMapLayout->setBuddy(d->cbMapLayout);
    hboxLayout1->addWidget(labelMapLayout);
    hboxLayout1->addWidget(d->cbMapLayout);
    hboxLayout1->addStretch(10);

    mainLayout->addLayout(hboxLayout1);

    readSettings();
}

SetupGeneral::~SetupGeneral()
{
    delete d;
}

void SetupGeneral::readSettings()
{
    GPSSetupGlobalObject* const setupGlobalObject = GPSSetupGlobalObject::instance();
    const MapLayout targetLayout = setupGlobalObject->readEntry(QStringLiteral("Map Layout")).value<MapLayout>();

    for (int i=0; i<d->cbMapLayout->count(); ++i)
    {
        const MapLayout itemLayout = d->cbMapLayout->itemData(i).value<MapLayout>();

        if (itemLayout == targetLayout)
        {
            d->cbMapLayout->setCurrentIndex(i);
            break;
        }
    }
}

void SetupGeneral::slotApplySettings()
{
    GPSSetupGlobalObject* const setupGlobalObject = GPSSetupGlobalObject::instance();
    setupGlobalObject->writeEntry(QStringLiteral("Map Layout"), d->cbMapLayout->itemData(d->cbMapLayout->currentIndex()));
}

} /* namespace Digikam */
