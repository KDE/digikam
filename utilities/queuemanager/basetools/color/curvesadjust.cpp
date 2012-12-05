/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-02
 * Description : Curves Adjust batch tool.
 *
 * Copyright (C) 2010-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <klocale.h>
#include <kstandarddirs.h>
#include <kcombobox.h>
#include <kvbox.h>
#include <kdebug.h>
#include <kglobal.h>

// Local includes

#include "dimg.h"
#include "curvesfilter.h"
#include "curvessettings.h"

namespace Digikam
{

CurvesAdjust::CurvesAdjust(QObject* const parent)
    : BatchTool("CurvesAdjust", ColorTool, parent),
      m_channelCB(0),
      m_settingsView(0)
{
    setToolTitle(i18n("Curves Adjust"));
    setToolDescription(i18n("Perform curves adjustments."));
    setToolIconName("adjustcurves");
}

CurvesAdjust::~CurvesAdjust()
{
}

void CurvesAdjust::registerSettingsWidget()
{
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

    m_settingsWidget = vbox;

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged()));

    BatchTool::registerSettingsWidget();
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
    prm.insert("curvesType", (int)defaultPrm.curvesType);
    prm.insert("curvesDepth",               defaultPrm.sixteenBit);
    prm.insert("values[LuminosityChannel]", defaultPrm.values[LuminosityChannel]);
    prm.insert("values[RedChannel]",        defaultPrm.values[RedChannel]);
    prm.insert("values[GreenChannel]",      defaultPrm.values[GreenChannel]);
    prm.insert("values[BlueChannel]",       defaultPrm.values[BlueChannel]);
    prm.insert("values[AlphaChannel]",      defaultPrm.values[AlphaChannel]);

    return prm;
}

void CurvesAdjust::slotAssignSettings2Widget()
{
    CurvesContainer prm;

    prm.curvesType                = (ImageCurves::CurveType)settings()["curvesType"].toInt();
    prm.sixteenBit                = settings()["curvesDepth"].toBool();
    prm.values[LuminosityChannel] = settings()["values[LuminosityChannel]"].value<QPolygon>();
    prm.values[RedChannel]        = settings()["values[RedChannel]"].value<QPolygon>();
    prm.values[GreenChannel]      = settings()["values[GreenChannel]"].value<QPolygon>();
    prm.values[BlueChannel]       = settings()["values[BlueChannel]"].value<QPolygon>();
    prm.values[AlphaChannel]      = settings()["values[AlphaChannel]"].value<QPolygon>();

    m_settingsView->setSettings(prm);
}

void CurvesAdjust::slotSettingsChanged()
{
    BatchToolSettings prm;
    CurvesContainer currentPrm = m_settingsView->settings();

    prm.insert("curvesType", (int)currentPrm.curvesType);
    prm.insert("curvesDepth",               currentPrm.sixteenBit);
    prm.insert("values[LuminosityChannel]", currentPrm.values[LuminosityChannel]);
    prm.insert("values[RedChannel]",        currentPrm.values[RedChannel]);
    prm.insert("values[GreenChannel]",      currentPrm.values[GreenChannel]);
    prm.insert("values[BlueChannel]",       currentPrm.values[BlueChannel]);
    prm.insert("values[AlphaChannel]",      currentPrm.values[AlphaChannel]);

    BatchTool::slotSettingsChanged(prm);
}

bool CurvesAdjust::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    CurvesContainer prm((ImageCurves::CurveType)settings()["curvesType"].toInt(),
                        settings()["curvesDepth"].toBool());
    prm.initialize();
    prm.values[LuminosityChannel] = settings()["values[LuminosityChannel]"].value<QPolygon>();
    prm.values[RedChannel]        = settings()["values[RedChannel]"].value<QPolygon>();
    prm.values[GreenChannel]      = settings()["values[GreenChannel]"].value<QPolygon>();
    prm.values[BlueChannel]       = settings()["values[BlueChannel]"].value<QPolygon>();
    prm.values[AlphaChannel]      = settings()["values[AlphaChannel]"].value<QPolygon>();

    CurvesFilter curves(&image(), 0L, prm);
    applyFilter(&curves);

    return (savefromDImg());
}

}  // namespace Digikam
