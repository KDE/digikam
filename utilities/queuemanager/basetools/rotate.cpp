/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-10
 * Description : rotate image batch tool.
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
#include <QCheckBox>

// KDE includes.

#include <kvbox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kcombobox.h>

// Local includes.

#include "jpegutils.h"
#include "dimg.h"
#include "dmetadata.h"

namespace Digikam
{

Rotate::Rotate(QObject* parent)
      : BatchTool("Rotate", BaseTool, parent)
{
    setToolTitle(i18n("Rotate"));
    setToolDescription(i18n("A tool to rotate image by 90/180/270 degrees"));
    setToolIcon(KIcon(SmallIcon("object-rotate-right")));

    KVBox *vbox = new KVBox;
    m_useExif   = new QCheckBox(i18n("Use Exif Orientation"), vbox);

    m_label     = new QLabel(vbox);
    m_comboBox  = new KComboBox(vbox);
    m_comboBox->insertItem(DImg::ROT90,  i18n("90 degrees"));
    m_comboBox->insertItem(DImg::ROT180, i18n("180 degrees"));
    m_comboBox->insertItem(DImg::ROT270, i18n("270 degrees"));
    m_label->setText(i18n("Angle:"));

    QLabel *space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    setSettingsWidget(vbox);

    connect(m_comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSettingsChanged()));

    connect(m_useExif, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));
}

Rotate::~Rotate()
{
}

BatchToolSettings Rotate::defaultSettings()
{
    BatchToolSettings settings;
    settings.insert("UseExif",  true);
    settings.insert("Rotation", DImg::ROT90);
    return settings;
}

void Rotate::assignSettings2Widget()
{
    m_useExif->setChecked(settings()["UseExif"].toBool());
    m_comboBox->setCurrentIndex(settings()["Rotation"].toInt());
}

void Rotate::slotSettingsChanged()
{
    m_comboBox->setEnabled(!m_useExif->isChecked());
    m_label->setEnabled(!m_useExif->isChecked());

    BatchToolSettings settings;
    settings.insert("UseExif",  m_useExif->isChecked());
    settings.insert("Rotation", m_comboBox->currentIndex());
    setSettings(settings);
}

bool Rotate::toolOperations()
{
    bool useExif      = settings()["UseExif"].toBool();
    DImg::ANGLE angle = (DImg::ANGLE)(settings()["Rotation"].toInt());

    // JPEG image : lossless method.

    if (isJpegImage(inputUrl().path()))
    {
        if (useExif)
        {
            if (!exifTransform(inputUrl().path(), inputUrl().fileName(), outputUrl().path(), Auto))
                return false;
        }
        else
        {
            switch(angle)
            {
                case DImg::ROT90:
                    return (exifTransform(inputUrl().path(), inputUrl().fileName(), outputUrl().path(), Rotate90));
                    break;
                case DImg::ROT180:
                    return (exifTransform(inputUrl().path(), inputUrl().fileName(), outputUrl().path(), Rotate180));
                    break;
                case DImg::ROT270:
                    return (exifTransform(inputUrl().path(), inputUrl().fileName(), outputUrl().path(), Rotate270));
                    break;
                default:
                    kDebug(50003) << "Unknow rotate action" << endl;
                    return false;
                    break;
            }
        }
    }

    // Non-JPEG image: DImg

    if (!loadToDImg()) return false;

    if (useExif)
    {
        DMetadata meta(inputUrl().path());
        switch(meta.getImageOrientation())
        {
            case DMetadata::ORIENTATION_HFLIP:
                image().flip(DImg::HORIZONTAL);
                break;

            case DMetadata::ORIENTATION_ROT_180:
                image().rotate(DImg::ROT180);
                break;

            case DMetadata::ORIENTATION_VFLIP:
                image().flip(DImg::VERTICAL);
                break;

            case DMetadata::ORIENTATION_ROT_90_HFLIP:
                image().flip(DImg::HORIZONTAL);
                image().rotate(DImg::ROT90);
                break;

            case DMetadata::ORIENTATION_ROT_90:
                image().rotate(DImg::ROT90);
                break;

            case DMetadata::ORIENTATION_ROT_90_VFLIP:
                image().flip(DImg::VERTICAL);
                image().rotate(DImg::ROT90);
                break;

            case DMetadata::ORIENTATION_ROT_270:
                image().rotate(DImg::ROT270);
                break;

            default:
                // DMetadata::ORIENTATION_NORMAL
                // DMetadata::ORIENTATION_UNSPECIFIED
                // Nothing to do...
                break;
        }
    }
    else
    {
        image().rotate(angle);
    }

    return (savefromDImg());
}

}  // namespace Digikam
