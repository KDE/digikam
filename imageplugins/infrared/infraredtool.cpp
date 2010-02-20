/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-22
 * Description : a digiKam image editor plugin for simulate
 *               infrared film.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "infraredtool.moc"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QImage>
#include <QLabel>

// KDE includes

#include <kaboutdata.h>
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

#include "daboutdata.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "infraredfilter.h"
#include "version.h"

using namespace KDcrawIface;

namespace DigikamInfraredImagesPlugin
{

class InfraredToolPriv
{
public:

    InfraredToolPriv() :
        configGroupName("infrared Tool"),
        configSensitivityAdjustmentEntry("SensitivityAdjustment"),
        configAddFilmGrainEntry("AddFilmGrain"),

        addFilmGrain(0),
        sensibilityInput(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString       configGroupName;
    const QString       configSensitivityAdjustmentEntry;
    const QString       configAddFilmGrainEntry;

    QCheckBox*          addFilmGrain;
    
    RIntNumInput*       sensibilityInput;
    
    ImageRegionWidget*  previewWidget;
    EditorToolSettings* gboxSettings;
};


InfraredTool::InfraredTool(QObject* parent)
            : EditorToolThreaded(parent),
              d(new InfraredToolPriv)
{
    setObjectName("infrared");
    setToolName(i18n("Infrared"));
    setToolIcon(SmallIcon("infrared"));

    // -------------------------------------------------------------

    d->gboxSettings  = new EditorToolSettings;
    d->previewWidget = new ImageRegionWidget;

        
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Try);
                                
    // -------------------------------------------------------------

    QLabel *label1       = new QLabel(i18n("Sensitivity (ISO):"));
    d->sensibilityInput = new RIntNumInput;
    d->sensibilityInput->setRange(100, 800, 10);
    d->sensibilityInput->setSliderEnabled(true);
    d->sensibilityInput->setDefaultValue(200);
    d->sensibilityInput->setWhatsThis(i18n("<p>Set here the ISO-sensitivity of the simulated infrared film. "
                                           "Increasing this value will increase the proportion of green color in the mix. "
                                           "It will also increase the halo effect on the highlights, and the film "
                                           "graininess (if that box is checked).</p>"
                                           "<p>Note: to simulate an <b>Ilford SFX200</b> infrared film, use a sensitivity "
                                           "excursion of 200 to 800. "
                                           "A sensitivity over 800 simulates <b>Kodak HIE</b> high-speed infrared film. "
                                           "This last one creates a more "
                                           "dramatic photographic style.</p>"));

    // -------------------------------------------------------------

    d->addFilmGrain = new QCheckBox( i18n("Add film grain"));
    d->addFilmGrain->setChecked( true );
    d->addFilmGrain->setWhatsThis( i18n("This option adds infrared film grain to "
                                        "the image depending on ISO-sensitivity."));

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(label1,              0, 0, 1, 2);
    mainLayout->addWidget(d->sensibilityInput, 1, 0, 1, 2);
    mainLayout->addWidget(d->addFilmGrain,     2, 0, 1, 2);
    mainLayout->setRowStretch(3, 10);
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

    connect(d->previewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotTimer()));
    
    connect(d->addFilmGrain, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));
}

InfraredTool::~InfraredTool()
{
    delete d;
}

void InfraredTool::renderingFinished()
{
    d->sensibilityInput->setEnabled(true);
    d->addFilmGrain->setEnabled(true);
    toolView()->setEnabled(true);    
}

void InfraredTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->sensibilityInput->blockSignals(true);
    d->addFilmGrain->blockSignals(true);

    d->sensibilityInput->setValue(group.readEntry(d->configSensitivityAdjustmentEntry, d->sensibilityInput->defaultValue()));
    d->addFilmGrain->setChecked(group.readEntry(d->configAddFilmGrainEntry,            false));

    d->sensibilityInput->blockSignals(false);
    d->addFilmGrain->blockSignals(false);
    slotEffect();
}

void InfraredTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configSensitivityAdjustmentEntry, d->sensibilityInput->value());
    group.writeEntry(d->configAddFilmGrainEntry,          d->addFilmGrain->isChecked());
    group.sync();
}

void InfraredTool::slotResetSettings()
{
    d->sensibilityInput->blockSignals(true);
    d->addFilmGrain->blockSignals(true);
    d->sensibilityInput->slotReset();
    d->addFilmGrain->setChecked(false);
    d->sensibilityInput->blockSignals(false);
    d->addFilmGrain->blockSignals(false);
}

void InfraredTool::prepareEffect()
{
    d->addFilmGrain->setEnabled(false);
    d->sensibilityInput->setEnabled(false);
    toolView()->setEnabled(false);    

    DImg image = d->previewWidget->getOriginalRegionImage();
    int  s     = d->sensibilityInput->value();
    bool g     = d->addFilmGrain->isChecked();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new Infrared(&image, this, s, g)));
}

void InfraredTool::prepareFinal()
{
    d->addFilmGrain->setEnabled(false);
    d->sensibilityInput->setEnabled(false);
    toolView()->setEnabled(false);    

    int  s = d->sensibilityInput->value();
    bool g = d->addFilmGrain->isChecked();

    ImageIface iface(0, 0);
    setFilter(dynamic_cast<DImgThreadedFilter*>(new Infrared(iface.getOriginalImg(), this, s, g)));
}

void InfraredTool::putPreviewData()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void InfraredTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Infrared"), filter()->getTargetImage().bits());
}

}  // namespace DigikamInfraredImagesPlugin
