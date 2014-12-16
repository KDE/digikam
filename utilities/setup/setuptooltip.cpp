/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-07-09
 * Description : item tool tip configuration setup tab
 *
 * Copyright (C) 2006-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setuptooltip.moc"

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

// KDE includes

#include <kdialog.h>
#include <klocale.h>
#include <ktabwidget.h>
#include <kvbox.h>
#include <kseparator.h>

// Local includes

#include "applicationsettings.h"
#include "importsettings.h"
#include "setupcamera.h"
#include "dfontselect.h"

namespace Digikam
{

class SetupToolTip::Private
{
public:

    Private() :
        cameraUseFileMetadata(false),
        showToolTipsBox(0),
        showFileNameBox(0),
        showFileDateBox(0),
        showFileSizeBox(0),
        showImageTypeBox(0),
        showImageDimBox(0),
        showImageARBox(0),
        showPhotoMakeBox(0),
        showPhotoDateBox(0),
        showPhotoFocalBox(0),
        showPhotoExpoBox(0),
        showPhotoModeBox(0),
        showPhotoFlashBox(0),
        showPhotoWbBox(0),
        showVideoAspectRatio(0),
        showVideoAudioBitRate(0),
        showVideoAudioChannelType(0),
        showVideoAudioCompressor(0),
        showVideoDuration(0),
        showVideoFrameRate(0),
        showVideoVideoCodec(0),
        showAlbumNameBox(0),
        showTitlesBox(0),
        showCommentsBox(0),
        showTagsBox(0),
        showLabelsBox(0),
        showAlbumToolTipsBox(0),
        showAlbumTitleBox(0),
        showAlbumDateBox(0),
        showAlbumCollectionBox(0),
        showAlbumCategoryBox(0),
        showAlbumCaptionBox(0),
        showAlbumPreviewBox(0),
        showImportToolTipsBox(0),
        showItemTitleBox(0),
        showItemDateBox(0),
        showItemSizeBox(0),
        showItemTypeBox(0),
        showItemDimensionsBox(0),
        showItemPhotoMakeBox(0),
        showItemPhotoFocalBox(0),
        showItemPhotoExpoBox(0),
        showItemPhotoFlashBox(0),
        showItemPhotoWBBox(0),
        fileSettingBox(0),
        photoSettingBox(0),
        videoSettingBox(0),
        digikamSettingBox(0),
        albumSettingBox(0),
        importSettingBox(0),
        tab(0),
        fontSelect(0)
    {
    }

    bool         cameraUseFileMetadata;

    QCheckBox*   showToolTipsBox;

    QCheckBox*   showFileNameBox;
    QCheckBox*   showFileDateBox;
    QCheckBox*   showFileSizeBox;
    QCheckBox*   showImageTypeBox;
    QCheckBox*   showImageDimBox;
    QCheckBox*   showImageARBox;

    QCheckBox*   showPhotoMakeBox;
    QCheckBox*   showPhotoDateBox;
    QCheckBox*   showPhotoFocalBox;
    QCheckBox*   showPhotoExpoBox;
    QCheckBox*   showPhotoModeBox;
    QCheckBox*   showPhotoFlashBox;
    QCheckBox*   showPhotoWbBox;

    QCheckBox*   showVideoAspectRatio;
    QCheckBox*   showVideoAudioBitRate;
    QCheckBox*   showVideoAudioChannelType;
    QCheckBox*   showVideoAudioCompressor;
    QCheckBox*   showVideoDuration;
    QCheckBox*   showVideoFrameRate;
    QCheckBox*   showVideoVideoCodec;

    QCheckBox*   showAlbumNameBox;
    QCheckBox*   showTitlesBox;
    QCheckBox*   showCommentsBox;
    QCheckBox*   showTagsBox;
    QCheckBox*   showLabelsBox;

    QCheckBox*   showAlbumToolTipsBox;
    QCheckBox*   showAlbumTitleBox;
    QCheckBox*   showAlbumDateBox;
    QCheckBox*   showAlbumCollectionBox;
    QCheckBox*   showAlbumCategoryBox;
    QCheckBox*   showAlbumCaptionBox;
    QCheckBox*   showAlbumPreviewBox;

    QCheckBox*   showImportToolTipsBox;
    QCheckBox*   showItemTitleBox;
    QCheckBox*   showItemDateBox;
    QCheckBox*   showItemSizeBox;
    QCheckBox*   showItemTypeBox;
    QCheckBox*   showItemDimensionsBox;
    QCheckBox*   showItemPhotoMakeBox;
    QCheckBox*   showItemPhotoFocalBox;
    QCheckBox*   showItemPhotoExpoBox;
    QCheckBox*   showItemPhotoFlashBox;
    QCheckBox*   showItemPhotoWBBox;

    QGroupBox*   fileSettingBox;
    QGroupBox*   photoSettingBox;
    QGroupBox*   videoSettingBox;
    QGroupBox*   digikamSettingBox;
    QGroupBox*   albumSettingBox;
    QGroupBox*   importSettingBox;

    KTabWidget*  tab;

    DFontSelect* fontSelect;
};

SetupToolTip::SetupToolTip(QWidget* const parent)
    : QScrollArea(parent), d(new Private)
{
    QWidget* const panel    = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* const vlay = new QVBoxLayout(panel);

    d->fontSelect           = new DFontSelect(i18n("Tool-Tips Font:"), panel);
    d->fontSelect->setToolTip(i18n("Select here the font used to display text in tool-tips."));

    d->tab                  = new KTabWidget(panel);

    // --------------------------------------------------------

    KVBox* const vbox       = new KVBox(panel);

    d->showToolTipsBox      = new QCheckBox(i18n("Show icon-view and thumb-bar items' tool-tips"), vbox);
    d->showToolTipsBox->setWhatsThis(i18n("Set this option to display image information when "
                                          "the mouse hovers over an icon-view or thumb-bar item."));

    d->fileSettingBox        = new QGroupBox(i18n("File/Item Information"), vbox);
    QVBoxLayout* const vlay2 = new QVBoxLayout(d->fileSettingBox);

    d->showFileNameBox    = new QCheckBox(i18n("Show file name"), d->fileSettingBox);
    d->showFileNameBox->setWhatsThis(i18n("Set this option to display the image file name."));

    d->showFileDateBox    = new QCheckBox(i18n("Show file date"), d->fileSettingBox);
    d->showFileDateBox->setWhatsThis(i18n("Set this option to display the image file date."));

    d->showFileSizeBox    = new QCheckBox(i18n("Show file size"), d->fileSettingBox);
    d->showFileSizeBox->setWhatsThis(i18n("Set this option to display the image file size."));

    d->showImageTypeBox   = new QCheckBox(i18n("Show image type"), d->fileSettingBox);
    d->showImageTypeBox->setWhatsThis(i18n("Set this option to display the image type."));

    d->showImageDimBox    = new QCheckBox(i18n("Show image dimensions"), d->fileSettingBox);
    d->showImageDimBox->setWhatsThis(i18n("Set this option to display the image dimensions in pixels."));

    d->showImageARBox     = new QCheckBox(i18n("Show image aspect ratio"), d->fileSettingBox);
    d->showImageARBox->setWhatsThis(i18n("Set this option to display the image aspect ratio."));

    vlay2->addWidget(d->showFileNameBox);
    vlay2->addWidget(d->showFileDateBox);
    vlay2->addWidget(d->showFileSizeBox);
    vlay2->addWidget(d->showImageTypeBox);
    vlay2->addWidget(d->showImageDimBox);
    vlay2->addWidget(d->showImageARBox);

    vlay2->setMargin(KDialog::spacingHint());
    vlay2->setSpacing(0);

    // --------------------------------------------------------

    d->photoSettingBox       = new QGroupBox(i18n("Photograph Information"), vbox);
    QVBoxLayout* const vlay3 = new QVBoxLayout(d->photoSettingBox);

    d->showPhotoMakeBox  = new QCheckBox(i18n("Show camera make and model"), d->photoSettingBox);
    d->showPhotoMakeBox->setWhatsThis(i18n("Set this option to display the make and model of the "
                                           "camera with which the image has been taken."));

    d->showPhotoDateBox  = new QCheckBox(i18n("Show camera date"), d->photoSettingBox);
    d->showPhotoDateBox->setWhatsThis(i18n("Set this option to display the date when the image was taken."));

    d->showPhotoFocalBox = new QCheckBox(i18n("Show camera aperture and focal length"), d->photoSettingBox);
    d->showPhotoFocalBox->setWhatsThis(i18n("Set this option to display the camera aperture and focal settings "
                                            "used to take the image."));

    d->showPhotoExpoBox  = new QCheckBox(i18n("Show camera exposure and sensitivity"), d->photoSettingBox);
    d->showPhotoExpoBox->setWhatsThis(i18n("Set this option to display the camera exposure and sensitivity "
                                           "used to take the image."));

    d->showPhotoModeBox  = new QCheckBox(i18n("Show camera mode and program"), d->photoSettingBox);
    d->showPhotoModeBox->setWhatsThis(i18n("Set this option to display the camera mode and program "
                                           "used to take the image."));

    d->showPhotoFlashBox = new QCheckBox(i18n("Show camera flash settings"), d->photoSettingBox);
    d->showPhotoFlashBox->setWhatsThis(i18n("Set this option to display the camera flash settings "
                                            "used to take the image."));

    d->showPhotoWbBox    = new QCheckBox(i18n("Show camera white balance settings"), d->photoSettingBox);
    d->showPhotoWbBox->setWhatsThis(i18n("Set this option to display the camera white balance settings "
                                         "used to take the image."));

    vlay3->addWidget(d->showPhotoMakeBox);
    vlay3->addWidget(d->showPhotoDateBox);
    vlay3->addWidget(d->showPhotoFocalBox);
    vlay3->addWidget(d->showPhotoExpoBox);
    vlay3->addWidget(d->showPhotoModeBox);
    vlay3->addWidget(d->showPhotoFlashBox);
    vlay3->addWidget(d->showPhotoWbBox);
    vlay3->setMargin(KDialog::spacingHint());
    vlay3->setSpacing(0);

    // --------------------------------------------------------

    d->digikamSettingBox     = new QGroupBox(i18n("digiKam Information"), vbox);
    QVBoxLayout* const vlay4 = new QVBoxLayout(d->digikamSettingBox);

    d->showAlbumNameBox  = new QCheckBox(i18n("Show album name"), d->digikamSettingBox);
    d->showAlbumNameBox->setWhatsThis(i18n("Set this option to display the album name."));

    d->showTitlesBox     = new QCheckBox(i18n("Show image title"), d->digikamSettingBox);
    d->showTitlesBox->setWhatsThis(i18n("Set this option to display the image title."));

    d->showCommentsBox   = new QCheckBox(i18n("Show image caption"), d->digikamSettingBox);
    d->showCommentsBox->setWhatsThis(i18n("Set this option to display the image captions."));

    d->showTagsBox       = new QCheckBox(i18n("Show image tags"), d->digikamSettingBox);
    d->showTagsBox->setWhatsThis(i18n("Set this option to display the image tags."));

    d->showLabelsBox     = new QCheckBox(i18n("Show image labels"), d->digikamSettingBox);
    d->showLabelsBox->setWhatsThis(i18n("Set this option to display the image pick, color, rating labels."));

    vlay4->addWidget(d->showAlbumNameBox);
    vlay4->addWidget(d->showTitlesBox);
    vlay4->addWidget(d->showCommentsBox);
    vlay4->addWidget(d->showTagsBox);
    vlay4->addWidget(d->showLabelsBox);
    vlay4->setMargin(KDialog::spacingHint());
    vlay4->setSpacing(0);

    // --------------------------------------------------------

    d->videoSettingBox             = new QGroupBox(i18n("Video Information"), vbox);
    QVBoxLayout* const vlay5       = new QVBoxLayout(d->videoSettingBox);

    d->showVideoAspectRatio        = new QCheckBox(i18n("Show Video Aspect Ratio"), d->videoSettingBox);
    d->showVideoAspectRatio->setWhatsThis(i18n("Set this option to display the Aspect Ratio of the Video"));

    d->showVideoAudioBitRate       = new QCheckBox(i18n("Show Audio Bit Rate"), d->videoSettingBox);
    d->showVideoAudioBitRate->setWhatsThis(i18n("Set this option to display the Audio Bit Rate of the Video"));

    d->showVideoAudioChannelType   = new QCheckBox(i18n("Show Audio Channel Type"), d->videoSettingBox);
    d->showVideoAudioChannelType->setWhatsThis(i18n("Set this option to display the Audio Channel Type of the Video"));

    d->showVideoAudioCompressor    = new QCheckBox(i18n("Show Audio Compressor"), d->videoSettingBox);
    d->showVideoAudioCompressor->setWhatsThis(i18n("Set this option to display the Audio Compressor of the Video"));

    d->showVideoDuration           = new QCheckBox(i18n("Show Video Duration"), d->videoSettingBox);
    d->showVideoDuration->setWhatsThis(i18n("Set this option to display the Duration of the Video"));

    d->showVideoFrameRate          = new QCheckBox(i18n("Show Video Frame Rate"), d->videoSettingBox);
    d->showVideoFrameRate->setWhatsThis(i18n("Set this option to display the Aspect Ratio of the Video"));

    d->showVideoVideoCodec         = new QCheckBox(i18n("Show Video Codec"), d->videoSettingBox);
    d->showVideoVideoCodec->setWhatsThis(i18n("Set this option to display the Codec of the Video"));

    vlay5->addWidget(d->showVideoAspectRatio);
    vlay5->addWidget(d->showVideoAudioBitRate);
    vlay5->addWidget(d->showVideoAudioChannelType);
    vlay5->addWidget(d->showVideoAudioCompressor);
    vlay5->addWidget(d->showVideoDuration);
    vlay5->addWidget(d->showVideoFrameRate);
    vlay5->addWidget(d->showVideoVideoCodec);
    vlay5->setMargin(KDialog::spacingHint());
    vlay5->setSpacing(0);

    QWidget* const space = new QWidget(vbox);
    vbox->setStretchFactor(space, 10);
    vbox->setMargin(KDialog::spacingHint());
    vbox->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    KVBox* const vbox2        = new KVBox(panel);
    d->showAlbumToolTipsBox   = new QCheckBox(i18n("Show album items' tool-tips"), vbox2);
    d->albumSettingBox        = new QGroupBox(i18n("Album Information"), vbox2);

    d->showAlbumToolTipsBox->setWhatsThis(i18n("Set this option to display album information when "
                                               "the mouse hovers over a folder-view item."));

    d->showAlbumTitleBox      = new QCheckBox(i18n("Show album name"));
    d->showAlbumTitleBox->setWhatsThis(i18n("Set this option to display the album name."));

    d->showAlbumDateBox       = new QCheckBox(i18n("Show album date"));
    d->showAlbumDateBox->setWhatsThis(i18n("Set this option to display the album date."));

    d->showAlbumCollectionBox = new QCheckBox(i18n("Show album collection"));
    d->showAlbumCollectionBox->setWhatsThis(i18n("Set this option to display the album collection."));

    d->showAlbumCategoryBox   = new QCheckBox(i18n("Show album category"));
    d->showAlbumCategoryBox->setWhatsThis(i18n("Set this option to display the album category."));

    d->showAlbumCaptionBox    = new QCheckBox(i18n("Show album caption"));
    d->showAlbumCaptionBox->setWhatsThis(i18n("Set this option to display the album caption."));

    d->showAlbumPreviewBox    = new QCheckBox(i18n("Show album preview"));
    d->showAlbumPreviewBox->setWhatsThis(i18n("Set this option to display the album preview."));

    QVBoxLayout* const albumSettingBoxLayout = new QVBoxLayout;
    albumSettingBoxLayout->addWidget(d->showAlbumTitleBox);
    albumSettingBoxLayout->addWidget(d->showAlbumDateBox);
    albumSettingBoxLayout->addWidget(d->showAlbumCollectionBox);
    albumSettingBoxLayout->addWidget(d->showAlbumCategoryBox);
    albumSettingBoxLayout->addWidget(d->showAlbumCaptionBox);
    albumSettingBoxLayout->addWidget(d->showAlbumPreviewBox);
    d->albumSettingBox->setLayout(albumSettingBoxLayout);

    QWidget* const space2 = new QWidget(vbox2);
    vbox2->setStretchFactor(space2, 10);
    vbox2->setMargin(KDialog::spacingHint());
    vbox2->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    KVBox* const vbox3       = new KVBox(panel);
    d->showImportToolTipsBox = new QCheckBox(i18n("Show import items' tool-tips"), vbox3);
    d->importSettingBox      = new QGroupBox(i18n("Import Information"), vbox3);

    d->showAlbumToolTipsBox->setWhatsThis(i18n("Set this option to display album information when "
                                               "the mouse hovers over a folder-view item."));

    d->showItemTitleBox      = new QCheckBox(i18n("Show item name"));
    d->showItemTitleBox->setWhatsThis(i18n("Set this option to display the item name."));

    d->showItemDateBox       = new QCheckBox(i18n("Show item date"));
    d->showItemDateBox->setWhatsThis(i18n("Set this option to display the item date."));

    d->showItemSizeBox       = new QCheckBox(i18n("Show item size"));
    d->showItemSizeBox->setWhatsThis(i18n("Set this option to display the item size."));

    d->showItemTypeBox       = new QCheckBox(i18n("Show item type"));
    d->showItemTypeBox->setWhatsThis(i18n("Set this option to display the item type."));

    d->showItemDimensionsBox = new QCheckBox(i18n("Show item dimensions"));
    d->showItemDimensionsBox->setWhatsThis(i18n("Set this option to display the item dimensions."));

    KSeparator* const line   = new KSeparator(Qt::Horizontal, d->photoSettingBox);
    QLabel* const label      = new QLabel(i18n("Note: these settings require \"Use File Metadata\" option from Camera Setup Behavior page."), d->photoSettingBox);

    d->showItemPhotoMakeBox  = new QCheckBox(i18n("Show camera make and model"), d->photoSettingBox);
    d->showItemPhotoMakeBox->setWhatsThis(i18n("Set this option to display the make and model of the "
                                               "camera with which the image has been taken."));

    d->showItemPhotoFocalBox = new QCheckBox(i18n("Show camera aperture and focal length"), d->photoSettingBox);
    d->showPhotoFocalBox->setWhatsThis(i18n("Set this option to display the camera aperture and focal settings "
                                            "used to take the image."));

    d->showItemPhotoExpoBox  = new QCheckBox(i18n("Show camera exposure and sensitivity"), d->photoSettingBox);
    d->showPhotoExpoBox->setWhatsThis(i18n("Set this option to display the camera exposure and sensitivity "
                                           "used to take the image."));

    d->showItemPhotoFlashBox = new QCheckBox(i18n("Show camera flash settings"), d->photoSettingBox);
    d->showPhotoFlashBox->setWhatsThis(i18n("Set this option to display the camera flash settings "
                                            "used to take the image."));

    d->showItemPhotoWBBox    = new QCheckBox(i18n("Show camera white balance settings"), d->photoSettingBox);
    d->showItemPhotoWBBox->setWhatsThis(i18n("Set this option to display the camera white balance settings "
                                             "used to take the image."));

    QVBoxLayout* const importSettingBoxLayout = new QVBoxLayout;
    importSettingBoxLayout->addWidget(d->showItemTitleBox);
    importSettingBoxLayout->addWidget(d->showItemDateBox);
    importSettingBoxLayout->addWidget(d->showItemSizeBox);
    importSettingBoxLayout->addWidget(d->showItemTypeBox);
    importSettingBoxLayout->addWidget(d->showItemDimensionsBox);
    importSettingBoxLayout->addWidget(line);
    importSettingBoxLayout->addWidget(label);
    importSettingBoxLayout->addWidget(d->showItemPhotoMakeBox);
    importSettingBoxLayout->addWidget(d->showItemPhotoFocalBox);
    importSettingBoxLayout->addWidget(d->showItemPhotoExpoBox);
    importSettingBoxLayout->addWidget(d->showItemPhotoFlashBox);
    importSettingBoxLayout->addWidget(d->showItemPhotoWBBox);
    d->importSettingBox->setLayout(importSettingBoxLayout);

    QWidget* const space3 = new QWidget(vbox3);
    vbox3->setStretchFactor(space3, 10);
    vbox3->setMargin(KDialog::spacingHint());
    vbox3->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    d->tab->insertTab(0, vbox,  i18n("Icon Items"));
    d->tab->insertTab(1, vbox2, i18n("Album Items"));
    d->tab->insertTab(2, vbox3, i18n("Import Items"));

    vlay->addWidget(d->fontSelect);
    vlay->addWidget(d->tab);
    vlay->addStretch();
    vlay->setMargin(KDialog::spacingHint());
    vlay->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->fileSettingBox, SLOT(setEnabled(bool)));

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->photoSettingBox, SLOT(setEnabled(bool)));

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->videoSettingBox, SLOT(setEnabled(bool)));

    connect(d->showToolTipsBox, SIGNAL(toggled(bool)),
            d->digikamSettingBox, SLOT(setEnabled(bool)));

    connect(d->showAlbumToolTipsBox, SIGNAL(toggled(bool)),
            d->albumSettingBox, SLOT(setEnabled(bool)));

    connect(d->showImportToolTipsBox, SIGNAL(toggled(bool)),
            this, SLOT(slotImportToolTipsChanged()));    

    // --------------------------------------------------------

    readSettings();
    adjustSize();

    // --------------------------------------------------------
}

SetupToolTip::~SetupToolTip()
{
    delete d;
}

void SetupToolTip::applySettings()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setToolTipsFont(d->fontSelect->font());

    settings->setShowToolTips(d->showToolTipsBox->isChecked());
    settings->setToolTipsShowFileName(d->showFileNameBox->isChecked());
    settings->setToolTipsShowFileDate(d->showFileDateBox->isChecked());
    settings->setToolTipsShowFileSize(d->showFileSizeBox->isChecked());
    settings->setToolTipsShowImageAR(d->showImageARBox->isChecked());
    settings->setToolTipsShowImageType(d->showImageTypeBox->isChecked());
    settings->setToolTipsShowImageDim(d->showImageDimBox->isChecked());

    settings->setToolTipsShowPhotoMake(d->showPhotoMakeBox->isChecked());
    settings->setToolTipsShowPhotoDate(d->showPhotoDateBox->isChecked());
    settings->setToolTipsShowPhotoFocal(d->showPhotoFocalBox->isChecked());
    settings->setToolTipsShowPhotoExpo(d->showPhotoExpoBox->isChecked());
    settings->setToolTipsShowPhotoMode(d->showPhotoModeBox->isChecked());
    settings->setToolTipsShowPhotoFlash(d->showPhotoFlashBox->isChecked());
    settings->setToolTipsShowPhotoWB(d->showPhotoWbBox->isChecked());

    settings->setToolTipsShowVideoAspectRatio(d->showVideoAspectRatio->isChecked());
    settings->setToolTipsShowVideoAudioBitRate(d->showVideoAudioBitRate->isChecked());
    settings->setToolTipsShowVideoAudioChannelType(d->showVideoAudioChannelType->isChecked());
    settings->setToolTipsShowVideoAudioCompressor(d->showVideoAudioCompressor->isChecked());
    settings->setToolTipsShowVideoDuration(d->showVideoDuration->isChecked());
    settings->setToolTipsShowVideoFrameRate(d->showVideoFrameRate->isChecked());
    settings->setToolTipsShowVideoVideoCodec(d->showVideoVideoCodec->isChecked());

    settings->setToolTipsShowAlbumName(d->showAlbumNameBox->isChecked());
    settings->setToolTipsShowTitles(d->showTitlesBox->isChecked());
    settings->setToolTipsShowComments(d->showCommentsBox->isChecked());
    settings->setToolTipsShowTags(d->showTagsBox->isChecked());
    settings->setToolTipsShowLabelRating(d->showLabelsBox->isChecked());

    settings->setShowAlbumToolTips(d->showAlbumToolTipsBox->isChecked());
    settings->setToolTipsShowAlbumTitle(d->showAlbumTitleBox->isChecked());
    settings->setToolTipsShowAlbumDate(d->showAlbumDateBox->isChecked());
    settings->setToolTipsShowAlbumCollection(d->showAlbumCollectionBox->isChecked());
    settings->setToolTipsShowAlbumCategory(d->showAlbumCategoryBox->isChecked());
    settings->setToolTipsShowAlbumCaption(d->showAlbumCaptionBox->isChecked());
    settings->setToolTipsShowAlbumPreview(d->showAlbumPreviewBox->isChecked());

    settings->saveSettings();

    // -- Import Settings ------------------------------------------------------------------------

    ImportSettings* const importSettings = ImportSettings::instance();

    if (!importSettings)
    {
        return;
    }

    importSettings->setShowToolTips(d->showImportToolTipsBox->isChecked());
    importSettings->setToolTipsShowFileName(d->showItemTitleBox->isChecked());
    importSettings->setToolTipsShowFileDate(d->showItemDateBox->isChecked());
    importSettings->setToolTipsShowFileSize(d->showItemSizeBox->isChecked());
    importSettings->setToolTipsShowImageType(d->showItemTypeBox->isChecked());
    importSettings->setToolTipsShowImageDim(d->showItemDimensionsBox->isChecked());
    importSettings->setToolTipsShowPhotoMake(d->showItemPhotoMakeBox->isChecked());
    importSettings->setToolTipsShowPhotoFocal(d->showItemPhotoFocalBox->isChecked());
    importSettings->setToolTipsShowPhotoExpo(d->showItemPhotoExpoBox->isChecked());
    importSettings->setToolTipsShowPhotoFlash(d->showItemPhotoFlashBox->isChecked());
    importSettings->setToolTipsShowPhotoWB(d->showItemPhotoWBBox->isChecked());

    importSettings->saveSettings();
}

void SetupToolTip::readSettings()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    if (!settings)
    {
        return;
    }

    d->fontSelect->setFont(settings->getToolTipsFont());

    d->showToolTipsBox->setChecked(settings->getShowToolTips());
    d->showFileNameBox->setChecked(settings->getToolTipsShowFileName());
    d->showFileDateBox->setChecked(settings->getToolTipsShowFileDate());
    d->showFileSizeBox->setChecked(settings->getToolTipsShowFileSize());
    d->showImageARBox->setChecked(settings->getToolTipsShowImageAR());
    d->showImageTypeBox->setChecked(settings->getToolTipsShowImageType());
    d->showImageDimBox->setChecked(settings->getToolTipsShowImageDim());

    d->showPhotoMakeBox->setChecked(settings->getToolTipsShowPhotoMake());
    d->showPhotoDateBox->setChecked(settings->getToolTipsShowPhotoDate());
    d->showPhotoFocalBox->setChecked(settings->getToolTipsShowPhotoFocal());
    d->showPhotoExpoBox->setChecked(settings->getToolTipsShowPhotoExpo());
    d->showPhotoModeBox->setChecked(settings->getToolTipsShowPhotoMode());
    d->showPhotoFlashBox->setChecked(settings->getToolTipsShowPhotoFlash());
    d->showPhotoWbBox->setChecked(settings->getToolTipsShowPhotoWB());

    d->showVideoAspectRatio->setChecked(settings->getToolTipsShowVideoAspectRatio());
    d->showVideoAudioBitRate->setChecked(settings->getToolTipsShowVideoAudioBitRate());
    d->showVideoAudioChannelType->setChecked(settings->getToolTipsShowVideoAudioChannelType());
    d->showVideoAudioCompressor->setChecked(settings->getToolTipsShowVideoAudioCompressor());
    d->showVideoDuration->setChecked(settings->getToolTipsShowVideoDuration());
    d->showVideoFrameRate->setChecked(settings->getToolTipsShowVideoFrameRate());
    d->showVideoVideoCodec->setChecked(settings->getToolTipsShowVideoVideoCodec());

    d->showAlbumNameBox->setChecked(settings->getToolTipsShowAlbumName());
    d->showTitlesBox->setChecked(settings->getToolTipsShowTitles());
    d->showCommentsBox->setChecked(settings->getToolTipsShowComments());
    d->showTagsBox->setChecked(settings->getToolTipsShowTags());
    d->showLabelsBox->setChecked(settings->getToolTipsShowLabelRating());

    d->fileSettingBox->setEnabled(d->showToolTipsBox->isChecked());
    d->photoSettingBox->setEnabled(d->showToolTipsBox->isChecked());
    d->digikamSettingBox->setEnabled(d->showToolTipsBox->isChecked());
    d->videoSettingBox->setEnabled(d->showToolTipsBox->isChecked());

    d->albumSettingBox->setEnabled(d->showAlbumToolTipsBox->isChecked());

    d->showAlbumToolTipsBox->setChecked(settings->getShowAlbumToolTips());
    d->showAlbumTitleBox->setChecked(settings->getToolTipsShowAlbumTitle());
    d->showAlbumDateBox->setChecked(settings->getToolTipsShowAlbumDate());
    d->showAlbumCollectionBox->setChecked(settings->getToolTipsShowAlbumCollection());
    d->showAlbumCategoryBox->setChecked(settings->getToolTipsShowAlbumCategory());
    d->showAlbumCaptionBox->setChecked(settings->getToolTipsShowAlbumCaption());
    d->showAlbumPreviewBox->setChecked(settings->getToolTipsShowAlbumPreview());

    // -- Import Settings ------------------------------------------------------------------------

    ImportSettings* const importSettings = ImportSettings::instance();

    if (!importSettings)
    {
        return;
    }

    d->showImportToolTipsBox->setChecked(importSettings->getShowToolTips());
    d->showItemTitleBox->setChecked(importSettings->getToolTipsShowFileName());
    d->showItemDateBox->setChecked(importSettings->getToolTipsShowFileDate());
    d->showItemSizeBox->setChecked(importSettings->getToolTipsShowFileSize());
    d->showItemTypeBox->setChecked(importSettings->getToolTipsShowImageType());
    d->showItemDimensionsBox->setChecked(importSettings->getToolTipsShowImageDim());
    d->showItemPhotoMakeBox->setChecked(importSettings->getToolTipsShowPhotoMake());
    d->showItemPhotoFocalBox->setChecked(importSettings->getToolTipsShowPhotoFocal());
    d->showItemPhotoExpoBox->setChecked(importSettings->getToolTipsShowPhotoExpo());
    d->showItemPhotoFlashBox->setChecked(importSettings->getToolTipsShowPhotoFlash());
    d->showItemPhotoWBBox->setChecked(importSettings->getToolTipsShowPhotoWB());

    refreshCameraOptions();
}

void SetupToolTip::slotUseFileMetadataChanged(bool b)
{
    d->cameraUseFileMetadata = b;
    refreshCameraOptions();
}

void SetupToolTip::slotImportToolTipsChanged()
{
    refreshCameraOptions();
}

void SetupToolTip::refreshCameraOptions()
{
    bool b = d->showImportToolTipsBox->isChecked();
    d->importSettingBox->setEnabled(b);
    d->showItemPhotoMakeBox->setEnabled(b && d->cameraUseFileMetadata);
    d->showItemPhotoFocalBox->setEnabled(b && d->cameraUseFileMetadata);
    d->showItemPhotoExpoBox->setEnabled(b && d->cameraUseFileMetadata);
    d->showItemPhotoFlashBox->setEnabled(b && d->cameraUseFileMetadata);
    d->showItemPhotoWBBox->setEnabled(b && d->cameraUseFileMetadata);
}

}  // namespace Digikam
