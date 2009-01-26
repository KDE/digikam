/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-22
 * Description : a digiKam image editor plugin for simulate
 *               infrared film.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qimage.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qlcdnumber.h>
#include <qslider.h>
#include <qlayout.h>
#include <qdatetime.h>
#include <qcheckbox.h>

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
#include "infrared.h"
#include "infraredtool.h"
#include "infraredtool.moc"

using namespace Digikam;

namespace DigikamInfraredImagesPlugin
{

InfraredTool::InfraredTool(QObject* parent)
            : EditorToolThreaded(parent)
{
    setName("infrared");
    setToolName(i18n("Infrared"));
    setToolIcon(SmallIcon("infrared"));

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel|
                                            EditorToolSettings::Try,
                                            EditorToolSettings::PanIcon);

    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage(), 3, 1);
    QLabel *label1 = new QLabel(i18n("Sensitivity (ISO):"), m_gboxSettings->plainPage());

    m_sensibilitySlider = new QSlider(1, 25, 1, 1, Qt::Horizontal, m_gboxSettings->plainPage());
    m_sensibilitySlider->setTracking(false);
    m_sensibilitySlider->setTickInterval(1);
    m_sensibilitySlider->setTickmarks(QSlider::Below);

    m_sensibilityLCDValue = new QLCDNumber(4, m_gboxSettings->plainPage());
    m_sensibilityLCDValue->setSegmentStyle(QLCDNumber::Flat);
    m_sensibilityLCDValue->display(QString::number(200));
    QString whatsThis = i18n("<p>Set here the ISO-sensitivity of the simulated infrared film. "
                             "Increasing this value will increase the proportion of green color in the mix. "
                             "It will also increase the halo effect on the hightlights, and the film "
                             "graininess (if that box is checked).</p>"
                             "<p>Note: to simulate an <b>Ilford SFX200</b> infrared film, use a sensitivity excursion of 200 to 800. "
                             "A sensitivity over 800 simulates <b>Kodak HIE</b> high-speed infrared film. This last one creates a more "
                             "dramatic photographic style.</p>");

    QWhatsThis::add(m_sensibilityLCDValue,  whatsThis);
    QWhatsThis::add(m_sensibilitySlider,    whatsThis);

    // -------------------------------------------------------------

    m_addFilmGrain = new QCheckBox(i18n("Add film grain"), m_gboxSettings->plainPage());
    m_addFilmGrain->setChecked(true);
    QWhatsThis::add( m_addFilmGrain, i18n("<p>This option adds infrared film grain to "
                                          "the image depending on ISO-sensitivity."));

    grid->addMultiCellWidget(label1,                0, 0, 0, 1);
    grid->addMultiCellWidget(m_sensibilitySlider,   1, 1, 0, 0);
    grid->addMultiCellWidget(m_sensibilityLCDValue, 1, 1, 1, 1);
    grid->addMultiCellWidget(m_addFilmGrain,        2, 2, 0, 1);
    grid->setRowStretch(3, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);

    // -------------------------------------------------------------

    m_previewWidget = new ImagePanelWidget(470, 350, "infrared Tool", m_gboxSettings->panIconView());

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

    connect( m_addFilmGrain, SIGNAL(toggled (bool)),
             this, SLOT(slotEffect()) );
}

InfraredTool::~InfraredTool()
{
}

void InfraredTool::renderingFinished()
{
    m_sensibilitySlider->setEnabled(true);
    m_addFilmGrain->setEnabled(true);
}

void InfraredTool::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("infrared Tool");
    m_sensibilitySlider->blockSignals(true);
    m_addFilmGrain->blockSignals(true);
    m_sensibilitySlider->setValue(config->readNumEntry("SensitivityAjustment", 1));
    m_addFilmGrain->setChecked(config->readBoolEntry("AddFilmGrain", false));
    m_sensibilitySlider->blockSignals(false);
    m_addFilmGrain->blockSignals(false);
    slotSliderMoved(m_sensibilitySlider->value());
}

void InfraredTool::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("infrared Tool");
    config->writeEntry("SensitivityAjustment", m_sensibilitySlider->value());
    config->writeEntry("AddFilmGrain", m_addFilmGrain->isChecked());
    m_previewWidget->writeSettings();
    config->sync();
}

void InfraredTool::slotResetSettings()
{
    m_sensibilitySlider->blockSignals(true);
    m_addFilmGrain->blockSignals(true);
    m_sensibilitySlider->setValue(1);
    m_addFilmGrain->setChecked(false);
    m_sensibilitySlider->blockSignals(false);
    m_addFilmGrain->blockSignals(false);
}

void InfraredTool::slotSliderMoved(int v)
{
    m_sensibilityLCDValue->display( QString::number(100 + 100 * v) );
}

void InfraredTool::prepareEffect()
{
    m_addFilmGrain->setEnabled(false);
    m_sensibilitySlider->setEnabled(false);

    DImg image = m_previewWidget->getOriginalRegionImage();
    int  s = 100 + 100 * m_sensibilitySlider->value();
    bool  g = m_addFilmGrain->isChecked();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new Infrared(&image, this, s, g)));
}

void InfraredTool::prepareFinal()
{
    m_addFilmGrain->setEnabled(false);
    m_sensibilitySlider->setEnabled(false);

    int  s = 100 + 100 * m_sensibilitySlider->value();
    bool g = m_addFilmGrain->isChecked();

    ImageIface iface(0, 0);

    setFilter(dynamic_cast<DImgThreadedFilter*>(new Infrared(iface.getOriginalImg(), this, s, g)));
}

void InfraredTool::putPreviewData()
{
    m_previewWidget->setPreviewImage(filter()->getTargetImage());
}

void InfraredTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Infrared"), filter()->getTargetImage().bits());
}

}  // NameSpace DigikamInfraredImagesPlugin
