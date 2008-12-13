/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-09
 * Description : a tool to blur an image
 *
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>

// KDE includes.

#include <kaboutdata.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kcursor.h>
#include <klocale.h>
#include <kapplication.h>

// Digikam includes.

#include "ddebug.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "editortoolsettings.h"
#include "dimggaussianblur.h"

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "blurtool.h"
#include "blurtool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

BlurTool::BlurTool(QObject* parent)
        : EditorToolThreaded(parent)
{
    setName("gaussianblur");
    setToolName(i18n("Blur"));
    setToolIcon(SmallIcon("blurimage"));
    setToolHelp("blursharpentool.anchor");

    // ---------------------------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel|
                                            EditorToolSettings::Try,
                                            EditorToolSettings::PanIcon);
    QGridLayout* grid = new QGridLayout( m_gboxSettings->plainPage(), 2, 1);
    QLabel *label     = new QLabel(i18n("Smoothness:"), m_gboxSettings->plainPage());

    m_radiusInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_radiusInput->setRange(0, 100, 1);
    m_radiusInput->setDefaultValue(0);
    QWhatsThis::add(m_radiusInput, i18n("<p>A smoothness of 0 has no effect, "
                                        "1 and above determine the Gaussian blur matrix radius "
                                        "that determines how much to blur the image."));

    grid->addMultiCellWidget(label,         0, 0, 0, 1);
    grid->addMultiCellWidget(m_radiusInput, 1, 1, 0, 1);
    grid->setRowStretch(2, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);

    m_previewWidget = new ImagePanelWidget(470, 350, "gaussianblur Tool", m_gboxSettings->panIconView());

    setToolView(m_previewWidget);
    init();
}

BlurTool::~BlurTool()
{
}

void BlurTool::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("gaussianblur Tool");
    m_radiusInput->setValue(config->readNumEntry("RadiusAjustment", m_radiusInput->defaultValue()));
}

void BlurTool::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("gaussianblur Tool");
    config->writeEntry("RadiusAjustment", m_radiusInput->value());
    config->sync();
}

void BlurTool::slotResetSettings()
{
    m_radiusInput->blockSignals(true);
    m_radiusInput->slotReset();
    m_radiusInput->blockSignals(false);
}

void BlurTool::prepareEffect()
{
    m_radiusInput->setEnabled(false);

    DImg img = m_previewWidget->getOriginalRegionImage();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgGaussianBlur(&img, this, m_radiusInput->value())));
}

void BlurTool::prepareFinal()
{
    m_radiusInput->setEnabled(false);

    ImageIface iface(0, 0);
    uchar *data            = iface.getOriginalImage();
    int w                  = iface.originalWidth();
    int h                  = iface.originalHeight();
    bool sixteenBit        = iface.originalSixteenBit();
    bool hasAlpha          = iface.originalHasAlpha();
    DImg orgImage = DImg(w, h, sixteenBit, hasAlpha ,data);
    delete [] data;
    setFilter(dynamic_cast<DImgThreadedFilter*>(new DImgGaussianBlur(&orgImage, this, m_radiusInput->value())));
}

void BlurTool::putPreviewData()
{
    DImg imDest = filter()->getTargetImage();
    m_previewWidget->setPreviewImage(imDest);
}

void BlurTool::putFinalData()
{
    ImageIface iface(0, 0);
    DImg imDest = filter()->getTargetImage();
    iface.putOriginalImage(i18n("Gaussian Blur"), imDest.bits());
}

void BlurTool::renderingFinished()
{
    m_radiusInput->setEnabled(true);
}

}  // NameSpace DigikamImagesPluginCore
