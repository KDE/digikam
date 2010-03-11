/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-02
 * Description : Curves Adjust batch tool.
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "curvesadjust.moc"

// Qt includes

#include <QLabel>
#include <QWidget>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kcombobox.h>
#include <kvbox.h>

// Local includes

#include "dimg.h"
#include "curvesfilter.h"
#include "curvessettings.h"

namespace Digikam
{

CurvesAdjust::CurvesAdjust(QObject* parent)
            : BatchTool("CurvesAdjust", ColorTool, parent)
{
    setToolTitle(i18n("Curves Adjust"));
    setToolDescription(i18n("A tool to perform curves adjustments."));
    setToolIcon(KIcon(SmallIcon("adjustcurves")));

    KVBox* vbox          = new KVBox;
    KHBox* hbox          = new KHBox(vbox);
    QLabel* channelLabel = new QLabel(hbox);
    channelLabel->setText(i18n("Channel:"));
    m_channelCB          = new KComboBox(hbox);
    m_channelCB->addItem(i18n("Luminosity"), QVariant(LuminosityChannel));
    m_channelCB->addItem(i18n("Red"),        QVariant(RedChannel));
    m_channelCB->addItem(i18n("Green"),      QVariant(GreenChannel));
    m_channelCB->addItem(i18n("Blue"),       QVariant(BlueChannel));
    m_channelCB->addItem(i18n("Alpha"),      QVariant(AlphaChannel));

    m_settingsView = new CurvesSettings(vbox, &m_preview);
    QLabel* space  = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged()));
}

CurvesAdjust::~CurvesAdjust()
{
}

void CurvesAdjust::slotChannelChanged()
{
    int index = m_channelCB->currentIndex();
    m_settingsView->setCurrentChannel((ChannelType)(m_channelCB->itemData(index).toInt()));
}

void CurvesAdjust::slotResetSettingsToDefault()
{
    // We need to call this method there to reset all curves points. 
    // Curves values are cleaned with default settings passed after.
    m_settingsView->resetToDefault();
    BatchTool::slotResetSettingsToDefault();
}

BatchToolSettings CurvesAdjust::defaultSettings()
{
    BatchToolSettings prm;
    CurvesContainer defaultPrm = m_settingsView->defaultSettings();
    prm.insert("curvesType",     (int)defaultPrm.curvesType);
    prm.insert("lumCurveVals",   defaultPrm.lumCurveVals);
    prm.insert("redCurveVals",   defaultPrm.redCurveVals);
    prm.insert("greenCurveVals", defaultPrm.greenCurveVals);
    prm.insert("blueCurveVals",  defaultPrm.blueCurveVals);
    prm.insert("alphaCurveVals", defaultPrm.alphaCurveVals);

    return prm;
}

void CurvesAdjust::slotAssignSettings2Widget()
{
    CurvesContainer prm;

    prm.curvesType     = (ImageCurves::CurveType)settings()["curvesType"].toInt();
    prm.lumCurveVals   = settings()["lumCurveVals"].value<QPolygon>();
    prm.redCurveVals   = settings()["redCurveVals"].value<QPolygon>();
    prm.greenCurveVals = settings()["greenCurveVals"].value<QPolygon>();
    prm.blueCurveVals  = settings()["blueCurveVals"].value<QPolygon>();
    prm.alphaCurveVals = settings()["alphaCurveVals"].value<QPolygon>();

    m_settingsView->setSettings(prm);
}

void CurvesAdjust::slotSettingsChanged()
{
    BatchToolSettings prm;
    CurvesContainer currentPrm = m_settingsView->settings();

    prm.insert("curvesType",     (int)currentPrm.curvesType);
    prm.insert("lumCurveVals",   currentPrm.lumCurveVals);
    prm.insert("redCurveVals",   currentPrm.redCurveVals);
    prm.insert("greenCurveVals", currentPrm.greenCurveVals);
    prm.insert("blueCurveVals",  currentPrm.blueCurveVals);
    prm.insert("alphaCurveVals", currentPrm.alphaCurveVals);

    setSettings(prm);
}

bool CurvesAdjust::toolOperations()
{
    if (!loadToDImg()) return false;

    CurvesContainer prm;

    prm.curvesType     = (ImageCurves::CurveType)settings()["curvesType"].toInt();
    prm.lumCurveVals   = settings()["lumCurveVals"].value<QPolygon>();
    prm.redCurveVals   = settings()["redCurveVals"].value<QPolygon>();
    prm.greenCurveVals = settings()["greenCurveVals"].value<QPolygon>();
    prm.blueCurveVals  = settings()["blueCurveVals"].value<QPolygon>();
    prm.alphaCurveVals = settings()["alphaCurveVals"].value<QPolygon>();

    CurvesFilter curves(&image(), 0L, prm);
    curves.startFilterDirectly();
    image().putImageData(curves.getTargetImage().bits());

    return (savefromDImg());
}

}  // namespace Digikam
