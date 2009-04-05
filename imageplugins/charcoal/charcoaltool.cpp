/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digikam image editor plugin to
 *               simulate charcoal drawing.
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


#include "charcoaltool.h"
#include "charcoaltool.moc"

// Qt includes

#include <QLabel>
#include <QGridLayout>

// KDE includes

#include <kconfig.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <knuminput.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kconfiggroup.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "version.h"
#include "daboutdata.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "editortoolsettings.h"
#include "charcoal.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamCharcoalImagesPlugin
{

CharcoalTool::CharcoalTool(QObject* parent)
            : EditorToolThreaded(parent)
{
    setObjectName("charcoal");
    setToolName(i18n("Charcoal"));
    setToolIcon(SmallIcon("charcoaltool"));

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel|
                                            EditorToolSettings::Try,
                                            EditorToolSettings::PanIcon);
    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage());

    QLabel *label1 = new QLabel(i18n("Pencil size:"), m_gboxSettings->plainPage());

    m_pencilInput  = new RIntNumInput(m_gboxSettings->plainPage());
    m_pencilInput->setRange(1, 100, 1);
    m_pencilInput->setSliderEnabled(true);
    m_pencilInput->setDefaultValue(5);
    m_pencilInput->setWhatsThis( i18n("Set here the charcoal pencil size used to simulate the drawing."));

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18nc("smoothing value of the pencil", "Smooth:"), m_gboxSettings->plainPage());

    m_smoothInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_smoothInput->setRange(1, 100, 1);
    m_smoothInput->setSliderEnabled(true);
    m_smoothInput->setDefaultValue(10);
    m_smoothInput->setWhatsThis( i18n("This value controls the smoothing effect of the pencil "
                                      "under the canvas."));

    // -------------------------------------------------------------

    grid->addWidget(label1,        0, 0, 1, 2);
    grid->addWidget(m_pencilInput, 1, 0, 1, 2);
    grid->addWidget(label2,        2, 0, 1, 2);
    grid->addWidget(m_smoothInput, 3, 0, 1, 2);
    grid->setRowStretch(4, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);

    // -------------------------------------------------------------

    m_previewWidget = new ImagePanelWidget(470, 350, "charcoal Tool", m_gboxSettings->panIconView());

    setToolView(m_previewWidget);
    init();

    // -------------------------------------------------------------

    connect(m_pencilInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_smoothInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
}

CharcoalTool::~CharcoalTool()
{
}

void CharcoalTool::renderingFinished()
{
    m_pencilInput->setEnabled(true);
    m_smoothInput->setEnabled(true);
}

void CharcoalTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("charcoal Tool");
    m_pencilInput->blockSignals(true);
    m_smoothInput->blockSignals(true);
    m_pencilInput->setValue(group.readEntry("PencilAdjustment", m_pencilInput->defaultValue()));
    m_smoothInput->setValue(group.readEntry("SmoothAdjustment", m_smoothInput->defaultValue()));
    m_pencilInput->blockSignals(false);
    m_smoothInput->blockSignals(false);
}

void CharcoalTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("charcoal Tool");
    group.writeEntry("PencilAdjustment", m_pencilInput->value());
    group.writeEntry("SmoothAdjustment", m_smoothInput->value());
    m_previewWidget->writeSettings();
    config->sync();
}

void CharcoalTool::slotResetSettings()
{
    m_pencilInput->blockSignals(true);
    m_smoothInput->blockSignals(true);
    m_pencilInput->slotReset();
    m_smoothInput->slotReset();
    m_pencilInput->blockSignals(false);
    m_smoothInput->blockSignals(false);
}

void CharcoalTool::prepareEffect()
{
    m_pencilInput->setEnabled(false);
    m_smoothInput->setEnabled(false);

    double pencil = (double)m_pencilInput->value()/10.0;
    double smooth = (double)m_smoothInput->value();
    DImg image    = m_previewWidget->getOriginalRegionImage();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new Charcoal(&image, this, pencil, smooth)));
}

void CharcoalTool::prepareFinal()
{
    m_pencilInput->setEnabled(false);
    m_smoothInput->setEnabled(false);

    double pencil = (double)m_pencilInput->value()/10.0;
    double smooth = (double)m_smoothInput->value();

    ImageIface iface(0, 0);
    setFilter(dynamic_cast<DImgThreadedFilter*>(new Charcoal(iface.getOriginalImg(), this, pencil, smooth)));
}

void CharcoalTool::putPreviewData()
{
    m_previewWidget->setPreviewImage(filter()->getTargetImage());
}

void CharcoalTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Charcoal"), filter()->getTargetImage().bits());
}

}  // namespace DigikamCharcoalImagesPlugin
