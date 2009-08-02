/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-08-03
 * Description : setup Metadata tab.
 *
 * Copyright (C) 2003-2004 by Ralf Holzer <ralf at well.com>
 * Copyright (C) 2003-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupmetadata.h"
#include "setupmetadata.moc"

// Qt includes

#include <QButtonGroup>
#include <QGroupBox>
#include <QCheckBox>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>

// KDE includes

#include <klocale.h>
#include <kdialog.h>
#include <kurllabel.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kvbox.h>
#include <ktoolinvocation.h>
#include <ktabwidget.h>
#include <kapplication.h>
#include <kconfig.h>

// Libkexiv2 includes

#include <libkexiv2/version.h>
#include <libkexiv2/kexiv2.h>

// Local includes

#include "config-digikam.h"
#include "metadatapanel.h"
#include "albumsettings.h"

namespace Digikam
{

class SetupMetadataPriv
{
public:

    SetupMetadataPriv()
    {
        exifAutoRotateAsChanged = false;
        saveCommentsBox         = 0;
        exifRotateBox           = 0;
        exifSetOrientationBox   = 0;
        saveRatingBox           = 0;
        saveTagsBox             = 0;
        saveDateTimeBox         = 0;
        saveTemplateBox         = 0;
        writeRawFilesBox        = 0;
        updateFileTimeStampBox  = 0;
        saveToNepomukBox        = 0;
        readFromNepomukBox      = 0;
        tagsCfgPanel            = 0;
        tab                     = 0;
    }

    bool              exifAutoRotateAsChanged;
    bool              exifAutoRotateOrg;

    QCheckBox        *saveCommentsBox;
    QCheckBox        *exifRotateBox;
    QCheckBox        *exifSetOrientationBox;
    QCheckBox        *saveRatingBox;
    QCheckBox        *saveTagsBox;
    QCheckBox        *saveDateTimeBox;
    QCheckBox        *saveTemplateBox;
    QCheckBox        *writeRawFilesBox;
    QCheckBox        *updateFileTimeStampBox;

    QCheckBox        *saveToNepomukBox;
    QCheckBox        *readFromNepomukBox;

    KTabWidget       *tab;

    MetadataPanel    *tagsCfgPanel;
};

SetupMetadata::SetupMetadata(QWidget* parent )
             : QScrollArea(parent), d(new SetupMetadataPriv)
{
    d->tab = new KTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);
    viewport()->setAutoFillBackground(false);

    QWidget *panel          = new QWidget(d->tab);
    QVBoxLayout *mainLayout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox *ExifGroup  = new QGroupBox(i18n("EXIF Actions"), panel);
    QVBoxLayout *gLayout1 = new QVBoxLayout(ExifGroup);

    d->exifRotateBox      = new QCheckBox(ExifGroup);
    d->exifRotateBox->setText(i18n("Show images/thumbnails &rotated according to orientation tag."));

    d->exifSetOrientationBox = new QCheckBox(ExifGroup);
    d->exifSetOrientationBox->setText(i18n("Set orientation tag to normal after rotate/flip."));

    gLayout1->addWidget(d->exifRotateBox);
    gLayout1->addWidget(d->exifSetOrientationBox);
    gLayout1->setMargin(KDialog::spacingHint());
    gLayout1->setSpacing(0);

    // --------------------------------------------------------

    QGroupBox *commonGroup = new QGroupBox(i18n("Common Metadata Actions"), panel);
    QVBoxLayout *gLayout2  = new QVBoxLayout(commonGroup);

    d->saveTagsBox = new QCheckBox(commonGroup);
    d->saveTagsBox->setText(i18n("&Save image tags as \"Keywords\" tag"));
    d->saveTagsBox->setWhatsThis( i18n("Turn on this option to store the image tags "
                                       "in the XMP and IPTC tags."));

    d->saveTemplateBox = new QCheckBox(commonGroup);
    d->saveTemplateBox->setText(i18n("&Save metadata template as tags"));
    d->saveTemplateBox->setWhatsThis( i18n("Turn on this option to store the metadata "
                                           "template in the XMP and the IPTC tags. "
                                           "You can set template values to Template setup page."));

    d->saveCommentsBox = new QCheckBox(commonGroup);
    d->saveCommentsBox->setText(i18n("&Save image captions as embedded text"));
    d->saveCommentsBox->setWhatsThis( i18n("Turn on this option to store image captions "
                                           "in the JFIF Comment section, the EXIF tag, the XMP tag, "
                                           "and the IPTC tag."));

    d->saveDateTimeBox = new QCheckBox(commonGroup);
    d->saveDateTimeBox->setText(i18n("&Save image timestamps as tags"));
    d->saveDateTimeBox->setWhatsThis( i18n("Turn on this option to store the image date and time "
                                           "in the EXIF, XMP, and IPTC tags."));

    d->saveRatingBox = new QCheckBox(commonGroup);
    d->saveRatingBox->setText(i18n("&Save image rating as tags"));
    d->saveRatingBox->setWhatsThis( i18n("Turn on this option to store the image rating "
                                         "in the EXIF tag and the XMP tags."));

    d->writeRawFilesBox = new QCheckBox(commonGroup);
    d->writeRawFilesBox->setText(i18n("&Write Metadata to RAW files (experimental)"));
    d->writeRawFilesBox->setWhatsThis( i18n("Turn on this option to write metadata into RAW TIFF/EP files. "
                                            "This feature requires the Exiv2 shared library, version >= 0.18.0. It is still "
                                            "experimental, and is disabled by default."));
    d->writeRawFilesBox->setEnabled(KExiv2Iface::KExiv2::supportMetadataWritting("image/x-raw"));

    d->updateFileTimeStampBox = new QCheckBox(commonGroup);
    d->updateFileTimeStampBox->setText(i18n("&Update file timestamp when Metadata are saved"));
    d->updateFileTimeStampBox->setWhatsThis( i18n("Turn on this option to update file timestamps when metadata are saved."));

#if KEXIV2_VERSION >= 0x000600
    d->updateFileTimeStampBox->show();
#else
    d->updateFileTimeStampBox->hide();
#endif

    gLayout2->addWidget(d->saveTagsBox);
    gLayout2->addWidget(d->saveTemplateBox);
    gLayout2->addWidget(d->saveCommentsBox);
    gLayout2->addWidget(d->saveDateTimeBox);
    gLayout2->addWidget(d->saveRatingBox);
    gLayout2->addWidget(d->writeRawFilesBox);
    gLayout2->addWidget(d->updateFileTimeStampBox);
    gLayout2->setMargin(KDialog::spacingHint());
    gLayout2->setSpacing(0);

    // --------------------------------------------------------

    QFrame      *box  = new QFrame(panel);
    QGridLayout *grid = new QGridLayout(box);
    box->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    KUrlLabel *exiv2LogoLabel = new KUrlLabel(box);
    exiv2LogoLabel->setText(QString());
    exiv2LogoLabel->setUrl("http://www.exiv2.org");
    exiv2LogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-exiv2.png")));
    exiv2LogoLabel->setWhatsThis(i18n("Visit Exiv2 project website"));

    QLabel* explanation = new QLabel(box);
    explanation->setOpenExternalLinks(true);
    explanation->setWordWrap(true);
    QString txt;

    txt.append(i18n("<p><a href='http://en.wikipedia.org/wiki/Exif'>EXIF</a> - "
                    "a standard used by most digital cameras today to store technical "
                    "information (like aperture and shutter speed) about an image.</p>"));

    txt.append(i18n("<p><a href='http://en.wikipedia.org/wiki/IPTC'>IPTC</a> - "
                    "an older standard used in digital photography to store "
                    "photographer information in images.</p>"));

    if (KExiv2Iface::KExiv2::supportXmp())
        txt.append(i18n("<p><a href='http://en.wikipedia.org/wiki/Extensible_Metadata_Platform'>XMP</a> - "
                        "a new standard used in digital photography, designed to replace IPTC.</p>"));

    explanation->setText(txt);
    explanation->setFont(KGlobalSettings::smallestReadableFont());

    grid->addWidget(exiv2LogoLabel, 0, 0, 1, 1);
    grid->addWidget(explanation,    0, 1, 1, 2);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(1, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(0);

    // --------------------------------------------------------

    mainLayout->setMargin(0);
    mainLayout->setSpacing(KDialog::spacingHint());
    mainLayout->addWidget(ExifGroup);
    mainLayout->addWidget(commonGroup);
    mainLayout->addSpacing(KDialog::spacingHint());
    mainLayout->addWidget(box);
    mainLayout->addStretch();

    d->tab->insertTab(0, panel, i18n("Behavior"));

    // --------------------------------------------------------

#ifdef HAVE_NEPOMUK

    QWidget *nepoPanel      = new QWidget(d->tab);
    QVBoxLayout *nepoLayout = new QVBoxLayout(nepoPanel);

    QGroupBox *nepoGroup = new QGroupBox(i18n("Nepomuk Semantic Desktop"), nepoPanel);
    QVBoxLayout *gLayout3  = new QVBoxLayout(nepoGroup);

    d->saveToNepomukBox = new QCheckBox;
    d->saveToNepomukBox->setText(i18n("Store metadata from digiKam in Nepomuk"));
    d->saveToNepomukBox->setWhatsThis( i18n("Turn on this option to push rating, comments and tags "
                                            "from digiKam into the Nepomuk storage"));

    d->readFromNepomukBox = new QCheckBox;
    d->readFromNepomukBox->setText(i18n("Read metadata from Nepomuk"));
    d->readFromNepomukBox->setWhatsThis( i18n("Turn on this option if you want to apply changes to "
                                              "rating, comments and tags made in Nepomuk to digiKam's metadata storage. "
                                              "Please note that image metadata will not be edited automatically."));

    gLayout3->addWidget(d->saveToNepomukBox);
    gLayout3->addWidget(d->readFromNepomukBox);

    nepoLayout->addWidget(nepoGroup);

    d->tab->addTab(nepoPanel, i18n("Nepomuk"));

    // --------------------------------------------------------

    QFrame      *nepoBox  = new QFrame(nepoPanel);
    QGridLayout *nepoGrid = new QGridLayout(nepoBox);
    nepoBox->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QLabel *nepoLogoLabel = new QLabel;
    nepoLogoLabel->setPixmap(KIconLoader::global()->loadIcon("nepomuk", KIconLoader::NoGroup, KIconLoader::SizeLarge));

    QLabel* nepoExplanation = new QLabel(nepoBox);
    nepoExplanation->setOpenExternalLinks(true);
    nepoExplanation->setWordWrap(true);
    QString nepotxt;

    nepotxt.append(i18n("<p><a href='http://nepomuk.kde.org'>Nepomuk</a> "
                    "provides the basis to handle all kinds of metadata on the KDE desktop in a generic fashion. "
                    "It allows you to tag, rate and comment your files in KDE applications like Dolphin.</p> "
                    "<p>Please set here if you want to synchronize the metadata stored by digiKam desktop-wide with the "
                    "Nepomuk Semantic Desktop.</p> "
                    "<p>If you have enabled writing of metadata to files, please note that changes done through Nepomuk "
                    "are not automatically applied to the image's metadata when read into digiKam's database.</p> "));

    nepoExplanation->setText(nepotxt);
    nepoExplanation->setFont(KGlobalSettings::smallestReadableFont());

    nepoGrid->addWidget(nepoLogoLabel, 0, 0, 1, 1);
    nepoGrid->addWidget(nepoExplanation,    0, 1, 1, 2);
    nepoGrid->setColumnStretch(1, 10);
    nepoGrid->setRowStretch(1, 10);
    nepoGrid->setMargin(KDialog::spacingHint());
    nepoGrid->setSpacing(0);

    // --------------------------------------------------------

    nepoLayout->setMargin(0);
    nepoLayout->setSpacing(KDialog::spacingHint());
    nepoLayout->addWidget(nepoGroup);
    nepoLayout->addSpacing(KDialog::spacingHint());
    nepoLayout->addWidget(nepoBox);
    nepoLayout->addStretch();

#endif // HAVE_NEPOMUK

    // --------------------------------------------------------

    d->tagsCfgPanel = new MetadataPanel(d->tab);

    // --------------------------------------------------------

    readSettings();

    connect(exiv2LogoLabel, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(slotProcessExiv2Url(const QString&)));

    connect(d->exifRotateBox, SIGNAL(toggled(bool)),
            this, SLOT(slotExifAutoRotateToggled(bool)));
}

SetupMetadata::~SetupMetadata()
{
    delete d;
}

void SetupMetadata::slotProcessExiv2Url(const QString& url)
{
    KToolInvocation::self()->invokeBrowser(url);
}

void SetupMetadata::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    settings->setExifRotate(d->exifRotateBox->isChecked());
    settings->setExifSetOrientation(d->exifSetOrientationBox->isChecked());
    settings->setSaveComments(d->saveCommentsBox->isChecked());
    settings->setSaveDateTime(d->saveDateTimeBox->isChecked());
    settings->setSaveRating(d->saveRatingBox->isChecked());
    settings->setSaveTags(d->saveTagsBox->isChecked());
    settings->setSaveTemplate(d->saveTemplateBox->isChecked());
    settings->setWriteRawFiles(d->writeRawFilesBox->isChecked());
    settings->setUpdateFileTimeStamp(d->updateFileTimeStampBox->isChecked());
#ifdef HAVE_NEPOMUK
    settings->setSyncDigikamToNepomuk(d->saveToNepomukBox->isChecked());
    settings->setSyncNepomukToDigikam(d->readFromNepomukBox->isChecked());
#endif
    settings->saveSettings();

    d->tagsCfgPanel->applySettings();
}

void SetupMetadata::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    d->exifAutoRotateOrg = settings->getExifRotate();
    d->exifRotateBox->setChecked(d->exifAutoRotateOrg);
    d->exifSetOrientationBox->setChecked(settings->getExifSetOrientation());
    d->saveCommentsBox->setChecked(settings->getSaveComments());
    d->saveDateTimeBox->setChecked(settings->getSaveDateTime());
    d->saveRatingBox->setChecked(settings->getSaveRating());
    d->saveTagsBox->setChecked(settings->getSaveTags());
    d->saveTemplateBox->setChecked(settings->getSaveTemplate());
    d->writeRawFilesBox->setChecked(settings->getWriteRawFiles());
    d->updateFileTimeStampBox->setChecked(settings->getUpdateFileTimeStamp());
#ifdef HAVE_NEPOMUK
    d->saveToNepomukBox->setChecked(settings->getSyncDigikamToNepomuk());
    d->readFromNepomukBox->setChecked(settings->getSyncNepomukToDigikam());
#endif
}

bool SetupMetadata::exifAutoRotateAsChanged()
{
    return d->exifAutoRotateAsChanged;
}

void SetupMetadata::slotExifAutoRotateToggled(bool b)
{
    if ( b != d->exifAutoRotateOrg)
        d->exifAutoRotateAsChanged = true;
    else
        d->exifAutoRotateAsChanged = false;
}

}  // namespace Digikam
