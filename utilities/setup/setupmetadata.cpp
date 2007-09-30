/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-08-03
 * Description : setup Metadata tab.
 * 
 * Copyright (C) 2003-2004 by Ralf Holzer <ralf at well.com>
 * Copyright (C) 2003-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QButtonGroup>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kurllabel.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kvbox.h>
#include <ktoolinvocation.h>

// Libkexiv2 includes.

#include <libkexiv2/kexiv2.h>

// // Local includes.

#include "albumsettings.h"
#include "setupmetadata.h"
#include "setupmetadata.moc"

namespace Digikam
{

class SetupMetadataPriv
{
public:

    SetupMetadataPriv()
    {
        ExifAutoRotateAsChanged   = false;
        saveCommentsBox           = 0;
        ExifRotateBox             = 0;
        ExifSetOrientationBox     = 0;
        saveRatingBox             = 0;
        saveTagsIptcBox           = 0;
        saveDateTimeBox           = 0;
        savePhotographerIdIptcBox = 0;
        saveCreditsIptcBox        = 0;
    }

    bool       ExifAutoRotateAsChanged;
    bool       ExifAutoRotateOrg;
    
    QCheckBox *saveCommentsBox;
    QCheckBox *ExifRotateBox;
    QCheckBox *ExifSetOrientationBox;
    QCheckBox *saveRatingBox;
    QCheckBox *saveTagsIptcBox;
    QCheckBox *saveDateTimeBox;
    QCheckBox *savePhotographerIdIptcBox;
    QCheckBox *saveCreditsIptcBox;
};

SetupMetadata::SetupMetadata(QWidget* parent )
             : QWidget(parent)
{
    d = new SetupMetadataPriv;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // --------------------------------------------------------
  
    QGroupBox *ExifGroup  = new QGroupBox(i18n("EXIF Actions"), this);
    QVBoxLayout *gLayout1 = new QVBoxLayout(ExifGroup);

    d->ExifRotateBox = new QCheckBox(ExifGroup);
    d->ExifRotateBox->setText(i18n("Show images/thumbs &rotated according to orientation tag"));
    
    d->ExifSetOrientationBox = new QCheckBox(ExifGroup);
    d->ExifSetOrientationBox->setText(i18n("Set orientation tag to normal after rotate/flip"));
    
    gLayout1->addWidget(d->ExifRotateBox);
    gLayout1->addWidget(d->ExifSetOrientationBox);
    gLayout1->setMargin(KDialog::spacingHint());
    gLayout1->setSpacing(0);

    // --------------------------------------------------------
  
    QGroupBox *IptcGroup  = new QGroupBox(i18n("IPTC Actions"), this);
    QVBoxLayout *gLayout2 = new QVBoxLayout(IptcGroup);

    d->saveTagsIptcBox = new QCheckBox(IptcGroup);
    d->saveTagsIptcBox->setText(i18n("&Save image tags as \"Keywords\" tag"));
    d->saveTagsIptcBox->setWhatsThis( i18n("<p>Turn this option on to store the image tags "
                                           "in the IPTC <i>Keywords</i> tag."));
  
    d->savePhotographerIdIptcBox = new QCheckBox(IptcGroup);
    d->savePhotographerIdIptcBox->setText(i18n("&Save default photographer identity as tags"));
    d->savePhotographerIdIptcBox->setWhatsThis( i18n("<p>Turn this option on to store the default "
                                                     "photographer identity into the IPTC tags. You can set this "
                                                     "value in the Identity setup page."));

    d->saveCreditsIptcBox = new QCheckBox(IptcGroup);
    d->saveCreditsIptcBox->setText(i18n("&Save default credit and copyright identity as tags"));
    d->saveCreditsIptcBox->setWhatsThis( i18n("<p>Turn this option on to store the default "
                                              "credit and copyright identity into the IPTC tags. "
                                              "You can set this value in the Identity setup page."));

    gLayout2->addWidget(d->saveTagsIptcBox);
    gLayout2->addWidget(d->savePhotographerIdIptcBox);
    gLayout2->addWidget(d->saveCreditsIptcBox);
    gLayout2->setMargin(KDialog::spacingHint());
    gLayout2->setSpacing(0);
                                                           
    // --------------------------------------------------------
  
    QGroupBox *commonGroup = new QGroupBox(i18n("Common Metadata Actions"), this);
    QVBoxLayout *gLayout3  = new QVBoxLayout(commonGroup);

    d->saveCommentsBox = new QCheckBox(commonGroup);
    d->saveCommentsBox->setText(i18n("&Save image captions as embedded text"));
    d->saveCommentsBox->setWhatsThis( i18n("<p>Turn this option on to store image captions "
                                           "into the JFIF section, EXIF tag, and IPTC tag."));

    d->saveDateTimeBox = new QCheckBox(commonGroup);
    d->saveDateTimeBox->setText(i18n("&Save image timestamp as tags"));
    d->saveDateTimeBox->setWhatsThis( i18n("<p>Turn this option on to store the image date and time "
                                           "into the EXIF, XMP, and IPTC tags."));

    d->saveRatingBox = new QCheckBox(commonGroup);
    d->saveRatingBox->setText(i18n("&Save image rating as tags"));
    d->saveRatingBox->setWhatsThis( i18n("<p>Turn this option on to store the image rating "
                                         "into EXIF tag and IPTC <i>Urgency</i> tag."));
    
    gLayout3->addWidget(d->saveCommentsBox);
    gLayout3->addWidget(d->saveDateTimeBox);
    gLayout3->addWidget(d->saveRatingBox);
    gLayout3->setMargin(KDialog::spacingHint());
    gLayout3->setSpacing(0);

    // --------------------------------------------------------
    
    QWidget     *box  = new QWidget(this);
    QGridLayout *grid = new QGridLayout(box);

    KUrlLabel *exiv2LogoLabel = new KUrlLabel(box);
    exiv2LogoLabel->setText(QString());
    exiv2LogoLabel->setUrl("http://www.exiv2.org");
    exiv2LogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-exiv2.png")));
    exiv2LogoLabel->setWhatsThis(i18n("Visit Exiv2 project website"));

    QLabel* explanation = new QLabel(box);
    explanation->setOpenExternalLinks(true);
    explanation->setWordWrap(true);
    QString txt = i18n("<p><b><a href='http://en.wikipedia.org/wiki/Exif'>EXIF</a></b> is "
                       "a standard used by most digital cameras today to store technical "
                       "informations about photograph.</p>"
                       "<p><b><a href='http://en.wikipedia.org/wiki/IPTC'>IPTC</a></b> is "
                       "an old standard used in digital photography to store "
                       "photographer informations in pictures.</p>");

    if (KExiv2Iface::KExiv2::supportXmp())
        txt.append(i18n("<p><b><a href='http://en.wikipedia.org/wiki/Extensible_Metadata_Platform'>"
                        "XMP</a></b> is a new standard used in digital photography dedicaced to "
                        "remplace IPTC.</p>"));

    explanation->setText(txt);

    grid->addWidget(exiv2LogoLabel, 0, 0, 1, 1);
    grid->addWidget(explanation, 0, 1, 1, 2);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(1, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(0);
                   
    // --------------------------------------------------------
    
    mainLayout->setMargin(0);
    mainLayout->setSpacing(KDialog::spacingHint());
    mainLayout->addWidget(ExifGroup);
    mainLayout->addWidget(IptcGroup);
    mainLayout->addWidget(commonGroup);
    mainLayout->addSpacing(KDialog::spacingHint());
    mainLayout->addWidget(box);
    mainLayout->addStretch();

    // --------------------------------------------------------

    readSettings();
  
    connect(exiv2LogoLabel, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(processExiv2Url(const QString&)));

    connect(d->ExifRotateBox, SIGNAL(toggled(bool)),
            this, SLOT(slotExifAutoRotateToggled(bool)));
}

SetupMetadata::~SetupMetadata()
{
    delete d;
}

void SetupMetadata::processExiv2Url(const QString& url)
{
    KToolInvocation::self()->invokeBrowser(url);
}

void SetupMetadata::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    settings->setExifRotate(d->ExifRotateBox->isChecked());
    settings->setExifSetOrientation(d->ExifSetOrientationBox->isChecked());
    settings->setSaveComments(d->saveCommentsBox->isChecked());
    settings->setSaveDateTime(d->saveDateTimeBox->isChecked());
    settings->setSaveRating(d->saveRatingBox->isChecked());
    settings->setSaveIptcTags(d->saveTagsIptcBox->isChecked());
    settings->setSaveIptcPhotographerId(d->savePhotographerIdIptcBox->isChecked());
    settings->setSaveIptcCredits(d->saveCreditsIptcBox->isChecked());

    settings->saveSettings();
}

void SetupMetadata::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    d->ExifAutoRotateOrg = settings->getExifRotate();
    d->ExifRotateBox->setChecked(d->ExifAutoRotateOrg);
    d->ExifSetOrientationBox->setChecked(settings->getExifSetOrientation());
    d->saveCommentsBox->setChecked(settings->getSaveComments());
    d->saveDateTimeBox->setChecked(settings->getSaveDateTime());
    d->saveRatingBox->setChecked(settings->getSaveRating());
    d->saveTagsIptcBox->setChecked(settings->getSaveIptcTags());
    d->savePhotographerIdIptcBox->setChecked(settings->getSaveIptcPhotographerId());
    d->saveCreditsIptcBox->setChecked(settings->getSaveIptcCredits());
}

bool SetupMetadata::exifAutoRotateAsChanged()
{
    return d->ExifAutoRotateAsChanged;
}

void SetupMetadata::slotExifAutoRotateToggled(bool b)
{
    if ( b != d->ExifAutoRotateOrg)
        d->ExifAutoRotateAsChanged = true;
    else
        d->ExifAutoRotateAsChanged = false;
}

}  // namespace Digikam
