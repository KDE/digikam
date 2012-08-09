/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-08-03
 * Description : setup Metadata tab.
 *
 * Copyright (C) 2003-2004 by Ralf Holzer <ralf at well.com>
 * Copyright (C) 2003-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
#include <QRadioButton>
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
        fieldsGroup(0),
        readWriteGroup(0),
        rotationGroup(0),
        rotationAdvGroup(0),
        saveTagsBox(0),
        saveCommentsBox(0),
        saveRatingBox(0),
        savePickLabelBox(0),
        saveColorLabelBox(0),
        saveDateTimeBox(0),
        saveTemplateBox(0),
        writeRawFilesBox(0),
        writeXMPSidecarBox(0),
        readXMPSidecarBox(0),
        updateFileTimeStampBox(0),
        writingModeCombo(0),
        rotateByFlag(0),
        rotateByContents(0),
        allowRotateByMetadata(0),
        allowLossyRotate(0),
        exifRotateBox(0),
        exifSetOrientationBox(0),
        saveToNepomukBox(0),
        readFromNepomukBox(0),
        resyncButton(0),
        tab(0),
        tagsCfgPanel(0)
    {
    }

    bool           exifAutoRotateAsChanged;
    bool           exifAutoRotateOrg;

    QGroupBox*     fieldsGroup;
    QGroupBox*     readWriteGroup;
    QGroupBox*     rotationGroup;
    QGroupBox*     rotationAdvGroup;

    QCheckBox*     saveTagsBox;
    QCheckBox*     saveCommentsBox;
    QCheckBox*     saveRatingBox;
    QCheckBox*     savePickLabelBox;
    QCheckBox*     saveColorLabelBox;
    QCheckBox*     saveDateTimeBox;
    QCheckBox*     saveTemplateBox;

    QCheckBox*     writeRawFilesBox;
    QCheckBox*     writeXMPSidecarBox;
    QCheckBox*     readXMPSidecarBox;
    QCheckBox*     updateFileTimeStampBox;
    KComboBox*     writingModeCombo;

    QRadioButton*  rotateByFlag;
    QRadioButton*  rotateByContents;
    QCheckBox*     allowRotateByMetadata;
    QCheckBox*     allowLossyRotate;
    QCheckBox*     exifRotateBox;
    QCheckBox*     exifSetOrientationBox;

    QCheckBox*     saveToNepomukBox;
    QCheckBox*     readFromNepomukBox;
    QToolButton*   resyncButton;

    KTabWidget*    tab;

    MetadataPanel* tagsCfgPanel;
};

SetupMetadata::SetupMetadata(QWidget* parent)
    : QScrollArea(parent), d(new SetupMetadataPriv)
{
    d->tab = new KTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    QWidget* panel          = new QWidget;
    QVBoxLayout* mainLayout = new QVBoxLayout;

    // --------------------------------------------------------

    d->fieldsGroup            = new QGroupBox;
    QGridLayout* fieldsLayout = new QGridLayout;

    d->fieldsGroup->setWhatsThis(i18nc("@info:whatsthis",
                                       "<para>In addition to the pixel content, image files usually "
                                       "contain a variety of metadata. A lot of the parameters you can use "
                                       "in digiKam to manage files, such as rating or comment, can be written "
                                       "to the files' metadata.</para> "
                                       "<para>Storing in metadata allows to preserve this information "
                                       "when moving or sending the files to different systems.</para>"));

    QLabel* fieldsIconLabel = new QLabel;
    fieldsIconLabel->setPixmap(SmallIcon("format-list-unordered", KIconLoader::SizeMedium));

    QLabel* fieldsLabel     = new QLabel(i18nc("@label", "Write This Information to the Metadata"));

    d->saveTagsBox          = new QCheckBox;
    d->saveTagsBox->setText(i18nc("@option:check", "Image tags"));
    d->saveTagsBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store the image tags "
                                       "in the XMP and IPTC tags."));

    d->saveCommentsBox = new QCheckBox;
    d->saveCommentsBox->setText(i18nc("@option:check", "Captions and title"));
    d->saveCommentsBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store image caption and title "
                                           "in the JFIF Comment section, the EXIF tag, the XMP tag, "
                                           "and the IPTC tag."));

    d->saveRatingBox = new QCheckBox;
    d->saveRatingBox->setText(i18nc("@option:check", "Rating"));
    d->saveRatingBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store the image rating "
                                         "in the EXIF tag and the XMP tags."));

    d->savePickLabelBox = new QCheckBox;
    d->savePickLabelBox->setText(i18nc("@option:check", "Pick label"));
    d->savePickLabelBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store the image pick label "
                                            "in the XMP tags."));

    d->saveColorLabelBox = new QCheckBox;
    d->saveColorLabelBox->setText(i18nc("@option:check", "Color label"));
    d->saveColorLabelBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store the image color label "
                                             "in the XMP tags."));

    d->saveDateTimeBox = new QCheckBox;
    d->saveDateTimeBox->setText(i18nc("@option:check", "Timestamps"));
    d->saveDateTimeBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store the image date and time "
                                           "in the EXIF, XMP, and IPTC tags."));

    d->saveTemplateBox = new QCheckBox;
    d->saveTemplateBox->setText(i18nc("@option:check", "Metadata templates (Copyright etc.)"));
    d->saveTemplateBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store the metadata "
                                           "template in the XMP and the IPTC tags. "
                                           "You can set template values to Template setup page."));

    fieldsLayout->addWidget(fieldsIconLabel,       0, 0);
    fieldsLayout->addWidget(fieldsLabel,           0, 1);
    fieldsLayout->addWidget(d->saveTagsBox,        1, 0, 1, 3);
    fieldsLayout->addWidget(d->saveCommentsBox,    2, 0, 1, 3);
    fieldsLayout->addWidget(d->saveRatingBox,      3, 0, 1, 3);
    fieldsLayout->addWidget(d->savePickLabelBox,   4, 0, 1, 3);
    fieldsLayout->addWidget(d->saveColorLabelBox,  5, 0, 1, 3);
    fieldsLayout->addWidget(d->saveDateTimeBox,    6, 0, 1, 3);
    fieldsLayout->addWidget(d->saveTemplateBox,    7, 0, 1, 3);
    fieldsLayout->setColumnStretch(2, 1);
    d->fieldsGroup->setLayout(fieldsLayout);

    // --------------------------------------------------------

    d->readWriteGroup            = new QGroupBox;
    QGridLayout* readWriteLayout = new QGridLayout;

    QLabel* readWriteIconLabel = new QLabel;
    readWriteIconLabel->setPixmap(SmallIcon("document-open", KIconLoader::SizeMedium));

    QLabel* readWriteLabel = new QLabel(i18nc("@label", "Reading and Writing Metadata"));

    d->readXMPSidecarBox   = new QCheckBox;
    d->readXMPSidecarBox->setText(i18nc("@option:check", "Read from sidecar files"));
    d->readXMPSidecarBox->setWhatsThis(i18nc("@info:whatsthis",
                                             "Turn on this option to read metadata from XMP sidecar files when reading metadata."));
    d->readXMPSidecarBox->setEnabled(KExiv2::supportXmp());

    d->writeXMPSidecarBox  = new QCheckBox;
    d->writeXMPSidecarBox->setText(i18nc("@option:check", "Write to sidecar files"));
    d->writeXMPSidecarBox->setWhatsThis(i18nc("@info:whatsthis",
                                              "Turn on this option to save, as specified, metadata to XMP sidecar files."));
    d->writeXMPSidecarBox->setEnabled(KExiv2::supportXmp());

    d->writingModeCombo    = new KComboBox;
    d->writingModeCombo->addItem(i18n("Write to XMP sidecar for read-only image only"), KExiv2::WRITETOSIDECARONLY4READONLYFILES);
    d->writingModeCombo->addItem(i18n("Write to XMP sidecar only"),                     KExiv2::WRITETOSIDECARONLY);
    d->writingModeCombo->addItem(i18n("Write to image and XMP Sidecar"),                KExiv2::WRITETOSIDECARANDIMAGE);
    d->writingModeCombo->setToolTip(i18nc("@info:tooltip", "Specify the exact mode of XMP sidecar writing"));
    d->writingModeCombo->setEnabled(false);

    connect(d->writeXMPSidecarBox, SIGNAL(toggled(bool)),
            d->writingModeCombo, SLOT(setEnabled(bool)));

    d->writeRawFilesBox = new QCheckBox;
    d->writeRawFilesBox->setText(i18nc("@option:check", "If possible write Metadata to RAW files (experimental)"));
    d->writeRawFilesBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to write metadata into RAW TIFF/EP files. "
                                            "This feature requires the Exiv2 shared library, version >= 0.18.0. It is still "
                                            "experimental, and is disabled by default."));
    d->writeRawFilesBox->setEnabled(KExiv2::supportMetadataWritting("image/x-raw"));

    d->updateFileTimeStampBox = new QCheckBox;
    d->updateFileTimeStampBox->setText(i18nc("@option:check", "&Update file timestamp when metadata is saved"));
    d->updateFileTimeStampBox->setWhatsThis(i18nc("@info:whatsthis",
                                                  "Turn on this option to update file timestamps when metadata saved."));

    readWriteLayout->addWidget(readWriteIconLabel,        0, 0);
    readWriteLayout->addWidget(readWriteLabel,            0, 1);
    readWriteLayout->addWidget(d->readXMPSidecarBox,      1, 0, 1, 3);
    readWriteLayout->addWidget(d->writeXMPSidecarBox,     2, 0, 1, 3);
    readWriteLayout->addWidget(d->writingModeCombo,       3, 1, 1, 2);
    readWriteLayout->addWidget(d->writeRawFilesBox,       4, 0, 1, 3);
    readWriteLayout->addWidget(d->updateFileTimeStampBox, 5, 0, 1, 3);
    readWriteLayout->setColumnStretch(3, 1);
    d->readWriteGroup->setLayout(readWriteLayout);

    // --------------------------------------------------------

    QFrame*      infoBox  = new QFrame;
    QGridLayout* infoBoxGrid = new QGridLayout;
    infoBox->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    KUrlLabel* exiv2LogoLabel = new KUrlLabel(infoBox);
    exiv2LogoLabel->setText(QString());
    exiv2LogoLabel->setUrl("http://www.exiv2.org");
    exiv2LogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-exiv2.png")));
    exiv2LogoLabel->setWhatsThis(i18n("Visit Exiv2 project website"));

    QLabel* explanation = new QLabel(infoBox);
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

    infoBoxGrid->addWidget(exiv2LogoLabel, 0, 0, 1, 1);
    infoBoxGrid->addWidget(explanation,    0, 1, 1, 2);
    infoBoxGrid->setColumnStretch(1, 10);
    infoBoxGrid->setRowStretch(1, 10);
    infoBoxGrid->setMargin(KDialog::spacingHint());
    infoBoxGrid->setSpacing(0);
    infoBox->setLayout(infoBoxGrid);

    // --------------------------------------------------------

    mainLayout->addWidget(d->fieldsGroup);
    mainLayout->addWidget(d->readWriteGroup);
    mainLayout->addWidget(infoBox);
    panel->setLayout(mainLayout);

    d->tab->addTab(panel, i18n("Behavior"));

    // --------------------------------------------------------

    QWidget* rotationPanel      = new QWidget(d->tab);
    QVBoxLayout* rotationLayout = new QVBoxLayout;

    d->rotationGroup                 = new QGroupBox;
    QGridLayout* rotationGroupLayout = new QGridLayout;

    QLabel* rotationExplanation = new QLabel(i18nc("@label", "When rotating a file"));
    QLabel* rotationIcon        = new QLabel;
    rotationIcon->setPixmap(SmallIcon("transform-rotate", KIconLoader::SizeMedium));

    d->rotateByFlag             = new QRadioButton(i18nc("@option:radio", "Rotate by only setting a flag"));
    d->rotateByContents         = new QRadioButton(i18nc("@option:radio", "Rotate by changing the content if possible"));
    d->allowLossyRotate         = new QCheckBox(i18nc("@option:check", "Even allow lossy rotation if necessary"));
    d->allowRotateByMetadata    = new QCheckBox(i18nc("@option:check", "Write flag to metadata if possible"));

    connect(d->rotateByContents, SIGNAL(toggled(bool)),
            d->allowLossyRotate, SLOT(setEnabled(bool)));

    d->rotateByFlag->setChecked(false);
    d->rotateByContents->setChecked(false);
    d->allowLossyRotate->setEnabled(false);
    d->allowLossyRotate->setChecked(false);
    d->allowRotateByMetadata->setChecked(true);

    d->rotateByFlag->setToolTip(i18nc("@info:tooltip",
                                      "Rotate files only by changing a flag, not touching the pixel data"));
    d->rotateByFlag->setWhatsThis(i18nc("@info:whatsthis",
                                        "<para>A file can be rotated in two ways:<br/> "
                                        "You can change the contents, rearranging the individual pixels of the image data.<br/> "
                                        "Or you can set a flag that the file is to be rotated before it is shown.</para> "
                                        "<para>Select this option if you always want to set only a flag. "
                                        "This is less obtrusive, but requires support if the file is accessed with another software. "
                                        "Ensure to allow setting the flag in the metadata if you want to share your files "
                                        "outside digiKam.</para>"));

    d->rotateByContents->setToolTip(i18nc("@info:tooltip",
                                          "If possible rotate files by changing the pixel data"));
    d->rotateByContents->setWhatsThis(i18nc("@info:whatsthis",
                                            "<para>A file can be rotated in two ways:<br/> "
                                            "You can change the contents, rearranging the individual pixels of the image data.<br/> "
                                            "Or you can set a flag that the file is to be rotated before it is shown.</para> "
                                            "<para>Select this option if you want the file to be rotated by changing the content. "
                                            "This is a lossless operation for JPEG files. For other formats it is a lossy operation, "
                                            "which you need to enable explicitly. "
                                            "It is not support for RAW and other read-only formats, "
                                            "which will be rotated by flag only.</para>"));

    d->allowLossyRotate->setToolTip(i18nc("@info:tooltip",
                                          "Rotate files by changing the pixel data even if the operation will incur quality loss"));
    d->allowLossyRotate->setWhatsThis(i18nc("@info:whatsthis",
                                            "For some file formats which apply lossy compression, "
                                            "data will be lost each time the content is rotated. "
                                            "Check this option to allow lossy rotation. "
                                            "If not enabled, these files will be rotated by flag."));

    d->allowRotateByMetadata->setToolTip(i18nc("@info:tooltip",
                                               "When rotating a file by setting a flag, also change this flag in the file's metadata"));
    d->allowRotateByMetadata->setWhatsThis(i18nc("@info:whatsthis",
                                                 "File metadata typically contains a flag describing "
                                                 "that a file shall be shown rotated. "
                                                 "Enable this option to allow editing this field. "));

    rotationGroupLayout->addWidget(rotationIcon,             0, 0, 1, 1);
    rotationGroupLayout->addWidget(rotationExplanation,      0, 1, 1, 2);
    rotationGroupLayout->addWidget(d->rotateByFlag,          1, 0, 1, 3);
    rotationGroupLayout->addWidget(d->rotateByContents,      2, 0, 1, 3);
    rotationGroupLayout->addWidget(d->allowLossyRotate,      3, 2, 1, 1);
    rotationGroupLayout->addWidget(d->allowRotateByMetadata, 4, 0, 1, 3);
    rotationGroupLayout->setColumnStretch(3, 10);

    d->rotationGroup->setLayout(rotationGroupLayout);

    // --------------------------------------------------------

    d->rotationAdvGroup            = new QGroupBox;
    QGridLayout* rotationAdvLayout = new QGridLayout;

    QLabel* rotationAdvExpl  = new QLabel(i18nc("@label", "Advanced Settings"));
    QLabel* rotationAdvIcon  = new QLabel;
    rotationAdvIcon->setPixmap(SmallIcon("configure", KIconLoader::SizeMedium));

    d->exifRotateBox         = new QCheckBox;
    d->exifRotateBox->setText(i18n("Show images/thumbnails &rotated according to orientation tag."));
    d->exifSetOrientationBox = new QCheckBox;
    d->exifSetOrientationBox->setText(i18n("Set orientation tag to normal after rotate/flip."));

    rotationAdvLayout->addWidget(rotationAdvIcon,          0, 0, 1, 1);
    rotationAdvLayout->addWidget(rotationAdvExpl,          0, 1, 1, 1);
    rotationAdvLayout->addWidget(d->exifRotateBox,         1, 0, 1, 3);
    rotationAdvLayout->addWidget(d->exifSetOrientationBox, 2, 0, 1, 3);
    rotationAdvLayout->setColumnStretch(2, 10);
    d->rotationAdvGroup->setLayout(rotationAdvLayout);

    // --------------------------------------------------------

    rotationLayout->addWidget(d->rotationGroup);
    rotationLayout->addWidget(d->rotationAdvGroup);
    rotationLayout->addStretch();
    rotationPanel->setLayout(rotationLayout);

    d->tab->addTab(rotationPanel, i18n("Rotation"));

    // --------------------------------------------------------

    QWidget* displayPanel      = new QWidget;
    QGridLayout* displayLayout = new QGridLayout;

    QLabel* displayLabel       = new QLabel(i18nc("@info:label", "Select Metadata Fields to Be Displayed"));

    QLabel* displayIcon        = new QLabel;
    displayIcon->setPixmap(SmallIcon("view-list-tree", KIconLoader::SizeMedium));//"folder-image"));

    KTabWidget* displaySubTab  = new KTabWidget;
    d->tagsCfgPanel            = new MetadataPanel(displaySubTab);

    displayLayout->addWidget(displayIcon,   0, 0);
    displayLayout->addWidget(displayLabel,  0, 1);
    displayLayout->addWidget(displaySubTab, 1, 1, 1, 2);
    displayLayout->setColumnStretch(2, 1);

    displayPanel->setLayout(displayLayout);
    d->tab->addTab(displayPanel, i18nc("@title:tab", "Display"));

    // --------------------------------------------------------

#ifdef HAVE_NEPOMUK

    QWidget* nepoPanel      = new QWidget(d->tab);
    QVBoxLayout* nepoLayout = new QVBoxLayout(nepoPanel);

    QGroupBox* nepoGroup    = new QGroupBox(i18n("Nepomuk Semantic Desktop"), nepoPanel);
    QVBoxLayout* gLayout3   = new QVBoxLayout(nepoGroup);

    d->saveToNepomukBox     = new QCheckBox;
    d->saveToNepomukBox->setText(i18n("Store metadata from digiKam in Nepomuk"));
    d->saveToNepomukBox->setWhatsThis(i18n("Turn on this option to push rating, comments and tags "
                                           "from digiKam into the Nepomuk storage"));

    d->readFromNepomukBox   = new QCheckBox;
    d->readFromNepomukBox->setText(i18n("Read metadata from Nepomuk"));
    d->readFromNepomukBox->setWhatsThis(i18n("Turn on this option if you want to apply changes to "
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

    QLabel* nepoLogoLabel   = new QLabel;
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

    readSettings();

    connect(exiv2LogoLabel, SIGNAL(leftClickedUrl(QString)),
            this, SLOT(slotProcessExiv2Url(QString)));

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

    set.rotationBehavior = MetadataSettingsContainer::RotateByInternalFlag;

    if (d->allowRotateByMetadata->isChecked())
    {
        set.rotationBehavior |= MetadataSettingsContainer::RotateByMetadataFlag;
    }

    if (d->rotateByContents->isChecked())
    {
        set.rotationBehavior |= MetadataSettingsContainer::RotateByLosslessRotation;

        if (d->allowLossyRotate->isChecked())
        {
            set.rotationBehavior |= MetadataSettingsContainer::RotateByLossyRotation;
        }
    }

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
    set.useXMPSidecar4Reading = d->readXMPSidecarBox->isChecked();

    if (d->writeXMPSidecarBox->isChecked())
    {
        set.metadataWritingMode   = (KExiv2::MetadataWritingMode)
                                    d->writingModeCombo->itemData(d->writingModeCombo->currentIndex()).toInt();
    }
    else
    {
        set.metadataWritingMode   = KExiv2::WRITETOIMAGEONLY;
    }

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

    if (set.rotationBehavior & MetadataSettingsContainer::RotatingPixels)
    {
        d->rotateByContents->setChecked(true);
    }
    else
    {
        d->rotateByFlag->setChecked(true);
    }

    d->allowRotateByMetadata->setChecked(set.rotationBehavior & MetadataSettingsContainer::RotateByMetadataFlag);
    d->allowLossyRotate->setChecked(set.rotationBehavior & MetadataSettingsContainer::RotateByLossyRotation);

    d->exifAutoRotateOrg = set.exifRotate;
    d->exifRotateBox->setChecked(d->exifAutoRotateOrg);
    d->exifSetOrientationBox->setChecked(set.exifSetOrientation);

    d->saveTagsBox->setChecked(set.saveTags);
    d->saveCommentsBox->setChecked(set.saveComments);
    d->saveRatingBox->setChecked(set.saveRating);
    d->savePickLabelBox->setChecked(set.savePickLabel);
    d->saveColorLabelBox->setChecked(set.saveColorLabel);
    d->saveDateTimeBox->setChecked(set.saveDateTime);
    d->saveTemplateBox->setChecked(set.saveTemplate);

    d->writeRawFilesBox->setChecked(set.writeRawFiles);
    d->readXMPSidecarBox->setChecked(set.useXMPSidecar4Reading);
    d->updateFileTimeStampBox->setChecked(set.updateFileTimeStamp);

    if (set.metadataWritingMode == KExiv2::WRITETOIMAGEONLY)
    {
        d->writeXMPSidecarBox->setChecked(false);
    }
    else
    {
        d->writeXMPSidecarBox->setChecked(true);
        d->writingModeCombo->setCurrentIndex(d->writingModeCombo->findData(set.metadataWritingMode));
    }

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
    if (b != d->exifAutoRotateOrg)
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
