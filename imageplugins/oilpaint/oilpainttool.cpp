/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-25
 * Description : a plugin to simulate Oil Painting
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


#include "oilpainttool.h"
#include "oilpainttool.moc"

// Qt includes

#include <QGridLayout>
#include <QImage>
#include <QLabel>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "version.h"
#include "oilpaint.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamOilPaintImagesPlugin
{

OilPaintTool::OilPaintTool(QObject* parent)
            : EditorToolThreaded(parent)
{
    setObjectName("oilpaint");
    setToolName(i18n("Oil Paint"));
    setToolIcon(SmallIcon("oilpaint"));

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel|
                                            EditorToolSettings::Try,
                                            EditorToolSettings::PanIcon);
    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage());

    QLabel *label1   = new QLabel(i18n("Brush size:"), m_gboxSettings->plainPage());
    m_brushSizeInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_brushSizeInput->setRange(1, 5, 1);
    m_brushSizeInput->setSliderEnabled(true);
    m_brushSizeInput->setDefaultValue(1);
    m_brushSizeInput->setWhatsThis(i18n("Set here the brush size to use for simulating the oil painting."));

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18nc("value of smoothing effect", "Smooth:"), m_gboxSettings->plainPage());
    m_smoothInput  = new RIntNumInput(m_gboxSettings->plainPage());
    m_smoothInput->setRange(10, 255, 1);
    m_smoothInput->setSliderEnabled(true);
    m_smoothInput->setDefaultValue(30);
    m_smoothInput->setWhatsThis(i18n("This value controls the smoothing effect of the brush under the canvas."));

    grid->addWidget(label1,           0, 0, 1, 2);
    grid->addWidget(m_brushSizeInput, 1, 0, 1, 2);
    grid->addWidget(label2,           2, 0, 1, 2);
    grid->addWidget(m_smoothInput,    3, 0, 1, 2);
    grid->setRowStretch(4, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);

    // -------------------------------------------------------------

    m_previewWidget = new ImagePanelWidget(470, 350, "oilpaint Tool", m_gboxSettings->panIconView());

    setToolView(m_previewWidget);
    init();
}

OilPaintTool::~OilPaintTool()
{
}

void OilPaintTool::renderingFinished()
{
    m_brushSizeInput->setEnabled(true);
    m_smoothInput->setEnabled(true);
}

void OilPaintTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("oilpaint Tool");
    m_brushSizeInput->blockSignals(true);
    m_smoothInput->blockSignals(true);
    m_brushSizeInput->setValue(group.readEntry("BrushSize", m_brushSizeInput->defaultValue()));
    m_smoothInput->setValue(group.readEntry("SmoothAdjustment", m_smoothInput->defaultValue()));
    m_brushSizeInput->blockSignals(false);
    m_smoothInput->blockSignals(false);
}

void OilPaintTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("oilpaint Tool");
    group.writeEntry("BrushSize", m_brushSizeInput->value());
    group.writeEntry("SmoothAdjustment", m_smoothInput->value());
    m_previewWidget->writeSettings();
    group.sync();
}

void OilPaintTool::slotResetSettings()
{
    m_brushSizeInput->blockSignals(true);
    m_smoothInput->blockSignals(true);

    m_brushSizeInput->slotReset();
    m_smoothInput->slotReset();

    m_brushSizeInput->blockSignals(false);
    m_smoothInput->blockSignals(false);

    slotEffect();
}

void OilPaintTool::prepareEffect()
{
    m_brushSizeInput->setEnabled(false);
    m_smoothInput->setEnabled(false);

    DImg image = m_previewWidget->getOriginalRegionImage();
    int b      = m_brushSizeInput->value();
    int s      = m_smoothInput->value();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new OilPaint(&image, this, b, s)));
}

void OilPaintTool::prepareFinal()
{
    m_brushSizeInput->setEnabled(false);
    m_smoothInput->setEnabled(false);

    int b = m_brushSizeInput->value();
    int s = m_smoothInput->value();

    ImageIface iface(0, 0);
    setFilter(dynamic_cast<DImgThreadedFilter*>(new OilPaint(iface.getOriginalImg(), this, b, s)));
}

void OilPaintTool::putPreviewData()
{
    m_previewWidget->setPreviewImage(filter()->getTargetImage());
}

void OilPaintTool::putFinalData()
{
    ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Oil Paint"), filter()->getTargetImage().bits());
}

}  // namespace DigikamOilPaintImagesPlugin
