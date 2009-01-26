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

// Qt includes.

#include <qlabel.h>
#include <qwhatsthis.h>
#include <qlayout.h>

// KDE includes.

#include <kconfig.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "editortoolsettings.h"
#include "charcoal.h"
#include "charcoaltool.h"
#include "charcoaltool.moc"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamCharcoalImagesPlugin
{

CharcoalTool::CharcoalTool(QObject* parent)
            : EditorToolThreaded(parent)
{
    setName("charcoal");
    setToolName(i18n("Charcoal"));
    setToolIcon(SmallIcon("charcoaltool"));

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel|
                                            EditorToolSettings::Try,
                                            EditorToolSettings::PanIcon);
    QGridLayout* grid = new QGridLayout( m_gboxSettings->plainPage(), 4, 1);
    QLabel *label1    = new QLabel(i18n("Pencil size:"), m_gboxSettings->plainPage());

    m_pencilInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_pencilInput->setRange(1, 100, 1);
    m_pencilInput->setDefaultValue(5);
    QWhatsThis::add( m_pencilInput, i18n("<p>Set here the charcoal pencil size used to simulate the drawing."));

    // -------------------------------------------------------------

    QLabel *label2 = new QLabel(i18n("Smooth:"), m_gboxSettings->plainPage());

    m_smoothInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_smoothInput->setRange(1, 100, 1);
    m_smoothInput->setDefaultValue(10);
    QWhatsThis::add( m_smoothInput, i18n("<p>This value controls the smoothing effect of the pencil "
                                         "under the canvas."));

    grid->addMultiCellWidget(label1,        0, 0, 0, 1);
    grid->addMultiCellWidget(m_pencilInput, 1, 1, 0, 1);
    grid->addMultiCellWidget(label2,        2, 2, 0, 1);
    grid->addMultiCellWidget(m_smoothInput, 3, 3, 0, 1);
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
    KConfig* config = kapp->config();
    config->setGroup("charcoal Tool");
    m_pencilInput->blockSignals(true);
    m_smoothInput->blockSignals(true);

    m_pencilInput->setValue(config->readNumEntry("PencilAjustment", m_pencilInput->defaultValue()));
    m_smoothInput->setValue(config->readNumEntry("SmoothAjustment", m_smoothInput->defaultValue()));

    m_pencilInput->blockSignals(false);
    m_smoothInput->blockSignals(false);
}

void CharcoalTool::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("charcoal Tool");
    config->writeEntry("PencilAjustment", m_pencilInput->value());
    config->writeEntry("SmoothAjustment", m_smoothInput->value());
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

    DImg image = m_previewWidget->getOriginalRegionImage();

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

}  // NameSpace DigikamCharcoalImagesPlugin
