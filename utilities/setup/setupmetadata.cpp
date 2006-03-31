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
        saveCommentsBox       = 0;
        ExifRotateBox         = 0;
        ExifSetOrientationBox = 0;
        saveRatingIptcBox     = 0;
        saveDateTimeBox       = 0;
    }

    QCheckBox *saveCommentsBox;
    QCheckBox *ExifRotateBox;
    QCheckBox *ExifSetOrientationBox;
    QCheckBox *saveRatingIptcBox;
    QCheckBox *saveDateTimeBox;
};

SetupMetadata::SetupMetadata(QWidget* parent )
             : QWidget(parent)
{
    d = new SetupMetadataPriv;
    QVBoxLayout *mainLayout = new QVBoxLayout(parent);

    // --------------------------------------------------------
  
    QGroupBox *ExifGroup = new QGroupBox(1, Qt::Horizontal, i18n("Exif Actions"), parent);
  
    QLabel* explanation = new QLabel(ExifGroup);
    explanation->setAlignment(explanation->alignment() | WordBreak);
    explanation->setText(i18n("<b>EXIF</b> is a standard used by most digital cameras today to store "
                              "technicals information about photograph. You can learn more "
                              "about EXIF at <i>www.exif.org</i>."));
  
    d->ExifRotateBox = new QCheckBox(ExifGroup);
    d->ExifRotateBox->setText(i18n("&Rotate images and thumbnails according to EXIF tag"));
  
    d->ExifSetOrientationBox = new QCheckBox(ExifGroup);
    d->ExifSetOrientationBox->setText(i18n("Set &EXIF orientation tag to normal after rotate/flip"));
    
    mainLayout->addWidget(ExifGroup);
  
    // --------------------------------------------------------
  
    QGroupBox *IptcGroup = new QGroupBox(1, Qt::Horizontal, i18n("IPTC Actions"), parent);
  
    QLabel* explanation2 = new QLabel(IptcGroup);
    explanation2->setAlignment(explanation2->alignment() | WordBreak);
    explanation2->setText(i18n("<b>IPTC</b> is an another standard used in digital photography to store "
                               "embeded informations in image files. You can learn more "
                               "about IPTC at <i>www.iptc.org</i>."));

    d->saveRatingIptcBox = new QCheckBox(IptcGroup);
    d->saveRatingIptcBox->setText(i18n("&Save image rating as IPTC tag"));
    QWhatsThis::add( d->saveRatingIptcBox, i18n("<p>Toogle on this option to store image rating "
                                                "into IPTC tag."));
    
    mainLayout->addWidget(IptcGroup);

    // --------------------------------------------------------
  
    QGroupBox *commonGroup = new QGroupBox(1, Qt::Horizontal, i18n("Common Metadata Actions"), parent);
  
    d->saveCommentsBox = new QCheckBox(commonGroup);
    d->saveCommentsBox->setText(i18n("&Save image comments as embedded comments"));
    QWhatsThis::add( d->saveCommentsBox, i18n("<p>Toogle on this option to store image comments "
                                              "into JFIF section, Exif tag, and IPTC tag."));

    d->saveDateTimeBox = new QCheckBox(commonGroup);
    d->saveDateTimeBox->setText(i18n("&Save image time stamp as tags"));
    QWhatsThis::add( d->saveDateTimeBox, i18n("<p>Toogle on this option to store image date and time "
                                              "into Exif and IPTC tags."));
    
    mainLayout->addWidget(commonGroup);

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

    settings->setExifRotate(d->ExifRotateBox->isChecked());
    settings->setExifSetOrientation(d->ExifSetOrientationBox->isChecked());
    settings->setSaveIptcRating(d->saveRatingIptcBox->isChecked());
    settings->setSaveComments(d->saveCommentsBox->isChecked());
    settings->setSaveDateTime(d->saveDateTimeBox->isChecked());
    settings->saveSettings();
}

void SetupMetadata::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    d->ExifRotateBox->setChecked(settings->getExifRotate());
    d->ExifSetOrientationBox->setChecked(settings->getExifSetOrientation());
    d->saveRatingIptcBox->setChecked(settings->getSaveIptcRating());
    d->saveCommentsBox->setChecked(settings->getSaveComments());
    d->saveDateTimeBox->setChecked(settings->getSaveDateTime());
}

}  // namespace Digikam

#include "setupmetadata.moc"
