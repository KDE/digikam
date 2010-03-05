/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor plugin for add film
 *               grain on an image.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "filmgraintool.moc"

// Qt includes

#include <QGridLayout>
#include <QImage>
#include <QLabel>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "dimg.h"
#include "editortoolsettings.h"
#include "filmgrainfilter.h"
#include "imageiface.h"
#include "imageregionwidget.h"

using namespace KDcrawIface;

namespace DigikamFilmGrainImagesPlugin
{

class FilmGrainToolPriv
{
public:

    FilmGrainToolPriv() :
        configGroupName("filmgrain Tool"),
        configSensitivityAdjustmentEntry("SensitivityAdjustment"),
        sensibilityInput(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString       configGroupName;
    const QString       configSensitivityAdjustmentEntry;

    RIntNumInput*       sensibilityInput;

    ImageRegionWidget*  previewWidget;
    EditorToolSettings* gboxSettings;
};

FilmGrainTool::FilmGrainTool(QObject* parent)
             : EditorToolThreaded(parent),
               d(new FilmGrainToolPriv)
{
    setObjectName("filmgrain");
    setToolName(i18n("Film Grain"));
    setToolIcon(SmallIcon("filmgrain"));

    // -------------------------------------------------------------

    d->gboxSettings  = new EditorToolSettings;
    d->previewWidget = new ImageRegionWidget;
    
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Try);

    // -------------------------------------------------------------

    QLabel *label1      = new QLabel(i18n("Sensitivity (ISO):"));
    d->sensibilityInput = new RIntNumInput;
    d->sensibilityInput->setRange(800, 6400, 10);
    d->sensibilityInput->setSliderEnabled(true);
    d->sensibilityInput->setDefaultValue(2400);
    d->sensibilityInput->setWhatsThis(i18n("Set here the film ISO-sensitivity to use for "
                                           "simulating the film graininess."));

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(label1,              0, 0, 1, 1);
    mainLayout->addWidget(d->sensibilityInput, 1, 0, 1, 1);
    mainLayout->setRowStretch(2, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);    
    init();

    // -------------------------------------------------------------

    connect(d->sensibilityInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));

    // -------------------------------------------------------------

    slotTimer();
}

FilmGrainTool::~FilmGrainTool()
{
    delete d;
}

void FilmGrainTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->sensibilityInput->blockSignals(true);
    d->sensibilityInput->setValue(group.readEntry(d->configSensitivityAdjustmentEntry, 
                                                  d->sensibilityInput->defaultValue()));
    d->sensibilityInput->blockSignals(false);
}

void FilmGrainTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configSensitivityAdjustmentEntry, d->sensibilityInput->value());
    config->sync();
}

void FilmGrainTool::slotResetSettings()
{
    d->sensibilityInput->blockSignals(true);
    d->sensibilityInput->slotReset();
    d->sensibilityInput->blockSignals(false);
}

void FilmGrainTool::prepareEffect()
{
    toolSettings()->setEnabled(false);
    toolView()->setEnabled(false);
    
    DImg image = d->previewWidget->getOriginalRegionImage();
    int s      = d->sensibilityInput->value();

    setFilter(new FilmGrainFilter(&image, this, s));
}

void FilmGrainTool::prepareFinal()
{
    toolSettings()->setEnabled(false);
    toolView()->setEnabled(false);    

    int s = d->sensibilityInput->value();

    ImageIface iface(0, 0);

    setFilter(new FilmGrainFilter(iface.getOriginalImg(), this, s));
}

void FilmGrainTool::putPreviewData()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void FilmGrainTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Film Grain"), filter()->getTargetImage().bits());
}

void FilmGrainTool::renderingFinished()
{
    toolSettings()->setEnabled(true);
    toolView()->setEnabled(true);
}

}  // namespace DigikamFilmGrainImagesPlugin
