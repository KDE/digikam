/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-03
 * Description : a batch tool to addd film grain to images.
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

#include "filmgrain.moc"

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
#include "filmgrainfilter.h"

namespace Digikam
{

FilmGrain::FilmGrain(QObject* parent)
         : BatchTool("FilmGrain", BaseTool, parent)
{
    setToolTitle(i18n("Film Grain"));
    setToolDescription(i18n("A tool to add film grain"));
    setToolIcon(KIcon(SmallIcon("filmgrain")));

    QWidget* box  = new QWidget;
    QLabel* label = new QLabel(i18n("Sensitivity (ISO):"));
    m_sensibilityInput = new RIntNumInput();
    m_sensibilityInput->setRange(800, 6400, 10);
    m_sensibilityInput->setSliderEnabled(true);
    m_sensibilityInput->setDefaultValue(2400);
    m_sensibilityInput->setWhatsThis(i18n("Set here the film ISO-sensitivity to use for "
                                          "simulating the film graininess."));    
    
    QGridLayout* grid = new QGridLayout(box);
    grid->addWidget(label,              0, 0, 1, 2);
    grid->addWidget(m_sensibilityInput, 1, 0, 1, 2);
    grid->setRowStretch(2, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());
    
    setSettingsWidget(box);

    connect(m_sensibilityInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));
}

FilmGrain::~FilmGrain()
{
}

BatchToolSettings FilmGrain::defaultSettings()
{
    BatchToolSettings settings;

    settings.insert("Sensibility", (int)m_sensibilityInput->defaultValue());

    return settings;
}

void FilmGrain::slotAssignSettings2Widget()
{
    m_sensibilityInput->setValue(settings()["Sensibility"].toInt());
}

void FilmGrain::slotSettingsChanged()
{
    BatchToolSettings settings;

    settings.insert("Sensibility", (int)m_sensibilityInput->value());
    
    setSettings(settings);
}

bool FilmGrain::toolOperations()
{
    if (!loadToDImg())
        return false;

    FilmGrainContainer prm;
    prm.lum_sensibility    = settings()["Sensibility"].toInt();
/*
    prm.lum_shadows        = ;
    prm.lum_midtones       = ;
    prm.lum_highlights     = ;
    prm.chroma_sensibility = ;
    prm.chroma_shadows     = ; 
    prm.chroma_midtones    = ;
    prm.chroma_highlights  = ;
*/    

    FilmGrainFilter fg(&image(), 0L, prm);
    fg.startFilterDirectly();
    image().putImageData(fg.getTargetImage().bits());

    return savefromDImg();
}

} // namespace Digikam
