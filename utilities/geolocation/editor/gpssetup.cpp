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

#include "gpssetup.h"

// Qt includes

#include <QPushButton>

// KDE includes

#include <kconfig.h>
#include <klocalizedstring.h>

// local includes

#include "gpssetupgeneral.h"

namespace Digikam
{

class GPSSetupGlobalObjectCreator
{
public:

    GPSSetupGlobalObject object;
};

Q_GLOBAL_STATIC(GPSSetupGlobalObjectCreator, setupGlobalObjectCreator)

// ----------------------------------------------------------------------------------

class GPSSetupGlobalObject::Private
{
public:

    Private()
    {
    }

    QHash<QString, QVariant> settings;
};

GPSSetupGlobalObject::GPSSetupGlobalObject()
    : QObject(),
      d(new Private())
{
}

GPSSetupGlobalObject::~GPSSetupGlobalObject()
{
    delete d;
}

GPSSetupGlobalObject* GPSSetupGlobalObject::instance()
{
    return &(setupGlobalObjectCreator->object);
}

QVariant GPSSetupGlobalObject::readEntry(const QString& name)
{
    return d->settings.value(name);
}

void GPSSetupGlobalObject::writeEntry(const QString& name, const QVariant& value)
{
    d->settings[name] = value;
}

void GPSSetupGlobalObject::triggerSignalSetupChanged()
{
    emit(signalSetupChanged());
}

// ----------------------------------------------------------------------------------

GPSSetupTemplate::GPSSetupTemplate(QWidget* const parent)
    : QWidget(parent)
{
}

GPSSetupTemplate::~GPSSetupTemplate()
{
}

// ----------------------------------------------------------------------------------

class GPSSetup::Private
{
public:

    Private()
    {
        pageGeneral  = 0;
        setupGeneral = 0;
    }

    KPageWidgetItem* pageGeneral;
    SetupGeneral*    setupGeneral;
};

GPSSetup::GPSSetup(QWidget* const parent)
    : KPageDialog(parent),
      d(new Private())
{
    setWindowTitle(i18n("Configure"));
    setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok | QDialogButtonBox::Apply);
    button(QDialogButtonBox::Ok)->setDefault(true);
    setModal(true);
    setFaceType(List);
    
    d->setupGeneral = new SetupGeneral(this);
    d->pageGeneral  = addPage(d->setupGeneral, i18nc("General setup", "General"));

    connect(buttonBox()->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &GPSSetup::slotApplyClicked);

    connect(buttonBox()->button(QDialogButtonBox::Ok), &QPushButton::clicked,
            this, &GPSSetup::slotOkClicked);
}

GPSSetup::~GPSSetup()
{
    delete d;
}

void GPSSetup::slotApplyClicked()
{
    d->setupGeneral->slotApplySettings();

    GPSSetupGlobalObject::instance()->triggerSignalSetupChanged();
}

void GPSSetup::slotOkClicked()
{
    slotApplyClicked();
    accept();
}

} /* namespace Digikam */
