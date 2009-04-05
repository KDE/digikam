/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor plugin for add film
 *               grain on an image.
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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


#include "filmgraintool.h"
#include "filmgraintool.moc"

// Qt includes

#include <QLabel>
#include <QLCDNumber>
#include <QSlider>
#include <QImage>
#include <QGridLayout>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kconfiggroup.h>

// Local includes

#include "version.h"
#include "daboutdata.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "editortoolsettings.h"
#include "filmgrain.h"

using namespace Digikam;

namespace DigikamFilmGrainImagesPlugin
{

FilmGrainTool::FilmGrainTool(QObject* parent)
             : EditorToolThreaded(parent)
{
    setObjectName("filmgrain");
    setToolName(i18n("Film Grain"));
    setToolIcon(SmallIcon("filmgrain"));

    // -------------------------------------------------------------

    m_gboxSettings      = new EditorToolSettings(EditorToolSettings::Default|
                                                 EditorToolSettings::Ok|
                                                 EditorToolSettings::Cancel|
                                                 EditorToolSettings::Try,
                                                 EditorToolSettings::PanIcon);
    QGridLayout* grid   = new QGridLayout( m_gboxSettings->plainPage() );

    QLabel *label1      = new QLabel(i18n("Sensitivity (ISO):"), m_gboxSettings->plainPage());

    m_sensibilitySlider = new QSlider(Qt::Horizontal, m_gboxSettings->plainPage());
    m_sensibilitySlider->setMinimum(2);
    m_sensibilitySlider->setMaximum(30);
    m_sensibilitySlider->setPageStep(1);
    m_sensibilitySlider->setValue(12);
    m_sensibilitySlider->setTracking(false);
    m_sensibilitySlider->setTickInterval(1);
    m_sensibilitySlider->setTickPosition(QSlider::TicksBelow);

    m_sensibilityLCDValue = new QLCDNumber(4, m_gboxSettings->plainPage());
    m_sensibilityLCDValue->setSegmentStyle( QLCDNumber::Flat );
    m_sensibilityLCDValue->display( QString::number(2400) );
    QString whatsThis = i18n("Set here the film ISO-sensitivity to use for "
                             "simulating the film graininess.");

    m_sensibilityLCDValue->setWhatsThis( whatsThis);
    m_sensibilitySlider->setWhatsThis( whatsThis);

    // -------------------------------------------------------------

    grid->addWidget(label1,                0, 0, 1, 2);
    grid->addWidget(m_sensibilitySlider,   1, 0, 1, 1);
    grid->addWidget(m_sensibilityLCDValue, 1, 1, 1, 1);
    grid->setRowStretch(2, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);

    // -------------------------------------------------------------

    m_previewWidget = new ImagePanelWidget(470, 350, "filmgrain Tool", m_gboxSettings->panIconView());

    setToolView(m_previewWidget);
    init();

    // -------------------------------------------------------------

    connect(m_sensibilitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()) );

    // this connection is necessary to change the LCD display when
    // the value is changed by single clicking on the slider
    connect(m_sensibilitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderMoved(int)) );

    connect(m_sensibilitySlider, SIGNAL(sliderMoved(int)),
            this, SLOT(slotSliderMoved(int)) );
}

FilmGrainTool::~FilmGrainTool()
{
}

void FilmGrainTool::renderingFinished()
{
    m_sensibilitySlider->setEnabled(true);
}

void FilmGrainTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("filmgrain Tool");
    m_sensibilitySlider->blockSignals(true);
    m_sensibilitySlider->setValue(group.readEntry("SensitivityAdjustment", 12));
    m_sensibilitySlider->blockSignals(false);
    slotSliderMoved(m_sensibilitySlider->value());
}

void FilmGrainTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("filmgrain Tool");
    group.writeEntry("SensitivityAdjustment", m_sensibilitySlider->value());
    m_previewWidget->writeSettings();
    config->sync();
}

void FilmGrainTool::slotResetSettings()
{
    m_sensibilitySlider->blockSignals(true);
    m_sensibilitySlider->setValue(12);
    m_sensibilitySlider->blockSignals(false);
}

void FilmGrainTool::slotSliderMoved(int v)
{
    m_sensibilityLCDValue->display( QString::number(400+200*v) );
}

void FilmGrainTool::prepareEffect()
{
    m_sensibilitySlider->setEnabled(false);

    DImg image = m_previewWidget->getOriginalRegionImage();
    int s      = 400 + 200 * m_sensibilitySlider->value();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new FilmGrain(&image, this, s)));
}

void FilmGrainTool::prepareFinal()
{
    m_sensibilitySlider->setEnabled(false);

    int s = 400 + 200 * m_sensibilitySlider->value();

    ImageIface iface(0, 0);

    setFilter(dynamic_cast<DImgThreadedFilter*>(new FilmGrain(iface.getOriginalImg(), this, s)));
}

void FilmGrainTool::putPreviewData()
{
    m_previewWidget->setPreviewImage(filter()->getTargetImage());
}

void FilmGrainTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Film Grain"), filter()->getTargetImage().bits());
}

}  // namespace DigikamFilmGrainImagesPlugin
