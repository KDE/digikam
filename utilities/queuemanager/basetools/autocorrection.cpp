/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-28
 * Description : auto colors correction batch tool.
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "autocorrection.moc"

// Qt includes.

#include <QWidget>
#include <QLabel>

// KDE includes.

#include <kvbox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kcombobox.h>

// Local includes.

#include "dimg.h"
#include "dimgimagefilters.h"
#include "whitebalance.h"

namespace Digikam
{

AutoCorrection::AutoCorrection(QObject* parent)
              : BatchTool("AutoCorrection", BaseTool, parent)
{
    setToolTitle(i18n("Colors Auto-correction"));
    setToolDescription(i18n("A tool to correct automaticaly image colors"));
    setToolIcon(KIcon(SmallIcon("autocorrection")));

    KVBox *vbox   = new KVBox;
    QLabel *label = new QLabel(vbox);
    m_comboBox    = new KComboBox(vbox);
    m_comboBox->insertItem(AutoLevelsCorrection,      i18n("Auto Levels"));
    m_comboBox->insertItem(NormalizeCorrection,       i18n("Normalize"));
    m_comboBox->insertItem(EqualizeCorrection,        i18n("Equalize"));
    m_comboBox->insertItem(StretchContrastCorrection, i18n("Stretch Contrast"));
    m_comboBox->insertItem(AutoExposureCorrection,    i18n("Auto Exposure"));
    label->setText(i18n("Filter:"));
    QLabel *space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    connect(m_comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));
}

AutoCorrection::~AutoCorrection()
{
}

BatchToolSettings AutoCorrection::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert("AutoCorrectionFilter", AutoLevelsCorrection);
    return settings;
}

void AutoCorrection::assignSettings2Widget()
{
    m_comboBox->setCurrentIndex(settings()["AutoCorrectionFilter"].toInt());
}

void AutoCorrection::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("AutoCorrectionFilter", (int)m_comboBox->currentIndex());
    setSettings(settings);
}

bool AutoCorrection::toolOperations()
{
    DImg img;
    if (!img.load(inputUrl().path()))
        return false;

    int type = settings()["AutoCorrectionFilter"].toInt();

    DImgImageFilters filter;

    switch (type)
    {
        case AutoLevelsCorrection:
            filter.autoLevelsCorrectionImage(img.bits(), img.width(), img.height(), img.sixteenBit());
            break;

        case NormalizeCorrection:
            filter.normalizeImage(img.bits(), img.width(), img.height(), img.sixteenBit());
            break;

        case EqualizeCorrection:
            filter.equalizeImage(img.bits(), img.width(), img.height(), img.sixteenBit());
            break;

        case StretchContrastCorrection:
            filter.stretchContrastImage(img.bits(), img.width(), img.height(), img.sixteenBit());
            break;

        case AutoExposureCorrection:
            WhiteBalance wbFilter(img.sixteenBit());
            double       blackLevel, exposureLevel;
            wbFilter.autoExposureAdjustement(img.bits(), img.width(), img.height(),
                                             img.sixteenBit(), blackLevel, exposureLevel);
            wbFilter.whiteBalance(img.bits(), img.width(), img.height(),
                                  img.sixteenBit(), blackLevel, exposureLevel);
            break;
    }

    DImg::FORMAT format = (DImg::FORMAT)(img.attribute("detectedFileFormat").toInt());

    img.updateMetadata(DImg::formatToMimeType(format), QString(), getExifSetOrientation());

    return( img.save(outputUrl().path(), format) );
}

}  // namespace Digikam
