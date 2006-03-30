/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Ralf Holzer <ralf at well.com>
 * Date   : 2003-08-03
 * Description : setup Metadata tab.
 * 
 * Copyright 2003-2004 by Ralf Holzer and Gilles Caulier
 * Copyright 2005-2006 by Gilles Caulier
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

// QT includes.

#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>

// // Local includes.

#include "albumsettings.h"
#include "setupmetadata.h"

namespace Digikam
{

class SetupMetadataPriv
{
public:

    SetupMetadataPriv()
    {
        iconSaveExifBox           = 0;
        iconExifRotateBox         = 0;
        iconExifSetOrientationBox = 0;
        iconSaveRatingIptcBox     = 0;
    }

    QCheckBox *iconSaveExifBox;
    QCheckBox *iconExifRotateBox;
    QCheckBox *iconExifSetOrientationBox;
    QCheckBox *iconSaveRatingIptcBox;
};

SetupMetadata::SetupMetadata(QWidget* parent )
             : QWidget(parent)
{
    d = new SetupMetadataPriv;
    QVBoxLayout *mainLayout = new QVBoxLayout(parent);

    // --------------------------------------------------------
  
    QGroupBox *iconExifGroup = new QGroupBox(1, Qt::Horizontal, i18n("Exif Actions"), parent);
  
    QLabel* explanation = new QLabel(iconExifGroup);
    explanation->setAlignment(explanation->alignment() | WordBreak);
    explanation->setText(i18n("<b>EXIF</b> is a standard used by most digital cameras today to store "
                              "information such as comments in image files. You can learn more "
                              "about EXIF at <i>www.exif.org</i>."));
  
    d->iconSaveExifBox = new QCheckBox(iconExifGroup);
    d->iconSaveExifBox->setText(i18n("&Save image comments as embedded comments (JFIF) in JPEG images"));
  
    d->iconExifRotateBox = new QCheckBox(iconExifGroup);
    d->iconExifRotateBox->setText(i18n("&Rotate images and thumbnails according to EXIF tag"));
  
    d->iconExifSetOrientationBox = new QCheckBox(iconExifGroup);
    d->iconExifSetOrientationBox->setText(i18n("Set &EXIF orientation tag to normal after rotate/flip"));
    
    mainLayout->addWidget(iconExifGroup);
  
    // --------------------------------------------------------
  
    QGroupBox *iconIptcGroup = new QGroupBox(1, Qt::Horizontal, i18n("IPTC Actions"), parent);
  
    QLabel* explanation2 = new QLabel(iconIptcGroup);
    explanation2->setAlignment(explanation2->alignment() | WordBreak);
    explanation2->setText(i18n("<b>IPTC</b> is an another standard used in digital photography to store "
                               "embeded information in image files. You can learn more "
                               "about IPTC at <i>www.iptc.org</i>."));
  
    d->iconSaveRatingIptcBox = new QCheckBox(iconIptcGroup);
    d->iconSaveRatingIptcBox->setText(i18n("&Save image rating as IPTC tag"));
    
    mainLayout->addWidget(iconIptcGroup);
  
    mainLayout->addStretch();
  
    readSettings();
    adjustSize();
  
    mainLayout->addWidget(this);
}

SetupMetadata::~SetupMetadata()
{
    delete d;
}

void SetupMetadata::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    settings->setSaveExifComments(d->iconSaveExifBox->isChecked());
    settings->setExifRotate(d->iconExifRotateBox->isChecked());
    settings->setExifSetOrientation(d->iconExifSetOrientationBox->isChecked());

    settings->setSaveIptcRating(d->iconSaveRatingIptcBox->isChecked());

    settings->saveSettings();
}

void SetupMetadata::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    d->iconSaveExifBox->setChecked(settings->getSaveExifComments());
    d->iconExifRotateBox->setChecked(settings->getExifRotate());
    d->iconExifSetOrientationBox->setChecked(settings->getExifSetOrientation());
    
    d->iconSaveRatingIptcBox->setChecked(settings->getSaveIptcRating());
}

}  // namespace Digikam

#include "setupmetadata.moc"
