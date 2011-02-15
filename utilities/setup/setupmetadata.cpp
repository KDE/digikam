/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-08-03
 * Description : setup Metadata tab.
 *
 * Copyright (C) 2003-2004 by Ralf Holzer <ralf at well.com>
 * Copyright (C) 2003-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupmetadata.moc"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>
#include <kvbox.h>
#include <khbox.h>
#include <kcombobox.h>

// LibKExiv2 includes

#include <libkexiv2/kexiv2.h>
#include <libkexiv2/version.h>

// Local includes

#include "albumsettings.h"
#include "config-digikam.h"
#include "metadatapanel.h"
#include "metadatasettings.h"

using namespace KExiv2Iface;

namespace Digikam
{

class SetupMetadata::SetupMetadataPriv
{
public:

    SetupMetadataPriv() :
        exifAutoRotateAsChanged(false),
        exifAutoRotateOrg(false),
        saveCommentsBox(0),
        exifRotateBox(0),
        exifSetOrientationBox(0),
        savePickLabelBox(0),
        saveColorLabelBox(0),
        saveRatingBox(0),
        saveTagsBox(0),
        saveDateTimeBox(0),
        saveTemplateBox(0),
        writeRawFilesBox(0),
        updateFileTimeStampBox(0),
        saveToNepomukBox(0),
        readFromNepomukBox(0),
        resyncButton(0),
        writingModeCombo(0),
        tab(0),
        tagsCfgPanel(0)
    {
    }

    bool           exifAutoRotateAsChanged;
    bool           exifAutoRotateOrg;

    QCheckBox*     saveCommentsBox;
    QCheckBox*     exifRotateBox;
    QCheckBox*     exifSetOrientationBox;
    QCheckBox*     savePickLabelBox;
    QCheckBox*     saveColorLabelBox;
    QCheckBox*     saveRatingBox;
    QCheckBox*     saveTagsBox;
    QCheckBox*     saveDateTimeBox;
    QCheckBox*     saveTemplateBox;
    QCheckBox*     writeRawFilesBox;
    QCheckBox*     useXMPSidecarBox;
    QCheckBox*     updateFileTimeStampBox;

    QCheckBox*     saveToNepomukBox;
    QCheckBox*     readFromNepomukBox;
    QToolButton*   resyncButton;

    KComboBox*     writingModeCombo;
    KTabWidget*    tab;

    MetadataPanel* tagsCfgPanel;
};

SetupMetadata::SetupMetadata(QWidget* parent)
    : QScrollArea(parent), d(new SetupMetadataPriv)
{
    d->tab = new KTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    QWidget* panel          = new QWidget(d->tab);
    QVBoxLayout* mainLayout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox* ExifGroup     = new QGroupBox(i18n("EXIF Actions"), panel);
    QVBoxLayout* gLayout1    = new QVBoxLayout(ExifGroup);

    d->exifRotateBox         = new QCheckBox(ExifGroup);
    d->exifRotateBox->setText(i18n("Show images/thumbnails &rotated according to orientation tag."));

    d->exifSetOrientationBox = new QCheckBox(ExifGroup);
    d->exifSetOrientationBox->setText(i18n("Set orientation tag to normal after rotate/flip."));

    gLayout1->addWidget(d->exifRotateBox);
    gLayout1->addWidget(d->exifSetOrientationBox);
    gLayout1->setMargin(KDialog::spacingHint());
    gLayout1->setSpacing(0);

    // --------------------------------------------------------

    QGroupBox* commonGroup = new QGroupBox(i18n("Common Metadata Actions"), panel);
    QVBoxLayout* gLayout2  = new QVBoxLayout(commonGroup);

    d->saveTagsBox = new QCheckBox(commonGroup);
    d->saveTagsBox->setText(i18n("&Save image tags as \"Keywords\" tags in metadata embedded in files"));
    d->saveTagsBox->setWhatsThis( i18n("Turn on this option to store the image tags "
                                       "in the XMP and IPTC tags."));

    d->saveTemplateBox = new QCheckBox(commonGroup);
    d->saveTemplateBox->setText(i18n("&Save metadata template as metadata embedded in files"));
    d->saveTemplateBox->setWhatsThis( i18n("Turn on this option to store the metadata "
                                           "template in the XMP and the IPTC tags. "
                                           "You can set template values to Template setup page."));

    d->saveCommentsBox = new QCheckBox(commonGroup);
    d->saveCommentsBox->setText(i18n("&Save image captions in metadata embedded in files "));
    d->saveCommentsBox->setWhatsThis( i18n("Turn on this option to store image captions "
                                           "in the JFIF Comment section, the EXIF tag, the XMP tag, "
                                           "and the IPTC tag."));

    d->saveDateTimeBox = new QCheckBox(commonGroup);
    d->saveDateTimeBox->setText(i18n("&Save image timestamps in metadata embedded in files"));
    d->saveDateTimeBox->setWhatsThis( i18n("Turn on this option to store the image date and time "
                                           "in the EXIF, XMP, and IPTC tags."));

    d->savePickLabelBox = new QCheckBox(commonGroup);
    d->savePickLabelBox->setText(i18n("&Save image pick label in metadata embedded in files"));
    d->savePickLabelBox->setWhatsThis( i18n("Turn on this option to store the image pick label "
                                            "in the XMP tags."));

    d->saveColorLabelBox = new QCheckBox(commonGroup);
    d->saveColorLabelBox->setText(i18n("&Save image color label in metadata embedded in files"));
    d->saveColorLabelBox->setWhatsThis( i18n("Turn on this option to store the image color label "
                                             "in the XMP tags."));

    d->saveRatingBox = new QCheckBox(commonGroup);
    d->saveRatingBox->setText(i18n("&Save image rating in metadata embedded in files"));
    d->saveRatingBox->setWhatsThis( i18n("Turn on this option to store the image rating "
                                         "in the EXIF tag and the XMP tags."));

    d->writeRawFilesBox = new QCheckBox(commonGroup);
    d->writeRawFilesBox->setText(i18n("&Write Metadata to RAW files (experimental)"));
    d->writeRawFilesBox->setWhatsThis( i18n("Turn on this option to write metadata into RAW TIFF/EP files. "
                                            "This feature requires the Exiv2 shared library, version >= 0.18.0. It is still "
                                            "experimental, and is disabled by default."));
    d->writeRawFilesBox->setEnabled(KExiv2::supportMetadataWritting("image/x-raw"));

    d->updateFileTimeStampBox = new QCheckBox(commonGroup);
    d->updateFileTimeStampBox->setText(i18n("&Update file timestamp when metadata are saved"));
    d->updateFileTimeStampBox->setWhatsThis( i18n("Turn on this option to update file timestamps when metadata are saved."));

    d->useXMPSidecarBox = new QCheckBox(commonGroup);
    d->useXMPSidecarBox->setText(i18n("&Read metadata from XMP sidecar files"));
    d->useXMPSidecarBox->setWhatsThis( i18n("Turn on this option to prefer metadata from XMP sidecar files when reading metadata."));
    d->useXMPSidecarBox->setEnabled(KExiv2::supportXmp());

    KHBox* hbox              = new KHBox(commonGroup);
    QLabel* writingModeLabel = new QLabel(i18n("Metadata Writing Mode:"), hbox);
    writingModeLabel->setEnabled(KExiv2::supportXmp());
    d->writingModeCombo      = new KComboBox(hbox);
    d->writingModeCombo->addItem(i18n("Write to image only"),                           KExiv2::WRITETOIMAGEONLY);
    d->writingModeCombo->addItem(i18n("Write to XMP sidecar only"),                     KExiv2::WRITETOSIDECARONLY);
    d->writingModeCombo->addItem(i18n("Write to image and XMP Sidecar"),                KExiv2::WRITETOSIDECARANDIMAGE);
    d->writingModeCombo->addItem(i18n("Write to XMP sidecar for read-only image only"), KExiv2::WRITETOSIDECARONLY4READONLYFILES);
    d->writingModeCombo->setToolTip(i18n("Choose here how metadata should be stored."));
    d->writingModeCombo->setEnabled(KExiv2::supportXmp());

    gLayout2->addWidget(d->saveTagsBox);
    gLayout2->addWidget(d->saveTemplateBox);
    gLayout2->addWidget(d->saveCommentsBox);
    gLayout2->addWidget(d->saveDateTimeBox);
    gLayout2->addWidget(d->savePickLabelBox);
    gLayout2->addWidget(d->saveColorLabelBox);
    gLayout2->addWidget(d->saveRatingBox);
    gLayout2->addWidget(d->updateFileTimeStampBox);
    gLayout2->addWidget(d->writeRawFilesBox);
    gLayout2->addWidget(d->useXMPSidecarBox);
    gLayout2->addWidget(hbox);
    gLayout2->setMargin(KDialog::spacingHint());
    gLayout2->setSpacing(0);

    // --------------------------------------------------------

    QFrame*      box  = new QFrame(panel);
    QGridLayout* grid = new QGridLayout(box);
    box->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    KUrlLabel* exiv2LogoLabel = new KUrlLabel(box);
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

    txt.append(i18n("<p><a href='http://en.wikipedia.org/wiki/IPTC_Information_Interchange_Model'>IPTC</a> - "
                    "an older standard used in digital photography to store "
                    "photographer information in images.</p>"));

    if (KExiv2::supportXmp())
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

    QWidget* nepoPanel      = new QWidget(d->tab);
    QVBoxLayout* nepoLayout = new QVBoxLayout(nepoPanel);

    QGroupBox* nepoGroup    = new QGroupBox(i18n("Nepomuk Semantic Desktop"), nepoPanel);
    QVBoxLayout* gLayout3   = new QVBoxLayout(nepoGroup);

    d->saveToNepomukBox     = new QCheckBox;
    d->saveToNepomukBox->setText(i18n("Store metadata from digiKam in Nepomuk"));
    d->saveToNepomukBox->setWhatsThis( i18n("Turn on this option to push rating, comments and tags "
                                            "from digiKam into the Nepomuk storage"));

    d->readFromNepomukBox   = new QCheckBox;
    d->readFromNepomukBox->setText(i18n("Read metadata from Nepomuk"));
    d->readFromNepomukBox->setWhatsThis( i18n("Turn on this option if you want to apply changes to "
                                         "rating, comments and tags made in Nepomuk to digiKam's metadata storage. "
                                         "Please note that image metadata will not be edited automatically."));

    gLayout3->addWidget(d->saveToNepomukBox);
    gLayout3->addWidget(d->readFromNepomukBox);

    d->resyncButton = new QToolButton;
    d->resyncButton->setText(i18n("Fully Resynchronize again"));
    d->resyncButton->setIcon(SmallIcon("edit-redo"));
    d->resyncButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->resyncButton->setCheckable(true);

    connect(d->saveToNepomukBox, SIGNAL(toggled(bool)),
            this, SLOT(slotNepomukToggled()));

    connect(d->readFromNepomukBox, SIGNAL(toggled(bool)),
            this, SLOT(slotNepomukToggled()));

    d->tab->addTab(nepoPanel, i18n("Nepomuk"));

    // --------------------------------------------------------

    QFrame*      nepoBox  = new QFrame(nepoPanel);
    QGridLayout* nepoGrid = new QGridLayout(nepoBox);
    nepoBox->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QLabel* nepoLogoLabel = new QLabel;
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

    nepoGrid->addWidget(nepoLogoLabel,   0, 0, 1, 1);
    nepoGrid->addWidget(nepoExplanation, 0, 1, 1, 2);
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
    nepoLayout->addWidget(d->resyncButton, 0, Qt::AlignRight);
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

    // --------------------------------------------------------

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    d->tab->setAutoFillBackground(false);
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
    AlbumSettings* aSettings    = AlbumSettings::instance();

    if (!aSettings)
    {
        return;
    }

    MetadataSettings* mSettings = MetadataSettings::instance();

    if (!mSettings)
    {
        return;
    }

    MetadataSettingsContainer set;
    set.exifRotate            = d->exifRotateBox->isChecked();
    set.exifSetOrientation    = d->exifSetOrientationBox->isChecked();
    set.saveComments          = d->saveCommentsBox->isChecked();
    set.saveDateTime          = d->saveDateTimeBox->isChecked();
    set.savePickLabel         = d->savePickLabelBox->isChecked();
    set.saveColorLabel        = d->saveColorLabelBox->isChecked();
    set.saveRating            = d->saveRatingBox->isChecked();
    set.saveTags              = d->saveTagsBox->isChecked();
    set.saveTemplate          = d->saveTemplateBox->isChecked();
    set.writeRawFiles         = d->writeRawFilesBox->isChecked();
    set.useXMPSidecar4Reading = d->useXMPSidecarBox->isChecked();
    set.metadataWritingMode   = d->writingModeCombo->currentIndex();
    set.updateFileTimeStamp   = d->updateFileTimeStampBox->isChecked();
    mSettings->setSettings(set);

#ifdef HAVE_NEPOMUK
    aSettings->setSyncDigikamToNepomuk(d->saveToNepomukBox->isChecked());
    aSettings->setSyncNepomukToDigikam(d->readFromNepomukBox->isChecked());

    if (d->resyncButton->isEnabled() && d->resyncButton->isChecked())
    {
        aSettings->triggerResyncWithNepomuk();
    }

#endif

    aSettings->saveSettings();

    d->tagsCfgPanel->applySettings();
}

void SetupMetadata::readSettings()
{
    AlbumSettings* aSettings      = AlbumSettings::instance();

    if (!aSettings)
    {
        return;
    }

    MetadataSettings* mSettings   = MetadataSettings::instance();

    if (!mSettings)
    {
        return;
    }

    MetadataSettingsContainer set = mSettings->settings();

    d->exifAutoRotateOrg = set.exifRotate;
    d->exifRotateBox->setChecked(d->exifAutoRotateOrg);
    d->exifSetOrientationBox->setChecked(set.exifSetOrientation);
    d->saveCommentsBox->setChecked(set.saveComments);
    d->saveDateTimeBox->setChecked(set.saveDateTime);
    d->savePickLabelBox->setChecked(set.savePickLabel);
    d->saveColorLabelBox->setChecked(set.saveColorLabel);
    d->saveRatingBox->setChecked(set.saveRating);
    d->saveTagsBox->setChecked(set.saveTags);
    d->saveTemplateBox->setChecked(set.saveTemplate);
    d->writeRawFilesBox->setChecked(set.writeRawFiles);
    d->useXMPSidecarBox->setChecked(set.useXMPSidecar4Reading);
    d->writingModeCombo->setCurrentIndex(set.metadataWritingMode);
    d->updateFileTimeStampBox->setChecked(set.updateFileTimeStamp);

#ifdef HAVE_NEPOMUK
    d->saveToNepomukBox->setChecked(aSettings->getSyncDigikamToNepomuk());
    d->readFromNepomukBox->setChecked(aSettings->getSyncNepomukToDigikam());
    slotNepomukToggled();
#endif
}

bool SetupMetadata::exifAutoRotateAsChanged()
{
    return d->exifAutoRotateAsChanged;
}

void SetupMetadata::slotExifAutoRotateToggled(bool b)
{
    if ( b != d->exifAutoRotateOrg)
    {
        d->exifAutoRotateAsChanged = true;
    }
    else
    {
        d->exifAutoRotateAsChanged = false;
    }
}

void SetupMetadata::slotNepomukToggled()
{
    d->resyncButton->setEnabled(d->saveToNepomukBox->isChecked() || d->readFromNepomukBox->isChecked());
}

}  // namespace Digikam
