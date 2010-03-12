/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-17
 * Description : Color profile conversion tool.
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "iccconvert.moc"

// Qt includes

#include <QWidget>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "dimg.h"
#include "icctransform.h"
#include "icctransformfilter.h"
#include "iccprofilesettings.h"
#include "iccprofile.h"
#include "iccsettings.h"
#include "iccsettingscontainer.h"

namespace Digikam
{

IccConvert::IccConvert(QObject* parent)
          : BatchTool("IccConvert", ColorTool, parent)
{
    setToolTitle(i18n("ICC Convert"));
    setToolDescription(i18n("A tool to convert image to a color space."));
    setToolIcon(KIcon(SmallIcon("colormanagement")));

    QWidget* box   = new QWidget;
    m_settingsView = new IccProfilesSettings(box);
    setSettingsWidget(box);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

IccConvert::~IccConvert()
{
}

BatchToolSettings IccConvert::defaultSettings()
{
    BatchToolSettings prm;
    IccProfile defaultProf = m_settingsView->defaultProfile();

    prm.insert("ProfilePath", defaultProf.filePath());

    return prm;
}

void IccConvert::slotAssignSettings2Widget()
{
    QString profPath = settings()["ProfilePath"].toString();
    m_settingsView->setCurrentProfile(profPath);
}

void IccConvert::slotSettingsChanged()
{
    BatchToolSettings prm;
    IccProfile currentProf = m_settingsView->currentProfile();

    prm.insert("ProfilePath", currentProf.filePath());

    BatchTool::slotSettingsChanged(prm);
}

bool IccConvert::toolOperations()
{
    if (!loadToDImg()) return false;

    QString              profPath = settings()["ProfilePath"].toString();
    IccProfile           in = image().getIccProfile();
    IccProfile           out(profPath);
    ICCSettingsContainer settings = IccSettings::instance()->settings();
    IccTransform         transform;

    transform.setIntent(settings.renderingIntent);
    transform.setUseBlackPointCompensation(settings.useBPC);
    transform.setInputProfile(in);
    transform.setOutputProfile(out);

    IccTransformFilter icc(&image(), 0L, transform);
    icc.startFilterDirectly();
    image().putImageData(icc.getTargetImage().bits());

    return (savefromDImg());
}

}  // namespace Digikam
