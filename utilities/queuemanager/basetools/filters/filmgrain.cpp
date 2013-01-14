/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-03
 * Description : a batch tool to addd film grain to images.
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

#include "filmgrain.moc"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QWidget>

// KDE includes

#include <klocale.h>
#include <kdialog.h>
#include <kglobal.h>

// Local includes

#include "dimg.h"
#include "filmgrainfilter.h"
#include "filmgrainsettings.h"

namespace Digikam
{

FilmGrain::FilmGrain(QObject* parent)
    : BatchTool("FilmGrain", FiltersTool, parent)
{
    m_settingsView = 0;

    setToolTitle(i18n("Film Grain"));
    setToolDescription(i18n("Add film grain"));
    setToolIconName("filmgrain");
}

FilmGrain::~FilmGrain()
{
}

void FilmGrain::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new FilmGrainSettings(m_settingsWidget);
    m_settingsView->resetToDefault();

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings FilmGrain::defaultSettings()
{
    BatchToolSettings prm;
    FilmGrainContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert("grainSize",               (int)defaultPrm.grainSize);
    prm.insert("photoDistribution",       (bool)defaultPrm.photoDistribution);
    prm.insert("addLuminanceNoise",       (bool)defaultPrm.addLuminanceNoise);
    prm.insert("lumaIntensity",           (int)defaultPrm.lumaIntensity);
    prm.insert("lumaShadows",             (int)defaultPrm.lumaShadows);
    prm.insert("lumaMidtones",            (int)defaultPrm.lumaMidtones);
    prm.insert("lumaHighlights",          (int)defaultPrm.lumaHighlights);
    prm.insert("addChrominanceBlueNoise", (bool)defaultPrm.addChrominanceBlueNoise);
    prm.insert("chromaBlueIntensity",     (int)defaultPrm.chromaBlueIntensity);
    prm.insert("chromaBlueShadows",       (int)defaultPrm.chromaBlueShadows);
    prm.insert("chromaBlueMidtones",      (int)defaultPrm.chromaBlueMidtones);
    prm.insert("chromaBlueHighlights",    (int)defaultPrm.chromaBlueHighlights);
    prm.insert("addChrominanceRedNoise",  (bool)defaultPrm.addChrominanceRedNoise);
    prm.insert("chromaRedIntensity",      (int)defaultPrm.chromaRedIntensity);
    prm.insert("chromaRedShadows",        (int)defaultPrm.chromaRedShadows);
    prm.insert("chromaRedMidtones",       (int)defaultPrm.chromaRedMidtones);
    prm.insert("chromaRedHighlights",     (int)defaultPrm.chromaRedHighlights);

    return prm;
}

void FilmGrain::slotAssignSettings2Widget()
{
    FilmGrainContainer prm;
    prm.grainSize               = settings()["grainSize"].toInt();
    prm.photoDistribution       = settings()["photoDistribution"].toBool();
    prm.addLuminanceNoise       = settings()["addLuminanceNoise"].toBool();
    prm.lumaIntensity           = settings()["lumaIntensity"].toInt();
    prm.lumaShadows             = settings()["lumaShadows"].toInt();
    prm.lumaMidtones            = settings()["lumaMidtones"].toInt();
    prm.lumaHighlights          = settings()["lumaHighlights"].toInt();
    prm.addChrominanceBlueNoise = settings()["addChrominanceBlueNoise"].toBool();
    prm.chromaBlueIntensity     = settings()["chromaBlueIntensity"].toInt();
    prm.chromaBlueShadows       = settings()["chromaBlueShadows"].toInt();
    prm.chromaBlueMidtones      = settings()["chromaBlueMidtones"].toInt();
    prm.chromaBlueHighlights    = settings()["chromaBlueHighlights"].toInt();
    prm.addChrominanceRedNoise  = settings()["addChrominanceRedNoise"].toBool();
    prm.chromaRedIntensity      = settings()["chromaRedIntensity"].toInt();
    prm.chromaRedShadows        = settings()["chromaRedShadows"].toInt();
    prm.chromaRedMidtones       = settings()["chromaRedMidtones"].toInt();
    prm.chromaRedHighlights     = settings()["chromaRedHighlights"].toInt();
    m_settingsView->setSettings(prm);
}

void FilmGrain::slotSettingsChanged()
{
    BatchToolSettings prm;
    FilmGrainContainer currentPrm = m_settingsView->settings();

    prm.insert("grainSize",               (int)currentPrm.grainSize);
    prm.insert("photoDistribution",       (bool)currentPrm.photoDistribution);
    prm.insert("addLuminanceNoise",       (bool)currentPrm.addLuminanceNoise);
    prm.insert("lumaIntensity",           (int)currentPrm.lumaIntensity);
    prm.insert("lumaShadows",             (int)currentPrm.lumaShadows);
    prm.insert("lumaMidtones",            (int)currentPrm.lumaMidtones);
    prm.insert("lumaHighlights",          (int)currentPrm.lumaHighlights);
    prm.insert("addChrominanceBlueNoise", (bool)currentPrm.addChrominanceBlueNoise);
    prm.insert("chromaBlueIntensity",     (int)currentPrm.chromaBlueIntensity);
    prm.insert("chromaBlueShadows",       (int)currentPrm.chromaBlueShadows);
    prm.insert("chromaBlueMidtones",      (int)currentPrm.chromaBlueMidtones);
    prm.insert("chromaBlueHighlights",    (int)currentPrm.chromaBlueHighlights);
    prm.insert("addChrominanceRedNoise",  (bool)currentPrm.addChrominanceRedNoise);
    prm.insert("chromaRedIntensity",      (int)currentPrm.chromaRedIntensity);
    prm.insert("chromaRedShadows",        (int)currentPrm.chromaRedShadows);
    prm.insert("chromaRedMidtones",       (int)currentPrm.chromaRedMidtones);
    prm.insert("chromaRedHighlights",     (int)currentPrm.chromaRedHighlights);

    BatchTool::slotSettingsChanged(prm);
}

bool FilmGrain::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    FilmGrainContainer prm;
    prm.grainSize               = settings()["grainSize"].toInt();
    prm.photoDistribution       = settings()["photoDistribution"].toBool();
    prm.addLuminanceNoise       = settings()["addLuminanceNoise"].toBool();
    prm.lumaIntensity           = settings()["lumaIntensity"].toInt();
    prm.lumaShadows             = settings()["lumaShadows"].toInt();
    prm.lumaMidtones            = settings()["lumaMidtones"].toInt();
    prm.lumaHighlights          = settings()["lumaHighlights"].toInt();
    prm.addChrominanceBlueNoise = settings()["addChrominanceBlueNoise"].toBool();
    prm.chromaBlueIntensity     = settings()["chromaBlueIntensity"].toInt();
    prm.chromaBlueShadows       = settings()["chromaBlueShadows"].toInt();
    prm.chromaBlueMidtones      = settings()["chromaBlueMidtones"].toInt();
    prm.chromaBlueHighlights    = settings()["chromaBlueHighlights"].toInt();
    prm.addChrominanceRedNoise  = settings()["addChrominanceRedNoise"].toBool();
    prm.chromaRedIntensity      = settings()["chromaRedIntensity"].toInt();
    prm.chromaRedShadows        = settings()["chromaRedShadows"].toInt();
    prm.chromaRedMidtones       = settings()["chromaRedMidtones"].toInt();
    prm.chromaRedHighlights     = settings()["chromaRedHighlights"].toInt();

    FilmGrainFilter fg(&image(), 0L, prm);
    applyFilter(&fg);

    return savefromDImg();
}

} // namespace Digikam
