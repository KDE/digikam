/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-28
 * Description : auto colors correction batch tool.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "autocorrection.h"

// Qt includes

#include <QLabel>
#include <QWidget>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dimg.h"
#include "autolevelsfilter.h"
#include "equalizefilter.h"
#include "stretchfilter.h"
#include "autoexpofilter.h"
#include "normalizefilter.h"

namespace Digikam
{

AutoCorrection::AutoCorrection(QObject* const parent)
    : BatchTool(QLatin1String("AutoCorrection"), ColorTool, parent)
{
    m_comboBox = 0;

    setToolTitle(i18n("Color Auto-correction"));
    setToolDescription(i18n("Automatically correct image colors."));
    setToolIconName(QLatin1String("autocorrection"));
}

AutoCorrection::~AutoCorrection()
{
}

void AutoCorrection::registerSettingsWidget()
{
    DVBox* const vbox   = new DVBox;
    QLabel* const label = new QLabel(vbox);
    m_comboBox          = new QComboBox(vbox);
    m_comboBox->insertItem(AutoLevelsCorrection,      i18n("Auto Levels"));
    m_comboBox->insertItem(NormalizeCorrection,       i18n("Normalize"));
    m_comboBox->insertItem(EqualizeCorrection,        i18n("Equalize"));
    m_comboBox->insertItem(StretchContrastCorrection, i18n("Stretch Contrast"));
    m_comboBox->insertItem(AutoExposureCorrection,    i18n("Auto Exposure"));
    label->setText(i18n("Filter:"));
    QLabel* const space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    m_settingsWidget = vbox;

    connect(m_comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings AutoCorrection::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("AutoCorrectionFilter"), AutoLevelsCorrection);
    return settings;
}

void AutoCorrection::slotAssignSettings2Widget()
{
    m_comboBox->setCurrentIndex(settings()[QLatin1String("AutoCorrectionFilter")].toInt());
}

void AutoCorrection::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert(QLatin1String("AutoCorrectionFilter"), (int)m_comboBox->currentIndex());
    BatchTool::slotSettingsChanged(settings);
}

bool AutoCorrection::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    int type = settings()[QLatin1String("AutoCorrectionFilter")].toInt();

    switch (type)
    {
        case AutoLevelsCorrection:
        {
            AutoLevelsFilter autolevels(&image(), &image());
            applyFilter(&autolevels);
            break;
        }

        case NormalizeCorrection:
        {
            NormalizeFilter normalize(&image(), &image());
            applyFilter(&normalize);
            break;
        }

        case EqualizeCorrection:
        {
            EqualizeFilter equalize(&image(), &image());
            applyFilter(&equalize);
            break;
        }

        case StretchContrastCorrection:
        {
            StretchFilter stretch(&image(), &image());
            applyFilter(&stretch);
            break;
        }

        case AutoExposureCorrection:
        {
            AutoExpoFilter expo(&image(), &image());
            applyFilter(&expo);
            break;
        }
    }

    return (savefromDImg());
}

}  // namespace Digikam
