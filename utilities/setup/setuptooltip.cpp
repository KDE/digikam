/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-07-09
 * Description : item tool tip configuration setup tab
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setuptooltip.h"

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "applicationsettings.h"
#include "importsettings.h"
#include "setupcamera.h"
#include "dfontselect.h"
#include "dexpanderbox.h"

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
        showPhotoLensBox(0),
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
        showItemPhotoLensBox(0),
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
    QCheckBox*   showPhotoLensBox;
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
    QCheckBox*   showItemPhotoLensBox;
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

    QTabWidget*  tab;

    DFontSelect* fontSelect;
};

SetupToolTip::SetupToolTip(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    QWidget* const panel    = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    const int spacing       = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QVBoxLayout* const vlay = new QVBoxLayout(panel);

    d->fontSelect           = new DFontSelect(i18n("Tool-Tips Font:"), panel);
    d->fontSelect->setToolTip(i18n("Select here the font used to display text in tool-tips."));

    d->tab                  = new QTabWidget(panel);

    // --------------------------------------------------------

    DVBox* const vbox        = new DVBox(panel);

    d->showToolTipsBox       = new QCheckBox(i18n("Show icon-view and thumb-bar items' tool-tips"), vbox);
    d->showToolTipsBox->setWhatsThis(i18n("Set this option to display image information when "
                                          "the mouse hovers over an icon-view or thumb-bar item."));

    d->fileSettingBox        = new QGroupBox(i18n("File/Item Information"), vbox);
    QGridLayout* const grid2 = new QGridLayout(d->fileSettingBox);

    d->showFileNameBox       = new QCheckBox(i18n("File name"), d->fileSettingBox);
    d->showFileNameBox->setWhatsThis(i18n("Set this option to display the image file name."));

    d->showFileDateBox       = new QCheckBox(i18n("File date"), d->fileSettingBox);
    d->showFileDateBox->setWhatsThis(i18n("Set this option to display the image file date."));

    d->showFileSizeBox       = new QCheckBox(i18n("File size"), d->fileSettingBox);
    d->showFileSizeBox->setWhatsThis(i18n("Set this option to display the image file size."));

    d->showImageTypeBox      = new QCheckBox(i18n("Image type"), d->fileSettingBox);
    d->showImageTypeBox->setWhatsThis(i18n("Set this option to display the image type."));

    d->showImageDimBox       = new QCheckBox(i18n("Image dimensions"), d->fileSettingBox);
    d->showImageDimBox->setWhatsThis(i18n("Set this option to display the image dimensions in pixels."));

    d->showImageARBox        = new QCheckBox(i18n("Image aspect ratio"), d->fileSettingBox);
    d->showImageARBox->setWhatsThis(i18n("Set this option to display the image aspect ratio."));

    grid2->addWidget(d->showFileNameBox,  0, 0, 1, 1);
    grid2->addWidget(d->showFileDateBox,  1, 0, 1, 1);
    grid2->addWidget(d->showFileSizeBox,  2, 0, 1, 1);
    grid2->addWidget(d->showImageTypeBox, 0, 1, 1, 1);
    grid2->addWidget(d->showImageDimBox,  1, 1, 1, 1);
    grid2->addWidget(d->showImageARBox,   2, 1, 1, 1);
    grid2->setContentsMargins(spacing, spacing, spacing, spacing);
    grid2->setSpacing(0);

    // --------------------------------------------------------

    d->photoSettingBox       = new QGroupBox(i18n("Photograph Information"), vbox);
    QGridLayout* const grid3 = new QGridLayout(d->photoSettingBox);

    d->showPhotoMakeBox      = new QCheckBox(i18n("Camera make and model"), d->photoSettingBox);
    d->showPhotoMakeBox->setWhatsThis(i18n("Set this option to display the make and model of the "
                                           "camera with which the image has been taken."));

    d->showPhotoLensBox      = new QCheckBox(i18n("Camera lens model"), d->photoSettingBox);
    d->showPhotoLensBox->setWhatsThis(i18n("Set this option to display the lens model with "
                                           "which the image was taken."));

    d->showPhotoDateBox      = new QCheckBox(i18n("Camera date"), d->photoSettingBox);
    d->showPhotoDateBox->setWhatsThis(i18n("Set this option to display the date when the image was taken."));

    d->showPhotoFocalBox     = new QCheckBox(i18n("Camera aperture and focal length"), d->photoSettingBox);
    d->showPhotoFocalBox->setWhatsThis(i18n("Set this option to display the camera aperture and focal settings "
                                            "used to take the image."));

    d->showPhotoExpoBox      = new QCheckBox(i18n("Camera exposure and sensitivity"), d->photoSettingBox);
    d->showPhotoExpoBox->setWhatsThis(i18n("Set this option to display the camera exposure and sensitivity "
                                           "used to take the image."));

    d->showPhotoModeBox      = new QCheckBox(i18n("Camera mode and program"), d->photoSettingBox);
    d->showPhotoModeBox->setWhatsThis(i18n("Set this option to display the camera mode and program "
                                           "used to take the image."));

    d->showPhotoFlashBox     = new QCheckBox(i18n("Camera flash settings"), d->photoSettingBox);
    d->showPhotoFlashBox->setWhatsThis(i18n("Set this option to display the camera flash settings "
                                            "used to take the image."));

    d->showPhotoWbBox        = new QCheckBox(i18n("Camera white balance settings"), d->photoSettingBox);
    d->showPhotoWbBox->setWhatsThis(i18n("Set this option to display the camera white balance settings "
                                         "used to take the image."));

    grid3->addWidget(d->showPhotoMakeBox,  0, 0, 1, 1);
    grid3->addWidget(d->showPhotoLensBox,  1, 0, 1, 1);
    grid3->addWidget(d->showPhotoDateBox,  2, 0, 1, 1);
    grid3->addWidget(d->showPhotoFocalBox, 3, 0, 1, 1);
    grid3->addWidget(d->showPhotoExpoBox,  0, 1, 1, 1);
    grid3->addWidget(d->showPhotoModeBox,  1, 1, 1, 1);
    grid3->addWidget(d->showPhotoFlashBox, 2, 1, 1, 1);
    grid3->addWidget(d->showPhotoWbBox,    3, 1, 1, 1);
    grid3->setContentsMargins(spacing, spacing, spacing, spacing);
    grid3->setSpacing(0);

    // --------------------------------------------------------

    d->digikamSettingBox     = new QGroupBox(i18n("digiKam Information"), vbox);
    QGridLayout* const grid4 = new QGridLayout(d->digikamSettingBox);

    d->showAlbumNameBox      = new QCheckBox(i18n("Album name"), d->digikamSettingBox);
    d->showAlbumNameBox->setWhatsThis(i18n("Set this option to display the album name."));

    d->showTitlesBox         = new QCheckBox(i18n("Image title"), d->digikamSettingBox);
    d->showTitlesBox->setWhatsThis(i18n("Set this option to display the image title."));

    d->showCommentsBox       = new QCheckBox(i18n("Image caption"), d->digikamSettingBox);
    d->showCommentsBox->setWhatsThis(i18n("Set this option to display the image captions."));

    d->showTagsBox           = new QCheckBox(i18n("Image tags"), d->digikamSettingBox);
    d->showTagsBox->setWhatsThis(i18n("Set this option to display the image tags."));

    d->showLabelsBox         = new QCheckBox(i18n("Image labels"), d->digikamSettingBox);
    d->showLabelsBox->setWhatsThis(i18n("Set this option to display the image pick, color, rating labels."));

    grid4->addWidget(d->showAlbumNameBox, 0, 0, 1, 1);
    grid4->addWidget(d->showTitlesBox,    1, 0, 1, 1);
    grid4->addWidget(d->showCommentsBox,  2, 0, 1, 1);
    grid4->addWidget(d->showTagsBox,      0, 1, 1, 1);
    grid4->addWidget(d->showLabelsBox,    1, 1, 1, 1);
    grid4->setContentsMargins(spacing, spacing, spacing, spacing);
    grid4->setSpacing(0);

    // --------------------------------------------------------

    d->videoSettingBox           = new QGroupBox(i18n("Video Information"), vbox);
    QGridLayout* const grid5     = new QGridLayout(d->videoSettingBox);

    d->showVideoAspectRatio      = new QCheckBox(i18n("Video Aspect Ratio"), d->videoSettingBox);
    d->showVideoAspectRatio->setWhatsThis(i18n("Set this option to display the Aspect Ratio of the Video."));

    d->showVideoAudioBitRate     = new QCheckBox(i18n("Audio Bit Rate"), d->videoSettingBox);
    d->showVideoAudioBitRate->setWhatsThis(i18n("Set this option to display the Audio Bit Rate of the Video."));

    d->showVideoAudioChannelType = new QCheckBox(i18n("Audio Channel Type"), d->videoSettingBox);
    d->showVideoAudioChannelType->setWhatsThis(i18n("Set this option to display the Audio Channel Type of the Video."));

    d->showVideoAudioCompressor  = new QCheckBox(i18n("Audio Compressor"), d->videoSettingBox);
    d->showVideoAudioCompressor->setWhatsThis(i18n("Set this option to display the Audio Compressor of the Video."));

    d->showVideoDuration         = new QCheckBox(i18n("Video Duration"), d->videoSettingBox);
    d->showVideoDuration->setWhatsThis(i18n("Set this option to display the Duration of the Video."));

    d->showVideoFrameRate        = new QCheckBox(i18n("Video Frame Rate"), d->videoSettingBox);
    d->showVideoFrameRate->setWhatsThis(i18n("Set this option to display the Aspect Ratio of the Video."));

    d->showVideoVideoCodec       = new QCheckBox(i18n("Video Codec"), d->videoSettingBox);
    d->showVideoVideoCodec->setWhatsThis(i18n("Set this option to display the Codec of the Video."));

    grid5->addWidget(d->showVideoAspectRatio,      0, 0, 1, 1);
    grid5->addWidget(d->showVideoAudioBitRate,     1, 0, 1, 1);
    grid5->addWidget(d->showVideoAudioChannelType, 2, 0, 1, 1);
    grid5->addWidget(d->showVideoAudioCompressor,  3, 0, 1, 1);
    grid5->addWidget(d->showVideoDuration,         0, 1, 1, 1);
    grid5->addWidget(d->showVideoFrameRate,        1, 1, 1, 1);
    grid5->addWidget(d->showVideoVideoCodec,       2, 1, 1, 1);
    grid5->setContentsMargins(spacing, spacing, spacing, spacing);
    grid5->setSpacing(0);

    QWidget* const space = new QWidget(vbox);
    vbox->setStretchFactor(space, 10);
    vbox->setContentsMargins(spacing, spacing, spacing, spacing);
    vbox->setSpacing(spacing);

    // --------------------------------------------------------

    DVBox* const vbox2        = new DVBox(panel);
    d->showAlbumToolTipsBox   = new QCheckBox(i18n("Show album items' tool-tips"), vbox2);
    d->albumSettingBox        = new QGroupBox(i18n("Album Information"), vbox2);

    d->showAlbumToolTipsBox->setWhatsThis(i18n("Set this option to display album information when "
                                               "the mouse hovers over a folder-view item."));

    d->showAlbumTitleBox      = new QCheckBox(i18n("Album name"));
    d->showAlbumTitleBox->setWhatsThis(i18n("Set this option to display the album name."));

    d->showAlbumDateBox       = new QCheckBox(i18n("Album date"));
    d->showAlbumDateBox->setWhatsThis(i18n("Set this option to display the album date."));

    d->showAlbumCollectionBox = new QCheckBox(i18n("Album collection"));
    d->showAlbumCollectionBox->setWhatsThis(i18n("Set this option to display the album collection."));

    d->showAlbumCategoryBox   = new QCheckBox(i18n("Album category"));
    d->showAlbumCategoryBox->setWhatsThis(i18n("Set this option to display the album category."));

    d->showAlbumCaptionBox    = new QCheckBox(i18n("Album caption"));
    d->showAlbumCaptionBox->setWhatsThis(i18n("Set this option to display the album caption."));

    d->showAlbumPreviewBox    = new QCheckBox(i18n("Album preview"));
    d->showAlbumPreviewBox->setWhatsThis(i18n("Set this option to display the album preview."));

    QGridLayout* const albumSettingBoxLayout = new QGridLayout;
    albumSettingBoxLayout->addWidget(d->showAlbumTitleBox,      0, 0, 1, 1);
    albumSettingBoxLayout->addWidget(d->showAlbumDateBox,       1, 0, 1, 1);
    albumSettingBoxLayout->addWidget(d->showAlbumCollectionBox, 2, 0, 1, 1);
    albumSettingBoxLayout->addWidget(d->showAlbumCategoryBox,   0, 1, 1, 1);
    albumSettingBoxLayout->addWidget(d->showAlbumCaptionBox,    1, 1, 1, 1);
    albumSettingBoxLayout->addWidget(d->showAlbumPreviewBox,    2, 1, 1, 1);
    d->albumSettingBox->setLayout(albumSettingBoxLayout);

    QWidget* const space2 = new QWidget(vbox2);
    vbox2->setStretchFactor(space2, 10);
    vbox2->setContentsMargins(spacing, spacing, spacing, spacing);
    vbox2->setSpacing(spacing);

    // --------------------------------------------------------

    DVBox* const vbox3       = new DVBox(panel);
    d->showImportToolTipsBox = new QCheckBox(i18n("Show import items' tool-tips"), vbox3);
    d->importSettingBox      = new QGroupBox(i18n("Import Information"), vbox3);

    d->showAlbumToolTipsBox->setWhatsThis(i18n("Set this option to display album information when "
                                               "the mouse hovers over a folder-view item."));

    d->showItemTitleBox      = new QCheckBox(i18n("Item name"));
    d->showItemTitleBox->setWhatsThis(i18n("Set this option to display the item name."));

    d->showItemDateBox       = new QCheckBox(i18n("Item date"));
    d->showItemDateBox->setWhatsThis(i18n("Set this option to display the item date."));

    d->showItemSizeBox       = new QCheckBox(i18n("Item size"));
    d->showItemSizeBox->setWhatsThis(i18n("Set this option to display the item size."));

    d->showItemTypeBox       = new QCheckBox(i18n("Item type"));
    d->showItemTypeBox->setWhatsThis(i18n("Set this option to display the item type."));

    d->showItemDimensionsBox = new QCheckBox(i18n("Item dimensions"));
    d->showItemDimensionsBox->setWhatsThis(i18n("Set this option to display the item dimensions."));

    DLineWidget* const line  = new DLineWidget(Qt::Horizontal, d->photoSettingBox);
    QLabel* const label      = new QLabel(i18n("Note: these settings require \"Use File Metadata\" option "
                                               "from Camera Setup Behavior page."), d->photoSettingBox);

    d->showItemPhotoMakeBox  = new QCheckBox(i18n("Camera make and model"), d->photoSettingBox);
    d->showItemPhotoMakeBox->setWhatsThis(i18n("Set this option to display the make and model of the "
                                               "camera with which the image has been taken."));

    d->showItemPhotoLensBox  = new QCheckBox(i18n("Camera lens model"), d->photoSettingBox);
    d->showItemPhotoLensBox->setWhatsThis(i18n("Set this option to display the lens model with "
                                               "which the image was taken."));

    d->showItemPhotoFocalBox = new QCheckBox(i18n("Camera aperture and focal length"), d->photoSettingBox);
    d->showPhotoFocalBox->setWhatsThis(i18n("Set this option to display the camera aperture and focal settings "
                                            "used to take the image."));

    d->showItemPhotoExpoBox  = new QCheckBox(i18n("Camera exposure and sensitivity"), d->photoSettingBox);
    d->showPhotoExpoBox->setWhatsThis(i18n("Set this option to display the camera exposure and sensitivity "
                                           "used to take the image."));

    d->showItemPhotoFlashBox = new QCheckBox(i18n("Camera flash settings"), d->photoSettingBox);
    d->showPhotoFlashBox->setWhatsThis(i18n("Set this option to display the camera flash settings "
                                            "used to take the image."));

    d->showItemPhotoWBBox    = new QCheckBox(i18n("Camera white balance settings"), d->photoSettingBox);
    d->showItemPhotoWBBox->setWhatsThis(i18n("Set this option to display the camera white balance settings "
                                             "used to take the image."));

    QGridLayout* const importSettingBoxLayout = new QGridLayout;
    importSettingBoxLayout->addWidget(d->showItemTitleBox,      0, 0, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemDateBox,       1, 0, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemSizeBox,       2, 0, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemTypeBox,       0, 1, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemDimensionsBox, 1, 1, 1, 1);
    importSettingBoxLayout->addWidget(line,                     3, 0, 1, 2);
    importSettingBoxLayout->addWidget(label,                    4, 0, 1, 2);
    importSettingBoxLayout->addWidget(d->showItemPhotoMakeBox,  5, 0, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemPhotoLensBox,  6, 0, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemPhotoFocalBox, 7, 0, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemPhotoExpoBox,  5, 1, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemPhotoFlashBox, 6, 1, 1, 1);
    importSettingBoxLayout->addWidget(d->showItemPhotoWBBox,    7, 1, 1, 1);
    d->importSettingBox->setLayout(importSettingBoxLayout);

    QWidget* const space3 = new QWidget(vbox3);
    vbox3->setStretchFactor(space3, 10);
    vbox3->setContentsMargins(spacing, spacing, spacing, spacing);
    vbox3->setSpacing(spacing);

    // --------------------------------------------------------

    d->tab->insertTab(0, vbox,  i18n("Icon Items"));
    d->tab->insertTab(1, vbox2, i18n("Album Items"));
    d->tab->insertTab(2, vbox3, i18n("Import Items"));

    vlay->addWidget(d->fontSelect);
    vlay->addWidget(d->tab);
    vlay->addStretch();
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);

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
    settings->setToolTipsShowPhotoLens(d->showPhotoLensBox->isChecked());
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
    importSettings->setToolTipsShowPhotoLens(d->showItemPhotoLensBox->isChecked());
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
    d->showPhotoLensBox->setChecked(settings->getToolTipsShowPhotoLens());
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
    d->showItemPhotoLensBox->setChecked(importSettings->getToolTipsShowPhotoLens());
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
    d->showItemPhotoLensBox->setEnabled(b && d->cameraUseFileMetadata);
    d->showItemPhotoFocalBox->setEnabled(b && d->cameraUseFileMetadata);
    d->showItemPhotoExpoBox->setEnabled(b && d->cameraUseFileMetadata);
    d->showItemPhotoFlashBox->setEnabled(b && d->cameraUseFileMetadata);
    d->showItemPhotoWBBox->setEnabled(b && d->cameraUseFileMetadata);
}

}  // namespace Digikam
