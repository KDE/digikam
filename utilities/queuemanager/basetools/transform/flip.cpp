/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-10
 * Description : flip image batch tool.
 *
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "flip.moc"

// Qt includes

#include <QLabel>
#include <QWidget>

// KDE includes

#include <kcombobox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kvbox.h>
#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "jpegutils.h"

namespace Digikam
{

Flip::Flip(QObject* parent)
    : BatchTool("Flip", TransformTool, parent)
{
    setToolTitle(i18n("Flip"));
    setToolDescription(i18n("Flip images horizontally or vertically."));
    setToolIcon(KIcon(SmallIcon("object-flip-vertical")));

    KVBox* vbox   = new KVBox;
    QLabel* label = new QLabel(vbox);
    m_comboBox    = new KComboBox(vbox);
    m_comboBox->insertItem(DImg::HORIZONTAL, i18n("Horizontal"));
    m_comboBox->insertItem(DImg::VERTICAL,   i18n("Vertical"));
    label->setText(i18n("Flip:"));
    QLabel* space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    setNeedResetExifOrientation(true);

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

void Flip::slotAssignSettings2Widget()
{
    m_comboBox->setCurrentIndex(settings()["Flip"].toInt());
}

void Flip::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("Flip", m_comboBox->currentIndex());
    BatchTool::slotSettingsChanged(settings);
}

bool Flip::toolOperations()
{
    DImg::FLIP flip = (DImg::FLIP)(settings()["Flip"].toInt());

    if (isJpegImage(inputUrl().toLocalFile()) && image().isNull())
    {
        switch (flip)
        {
            case DImg::HORIZONTAL:
                return (exifTransform(inputUrl().toLocalFile(), inputUrl().fileName(), outputUrl().toLocalFile(), FlipHorizontal));
                break;
            case DImg::VERTICAL:
                return (exifTransform(inputUrl().toLocalFile(), inputUrl().fileName(), outputUrl().toLocalFile(), FlipVertical));
                break;
            default:
                kDebug() << "Unknown flip action";
                return false;
                break;
        }
    }

    if (!loadToDImg())
    {
        return false;
    }

    image().flip(flip);

    return (savefromDImg());
}

}  // namespace Digikam
