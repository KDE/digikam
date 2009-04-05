/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor plugin to emboss
 *               an image.
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


#include "embosstool.h"
#include "embosstool.moc"

// Qt includes

#include <QGridLayout>
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

#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "version.h"
#include "emboss.h"

using namespace KDcrawIface;
using namespace Digikam;

using namespace KDcrawIface;

namespace DigikamEmbossImagesPlugin
{

EmbossTool::EmbossTool(QObject* parent)
          : EditorToolThreaded(parent)
{
    setObjectName("emboss");
    setToolName(i18n("Emboss"));
    setToolIcon(SmallIcon("embosstool"));

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel|
                                            EditorToolSettings::PanIcon);

    QGridLayout* grid = new QGridLayout( m_gboxSettings->plainPage() );

    QLabel *label1 = new QLabel(i18n("Depth:"), m_gboxSettings->plainPage());

    m_depthInput   = new RIntNumInput(m_gboxSettings->plainPage());
    m_depthInput->setRange(10, 300, 1);
    m_depthInput->setSliderEnabled(true);
    m_depthInput->setDefaultValue(30);
    m_depthInput->setWhatsThis( i18n("Set here the depth of the embossing image effect.") );

    // -------------------------------------------------------------

    grid->addWidget(label1,       0, 0, 1, 2);
    grid->addWidget(m_depthInput, 1, 0, 1, 2);
    grid->setRowStretch(2, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);

    // -------------------------------------------------------------

    m_previewWidget = new ImagePanelWidget(470, 350, "emboss Tool", m_gboxSettings->panIconView());

    setToolView(m_previewWidget);
    init();

    // -------------------------------------------------------------

    connect(m_depthInput, SIGNAL(valueChanged (int)),
            this, SLOT(slotTimer()));
}

EmbossTool::~EmbossTool()
{
}

void EmbossTool::renderingFinished()
{
    m_depthInput->setEnabled(true);
}

void EmbossTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("emboss Tool");
    m_depthInput->blockSignals(true);
    m_depthInput->setValue(group.readEntry("DepthAdjustment", m_depthInput->defaultValue()));
    m_depthInput->blockSignals(false);
}

void EmbossTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("emboss Tool");
    group.writeEntry("DepthAdjustment", m_depthInput->value());
    m_previewWidget->writeSettings();
    group.sync();
}

void EmbossTool::slotResetSettings()
{
    m_depthInput->blockSignals(true);
    m_depthInput->slotReset();
    m_depthInput->blockSignals(false);

    slotEffect();
}

void EmbossTool::prepareEffect()
{
    m_depthInput->setEnabled(false);

    DImg image = m_previewWidget->getOriginalRegionImage();
    int depth  = m_depthInput->value();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new Emboss(&image, this, depth)));
}

void EmbossTool::prepareFinal()
{
    m_depthInput->setEnabled(false);

    int depth = m_depthInput->value();

    ImageIface iface(0, 0);
    setFilter(dynamic_cast<DImgThreadedFilter*>(new Emboss(iface.getOriginalImg(), this, depth)));
}

void EmbossTool::putPreviewData()
{
    m_previewWidget->setPreviewImage(filter()->getTargetImage());
}

void EmbossTool::putFinalData()
{
    ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Emboss"), filter()->getTargetImage().bits());
}

}  // namespace DigikamEmbossImagesPlugin
