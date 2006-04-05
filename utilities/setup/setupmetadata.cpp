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
#include <qtooltip.h>
#include <qhbox.h>

// KDE includes.

#include <klocale.h>
#include <kactivelabel.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kurllabel.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kapplication.h>

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
        saveCommentsBox           = 0;
        ExifRotateBox             = 0;
        ExifSetOrientationBox     = 0;
        saveRatingIptcBox         = 0;
        saveTagsIptcBox           = 0;
        savePhotographerIdIptcBox = 0;
        saveDateTimeBox           = 0;
        authorEdit                = 0;
        authorTitleEdit           = 0;
        cityEdit                  = 0;
        provinceEdit              = 0;
        countryEdit               = 0;
        photographerIdGroup       = 0;
    }

    QCheckBox *saveCommentsBox;
    QCheckBox *ExifRotateBox;
    QCheckBox *ExifSetOrientationBox;
    QCheckBox *saveRatingIptcBox;
    QCheckBox *saveTagsIptcBox;
    QCheckBox *saveDateTimeBox;
    QCheckBox *savePhotographerIdIptcBox;

    QGroupBox *photographerIdGroup;

    KLineEdit *authorEdit;
    KLineEdit *authorTitleEdit;
    KLineEdit *cityEdit;
    KLineEdit *provinceEdit;
    KLineEdit *countryEdit;
};

SetupMetadata::SetupMetadata(QWidget* parent )
             : QWidget(parent)
{
    d = new SetupMetadataPriv;
    QVBoxLayout *mainLayout = new QVBoxLayout(parent);

    // --------------------------------------------------------
  
    QGroupBox *ExifGroup = new QGroupBox(1, Qt::Horizontal, i18n("EXIF Actions"), parent);
  
    d->ExifRotateBox = new QCheckBox(ExifGroup);
    d->ExifRotateBox->setText(i18n("&Rotate images/thumbs according to orientation tag"));
  
    d->ExifSetOrientationBox = new QCheckBox(ExifGroup);
    d->ExifSetOrientationBox->setText(i18n("Set orientation tag to normal after rotate/flip"));
    
    mainLayout->addWidget(ExifGroup);
  
    // --------------------------------------------------------
  
    QGroupBox *IptcGroup = new QGroupBox(1, Qt::Horizontal, i18n("IPTC Actions"), parent);

    d->saveTagsIptcBox = new QCheckBox(IptcGroup);
    d->saveTagsIptcBox->setText(i18n("&Save image tags as IPTC keywords tag"));
    QWhatsThis::add( d->saveTagsIptcBox, i18n("<p>Toogle on this option to store image tags "
                                              "into IPTC keywords tag."));
  
    d->saveRatingIptcBox = new QCheckBox(IptcGroup);
    d->saveRatingIptcBox->setText(i18n("&Save image rating as IPTC tag"));
    QWhatsThis::add( d->saveRatingIptcBox, i18n("<p>Toogle on this option to store image rating "
                                                "into IPTC tag."));

    d->savePhotographerIdIptcBox = new QCheckBox(IptcGroup);
    d->savePhotographerIdIptcBox->setText(i18n("&Save Photographer identity as IPTC tags"));
    QWhatsThis::add( d->savePhotographerIdIptcBox, i18n("<p>Toogle on this option to store "
                                                        "photographer identity into IPTC tags."));

    d->photographerIdGroup = new QGroupBox(0, Qt::Horizontal, IptcGroup);
    d->photographerIdGroup->setFrameStyle( QFrame::NoFrame );
    d->photographerIdGroup->setInsideMargin(0);
    QGridLayout* grid = new QGridLayout( d->photographerIdGroup->layout(), 5, 1, KDialog::spacingHint());

    QLabel *label1 = new QLabel(i18n("Author:"), d->photographerIdGroup);
    d->authorEdit = new KLineEdit(d->photographerIdGroup);
    d->authorEdit->setMaxLength(32);
    grid->addMultiCellWidget(label1, 0, 0, 0, 0);
    grid->addMultiCellWidget(d->authorEdit, 0, 0, 1, 1);
    QWhatsThis::add( d->authorEdit, i18n("<p>Set here the photographer name. This field is limited "
                                         "to 32 ASCII charactors."));

    QLabel *label2 = new QLabel(i18n("Author Title:"), d->photographerIdGroup);
    d->authorTitleEdit = new KLineEdit(d->photographerIdGroup);
    d->authorTitleEdit->setMaxLength(32);
    grid->addMultiCellWidget(label2, 1, 1, 0, 0);
    grid->addMultiCellWidget(d->authorTitleEdit, 1, 1, 1, 1);
    QWhatsThis::add( d->authorTitleEdit, i18n("<p>Set here the photographer title. This field is limited "
                                              "to 32 ASCII charactors."));

    QLabel *label3 = new QLabel(i18n("City:"), d->photographerIdGroup);
    d->cityEdit = new KLineEdit(d->photographerIdGroup);
    d->cityEdit->setMaxLength(32);
    grid->addMultiCellWidget(label3, 2, 2, 0, 0);
    grid->addMultiCellWidget(d->cityEdit, 2, 2, 1, 1);
    QWhatsThis::add( d->cityEdit, i18n("<p>Set here the city where photographer lives. This field is limited "
                                       "to 32 ASCII charactors."));

    QLabel *label4 = new QLabel(i18n("Province:"), d->photographerIdGroup);
    d->provinceEdit = new KLineEdit(d->photographerIdGroup);
    d->provinceEdit->setMaxLength(32);
    grid->addMultiCellWidget(label4, 3, 3, 0, 0);
    grid->addMultiCellWidget(d->provinceEdit, 3, 3, 1, 1);
    QWhatsThis::add( d->provinceEdit, i18n("<p>Set here the province where photographer lives. This field is "
                                           "limited to 32 ASCII charactors."));
    
    QLabel *label5 = new QLabel(i18n("Country:"), d->photographerIdGroup);
    d->countryEdit = new KLineEdit(d->photographerIdGroup);
    d->countryEdit->setMaxLength(64);
    grid->addMultiCellWidget(label5, 4, 4, 0, 0);
    grid->addMultiCellWidget(d->countryEdit, 4, 4, 1, 1);
    QWhatsThis::add( d->countryEdit, i18n("<p>Set here the country where photographer lives. This field is "
                                          "limited to 64 ASCII charactors."));
   
    mainLayout->addWidget(IptcGroup);

    // --------------------------------------------------------
  
    QGroupBox *commonGroup = new QGroupBox(1, Qt::Horizontal, i18n("Common Metadata Actions"), parent);
  
    d->saveCommentsBox = new QCheckBox(commonGroup);
    d->saveCommentsBox->setText(i18n("&Save image comments as embedded text"));
    QWhatsThis::add( d->saveCommentsBox, i18n("<p>Toogle on this option to store image comments "
                                              "into JFIF section, Exif tag, and IPTC tag."));

    d->saveDateTimeBox = new QCheckBox(commonGroup);
    d->saveDateTimeBox->setText(i18n("&Save image time stamp as tags"));
    QWhatsThis::add( d->saveDateTimeBox, i18n("<p>Toogle on this option to store image date and time "
                                              "into Exif and IPTC tags."));
    
    mainLayout->addWidget(commonGroup);
    mainLayout->addSpacing(KDialog::spacingHint());

    // --------------------------------------------------------
    
    QHBox *hbox = new QHBox(parent);

    KURLLabel *exiv2LogoLabel = new KURLLabel(hbox);
    exiv2LogoLabel->setText(QString::null);
    exiv2LogoLabel->setURL("http://www.exiv2.org");
    KGlobal::dirs()->addResourceType("exiv2logo", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("exiv2logo", "exiv2logo.png");
    exiv2LogoLabel->setPixmap( QPixmap( directory + "exiv2logo.png" ) );
    QToolTip::add(exiv2LogoLabel, i18n("Visit Exiv2 project website"));

    KActiveLabel* explanation = new KActiveLabel(hbox);
    explanation->setText(i18n("<p><b>EXIF</b> is a standard used by most digital cameras today to store "
                              "technicals information about photograph. You can learn more "
                              "about EXIF at <a href='http://www.exif.org'>www.exif.org</a>.</p>"
                              "<p><b>IPTC</b> is an another standard used in digital photography to store "
                              "embeded informations in pictures. You can learn more "
                              "about IPTC at <a href='http://www.iptc.org/IIM'>www.iptc.org</a>.</p>"));
    
    mainLayout->addWidget(hbox);
    mainLayout->addStretch();

    // --------------------------------------------------------

    connect(d->savePhotographerIdIptcBox, SIGNAL(toggled(bool)),
            d->photographerIdGroup, SLOT(setEnabled(bool)));
  
    connect(exiv2LogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processExiv2URL(const QString&)));

    readSettings();
    adjustSize();
  
    mainLayout->addWidget(this);
}

SetupMetadata::~SetupMetadata()
{
    delete d;
}

void SetupMetadata::processExiv2URL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

void SetupMetadata::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    settings->setExifRotate(d->ExifRotateBox->isChecked());
    settings->setExifSetOrientation(d->ExifSetOrientationBox->isChecked());
    settings->setSaveComments(d->saveCommentsBox->isChecked());
    settings->setSaveDateTime(d->saveDateTimeBox->isChecked());
    settings->setSaveIptcRating(d->saveRatingIptcBox->isChecked());
    settings->setSaveIptcTags(d->saveTagsIptcBox->isChecked());
    settings->setSaveIptcPhotographerId(d->savePhotographerIdIptcBox->isChecked());

    settings->setIptcAuthor(d->authorEdit->text());
    settings->setIptcAuthorTitle(d->authorTitleEdit->text());
    settings->setIptcCity(d->cityEdit->text());
    settings->setIptcProvince(d->provinceEdit->text());
    settings->setIptcCountry(d->countryEdit->text());

    settings->saveSettings();
}

void SetupMetadata::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings) return;

    d->ExifRotateBox->setChecked(settings->getExifRotate());
    d->ExifSetOrientationBox->setChecked(settings->getExifSetOrientation());
    d->saveCommentsBox->setChecked(settings->getSaveComments());
    d->saveDateTimeBox->setChecked(settings->getSaveDateTime());
    d->saveRatingIptcBox->setChecked(settings->getSaveIptcRating());
    d->saveTagsIptcBox->setChecked(settings->getSaveIptcTags());
    d->savePhotographerIdIptcBox->setChecked(settings->getSaveIptcPhotographerId());

    d->authorEdit->setText(settings->getIptcAuthor());
    d->authorTitleEdit->setText(settings->getIptcAuthorTitle());
    d->cityEdit->setText(settings->getIptcCity());
    d->provinceEdit->setText(settings->getIptcProvince());
    d->countryEdit->setText(settings->getIptcCountry());

    d->photographerIdGroup->setEnabled(d->savePhotographerIdIptcBox->isChecked());
}

}  // namespace Digikam

#include "setupmetadata.moc"
