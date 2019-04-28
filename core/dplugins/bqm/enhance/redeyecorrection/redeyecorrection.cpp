/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 16/08/2016
 * Description : A Red-Eye tool for automatic detection and correction filter.
 *
 * Copyright (C) 2016 by Omar Amin <Omar dot moh dot amin at gmail dot com>
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

#include "redeyecorrection.h"

// Qt includes

#include <QLabel>
#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"

namespace DigikamBqmRestorationPlugin
{

RedEyeCorrection::RedEyeCorrection(QObject* const parent)
    : BatchTool(QLatin1String("RedEyeCorrection"), EnhanceTool, parent),
      m_redEyeCFilter(nullptr)
{
    m_settingsView = nullptr;
}

RedEyeCorrection::~RedEyeCorrection()
{
}

void RedEyeCorrection::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new RedEyeCorrectionSettings(m_settingsWidget);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings RedEyeCorrection::defaultSettings()
{
    BatchToolSettings prm;
    RedEyeCorrectionContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("redtoavgratio"), (double)defaultPrm.m_redToAvgRatio);

    return prm;
}

void RedEyeCorrection::slotAssignSettings2Widget()
{
    RedEyeCorrectionContainer prm;
    prm.m_redToAvgRatio = settings()[QLatin1String("redtoavgratio")].toDouble();
    m_settingsView->setSettings(prm);
}

void RedEyeCorrection::slotSettingsChanged()
{
    BatchToolSettings prm;
    RedEyeCorrectionContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("redtoavgratio"), (double)currentPrm.m_redToAvgRatio);

    BatchTool::slotSettingsChanged(prm);
}

bool RedEyeCorrection::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    RedEyeCorrectionContainer prm;
    prm.m_redToAvgRatio = settings()[QLatin1String("redtoavgratio")].toDouble();

    m_redEyeCFilter = new RedEyeCorrectionFilter(&image(), nullptr, prm);
    applyFilter(m_redEyeCFilter);

    delete m_redEyeCFilter;
    m_redEyeCFilter = nullptr;

    return (savefromDImg());
}

void RedEyeCorrection::cancel()
{
    if (m_redEyeCFilter)
    {
        m_redEyeCFilter->cancelFilter();
    }

    BatchTool::cancel();
}

} // namespace DigikamBqmRestorationPlugin
