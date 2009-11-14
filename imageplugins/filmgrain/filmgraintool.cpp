/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor plugin for add film
 *               grain on an image.
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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


// #include "filmgraintool.h"
#include "filmgraintool.moc"

// Qt includes

#include <QGridLayout>
#include <QImage>
#include <QLCDNumber>
#include <QLabel>
#include <QSlider>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "daboutdata.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "filmgrain.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "version.h"

using namespace Digikam;

namespace DigikamFilmGrainImagesPlugin
{

class FilmGrainToolPriv
{
public:

    FilmGrainToolPriv() :
        configGroupName("filmgrain Tool"),
        configSensitivityAdjustmentEntry("SensitivityAdjustment"),
        sensibilitySlider(0),
        sensibilityLCDValue(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString       configGroupName;
    const QString       configSensitivityAdjustmentEntry;

    QSlider*            sensibilitySlider;

    QLCDNumber*         sensibilityLCDValue;

    ImagePanelWidget*   previewWidget;
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

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::PanIcon);

    d->previewWidget = new ImagePanelWidget(470, 350, "filmgrain Tool", d->gboxSettings->panIconView());

    // -------------------------------------------------------------

    QLabel *label1       = new QLabel(i18n("Sensitivity (ISO):"));
    d->sensibilitySlider = new QSlider(Qt::Horizontal);
    d->sensibilitySlider->setMinimum(2);
    d->sensibilitySlider->setMaximum(30);
    d->sensibilitySlider->setPageStep(1);
    d->sensibilitySlider->setValue(12);
    d->sensibilitySlider->setTracking(false);
    d->sensibilitySlider->setTickInterval(1);
    d->sensibilitySlider->setTickPosition(QSlider::TicksBelow);

    d->sensibilityLCDValue = new QLCDNumber(4);
    d->sensibilityLCDValue->setSegmentStyle( QLCDNumber::Flat );
    d->sensibilityLCDValue->display( QString::number(2400) );
    QString whatsThis = i18n("Set here the film ISO-sensitivity to use for "
                             "simulating the film graininess.");

    d->sensibilityLCDValue->setWhatsThis( whatsThis);
    d->sensibilitySlider->setWhatsThis( whatsThis);

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(label1,                 0, 0, 1, 2);
    mainLayout->addWidget(d->sensibilitySlider,   1, 0, 1, 1);
    mainLayout->addWidget(d->sensibilityLCDValue, 1, 1, 1, 1);
    mainLayout->setRowStretch(2, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    init();

    // -------------------------------------------------------------

    connect(d->sensibilitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()) );

    // this connection is necessary to change the LCD display when
    // the value is changed by single clicking on the slider
    connect(d->sensibilitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderMoved(int)) );

    connect(d->sensibilitySlider, SIGNAL(sliderMoved(int)),
            this, SLOT(slotSliderMoved(int)) );

    // -------------------------------------------------------------

    slotTimer();
}

FilmGrainTool::~FilmGrainTool()
{
    delete d;
}

void FilmGrainTool::renderingFinished()
{
    d->sensibilitySlider->setEnabled(true);
}

void FilmGrainTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->sensibilitySlider->blockSignals(true);
    d->sensibilitySlider->setValue(group.readEntry(d->configSensitivityAdjustmentEntry, 12));
    d->sensibilitySlider->blockSignals(false);
    slotSliderMoved(d->sensibilitySlider->value());
}

void FilmGrainTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configSensitivityAdjustmentEntry, d->sensibilitySlider->value());
    d->previewWidget->writeSettings();
    config->sync();
}

void FilmGrainTool::slotResetSettings()
{
    d->sensibilitySlider->blockSignals(true);
    d->sensibilitySlider->setValue(12);
    d->sensibilitySlider->blockSignals(false);
}

void FilmGrainTool::slotSliderMoved(int v)
{
    d->sensibilityLCDValue->display( QString::number(400+200*v) );
}

void FilmGrainTool::prepareEffect()
{
    d->sensibilitySlider->setEnabled(false);

    DImg image = d->previewWidget->getOriginalRegionImage();
    int s      = 400 + 200 * d->sensibilitySlider->value();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new FilmGrain(&image, this, s)));
}

void FilmGrainTool::prepareFinal()
{
    d->sensibilitySlider->setEnabled(false);

    int s = 400 + 200 * d->sensibilitySlider->value();

    ImageIface iface(0, 0);

    setFilter(dynamic_cast<DImgThreadedFilter*>(new FilmGrain(iface.getOriginalImg(), this, s)));
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

}  // namespace DigikamFilmGrainImagesPlugin
