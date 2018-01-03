/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-03
 * Description : blur image batch tool.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "blur.h"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QWidget>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "blurfilter.h"

namespace Digikam
{

Blur::Blur(QObject* const parent)
    : BatchTool(QLatin1String("Blur"), EnhanceTool, parent),
      m_radiusInput(0)
{
    m_changeSettings = true;

    setToolTitle(i18n("Blur Image"));
    setToolDescription(i18n("Blur images"));
    setToolIconName(QLatin1String("blurimage"));
}

Blur::~Blur()
{
}

void Blur::registerSettingsWidget()
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    m_settingsWidget = new QWidget;
    QLabel* label    = new QLabel(i18n("Smoothness:"));
    m_radiusInput    = new DIntNumInput();
    m_radiusInput->setRange(0, 100, 1);
    m_radiusInput->setDefaultValue(0);
    m_radiusInput->setWhatsThis(i18n("A smoothness of 0 has no effect, "
                                     "1 and above determine the Gaussian blur matrix radius "
                                     "that determines how much to blur the image."));

    QGridLayout* grid = new QGridLayout(m_settingsWidget);
    grid->addWidget(label,         0, 0, 1, 2);
    grid->addWidget(m_radiusInput, 1, 0, 1, 2);
    grid->setRowStretch(2, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    connect(m_radiusInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings Blur::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("Radius"), (int)m_radiusInput->defaultValue());
    return settings;
}

void Blur::slotAssignSettings2Widget()
{
    m_changeSettings = false;
    m_radiusInput->setValue(settings()[QLatin1String("Radius")].toInt());
    m_changeSettings = true;
}

void Blur::slotSettingsChanged()
{
    if (m_changeSettings)
    {
        BatchToolSettings settings;
        settings.insert(QLatin1String("Radius"), (int)m_radiusInput->value());
        BatchTool::slotSettingsChanged(settings);
    }
}

bool Blur::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    double radius = settings()[QLatin1String("Radius")].toInt();

    BlurFilter blur(&image(), 0L, radius);
    applyFilter(&blur);

    return savefromDImg();
}

} // namespace Digikam
