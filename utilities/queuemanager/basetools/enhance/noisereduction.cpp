/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Wavelets Noise Reduction batch tool.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "noisereduction.moc"

// Qt includes

#include <QLabel>
#include <QWidget>

// KDE includes

#include <kcombobox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <kvbox.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "dimg.h"
#include "dimgwaveletsnr.h"

using namespace KDcrawIface;

namespace Digikam
{

NoiseReduction::NoiseReduction(QObject* parent)
              : BatchTool("NoiseReduction", BaseTool, parent)
{
    setToolTitle(i18n("NoiseReduction"));
    setToolDescription(i18n("A tool to remove photographs noise using wavelets."));
    setToolIcon(KIcon(SmallIcon("noisereduction")));

    KVBox *vbox      = new KVBox;
    
    new QLabel(i18n("Threshold:"), vbox);
    m_thresholdInput = new RDoubleNumInput(vbox);
    m_thresholdInput->setDecimals(2);
    m_thresholdInput->input()->setRange(0.0, 10.0, 0.1, true);
    m_thresholdInput->setDefaultValue(1.2);
    m_thresholdInput->setWhatsThis(i18n("<b>Threshold</b>: Adjusts the threshold for denoising of "
                                         "the image in a range from 0.0 (none) to 10.0. "
                                         "The threshold is the value below which everything is considered noise."));

    // -------------------------------------------------------------

    new QLabel(i18n("Softness:"), vbox);
    m_softnessInput = new RDoubleNumInput(vbox);
    m_softnessInput->setDecimals(1);
    m_softnessInput->input()->setRange(0.0, 1.0, 0.1, true);
    m_softnessInput->setDefaultValue(0.1);
    m_softnessInput->setWhatsThis(i18n("<b>Softness</b>: This adjusts the softness of the thresholding "
                                        "(soft as opposed to hard thresholding). The higher the softness "
                                        "the more noise remains in the image."));

    QLabel *space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    connect(m_thresholdInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotSettingsChanged()));

    connect(m_softnessInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotSettingsChanged()));
}

NoiseReduction::~NoiseReduction()
{
}

BatchToolSettings NoiseReduction::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert("NRThreshold",  (double)1.2);
    settings.insert("NRSoftness", (double)0.1);
    return settings;
}

void NoiseReduction::slotAssignSettings2Widget()
{
    m_thresholdInput->setValue(settings()["NRThreshold"].toDouble());
    m_softnessInput->setValue(settings()["NRSoftness"].toDouble());
}

void NoiseReduction::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("NRThreshold", (double)m_thresholdInput->value());
    settings.insert("NRSoftness",  (double)m_softnessInput->value());
    setSettings(settings);
}

bool NoiseReduction::toolOperations()
{
    if (!loadToDImg()) return false;

    double th = settings()["NRThreshold"].toDouble();
    double so = settings()["NRSoftness"].toDouble();

    DImgWaveletsNR wnr(&image(), 0L, th, so);
    wnr.startFilterDirectly();
    DImg trg = wnr.getTargetImage();
    image().putImageData(trg.bits());

    return (savefromDImg());
}

}  // namespace Digikam
