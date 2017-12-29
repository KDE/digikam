/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-08-03
 * Description : setup Metadata tab.
 *
 * Copyright (C) 2003-2004 by Ralf Holzer <ralf at well dot com>
 * Copyright (C) 2003-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2017      by Simon Frei <freisim93 at gmail dot com>
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

// Qt includes

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRadioButton>
#include <QStandardPaths>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "advancedmetadatatab.h"
#include "applicationsettings.h"
#include "dactivelabel.h"
#include "digikam_config.h"
#include "digikam_debug.h"
#include "metaengine.h"
#include "metadatapanel.h"
#include "metadatasettings.h"
#include "setuputils.h"

namespace Digikam
{

class SetupMetadata::Private
{
public:

    Private() :
        exifAutoRotateOriginal(false),
        exifAutoRotateShowedInfo(false),
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
        saveFaceTags(0),
        useLazySync(0),
        writeRawFilesBox(0),
        writeXMPSidecarBox(0),
        readXMPSidecarBox(0),
        updateFileTimeStampBox(0),
        rescanImageIfModifiedBox(0),
        writingModeCombo(0),
        rotateByFlag(0),
        rotateByContents(0),
        allowRotateByMetadata(0),
        allowLossyRotate(0),
        exifRotateBox(0),
        exifSetOrientationBox(0),
        saveToBalooBox(0),
        readFromBalooBox(0),
        tab(0),
        displaySubTab(0),
        tagsCfgPanel(0),
        advTab(0),
        extensionsEdit(0)
    {
    }

    bool                 exifAutoRotateOriginal;
    bool                 exifAutoRotateShowedInfo;

    QGroupBox*           fieldsGroup;
    QGroupBox*           readWriteGroup;
    QGroupBox*           rotationGroup;
    QGroupBox*           rotationAdvGroup;

    QCheckBox*           saveTagsBox;
    QCheckBox*           saveCommentsBox;
    QCheckBox*           saveRatingBox;
    QCheckBox*           savePickLabelBox;
    QCheckBox*           saveColorLabelBox;
    QCheckBox*           saveDateTimeBox;
    QCheckBox*           saveTemplateBox;
    QCheckBox*           saveFaceTags;

    QCheckBox*           useLazySync;
    QCheckBox*           writeRawFilesBox;
    QCheckBox*           writeXMPSidecarBox;
    QCheckBox*           readXMPSidecarBox;
    QCheckBox*           updateFileTimeStampBox;
    QCheckBox*           rescanImageIfModifiedBox;
    QComboBox*           writingModeCombo;

    QRadioButton*        rotateByFlag;
    QRadioButton*        rotateByContents;
    QCheckBox*           allowRotateByMetadata;
    QCheckBox*           allowLossyRotate;
    QCheckBox*           exifRotateBox;
    QCheckBox*           exifSetOrientationBox;

    QCheckBox*           saveToBalooBox;
    QCheckBox*           readFromBalooBox;

    QTabWidget*          tab;
    QTabWidget*          displaySubTab;

    MetadataPanel*       tagsCfgPanel;
    AdvancedMetadataTab* advTab;

    QLineEdit*           extensionsEdit;
};

SetupMetadata::SetupMetadata(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    d->tab = new QTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    QWidget* const panel          = new QWidget;
    QVBoxLayout* const mainLayout = new QVBoxLayout;

    // --------------------------------------------------------

    d->fieldsGroup                  = new QGroupBox;
    QGridLayout* const fieldsLayout = new QGridLayout;

    d->fieldsGroup->setWhatsThis(xi18nc("@info:whatsthis",
                                       "<para>In addition to the pixel content, image files usually "
                                       "contain a variety of metadata. A lot of the parameters you can use "
                                       "in digiKam to manage files, such as rating or comment, can be written "
                                       "to the files' metadata.</para> "
                                       "<para>Storing in metadata allows one to preserve this information "
                                       "when moving or sending the files to different systems.</para>"));

    QLabel* const fieldsIconLabel = new QLabel;
    fieldsIconLabel->setPixmap(QIcon::fromTheme(QLatin1String("format-list-unordered")).pixmap(32));

    QLabel* const fieldsLabel     = new QLabel(i18nc("@label", "Write This Information to the Metadata"));

    d->saveTagsBox      = new QCheckBox;
    d->saveTagsBox->setText(i18nc("@option:check", "Image tags"));
    d->saveTagsBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store the image tags "
                                       "in the XMP and IPTC tags."));

    d->saveCommentsBox  = new QCheckBox;
    d->saveCommentsBox->setText(i18nc("@option:check", "Captions and title"));
    d->saveCommentsBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store image caption and title "
                                           "in the JFIF Comment section, the EXIF tag, the XMP tag, "
                                           "and the IPTC tag."));

    d->saveRatingBox    = new QCheckBox;
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

    d->saveDateTimeBox   = new QCheckBox;
    d->saveDateTimeBox->setText(i18nc("@option:check", "Timestamps"));
    d->saveDateTimeBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store the image date and time "
                                           "in the EXIF, XMP, and IPTC tags."));

    d->saveTemplateBox   = new QCheckBox;
    d->saveTemplateBox->setText(i18nc("@option:check", "Metadata templates (Copyright etc.)"));
    d->saveTemplateBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store the metadata "
                                           "template in the XMP and the IPTC tags. "
                                           "You can set template values to Template setup page."));
    d->saveFaceTags = new QCheckBox;
    d->saveFaceTags->setText(i18nc("@option:check", "Face Tags (including face areas)"));
    d->saveFaceTags->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to store face tags "
                                           "with face rectangles in the XMP tags."));

    fieldsLayout->addWidget(fieldsIconLabel,       0, 0, 2, 3);
    fieldsLayout->addWidget(fieldsLabel,           0, 1, 2, 3);
    fieldsLayout->addWidget(d->saveTagsBox,        2, 0, 1, 3);
    fieldsLayout->addWidget(d->saveCommentsBox,    3, 0, 1, 3);
    fieldsLayout->addWidget(d->saveRatingBox,      4, 0, 1, 3);
    fieldsLayout->addWidget(d->savePickLabelBox,   5, 0, 1, 3);
    fieldsLayout->addWidget(d->saveColorLabelBox,  6, 0, 1, 3);
    fieldsLayout->addWidget(d->saveDateTimeBox,    7, 0, 1, 3);
    fieldsLayout->addWidget(d->saveTemplateBox,    8, 0, 1, 3);
    fieldsLayout->addWidget(d->saveFaceTags,       9 ,0, 1, 3);
    fieldsLayout->setColumnStretch(3, 10);
    d->fieldsGroup->setLayout(fieldsLayout);

    // --------------------------------------------------------

    d->readWriteGroup                  = new QGroupBox;
    QGridLayout* const readWriteLayout = new QGridLayout;

    QLabel* const readWriteIconLabel   = new QLabel;
    readWriteIconLabel->setPixmap(QIcon::fromTheme(QLatin1String("document-open")).pixmap(32));

    QLabel* const readWriteLabel       = new QLabel(i18nc("@label", "Reading and Writing Metadata"));

    d->useLazySync        = new QCheckBox;
    d->useLazySync->setText(i18nc("@option:check", "Use lazy synchronization"));
    d->useLazySync->setWhatsThis(i18nc("@info:whatsthis",
                                             "Instead of synchronizing metadata, just schedule it for synchronization."
                                             "Synchronization can be done later by triggering the apply pending, or at digikam exit"));
    d->writeRawFilesBox = new QCheckBox;
    d->writeRawFilesBox->setText(i18nc("@option:check", "If possible write Metadata to RAW files (experimental)"));
    d->writeRawFilesBox->setWhatsThis(i18nc("@info:whatsthis", "Turn on this option to write metadata into RAW TIFF/EP files. "
                                            "This feature requires the Exiv2 shared library, version >= 0.18.0. It is still "
                                            "experimental, and is disabled by default."));
    d->writeRawFilesBox->setEnabled(MetaEngine::supportMetadataWritting(QLatin1String("image/x-raw")));

    d->updateFileTimeStampBox = new QCheckBox;
    d->updateFileTimeStampBox->setText(i18nc("@option:check", "&Update file timestamp when files are modified"));
    d->updateFileTimeStampBox->setWhatsThis(i18nc("@info:whatsthis",
                                                  "Turn off this option to not update file timestamps when files are changed as when you update metadata or image data. "
                                                  "Note: disabling this option can introduce some dysfunctions with applications which use file timestamps properties to "
                                                  "detect file modifications automatically."));

    d->rescanImageIfModifiedBox = new QCheckBox;
    d->rescanImageIfModifiedBox->setText(i18nc("@option:check", "&Rescan file when files are modified"));
    d->rescanImageIfModifiedBox->setWhatsThis(i18nc("@info:whatsthis",
                                                  "Turning this option on, will force digiKam to rescan files that has been modified outside digiKam. "
                                                  "If a file has changed it's file size or if the last modified timestamp has changed, a rescan of that "
                                                  "file will be performed when digiKam starts."));

    readWriteLayout->addWidget(readWriteIconLabel,          0, 0, 2, 3);
    readWriteLayout->addWidget(readWriteLabel,              0, 1, 2, 3);
    readWriteLayout->addWidget(d->useLazySync,              2, 0, 1, 3);
    readWriteLayout->addWidget(d->writeRawFilesBox,         3, 0, 1, 3);
    readWriteLayout->addWidget(d->updateFileTimeStampBox,   4, 0, 1, 3);
    readWriteLayout->addWidget(d->rescanImageIfModifiedBox, 5, 0, 1, 3);
    readWriteLayout->setColumnStretch(3, 10);
    d->readWriteGroup->setLayout(readWriteLayout);

    // --------------------------------------------------------

    QFrame* const infoBox           = new QFrame;
    QGridLayout* const infoBoxGrid  = new QGridLayout;
    infoBox->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    DActiveLabel* const exiv2LogoLabel = new DActiveLabel(QUrl(QLatin1String("http://www.exiv2.org")),
                                                          QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/logo-exiv2.png")),
                                                          infoBox);
    exiv2LogoLabel->setWhatsThis(i18n("Visit Exiv2 project website"));

    QLabel* const explanation = new QLabel(infoBox);
    explanation->setOpenExternalLinks(true);
    explanation->setWordWrap(true);
    QString txt;

    txt.append(i18n("<p><a href='http://en.wikipedia.org/wiki/Exif'>EXIF</a> - "
                    "a standard used by most digital cameras today to store technical "
                    "information (like aperture and shutter speed) about an image.</p>"));

    txt.append(i18n("<p><a href='http://en.wikipedia.org/wiki/IPTC_Information_Interchange_Model'>IPTC</a> - "
                    "an older standard used in digital photography to store "
                    "photographer information in images.</p>"));

    if (MetaEngine::supportXmp())
        txt.append(i18n("<p><a href='http://en.wikipedia.org/wiki/Extensible_Metadata_Platform'>XMP</a> - "
                        "a new standard used in digital photography, designed to replace IPTC.</p>"));

    explanation->setText(txt);

    infoBoxGrid->addWidget(exiv2LogoLabel, 0, 0, 1, 1);
    infoBoxGrid->addWidget(explanation,    0, 1, 1, 2);
    infoBoxGrid->setColumnStretch(1, 10);
    infoBoxGrid->setRowStretch(1, 10);
    infoBoxGrid->setSpacing(0);
    infoBox->setLayout(infoBoxGrid);

    // --------------------------------------------------------

    mainLayout->addWidget(d->fieldsGroup);
    mainLayout->addWidget(d->readWriteGroup);
    mainLayout->addWidget(infoBox);
    panel->setLayout(mainLayout);

    d->tab->insertTab(Behavior, panel, i18nc("@title:tab", "Behavior"));

    // --------------------------------------------------------

    QWidget* const rotationPanel      = new QWidget(d->tab);
    QVBoxLayout* const rotationLayout = new QVBoxLayout;

    d->rotationGroup                       = new QGroupBox;
    QGridLayout* const rotationGroupLayout = new QGridLayout;

    QLabel* const rotationExplanation = new QLabel(i18nc("@label", "When rotating a file"));
    QLabel* const rotationIcon        = new QLabel;
    rotationIcon->setPixmap(QIcon::fromTheme(QLatin1String("transform-rotate")).pixmap(32));

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
    d->rotateByFlag->setWhatsThis(xi18nc("@info:whatsthis",
                                        "<para>A file can be rotated in two ways:<nl/> "
                                        "You can change the contents, rearranging the individual pixels of the image data.<nl/> "
                                        "Or you can set a flag that the file is to be rotated before it is shown.</para> "
                                        "<para>Select this option if you always want to set only a flag. "
                                        "This is less obtrusive, but requires support if the file is accessed with another software. "
                                        "Ensure to allow setting the flag in the metadata if you want to share your files "
                                        "outside digiKam.</para>"));

    d->rotateByContents->setToolTip(i18nc("@info:tooltip",
                                          "If possible rotate files by changing the pixel data"));
    d->rotateByContents->setWhatsThis(xi18nc("@info:whatsthis",
                                            "<para>A file can be rotated in two ways:<nl/> "
                                            "You can change the contents, rearranging the individual pixels of the image data.<nl/> "
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

    d->rotationAdvGroup                  = new QGroupBox;
    QGridLayout* const rotationAdvLayout = new QGridLayout;

    QLabel* const rotationAdvExpl  = new QLabel(i18nc("@label", "Rotate actions"));
    QLabel* const rotationAdvIcon  = new QLabel;
    rotationAdvIcon->setPixmap(QIcon::fromTheme(QLatin1String("configure")).pixmap(32));

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

    d->tab->insertTab(Rotation, rotationPanel, i18nc("@title:tab", "Rotation"));

    // --------------------------------------------------------

    QWidget* const displayPanel      = new QWidget;
    QGridLayout* const displayLayout = new QGridLayout;

    QLabel* const displayLabel       = new QLabel(i18nc("@info:label", "Select Metadata Fields to Be Displayed"));

    QLabel* const displayIcon        = new QLabel;
    displayIcon->setPixmap(QIcon::fromTheme(QLatin1String("view-list-tree")).pixmap(32));

    d->displaySubTab                 = new QTabWidget;
    d->tagsCfgPanel                  = new MetadataPanel(d->displaySubTab);

    displayLayout->addWidget(displayIcon,      0, 0);
    displayLayout->addWidget(displayLabel,     0, 1);
    displayLayout->addWidget(d->displaySubTab, 1, 0, 1, 3);
    displayLayout->setColumnStretch(2, 1);

    displayPanel->setLayout(displayLayout);
    d->tab->insertTab(Display, displayPanel, i18nc("@title:tab", "Views"));

    // --------------------------------------------------------

#ifdef HAVE_KFILEMETADATA

    QWidget* const balooPanel      = new QWidget(d->tab);
    QVBoxLayout* const balooLayout = new QVBoxLayout(balooPanel);

    QGroupBox* const balooGroup    = new QGroupBox(i18n("Baloo Desktop Search"), balooPanel);
    QVBoxLayout* const gLayout3    = new QVBoxLayout(balooGroup);

    d->saveToBalooBox           = new QCheckBox;
    d->saveToBalooBox->setText(i18n("Store metadata from digiKam in Baloo"));
    d->saveToBalooBox->setWhatsThis(i18n("Turn on this option to push rating, comments and tags "
                                           "from digiKam into the Baloo storage"));

    d->readFromBalooBox         = new QCheckBox;
    d->readFromBalooBox->setText(i18n("Read metadata from Baloo"));
    d->readFromBalooBox->setWhatsThis(i18n("Turn on this option if you want to apply changes to "
                                             "rating, comments and tags made in Baloo to digiKam's metadata storage. "
                                             "Please note that image metadata will not be edited automatically."));

    gLayout3->addWidget(d->saveToBalooBox);
    gLayout3->addWidget(d->readFromBalooBox);

    d->tab->insertTab(Baloo, balooPanel, i18nc("@title:tab", "Baloo"));

    // --------------------------------------------------------

    QFrame* const balooBox         = new QFrame(balooPanel);
    QGridLayout* const balooGrid   = new QGridLayout(balooBox);
    balooBox->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QLabel* const balooLogoLabel   = new QLabel;
    balooLogoLabel->setPixmap(QIcon::fromTheme(QLatin1String("baloo")).pixmap(48));

    QLabel* const balooExplanation = new QLabel(balooBox);
    balooExplanation->setOpenExternalLinks(true);
    balooExplanation->setWordWrap(true);
    QString balootxt;

    balootxt.append(i18n("<p><a href='http://community.kde.org/Baloo'>Baloo</a> "
                        "provides the basis to handle all kinds of metadata on the KDE desktop in a generic fashion. "
                        "It allows you to tag, rate and comment your files in KDE applications like Dolphin.</p> "
                        "<p>Please set here if you want to synchronize the metadata stored by digiKam desktop-wide with the "
                        "Baloo Desktop Search.</p> "));

    balooExplanation->setText(balootxt);

    balooGrid->addWidget(balooLogoLabel,   0, 0, 1, 1);
    balooGrid->addWidget(balooExplanation, 0, 1, 1, 2);
    balooGrid->setColumnStretch(1, 10);
    balooGrid->setRowStretch(1, 10);
    balooGrid->setSpacing(0);

    // --------------------------------------------------------

    balooLayout->addWidget(balooGroup);
    balooLayout->addWidget(balooBox);
    //balooLayout->addWidget(d->resyncButton, 0, Qt::AlignRight);
    balooLayout->addStretch();

#endif // HAVE_KFILEMETADATA

    //--------------Advanced Metadata Configuration --------------

    d->advTab = new AdvancedMetadataTab(this);
    d->tab->insertTab(AdvancedConfig, d->advTab, i18nc("@title:tab", "Advanced"));

    //------------------------Sidecars-------------------------

    QWidget* const sidecarsPanel      = new QWidget(d->tab);
    QVBoxLayout* const sidecarsLayout = new QVBoxLayout(sidecarsPanel);

    // --------------------------------------------------------

    QGroupBox* rwSidecarsGroup          = new QGroupBox;
    QGridLayout* const rwSidecarsLayout = new QGridLayout;

    QLabel* const rwSidecarsLabel       = new QLabel(i18nc("@label", "Reading and Writing to Sidecars"));

    d->readXMPSidecarBox  = new QCheckBox;
    d->readXMPSidecarBox->setText(i18nc("@option:check", "Read from sidecar files"));
    d->readXMPSidecarBox->setWhatsThis(i18nc("@info:whatsthis",
                                             "Turn on this option to read metadata from XMP sidecar files when reading metadata."));
    d->readXMPSidecarBox->setEnabled(MetaEngine::supportXmp());

    d->writeXMPSidecarBox = new QCheckBox;
    d->writeXMPSidecarBox->setText(i18nc("@option:check", "Write to sidecar files"));
    d->writeXMPSidecarBox->setWhatsThis(i18nc("@info:whatsthis",
                                              "Turn on this option to save, as specified, metadata to XMP sidecar files."));
    d->writeXMPSidecarBox->setEnabled(MetaEngine::supportXmp());

    d->writingModeCombo   = new QComboBox;
    d->writingModeCombo->addItem(i18n("Write to XMP sidecar for read-only image only"), MetaEngine::WRITETOSIDECARONLY4READONLYFILES);
    d->writingModeCombo->addItem(i18n("Write to XMP sidecar only"),                     MetaEngine::WRITETOSIDECARONLY);
    d->writingModeCombo->addItem(i18n("Write to image and XMP Sidecar"),                MetaEngine::WRITETOSIDECARANDIMAGE);
    d->writingModeCombo->setToolTip(i18nc("@info:tooltip", "Specify the exact mode of XMP sidecar writing"));
    d->writingModeCombo->setEnabled(false);

    connect(d->writeXMPSidecarBox, SIGNAL(toggled(bool)),
            d->writingModeCombo, SLOT(setEnabled(bool)));

    rwSidecarsLayout->addWidget(rwSidecarsLabel,       0, 0, 1, 3);
    rwSidecarsLayout->addWidget(d->readXMPSidecarBox,  1, 0, 1, 3);
    rwSidecarsLayout->addWidget(d->writeXMPSidecarBox, 2, 0, 1, 3);
    rwSidecarsLayout->addWidget(d->writingModeCombo,   3, 1, 1, 2);
    rwSidecarsLayout->setColumnStretch(3, 10);
    rwSidecarsGroup->setLayout(rwSidecarsLayout);

    // --------------------------------------------------------

    QGroupBox* const extensionsGroup  = new QGroupBox(sidecarsPanel);
    QGridLayout* const extensionsGrid = new QGridLayout(extensionsGroup);

    QLabel* extensionsGroupLabel = new QLabel(
                i18n("<p>Add file types to be recognised as sidecars.</p>"
                     "<p>digiKam (optionally) writes metadata to *.xmp sidecar "
                     "files. Other programs might use different types, which "
                     "can be specified below. digiKam will neither display these "
                     "nor read from or write to them. But whenever a matching album "
                     "item (e.g. \"image.dng\" for \"image.dng.pp3\") is renamed, "
                     "moved, copied or deleted, the same operation will be done "
                     "on these sidecar files.</p>"
                     "<p>Multiple extensions must be separated by a semicolon "
                     "or a space.</p>"));
    extensionsGroupLabel->setWordWrap(true);

    QLabel* const extensionsLogo = new QLabel(extensionsGroup);
    extensionsLogo->setPixmap(QIcon::fromTheme(QLatin1String("text-x-texinfo")).pixmap(48));

    d->extensionsEdit = new QLineEdit(extensionsGroup);
    d->extensionsEdit->setWhatsThis(i18n("<p>Here you can add extra extensions "
                                         "of sidecars files to be processed alongside "
                                         "regular items. These files won't be visible, "
                                         "but regarded as an extension of the main file. "
                                         "Just write \"xyz abc\" to support files with "
                                         "the *.xyz and *.abc extensions. The internally "
                                         "used sidecars type *.xmp is always included.</p>"));
    d->extensionsEdit->setClearButtonEnabled(true);
    d->extensionsEdit->setPlaceholderText(i18n("Enter additional sidecars file extensions."));

    QLabel* const extensionsLabel = new QLabel(extensionsGroup);
    extensionsLabel->setText(i18n("Additional &sidecar file extensions"));
    extensionsLabel->setBuddy(d->extensionsEdit);

    extensionsGrid->addWidget(extensionsGroupLabel, 0, 0, 1, -1);
    extensionsGrid->addWidget(extensionsLogo,       1, 0, 2, 1);
    extensionsGrid->addWidget(extensionsLabel,      1, 1, 1, -1);
    extensionsGrid->addWidget(d->extensionsEdit,    2, 1, 1, -1);
    extensionsGrid->setColumnStretch(1, 10);

    // --------------------------------------------------------

    sidecarsLayout->addWidget(rwSidecarsGroup);
    sidecarsLayout->addWidget(extensionsGroup);
    sidecarsLayout->addStretch();

    d->tab->insertTab(Sidecars, sidecarsPanel, i18nc("@title:tab", "Sidecars"));

    // --------------------------------------------------------

    readSettings();

    connect(d->exifRotateBox, SIGNAL(toggled(bool)),
            this, SLOT(slotExifAutoRotateToggled(bool)));
}

SetupMetadata::~SetupMetadata()
{
    delete d;
}

void SetupMetadata::setActiveMainTab(MetadataTab tab)
{
    d->tab->setCurrentIndex(tab);
}

void SetupMetadata::setActiveSubTab(int tab)
{
    d->displaySubTab->setCurrentIndex(tab);
}

void SetupMetadata::applySettings()
{
    MetadataSettings* const mSettings = MetadataSettings::instance();

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
    set.saveFaceTags          = d->saveFaceTags->isChecked();

    set.useLazySync           = d->useLazySync->isChecked();
    set.writeRawFiles         = d->writeRawFilesBox->isChecked();
    set.useXMPSidecar4Reading = d->readXMPSidecarBox->isChecked();

    if (d->writeXMPSidecarBox->isChecked())
    {
        set.metadataWritingMode = (MetaEngine::MetadataWritingMode)
                                  d->writingModeCombo->itemData(d->writingModeCombo->currentIndex()).toInt();
    }
    else
    {
        set.metadataWritingMode = MetaEngine::WRITETOIMAGEONLY;
    }

    set.updateFileTimeStamp   = d->updateFileTimeStampBox->isChecked();
    set.rescanImageIfModified = d->rescanImageIfModifiedBox->isChecked();

    set.sidecarExtensions = cleanUserFilterString(d->extensionsEdit->text());
    set.sidecarExtensions.removeAll(QLatin1String("xmp"));
    set.sidecarExtensions.removeDuplicates();

    mSettings->setSettings(set);


#ifdef HAVE_KFILEMETADATA

    ApplicationSettings* const aSettings = ApplicationSettings::instance();

    if (!aSettings)
    {
        return;
    }

    aSettings->setSyncDigikamToBaloo(d->saveToBalooBox->isChecked());
    aSettings->setSyncBalooToDigikam(d->readFromBalooBox->isChecked());

    aSettings->saveSettings();

#endif // HAVE_KFILEMETADATA

    d->tagsCfgPanel->applySettings();

    d->advTab->applySettings();
}

void SetupMetadata::readSettings()
{
    MetadataSettings* const mSettings = MetadataSettings::instance();

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

    d->exifAutoRotateOriginal = set.exifRotate;
    d->exifRotateBox->setChecked(d->exifAutoRotateOriginal);
    d->exifSetOrientationBox->setChecked(set.exifSetOrientation);

    d->saveTagsBox->setChecked(set.saveTags);
    d->saveCommentsBox->setChecked(set.saveComments);
    d->saveRatingBox->setChecked(set.saveRating);
    d->savePickLabelBox->setChecked(set.savePickLabel);
    d->saveColorLabelBox->setChecked(set.saveColorLabel);
    d->saveDateTimeBox->setChecked(set.saveDateTime);
    d->saveTemplateBox->setChecked(set.saveTemplate);
    d->saveFaceTags->setChecked(set.saveFaceTags);

    d->useLazySync->setChecked(set.useLazySync);
    d->writeRawFilesBox->setChecked(set.writeRawFiles);
    d->readXMPSidecarBox->setChecked(set.useXMPSidecar4Reading);
    d->updateFileTimeStampBox->setChecked(set.updateFileTimeStamp);
    d->rescanImageIfModifiedBox->setChecked(set.rescanImageIfModified);

    if (set.metadataWritingMode == MetaEngine::WRITETOIMAGEONLY)
    {
        d->writeXMPSidecarBox->setChecked(false);
    }
    else
    {
        d->writeXMPSidecarBox->setChecked(true);
        d->writingModeCombo->setCurrentIndex(d->writingModeCombo->findData(set.metadataWritingMode));
    }

    d->extensionsEdit->setText(set.sidecarExtensions.join(QLatin1Char(' ')));

#ifdef HAVE_KFILEMETADATA

    ApplicationSettings* const aSettings = ApplicationSettings::instance();

    if (!aSettings)
    {
        return;
    }

    d->saveToBalooBox->setChecked(aSettings->getSyncDigikamToBaloo());
    d->readFromBalooBox->setChecked(aSettings->getSyncBalooToDigikam());

#endif // HAVE_KFILEMETADATA

}

bool SetupMetadata::exifAutoRotateHasChanged() const
{
    return d->exifAutoRotateOriginal != d->exifRotateBox->isChecked();
}

void SetupMetadata::slotExifAutoRotateToggled(bool b)
{
    // Show info if rotation was switched off, and only once.
    if (!b && d->exifAutoRotateShowedInfo && exifAutoRotateHasChanged())
    {
        d->exifAutoRotateShowedInfo = true;
        QMessageBox::information(this, qApp->applicationName(),
                                 i18nc("@info",
                                 "Switching off exif auto rotation will most probably show your images in a wrong orientation, "
                                 "so only change this option if you explicitly require this. "
                                 "Furthermore, you need to regenerate all already stored thumbnails via "
                                 "the <interface>Tools / Maintenance</interface> menu."));
    }
}

}  // namespace Digikam
