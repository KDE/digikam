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

// Qt includes.

#include <qlabel.h>
#include <qwhatsthis.h>
#include <qlcdnumber.h>
#include <qslider.h>
#include <qlayout.h>
#include <qimage.h>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kconfig.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "editortoolsettings.h"
#include "filmgrain.h"
#include "filmgraintool.h"
#include "filmgraintool.moc"

using namespace Digikam;

namespace DigikamFilmGrainImagesPlugin
{

FilmGrainTool::FilmGrainTool(QObject* parent)
             : EditorToolThreaded(parent)
{
    setName("filmgrain");
    setToolName(i18n("Film Grain"));
    setToolIcon(SmallIcon("filmgrain"));

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel|
                                            EditorToolSettings::Try,
                                            EditorToolSettings::PanIcon);

    QGridLayout* grid = new QGridLayout( m_gboxSettings->plainPage(), 2, 1);
    QLabel *label1    = new QLabel(i18n("Sensitivity (ISO):"), m_gboxSettings->plainPage());

    m_sensibilitySlider = new QSlider(2, 30, 1, 12, Qt::Horizontal, m_gboxSettings->plainPage());
    m_sensibilitySlider->setTracking(false);
    m_sensibilitySlider->setTickInterval(1);
    m_sensibilitySlider->setTickmarks(QSlider::Below);

    m_sensibilityLCDValue = new QLCDNumber(4, m_gboxSettings->plainPage());
    m_sensibilityLCDValue->setSegmentStyle(QLCDNumber::Flat);
    m_sensibilityLCDValue->display(QString::number(2400));
    QString whatsThis = i18n("<p>Set here the film ISO-sensitivity to "
                             "use for simulating the film graininess.");

    QWhatsThis::add(m_sensibilityLCDValue,  whatsThis);
    QWhatsThis::add(m_sensibilitySlider,    whatsThis);

    grid->addMultiCellWidget(label1,                0, 0, 0, 1);
    grid->addMultiCellWidget(m_sensibilitySlider,   1, 1, 0, 0);
    grid->addMultiCellWidget(m_sensibilityLCDValue, 1, 1, 1, 1);
    grid->setRowStretch(2, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);

    // -------------------------------------------------------------

    m_previewWidget = new ImagePanelWidget(470, 350, "filmgrain Tool", m_gboxSettings->panIconView());

    setToolView(m_previewWidget);
    init();

    // -------------------------------------------------------------

    connect( m_sensibilitySlider, SIGNAL(valueChanged(int)),
             this, SLOT(slotTimer()) );

    // this connection is necessary to change the LCD display when
    // the value is changed by single clicking on the slider
    connect( m_sensibilitySlider, SIGNAL(valueChanged(int)),
             this, SLOT(slotSliderMoved(int)) );

    connect( m_sensibilitySlider, SIGNAL(sliderMoved(int)),
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
    KConfig* config = kapp->config();
    config->setGroup("filmgrain Tool");
    m_sensibilitySlider->blockSignals(true);
    m_sensibilitySlider->setValue(config->readNumEntry("SensitivityAjustment", 12));
    m_sensibilitySlider->blockSignals(false);
    slotSliderMoved(m_sensibilitySlider->value());
}

void FilmGrainTool::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("filmgrain Tool");
    config->writeEntry("SensitivityAjustment", m_sensibilitySlider->value());
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
    int s = 400 + 200 * m_sensibilitySlider->value();

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

}  // NameSpace DigikamFilmGrainImagesPlugin
