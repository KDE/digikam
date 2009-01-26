/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-30
 * Description : a plugin to add rain drop over an image
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

#include <qframe.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "raindrop.h"
#include "raindroptool.h"
#include "raindroptool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamRainDropImagesPlugin
{

RainDropTool::RainDropTool(QObject* parent)
            : EditorToolThreaded(parent)
{
    setName("raindrops");
    setToolName(i18n("Raindrops"));
    setToolIcon(SmallIcon("raindrop"));

    m_previewWidget = new ImageWidget("raindrops Tool", 0,
                                      i18n("<p>This is the preview of the Raindrop effect."
                                           "<p>Note: if you have previously selected an area in the editor, "
                                           "this will be unaffected by the filter. You can use this method to "
                                           "disable the Raindrops effect on a human face, for example."),
                                      false);

    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel);

    QGridLayout* gridSettings = new QGridLayout( m_gboxSettings->plainPage(), 7, 2);

    QLabel *label1 = new QLabel(i18n("Drop size:"), m_gboxSettings->plainPage());

    m_dropInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_dropInput->setRange(0, 200, 1);
    m_dropInput->setDefaultValue(80);
    QWhatsThis::add( m_dropInput, i18n("<p>Set here the raindrops' size."));

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Number:"), m_gboxSettings->plainPage());

    m_amountInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_amountInput->setRange(1, 500, 1);
    m_amountInput->setDefaultValue(150);
    QWhatsThis::add( m_amountInput, i18n("<p>This value controls the maximum number of raindrops."));

    // -------------------------------------------------------------

    QLabel *label3 = new QLabel(i18n("Fish eyes:"), m_gboxSettings->plainPage());

    m_coeffInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_coeffInput->setRange(1, 100, 1);
    m_coeffInput->setDefaultValue(30);
    QWhatsThis::add( m_coeffInput, i18n("<p>This value is the fish-eye-effect optical "
                                        "distortion coefficient."));

    gridSettings->addMultiCellWidget(label1,        0, 0, 0, 1);
    gridSettings->addMultiCellWidget(m_dropInput,   1, 1, 0, 1);
    gridSettings->addMultiCellWidget(label2,        2, 2, 0, 1);
    gridSettings->addMultiCellWidget(m_amountInput, 3, 3, 0, 1);
    gridSettings->addMultiCellWidget(label3,        4, 4, 0, 1);
    gridSettings->addMultiCellWidget(m_coeffInput,  5, 5, 0, 1);
    gridSettings->setRowStretch(6, 10);

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_dropInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_amountInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_coeffInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
}

RainDropTool::~RainDropTool()
{
}

void RainDropTool::renderingFinished()
{
    m_dropInput->setEnabled(true);
    m_amountInput->setEnabled(true);
    m_coeffInput->setEnabled(true);
}

void RainDropTool::readSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("raindrops Tool");

    m_dropInput->blockSignals(true);
    m_amountInput->blockSignals(true);
    m_coeffInput->blockSignals(true);

    m_dropInput->setValue(config->readNumEntry("DropAdjustment",        m_dropInput->defaultValue()));
    m_amountInput->setValue(config->readNumEntry("AmountAdjustment",    m_amountInput->defaultValue()));
    m_coeffInput->setValue(config->readNumEntry("CoeffAdjustment",      m_coeffInput->defaultValue()));

    m_dropInput->blockSignals(false);
    m_amountInput->blockSignals(false);
    m_coeffInput->blockSignals(false);

    slotEffect();
}

void RainDropTool::writeSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("raindrops Tool");
    config->writeEntry("DropAdjustment",    m_dropInput->value());
    config->writeEntry("AmountAdjustment",  m_amountInput->value());
    config->writeEntry("CoeffAdjustment",   m_coeffInput->value());
    m_previewWidget->writeSettings();
    config->sync();
}

void RainDropTool::slotResetSettings()
{
    m_dropInput->blockSignals(true);
    m_amountInput->blockSignals(true);
    m_coeffInput->blockSignals(true);

    m_dropInput->slotReset();
    m_amountInput->slotReset();
    m_coeffInput->slotReset();

    m_dropInput->blockSignals(false);
    m_amountInput->blockSignals(false);
    m_coeffInput->blockSignals(false);

    slotEffect();
}

void RainDropTool::prepareEffect()
{
    m_dropInput->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_coeffInput->setEnabled(false);

    int d        = m_dropInput->value();
    int a        = m_amountInput->value();
    int c        = m_coeffInput->value();

    ImageIface* iface = m_previewWidget->imageIface();

    // Selected data from the image
    QRect selection(iface->selectedXOrg(), iface->selectedYOrg(),
                    iface->selectedWidth(), iface->selectedHeight());

    setFilter(dynamic_cast<DImgThreadedFilter *>
                (new RainDrop(iface->getOriginalImg(), this, d, a, c, &selection)));
}

void RainDropTool::prepareFinal()
{
    m_dropInput->setEnabled(false);
    m_amountInput->setEnabled(false);
    m_coeffInput->setEnabled(false);

    int d       = m_dropInput->value();
    int a       = m_amountInput->value();
    int c       = m_coeffInput->value();

    ImageIface iface(0, 0);

    // Selected data from the image
    QRect selection(iface.selectedXOrg(), iface.selectedYOrg(),
                    iface.selectedWidth(), iface.selectedHeight());

    setFilter(dynamic_cast<DImgThreadedFilter *>
                (new RainDrop(iface.getOriginalImg(), this, d, a, c, &selection)));
}

void RainDropTool::putPreviewData(void)
{
    ImageIface* iface = m_previewWidget->imageIface();

    DImg imDest = filter()->getTargetImage()
                            .smoothScale(iface->previewWidth(), iface->previewHeight());
    iface->putPreviewImage(imDest.bits());

    m_previewWidget->updatePreview();
}

void RainDropTool::putFinalData(void)
{
    ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("RainDrop"), filter()->getTargetImage().bits());
}

}  // NameSpace DigikamRainDropImagesPlugin

