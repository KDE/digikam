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
#include <QSize>

// KDE includes.

#include <kvbox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kcombobox.h>

// Local includes.

#include "dimg.h"

namespace Digikam
{

Resize::Resize(QObject* parent)
      : BatchTool("Resize", BaseTool, parent)
{
    setToolTitle(i18n("Resize"));
    setToolDescription(i18n("A tool to resize image with a customized width"));
    setToolIcon(KIcon(SmallIcon("transform-scale")));

    KVBox *vbox   = new KVBox;
    QLabel *label = new QLabel(vbox);
    m_comboBox    = new KComboBox(vbox);
    m_comboBox->insertItem(Tiny,   i18n("Tiny (%1 pixels)",   presetLenghtValue(Tiny)));
    m_comboBox->insertItem(Small,  i18n("Small (%1 pixels)",  presetLenghtValue(Small)));
    m_comboBox->insertItem(Medium, i18n("Medium (%1 pixels)", presetLenghtValue(Medium)));
    m_comboBox->insertItem(Big,    i18n("Big (%1 pixels)",    presetLenghtValue(Big)));
    m_comboBox->insertItem(Large,  i18n("Large (%1 pixels)",  presetLenghtValue(Large)));
    m_comboBox->insertItem(Huge,   i18n("Huge (%1 pixels)",   presetLenghtValue(Huge)));
    label->setText(i18n("Lenght:"));
    QLabel *space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    connect(m_comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));
}

Resize::~Resize()
{
}

BatchToolSettings Resize::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert("LenghtPreset", Medium);
    return settings;
}

void Resize::assignSettings2Widget()
{
    m_comboBox->setCurrentIndex(settings()["LenghtPreset"].toInt());
}

void Resize::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("LenghtPreset", m_comboBox->currentIndex());
    setSettings(settings);
}

int Resize::presetLenghtValue(WidthPreset preset)
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
    WidthPreset preset = (WidthPreset)(settings()["LenghtPreset"].toInt());
    int lenght         = presetLenghtValue(preset);

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
