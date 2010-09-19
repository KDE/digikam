/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-18
 * Description : lens auto-fix batch tool.
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

#include "lensautofix.moc"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QWidget>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <kdialog.h>
#include <kseparator.h>

// Local includes

#include "dimg.h"
#include "lensfunsettings.h"
#include "lensfuncameraselector.h"
#include "lensfuniface.h"

namespace Digikam
{

class LensAutoFix::LensAutoFixPriv
{
public:

    LensAutoFixPriv()
        : settingsView(0),
          cameraSelector(0),
          lfIface(0)
    {}

    LensFunSettings*       settingsView;
    LensFunCameraSelector* cameraSelector;
    LensFunIface*          lfIface;
};

LensAutoFix::LensAutoFix(QObject* parent)
           : BatchTool("LensAutoFix", EnhanceTool, parent),
             d(new LensAutoFixPriv)
{
    setToolTitle(i18n("Lens Auto-fix Image"));
    setToolDescription(i18n("A tool to fix automatically lens distorsions"));
    setToolIcon(KIcon(SmallIcon("lensautofix")));

    d->lfIface        = new LensFunIface();

    QWidget* box      = new QWidget;
    d->cameraSelector = new LensFunCameraSelector(d->lfIface);
    KSeparator* line  = new KSeparator(Qt::Horizontal);
    d->settingsView   = new LensFunSettings();
    d->cameraSelector->setPassiveMetadataUsage(true);
    d->cameraSelector->setUseMetadata(true);

    QGridLayout* grid = new QGridLayout(box);
    grid->addWidget(d->cameraSelector, 0, 0, 1, 2);
    grid->addWidget(line,              1, 0, 1, 2);
    grid->addWidget(d->settingsView,   2, 0, 1, 2);
    grid->setRowStretch(3, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    setSettingsWidget(box);

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(d->cameraSelector, SIGNAL(signalLensSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

LensAutoFix::~LensAutoFix()
{
    delete d;
}

BatchToolSettings LensAutoFix::defaultSettings()
{
    BatchToolSettings settings;

    //settings.insert("Radius", (double)m_radiusInput->defaultValue());

    return settings;
}

void LensAutoFix::slotAssignSettings2Widget()
{
    //m_radiusInput->setValue(settings()["Radius"].toDouble());
}

void LensAutoFix::slotSettingsChanged()
{
    BatchToolSettings settings;

    //settings.insert("Radius", (double)m_radiusInput->value());

    BatchTool::slotSettingsChanged(settings);
}

bool LensAutoFix::toolOperations()
{
    if (!loadToDImg())
        return false;

//    double radius = settings()["Radius"].toDouble();

    LensFunContainer settings;
    DMetadata        meta(image().getMetadata());
    bool             ret = d->lfIface->findFromMetadata(meta, settings);
    if (ret)
    {
        d->lfIface->setSettings(settings);
        LensFunFilter lfFilter(&image(), 0L, d->lfIface);
        lfFilter.startFilterDirectly();
        image().putImageData(lfFilter.getTargetImage().bits());
        return savefromDImg();
    }
    return false;
}

} // namespace Digikam
