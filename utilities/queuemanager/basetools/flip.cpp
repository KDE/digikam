/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-10
 * Description : flip image.
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

#include "flip.h"
#include "flip.moc"

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

#include "jpegutils.h"
#include "dimg.h"

namespace Digikam
{

Flip::Flip(QObject* parent)
    : BatchTool("Flip", BaseTool, parent)
{
    setToolTitle(i18n("Flip"));
    setToolDescription(i18n("A tool to flip image horizontaly or verticaly"));
    setToolIcon(KIcon(SmallIcon("object-flip-vertical")));

    KVBox *vbox   = new KVBox;
    QLabel *label = new QLabel(vbox);
    m_comboBox    = new KComboBox(vbox);
    m_comboBox->insertItem(DImg::HORIZONTAL, i18n("Horizontal"));
    m_comboBox->insertItem(DImg::VERTICAL,   i18n("Vertical"));
    label->setText(i18n("Flip:"));
    QLabel *space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    connect(m_comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));
}

Flip::~Flip()
{
}

BatchToolSettings Flip::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert("Flip", DImg::HORIZONTAL);
    return settings;
}

void Flip::assignSettings2Widget()
{
    m_comboBox->setCurrentIndex(settings()["Flip"].toInt());
}

void Flip::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("Flip", m_comboBox->currentIndex());
    setSettings(settings);
}

bool Flip::toolOperations()
{
    DImg::FLIP flip = (DImg::FLIP)(settings()["Flip"].toInt());

    if (isJpegImage(inputUrl().path()))
    {
        switch(flip)
        {
            case DImg::HORIZONTAL:
                return (exifTransform(inputUrl().path(), inputUrl().fileName(), outputUrl().path(), FlipHorizontal));
                break;
            case DImg::VERTICAL:
                return (exifTransform(inputUrl().path(), inputUrl().fileName(), outputUrl().path(), FlipVertical));
                break;
            default:
                kDebug(50003) << "Unknow rotate action" << endl;
                return false;
                break;
        }
    }

    DImg img;
    if (!img.load(inputUrl().path()))
        return false;

    img.flip(flip);

    DImg::FORMAT format = (DImg::FORMAT)(img.attribute("detectedFileFormat").toInt());

    return( img.save(outputUrl().path(), format) );
}

}  // namespace Digikam
