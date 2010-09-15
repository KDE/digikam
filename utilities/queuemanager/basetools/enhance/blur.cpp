/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-03
 * Description : blur image batch tool.
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

#include "blur.moc"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QWidget>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <kdialog.h>

// Local includes

#include "dimg.h"
#include "blurfilter.h"

namespace Digikam
{

Blur::Blur(QObject* parent)
    : BatchTool("Blur", EnhanceTool, parent)
{
    setToolTitle(i18n("Blur Image"));
    setToolDescription(i18n("A tool to blur images"));
    setToolIcon(KIcon(SmallIcon("blurimage")));

    QWidget* box  = new QWidget;
    QLabel* label = new QLabel(i18n("Smoothness:"));
    m_radiusInput = new RDoubleNumInput();
    m_radiusInput->setRange(0.0, 120.0, 0.1);
    m_radiusInput->setDefaultValue(0.0);
    m_radiusInput->setWhatsThis(i18n("A smoothness of 0 has no effect, "
                                     "1 and above determine the Gaussian blur matrix radius "
                                     "that determines how much to blur the image."));

    QGridLayout* grid = new QGridLayout(box);
    grid->addWidget(label,         0, 0, 1, 2);
    grid->addWidget(m_radiusInput, 1, 0, 1, 2);
    grid->setRowStretch(2, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    setSettingsWidget(box);

    connect(m_radiusInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotSettingsChanged()));
}

Blur::~Blur()
{
}

BatchToolSettings Blur::defaultSettings()
{
    BatchToolSettings settings;

    settings.insert("Radius", (double)m_radiusInput->defaultValue());

    return settings;
}

void Blur::slotAssignSettings2Widget()
{
    m_radiusInput->setValue(settings()["Radius"].toDouble());
}

void Blur::slotSettingsChanged()
{
    BatchToolSettings settings;

    settings.insert("Radius", (double)m_radiusInput->value());

    BatchTool::slotSettingsChanged(settings);
}

bool Blur::toolOperations()
{
    if (!loadToDImg())
        return false;

    double radius = settings()["Radius"].toDouble();

    BlurFilter blur(&image(), 0L, radius);
    blur.startFilterDirectly();
    image().putImageData(blur.getTargetImage().bits());

    return savefromDImg();
}

} // namespace Digikam
