/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-10
 * Description : rotate image.
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

#include "rotate.h"
#include "rotate.moc"

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

namespace Digikam
{

Rotate::Rotate(QObject* parent)
      : BatchTool("Rotate", BaseTool, parent)
{
    setToolTitle(i18n("Rotate"));
    setToolDescription(i18n("A tool to rotate image by 90/180/270 degrees"));
    setToolIcon(KIcon(SmallIcon("object-rotate-right")));

    KVBox *vbox   = new KVBox;
    QLabel *label = new QLabel(vbox);
    m_comboBox    = new KComboBox(vbox);
    m_comboBox->insertItem(DImg::ROT90,  i18n("90 degrees"));
    m_comboBox->insertItem(DImg::ROT180, i18n("180 degrees"));
    m_comboBox->insertItem(DImg::ROT270, i18n("270 degrees"));
    label->setText(i18n("Angle:"));
    QLabel *space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    connect(m_comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));
}

Rotate::~Rotate()
{
}

BatchToolSettings Rotate::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert("Rotation", DImg::ROT90);
    return settings;
}

void Rotate::assignSettings2Widget()
{
    m_comboBox->setCurrentIndex(settings()["Rotation"].toInt());
}

void Rotate::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("Rotation", m_comboBox->currentIndex());
    setSettings(settings);
}

bool Rotate::toolOperations()
{
    DImg img;
    if (!img.load(inputUrl().path()))
        return false;

    img.rotate((DImg::ANGLE)(settings()["Rotation"].toInt()));

    DImg::FORMAT format = (DImg::FORMAT)(img.attribute("detectedFileFormat").toInt());

    return( img.save(outputUrl().path(), format) );
}

}  // namespace Digikam
