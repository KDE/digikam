/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-17
 * Description : resize image.
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

#include "resize.h"
#include "resize.moc"

// Qt includes.

#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QSize>

// KDE includes.

#include <kvbox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <knuminput.h>

// Local includes.

#include "dimg.h"

namespace Digikam
{

Resize::Resize(QObject* parent)
      : BatchTool("Resize", BaseTool, parent)
{
    setToolTitle(i18n("Resize"));
    setToolDescription(i18n("A tool to resize image with a customized lenght"));
    setToolIcon(KIcon(SmallIcon("transform-scale")));

    KVBox *vbox   = new KVBox;
    m_labelPreset = new QLabel(i18n("Preset Lenght:"), vbox);
    m_comboBox    = new KComboBox(vbox);
    m_comboBox->insertItem(Tiny,   i18n("Tiny (%1 pixels)",   presetLengthValue(Tiny)));
    m_comboBox->insertItem(Small,  i18n("Small (%1 pixels)",  presetLengthValue(Small)));
    m_comboBox->insertItem(Medium, i18n("Medium (%1 pixels)", presetLengthValue(Medium)));
    m_comboBox->insertItem(Big,    i18n("Big (%1 pixels)",    presetLengthValue(Big)));
    m_comboBox->insertItem(Large,  i18n("Large (%1 pixels)",  presetLengthValue(Large)));
    m_comboBox->insertItem(Huge,   i18n("Huge (%1 pixels)",   presetLengthValue(Huge)));

    m_useCustom    = new QCheckBox(i18n("Use Custom Lenght"), vbox);
    m_customLenght = new KIntNumInput(vbox);
    m_customLenght->setRange(100, 4000);
    m_customLenght->setSliderEnabled(true);

    QLabel *space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    connect(m_comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(m_customLenght, SIGNAL(valueChanged(int)),
            this, SLOT(slotSettingsChanged()));

    connect(m_useCustom, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));
}

Resize::~Resize()
{
}

BatchToolSettings Resize::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert("UseCustom",    false);
    settings.insert("LenghtCustom", 1024);
    settings.insert("LenghtPreset", Medium);
    return settings;
}

void Resize::assignSettings2Widget()
{
    m_comboBox->setCurrentIndex(settings()["LenghtPreset"].toInt());
    m_useCustom->setChecked(settings()["UseCustom"].toBool());
    m_customLenght->setValue(settings()["LenghtCustom"].toInt());
}

void Resize::slotSettingsChanged()
{
    m_customLenght->setEnabled(m_useCustom->isChecked());
    m_labelPreset->setEnabled(!m_useCustom->isChecked());
    m_comboBox->setEnabled(!m_useCustom->isChecked());

    BatchToolSettings settings;
    settings.insert("LenghtPreset", m_comboBox->currentIndex());
    settings.insert("UseCustom",    m_useCustom->isChecked());
    settings.insert("LenghtCustom", m_customLenght->value());
    setSettings(settings);
}

int Resize::presetLengthValue(WidthPreset preset)
{
    int lenght;

    switch(preset)
    {
        case Tiny:
            lenght = 480;
            break;
        case Small:
            lenght = 640;
            break;
        case Medium:
            lenght = 800;
            break;
        case Big:
            lenght = 1024;
            break;
        case Large:
            lenght = 1280;
            break;
        default:   // Huge
            lenght = 1600;
            break;
    }

    return lenght;
}

bool Resize::toolOperations()
{
    bool useCustom     = settings()["UseCustom"].toBool();
    WidthPreset preset = (WidthPreset)(settings()["LengthPreset"].toInt());
    int lenght         = settings()["LenghtCustom"].toInt();
    if (!useCustom)
        lenght = presetLengthValue(preset);

    DImg img;
    if (!img.load(inputUrl().path()))
        return false;

    QSize newSize(img.size());
    newSize.scale(QSize(lenght, lenght), Qt::KeepAspectRatio);
    if (!newSize.isValid())
        return false;

    img.resize(newSize.width(), newSize.height());

    DImg::FORMAT format = (DImg::FORMAT)(img.attribute("detectedFileFormat").toInt());

    img.updateMetadata(DImg::formatToMimeType(format), QString(), getExifSetOrientation());

    return( img.save(outputUrl().path(), format) );
}

}  // namespace Digikam
