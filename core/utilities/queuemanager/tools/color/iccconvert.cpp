/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-17
 * Description : Color profile conversion tool.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "iccconvert.h"

// Qt includes

#include <QLabel>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "dmetadata.h"
#include "icctransform.h"
#include "icctransformfilter.h"
#include "iccprofilesettings.h"
#include "iccprofile.h"
#include "iccsettings.h"
#include "iccsettingscontainer.h"

namespace Digikam
{

IccConvert::IccConvert(QObject* const parent)
    : BatchTool(QLatin1String("IccConvert"), ColorTool, parent),
      m_settingsView(0)
{
    setToolTitle(i18n("ICC Convert"));
    setToolDescription(i18n("Convert image to a color space."));
    setToolIconName(QLatin1String("preferences-desktop-display-color"));
}

IccConvert::~IccConvert()
{
}

void IccConvert::registerSettingsWidget()
{
    DVBox* const vbox   = new DVBox;
    m_settingsView      = new IccProfilesSettings(vbox);
    QLabel* const space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);
    m_settingsWidget = vbox;

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings IccConvert::defaultSettings()
{
    BatchToolSettings prm;
    IccProfile defaultProf = m_settingsView->defaultProfile();

    prm.insert(QLatin1String("ProfilePath"), defaultProf.filePath());

    return prm;
}

void IccConvert::slotAssignSettings2Widget()
{
    QString profPath = settings()[QLatin1String("ProfilePath")].toString();
    m_settingsView->setCurrentProfile(profPath);
}

void IccConvert::slotSettingsChanged()
{
    BatchToolSettings prm;
    IccProfile currentProf = m_settingsView->currentProfile();

    prm.insert(QLatin1String("ProfilePath"), currentProf.filePath());

    BatchTool::slotSettingsChanged(prm);
}

bool IccConvert::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    QString              profPath = settings()[QLatin1String("ProfilePath")].toString();
    IccProfile           in       = image().getIccProfile();
    IccProfile           out(profPath);
    ICCSettingsContainer settings = IccSettings::instance()->settings();
    IccTransform         transform;

    transform.setIntent(settings.renderingIntent);
    transform.setUseBlackPointCompensation(settings.useBPC);
    transform.setInputProfile(in);
    transform.setOutputProfile(out);

    IccTransformFilter icc(&image(), 0L, transform);
    applyFilter(&icc);

    image().setIccProfile(icc.getTargetImage().getIccProfile());

    DMetadata meta(image().getMetadata());
    meta.removeExifColorSpace();
    image().setMetadata(meta.data());

    return (savefromDImg());
}

}  // namespace Digikam
