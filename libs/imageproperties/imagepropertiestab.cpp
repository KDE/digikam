/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-19
 * Description : A tab to display general image information
 *
 * Copyright (C) 2006-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013      by Michael G. Hansen <mike at mghansen dot de>
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

#include "imagepropertiestab.moc"

// Qt includes

#include <QGridLayout>
#include <QStyle>
#include <QDir>
#include <QFile>
#include <QPixmap>
#include <QPainter>
#include <QPair>
#include <QVariant>

// KDE includes

#include <klocale.h>
#include <kdialog.h>
#include <kstringhandler.h>

// Local includes

#include "imagepropertiestxtlabel.h"
#include "picklabelwidget.h"
#include "colorlabelwidget.h"
#include "tagscache.h"

namespace Digikam
{

class ImagePropertiesTab::Private
{
public:

    enum Section
    {
        FileProperties = 0,
        ImageProperties,
        PhotoProperties,
        VideoProperties,
        digiKamProperties
    };

public:

    Private() :
        file(0),
        folder(0),
        modifiedDate(0),
        size(0),
        owner(0),
        permissions(0),
        mime(0),
        dimensions(0),
        ratio(0),
        bitDepth(0),
        colorMode(0),
        make(0),
        model(0),
        photoDate(0),
        lens(0),
        aperture(0),
        focalLength(0),
        exposureTime(0),
        sensitivity(0),
        exposureMode(0),
        flash(0),
        whiteBalance(0),
        caption(0),
        tags(0),
        pickLabel(0),
        colorLabel(0),
        rating(0),
        labelFile(0),
        labelFolder(0),
        labelFileModifiedDate(0),
        labelFileSize(0),
        labelFileOwner(0),
        labelFilePermissions(0),
        labelImageMime(0),
        labelImageDimensions(0),
        labelImageRatio(0),
        labelImageBitDepth(0),
        labelImageColorMode(0),
        labelPhotoMake(0),
        labelPhotoModel(0),
        labelPhotoDateTime(0),
        labelPhotoLens(0),
        labelPhotoAperture(0),
        labelPhotoFocalLength(0),
        labelPhotoExposureTime(0),
        labelPhotoSensitivity(0),
        labelPhotoExposureMode(0),
        labelPhotoFlash(0),
        labelPhotoWhiteBalance(0),
        labelCaption(0),
        labelTags(0),
        labelPickLabel(0),
        labelColorLabel(0),
        labelRating(0),
        aspectRatio(0),
        duration(0),
        frameRate(0),
        videoCodec(0),
        audioBitRate(0),
        audioChannelType(0),
        audioCompressor(0),
        labelVideoAspectRatio(0),
        labelVideoDuration(0),
        labelVideoFrameRate(0),
        labelVideoVideoCodec(0),
        labelVideoAudioBitRate(0),
        labelVideoAudioChannelType(0),
        labelVideoAudioCompressor(0)
    {
    }

    DTextLabelName*  file;
    DTextLabelName*  folder;
    DTextLabelName*  modifiedDate;
    DTextLabelName*  size;
    DTextLabelName*  owner;
    DTextLabelName*  permissions;

    DTextLabelName*  mime;
    DTextLabelName*  dimensions;
    DTextLabelName*  ratio;
    DTextLabelName*  bitDepth;
    DTextLabelName*  colorMode;

    DTextLabelName*  make;
    DTextLabelName*  model;
    DTextLabelName*  photoDate;
    DTextLabelName*  lens;
    DTextLabelName*  aperture;
    DTextLabelName*  focalLength;
    DTextLabelName*  exposureTime;
    DTextLabelName*  sensitivity;
    DTextLabelName*  exposureMode;
    DTextLabelName*  flash;
    DTextLabelName*  whiteBalance;

    DTextLabelName*  caption;
    DTextLabelName*  tags;
    DTextLabelName*  pickLabel;
    DTextLabelName*  colorLabel;
    DTextLabelName*  rating;

    DTextLabelValue* labelFile;
    DTextLabelValue* labelFolder;
    DTextLabelValue* labelFileModifiedDate;
    DTextLabelValue* labelFileSize;
    DTextLabelValue* labelFileOwner;
    DTextLabelValue* labelFilePermissions;

    DTextLabelValue* labelImageMime;
    DTextLabelValue* labelImageDimensions;
    DTextLabelValue* labelImageRatio;
    DTextLabelValue* labelImageBitDepth;
    DTextLabelValue* labelImageColorMode;

    DTextLabelValue* labelPhotoMake;
    DTextLabelValue* labelPhotoModel;
    DTextLabelValue* labelPhotoDateTime;
    DTextLabelValue* labelPhotoLens;
    DTextLabelValue* labelPhotoAperture;
    DTextLabelValue* labelPhotoFocalLength;
    DTextLabelValue* labelPhotoExposureTime;
    DTextLabelValue* labelPhotoSensitivity;
    DTextLabelValue* labelPhotoExposureMode;
    DTextLabelValue* labelPhotoFlash;
    DTextLabelValue* labelPhotoWhiteBalance;

    DTextLabelValue* labelCaption;
    DTextLabelValue* labelTags;
    DTextLabelValue* labelPickLabel;
    DTextLabelValue* labelColorLabel;
    DTextLabelValue* labelRating;

    DTextLabelName*  aspectRatio;
    DTextLabelName*  duration;
    DTextLabelName*  frameRate;
    DTextLabelName*  videoCodec;
    DTextLabelName*  audioBitRate;
    DTextLabelName*  audioChannelType;
    DTextLabelName*  audioCompressor;

    DTextLabelValue* labelVideoAspectRatio;
    DTextLabelValue* labelVideoDuration;
    DTextLabelValue* labelVideoFrameRate;
    DTextLabelValue* labelVideoVideoCodec;
    DTextLabelValue* labelVideoAudioBitRate;
    DTextLabelValue* labelVideoAudioChannelType;
    DTextLabelValue* labelVideoAudioCompressor;
};

ImagePropertiesTab::ImagePropertiesTab(QWidget* const parent)
    : RExpanderBox(parent), d(new Private)
{
    setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    setLineWidth( style()->pixelMetric(QStyle::PM_DefaultFrameWidth) );

    // --------------------------------------------------

    QWidget* const w1         = new QWidget(this);
    QGridLayout* const glay1  = new QGridLayout(w1);

    d->file                   = new DTextLabelName(i18n("File: "),        w1);
    d->folder                 = new DTextLabelName(i18n("Folder: "),      w1);
    d->modifiedDate           = new DTextLabelName(i18n("Date: "),        w1);
    d->size                   = new DTextLabelName(i18n("Size: "),        w1);
    d->owner                  = new DTextLabelName(i18n("Owner: "),       w1);
    d->permissions            = new DTextLabelName(i18n("Permissions: "), w1);

    d->labelFile              = new DTextLabelValue(0, w1);
    d->labelFolder            = new DTextLabelValue(0, w1);
    d->labelFileModifiedDate  = new DTextLabelValue(0, w1);
    d->labelFileSize          = new DTextLabelValue(0, w1);
    d->labelFileOwner         = new DTextLabelValue(0, w1);
    d->labelFilePermissions   = new DTextLabelValue(0, w1);

    glay1->addWidget(d->file,                  0, 0, 1, 1);
    glay1->addWidget(d->labelFile,             0, 1, 1, 1);
    glay1->addWidget(d->folder,                1, 0, 1, 1);
    glay1->addWidget(d->labelFolder,           1, 1, 1, 1);
    glay1->addWidget(d->modifiedDate,          2, 0, 1, 1);
    glay1->addWidget(d->labelFileModifiedDate, 2, 1, 1, 1);
    glay1->addWidget(d->size,                  3, 0, 1, 1);
    glay1->addWidget(d->labelFileSize,         3, 1, 1, 1);
    glay1->addWidget(d->owner,                 4, 0, 1, 1);
    glay1->addWidget(d->labelFileOwner,        4, 1, 1, 1);
    glay1->addWidget(d->permissions,           5, 0, 1, 1);
    glay1->addWidget(d->labelFilePermissions,  5, 1, 1, 1);
    glay1->setMargin(KDialog::spacingHint());
    glay1->setSpacing(0);
    glay1->setColumnStretch(1, 10);

    insertItem(ImagePropertiesTab::Private::FileProperties,
               w1, SmallIcon("dialog-information"),
               i18n("File Properties"), QString("FileProperties"), true);

    // --------------------------------------------------

    QWidget* const w2         = new QWidget(this);
    QGridLayout* const glay2  = new QGridLayout(w2);

    d->mime                   = new DTextLabelName(i18n("Type: "),         w2);
    d->dimensions             = new DTextLabelName(i18n("Dimensions: "),   w2);
    d->ratio                  = new DTextLabelName(i18n("Aspect Ratio: "), w2);
    d->bitDepth               = new DTextLabelName(i18n("Bit depth: "),    w2);
    d->colorMode              = new DTextLabelName(i18n("Color mode: "),   w2);

    d->labelImageMime         = new DTextLabelValue(0, w2);
    d->labelImageDimensions   = new DTextLabelValue(0, w2);
    d->labelImageRatio        = new DTextLabelValue(0, w2);
    d->labelImageBitDepth     = new DTextLabelValue(0, w2);
    d->labelImageColorMode    = new DTextLabelValue(0, w2);

    glay2->addWidget(d->mime,                   0, 0, 1, 1);
    glay2->addWidget(d->labelImageMime,         0, 1, 1, 1);
    glay2->addWidget(d->dimensions,             1, 0, 1, 1);
    glay2->addWidget(d->labelImageDimensions,   1, 1, 1, 1);
    glay2->addWidget(d->ratio,                  2, 0, 1, 1);
    glay2->addWidget(d->labelImageRatio,        2, 1, 1, 1);
    glay2->addWidget(d->bitDepth,               3, 0, 1, 1);
    glay2->addWidget(d->labelImageBitDepth,     3, 1, 1, 1);
    glay2->addWidget(d->colorMode,              4, 0, 1, 1);
    glay2->addWidget(d->labelImageColorMode,    4, 1, 1, 1);
    glay2->setMargin(KDialog::spacingHint());
    glay2->setSpacing(0);
    glay2->setColumnStretch(1, 10);

    insertItem(ImagePropertiesTab::Private::ImageProperties,
               w2, SmallIcon("image-x-generic"),
               i18n("Item Properties"), QString("ItemProperties"), true);

    // --------------------------------------------------

    QWidget* const w3         = new QWidget(this);
    QGridLayout* const glay3  = new QGridLayout(w3);

    d->make                   = new DTextLabelName(i18n("Make: "),          w3);
    d->model                  = new DTextLabelName(i18n("Model: "),         w3);
    d->photoDate              = new DTextLabelName(i18n("Created: "),       w3);
    d->lens                   = new DTextLabelName(i18n("Lens: "),          w3);
    d->aperture               = new DTextLabelName(i18n("Aperture: "),      w3);
    d->focalLength            = new DTextLabelName(i18n("Focal: "),         w3);
    d->exposureTime           = new DTextLabelName(i18n("Exposure: "),      w3);
    d->sensitivity            = new DTextLabelName(i18n("Sensitivity: "),   w3);
    d->exposureMode           = new DTextLabelName(i18n("Mode/Program: "),  w3);
    d->flash                  = new DTextLabelName(i18n("Flash: "),         w3);
    d->whiteBalance           = new DTextLabelName(i18n("White balance: "), w3);

    d->labelPhotoMake         = new DTextLabelValue(0, w3);
    d->labelPhotoModel        = new DTextLabelValue(0, w3);
    d->labelPhotoDateTime     = new DTextLabelValue(0, w3);
    d->labelPhotoLens         = new DTextLabelValue(0, w3);
    d->labelPhotoAperture     = new DTextLabelValue(0, w3);
    d->labelPhotoFocalLength  = new DTextLabelValue(0, w3);
    d->labelPhotoExposureTime = new DTextLabelValue(0, w3);
    d->labelPhotoSensitivity  = new DTextLabelValue(0, w3);
    d->labelPhotoExposureMode = new DTextLabelValue(0, w3);
    d->labelPhotoFlash        = new DTextLabelValue(0, w3);
    d->labelPhotoWhiteBalance = new DTextLabelValue(0, w3);

    glay3->addWidget(d->make,                   0,  0, 1, 1);
    glay3->addWidget(d->labelPhotoMake,         0,  1, 1, 1);
    glay3->addWidget(d->model,                  1,  0, 1, 1);
    glay3->addWidget(d->labelPhotoModel,        1,  1, 1, 1);
    glay3->addWidget(d->photoDate,              2,  0, 1, 1);
    glay3->addWidget(d->labelPhotoDateTime,     2,  1, 1, 1);
    glay3->addWidget(d->lens,                   3,  0, 1, 1);
    glay3->addWidget(d->labelPhotoLens,         3,  1, 1, 1);
    glay3->addWidget(d->aperture,               4,  0, 1, 1);
    glay3->addWidget(d->labelPhotoAperture,     4,  1, 1, 1);
    glay3->addWidget(d->focalLength,            5,  0, 1, 1);
    glay3->addWidget(d->labelPhotoFocalLength,  5,  1, 1, 1);
    glay3->addWidget(d->exposureTime,           6,  0, 1, 1);
    glay3->addWidget(d->labelPhotoExposureTime, 6,  1, 1, 1);
    glay3->addWidget(d->sensitivity,            7,  0, 1, 1);
    glay3->addWidget(d->labelPhotoSensitivity,  7,  1, 1, 1);
    glay3->addWidget(d->exposureMode,           8,  0, 1, 1);
    glay3->addWidget(d->labelPhotoExposureMode, 8,  1, 1, 1);
    glay3->addWidget(d->flash,                  9,  0, 1, 1);
    glay3->addWidget(d->labelPhotoFlash,        9,  1, 1, 1);
    glay3->addWidget(d->whiteBalance,           10, 0, 1, 1);
    glay3->addWidget(d->labelPhotoWhiteBalance, 10, 1, 1, 1);
    glay3->setColumnStretch(1, 10);
    glay3->setMargin(KDialog::spacingHint());
    glay3->setSpacing(0);

    insertItem(ImagePropertiesTab::Private::PhotoProperties,
               w3, SmallIcon("camera-photo"),
               i18n("Photograph Properties"), QString("PhotographProperties"), true);

    // --------------------------------------------------

    QWidget* const w4             = new QWidget(this);
    QGridLayout* const glay4      = new QGridLayout(w4);

    d->aspectRatio                = new DTextLabelName(i18n("Aspect Ratio: "),       w4);
    d->duration                   = new DTextLabelName(i18n("Duration: "),           w4);
    d->frameRate                  = new DTextLabelName(i18n("Frame Rate: "),         w4);
    d->videoCodec                 = new DTextLabelName(i18n("Video Codec: "),        w4);
    d->audioBitRate               = new DTextLabelName(i18n("Audio Bit Rate: "),     w4);
    d->audioChannelType           = new DTextLabelName(i18n("Audio Channel Type: "), w4);
    d->audioCompressor            = new DTextLabelName(i18n("Audio Compressor: "),   w4);

    d->labelVideoAspectRatio      = new DTextLabelValue(0, w4);
    d->labelVideoDuration         = new DTextLabelValue(0, w4);
    d->labelVideoFrameRate        = new DTextLabelValue(0, w4);
    d->labelVideoVideoCodec       = new DTextLabelValue(0, w4);
    d->labelVideoAudioBitRate     = new DTextLabelValue(0, w4);
    d->labelVideoAudioChannelType = new DTextLabelValue(0, w4);
    d->labelVideoAudioCompressor  = new DTextLabelValue(0, w4);

    glay4->addWidget(d->aspectRatio,                0, 0, 1, 1);
    glay4->addWidget(d->labelVideoAspectRatio,      0, 1, 1, 1);
    glay4->addWidget(d->duration,                   1, 0, 1, 1);
    glay4->addWidget(d->labelVideoDuration,         1, 1, 1, 1);
    glay4->addWidget(d->frameRate,                  2, 0, 1, 1);
    glay4->addWidget(d->labelVideoFrameRate,        2, 1, 1, 1);
    glay4->addWidget(d->videoCodec,                 3, 0, 1, 1);
    glay4->addWidget(d->labelVideoVideoCodec,       3, 1, 1, 1);
    glay4->addWidget(d->audioBitRate,               4, 0, 1, 1);
    glay4->addWidget(d->labelVideoAudioBitRate,     4, 1, 1, 1);
    glay4->addWidget(d->audioChannelType,           5, 0, 1, 1);
    glay4->addWidget(d->labelVideoAudioChannelType, 5, 1, 1, 1);
    glay4->addWidget(d->audioCompressor,            6, 0, 1, 1);
    glay4->addWidget(d->labelVideoAudioCompressor,  6, 1, 1, 1);
    glay4->setMargin(KDialog::spacingHint());
    glay4->setSpacing(0);
    glay4->setColumnStretch(1, 10);

    insertItem(ImagePropertiesTab::Private::VideoProperties,
               w4, SmallIcon("video-x-generic"),
               i18n("Video Properties"), QString("VideoProperties"), true);

    // --------------------------------------------------

    QWidget* const w5         = new QWidget(this);
    QGridLayout* const glay5  = new QGridLayout(w5);

    d->caption                = new DTextLabelName(i18n("Caption: "),     w5);
    d->pickLabel              = new DTextLabelName(i18n("Pick label: "),  w5);
    d->colorLabel             = new DTextLabelName(i18n("Color label: "), w5);
    d->rating                 = new DTextLabelName(i18n("Rating: "),      w5);
    d->tags                   = new DTextLabelName(i18n("Tags: "),        w5);

    d->labelCaption           = new DTextLabelValue(0, w5);
    d->labelPickLabel         = new DTextLabelValue(0, w5);
    d->labelColorLabel        = new DTextLabelValue(0, w5);
    d->labelRating            = new DTextLabelValue(0, w5);
    d->labelTags              = new DTextLabelValue(0, w5);
    d->labelTags->setTextElideMode(Qt::ElideLeft);

    glay5->addWidget(d->caption,         0, 0, 1, 1);
    glay5->addWidget(d->labelCaption,    0, 1, 1, 1);
    glay5->addWidget(d->tags,            1, 0, 1, 1);
    glay5->addWidget(d->labelTags,       1, 1, 1, 1);
    glay5->addWidget(d->pickLabel,       2, 0, 1, 1);
    glay5->addWidget(d->labelPickLabel,  2, 1, 1, 1);
    glay5->addWidget(d->colorLabel,      3, 0, 1, 1);
    glay5->addWidget(d->labelColorLabel, 3, 1, 1, 1);
    glay5->addWidget(d->rating,          4, 0, 1, 1);
    glay5->addWidget(d->labelRating,     4, 1, 1, 1);
    glay5->setMargin(KDialog::spacingHint());
    glay5->setSpacing(0);
    glay5->setColumnStretch(1, 10);

    insertItem(ImagePropertiesTab::Private::digiKamProperties,
               w5, SmallIcon("imagecomment"),
               i18n("digiKam Properties"), QString("DigikamProperties"), true);

    // --------------------------------------------------

    addStretch();
}

ImagePropertiesTab::~ImagePropertiesTab()
{
    delete d;
}

void ImagePropertiesTab::setCurrentURL(const KUrl& url)
{
    if (url.isEmpty())
    {
        d->labelFile->clear();
        d->labelFolder->clear();
        d->labelFileModifiedDate->clear();
        d->labelFileSize->clear();
        d->labelFileOwner->clear();
        d->labelFilePermissions->clear();

        d->labelImageMime->clear();
        d->labelImageDimensions->clear();
        d->labelImageRatio->clear();
        d->labelImageBitDepth->clear();
        d->labelImageColorMode->clear();

        d->labelPhotoMake->clear();
        d->labelPhotoModel->clear();
        d->labelPhotoDateTime->clear();
        d->labelPhotoLens->clear();
        d->labelPhotoAperture->clear();
        d->labelPhotoFocalLength->clear();
        d->labelPhotoExposureTime->clear();
        d->labelPhotoSensitivity->clear();
        d->labelPhotoExposureMode->clear();
        d->labelPhotoFlash->clear();
        d->labelPhotoWhiteBalance->clear();

        d->labelCaption->clear();
        d->labelPickLabel->clear();
        d->labelColorLabel->clear();
        d->labelRating->clear();
        d->labelTags->clear();

        d->labelVideoAspectRatio->clear();
        d->labelVideoDuration->clear();
        d->labelVideoFrameRate->clear();
        d->labelVideoVideoCodec->clear();
        d->labelVideoAudioBitRate->clear();
        d->labelVideoAudioChannelType->clear();
        d->labelVideoAudioCompressor->clear();

        setEnabled(false);
        return;
    }

    setEnabled(true);

    d->labelFile->setText(url.fileName());
    d->labelFolder->setText(QDir::toNativeSeparators(url.directory()));
}

void ImagePropertiesTab::setPhotoInfoDisable(const bool b)
{
    if (b)
    {
        widget(ImagePropertiesTab::Private::PhotoProperties)->hide();
    }
    else
    {
        widget(ImagePropertiesTab::Private::PhotoProperties)->show();
    }
}

void ImagePropertiesTab::setVideoInfoDisable(const bool b)
{
    if (b)
    {
        widget(ImagePropertiesTab::Private::VideoProperties)->hide();
    }
    else
    {
        widget(ImagePropertiesTab::Private::VideoProperties)->show();
    }
}

void ImagePropertiesTab::setFileModifiedDate(const QString& str)
{
    d->labelFileModifiedDate->setText(str);
}

void ImagePropertiesTab::setFileSize(const QString& str)
{
    d->labelFileSize->setText(str);
}

void ImagePropertiesTab::setFileOwner(const QString& str)
{
    d->labelFileOwner->setText(str);
}

void ImagePropertiesTab::setFilePermissions(const QString& str)
{
    d->labelFilePermissions->setText(str);
}

void ImagePropertiesTab::setImageMime(const QString& str)
{
    d->labelImageMime->setText(str);
}

void ImagePropertiesTab::setImageDimensions(const QString& str)
{
    d->labelImageDimensions->setText(str);
}

void ImagePropertiesTab::setImageRatio(const QString& str)
{
    d->labelImageRatio->setText(str);
}

void ImagePropertiesTab::setImageBitDepth(const QString& str)
{
    d->labelImageBitDepth->setText(str);
}

void ImagePropertiesTab::setImageColorMode(const QString& str)
{
    d->labelImageColorMode->setText(str);
}

void ImagePropertiesTab::setPhotoMake(const QString& str)
{
    d->labelPhotoMake->setText(str);
}

void ImagePropertiesTab::setPhotoModel(const QString& str)
{
    d->labelPhotoModel->setText(str);
}

void ImagePropertiesTab::setPhotoDateTime(const QString& str)
{
    d->labelPhotoDateTime->setText(str);
}

void ImagePropertiesTab::setPhotoLens(const QString& str)
{
    d->labelPhotoLens->setText(str);
}

void ImagePropertiesTab::setPhotoAperture(const QString& str)
{
    d->labelPhotoAperture->setText(str);
}

void ImagePropertiesTab::setPhotoFocalLength(const QString& str)
{
    d->labelPhotoFocalLength->setText(str);
}

void ImagePropertiesTab::setPhotoExposureTime(const QString& str)
{
    d->labelPhotoExposureTime->setText(str);
}

void ImagePropertiesTab::setPhotoSensitivity(const QString& str)
{
    d->labelPhotoSensitivity->setText(str);
}

void ImagePropertiesTab::setPhotoExposureMode(const QString& str)
{
    d->labelPhotoExposureMode->setText(str);
}

void ImagePropertiesTab::setPhotoFlash(const QString& str)
{
    d->labelPhotoFlash->setText(str);
}

void ImagePropertiesTab::setPhotoWhiteBalance(const QString& str)
{
    d->labelPhotoWhiteBalance->setText(str);
}

void ImagePropertiesTab::showOrHideCaptionAndTags()
{
    bool hasCaption    = !d->labelCaption->text().isEmpty();
    bool hasPickLabel  = !d->labelPickLabel->text().isEmpty();
    bool hasColorLabel = !d->labelColorLabel->text().isEmpty();
    bool hasRating     = !d->labelRating->text().isEmpty();
    bool hasTags       = !d->labelTags->text().isEmpty();

    d->caption->setVisible(hasCaption);
    d->labelCaption->setVisible(hasCaption);
    d->pickLabel->setVisible(hasPickLabel);
    d->labelPickLabel->setVisible(hasPickLabel);
    d->colorLabel->setVisible(hasColorLabel);
    d->labelColorLabel->setVisible(hasColorLabel);
    d->rating->setVisible(hasRating);
    d->labelRating->setVisible(hasRating);
    d->tags->setVisible(hasTags);
    d->labelTags->setVisible(hasTags);

    widget(ImagePropertiesTab::Private::digiKamProperties)->setVisible(hasCaption || hasRating || hasTags || hasPickLabel || hasColorLabel);
}

void ImagePropertiesTab::setCaption(const QString& str)
{
    d->labelCaption->setText(str);
}

void ImagePropertiesTab::setColorLabel(int colorId)
{
    if (colorId == NoColorLabel)
    {
        d->labelColorLabel->setText(QString());
    }
    else
    {
        d->labelColorLabel->setText(ColorLabelWidget::labelColorName((ColorLabel)colorId));
    }
}

void ImagePropertiesTab::setPickLabel(int pickId)
{
    if (pickId == NoPickLabel)
    {
        d->labelPickLabel->setText(QString());
    }
    else
    {
        d->labelPickLabel->setText(PickLabelWidget::labelPickName((PickLabel)pickId));
    }
}

void ImagePropertiesTab::setRating(int rating)
{
    QString str;

    if (rating > RatingMin && rating <= RatingMax)
    {
        str = ' ';

        for (int i=0; i<rating; ++i)
        {
            str += QChar(0x2730);
            str += ' ';
        }
    }

    d->labelRating->setText(str);
}

void ImagePropertiesTab::setVideoAspectRatio(const QString& str)
{
    d->labelVideoAspectRatio->setText(str);
}

void ImagePropertiesTab::setVideoAudioBitRate(const QString& str)
{
    // use string given as parameter by default because it contains the value for "unavailable" if needed
    QString audioBitRateString = str;
    bool ok;
    const int audioBitRateInt = str.toInt(&ok);
    if (ok)
    {
        audioBitRateString = KGlobal::locale()->formatNumber(audioBitRateInt, 0);
    }

    d->labelVideoAudioBitRate->setText(audioBitRateString);
}

void ImagePropertiesTab::setVideoAudioChannelType(const QString& str)
{
    d->labelVideoAudioChannelType->setText(str);
}

void ImagePropertiesTab::setVideoAudioCompressor(const QString& str)
{
    d->labelVideoAudioCompressor->setText(str);
}

void ImagePropertiesTab::setVideoDuration(const QString& str)
{
    // duration is given as a string in milliseconds
    // use string given as parameter by default because it contains the value for "unavailable" if needed
    QString durationString = str;
    bool ok;
    const double durationDouble = str.toDouble(&ok);
    if (ok)
    {
        const QTime durationTime = QTime().addMSecs(durationDouble);
        durationString = KGlobal::locale()->formatTime(durationTime, true, true);
    }

    d->labelVideoDuration->setText(durationString);
}

void ImagePropertiesTab::setVideoFrameRate(const QString& str)
{
    // use string given as parameter by default because it contains the value for "unavailable" if needed
    QString frameRateString = str;
    bool ok;
    const double frameRateDouble = str.toDouble(&ok);
    if (ok)
    {
        frameRateString = KGlobal::locale()->formatNumber(frameRateDouble);
    }

    d->labelVideoFrameRate->setText(frameRateString);
}

void ImagePropertiesTab::setVideoVideoCodec(const QString& str)
{
    d->labelVideoVideoCodec->setText(str);
}

void ImagePropertiesTab::setTags(const QStringList& tagPaths, const QStringList& tagNames)
{
    Q_UNUSED(tagNames);
    d->labelTags->setText(shortenedTagPaths(tagPaths).join("\n"));
}

typedef QPair<QString, QVariant> PathValuePair;

static bool naturalLessThan(const PathValuePair& a, const PathValuePair& b)
{
    return KStringHandler::naturalCompare(a.first, b.first) < 0;
}

QStringList ImagePropertiesTab::shortenedTagPaths(const QStringList& tagPaths, QList<QVariant>* identifiers)
{
    QList<PathValuePair> tagsSorted;

    if (identifiers)
    {
        for (int i = 0; i < tagPaths.size(); ++i)
        {
            tagsSorted << PathValuePair(tagPaths.at(i), (*identifiers).at(i));
        }
    }
    else
    {
        for (int i = 0; i < tagPaths.size(); ++i)
        {
            tagsSorted << PathValuePair(tagPaths.at(i), QVariant());
        }
    }

    qStableSort(tagsSorted.begin(), tagsSorted.end(), naturalLessThan);

    if (identifiers)
    {
        identifiers->clear();
    }

    QStringList tagsShortened;
    QString previous;

    foreach(const PathValuePair& pair, tagsSorted)
    {
        const QString& tagPath   = pair.first;
        QString shortenedPath    = tagPath;
        QStringList currentPath  = tagPath.split('/', QString::SkipEmptyParts);
        QStringList previousPath = previous.split('/', QString::SkipEmptyParts);
        int depth;

        for (depth = 0; depth < currentPath.size() && depth < previousPath.size(); ++depth)
        {
            if (currentPath.at(depth) != previousPath.at(depth))
                break;
        }

        if (depth)
        {
            QString indent;
            indent.fill(' ', qMin(depth, 5));
            //indent += QChar(0x2026);
            shortenedPath = indent + tagPath.section('/', depth);
        }

        shortenedPath.replace('/', " / ");
        tagsShortened << shortenedPath;
        previous = tagPath;

        if (identifiers)
        {
            (*identifiers) << pair.second;
        }
    }

    return tagsShortened;
}

void ImagePropertiesTab::shortenedMakeInfo(QString& make)
{
    make.remove(" CORPORATION",      Qt::CaseInsensitive);        // from Nikon, Pentax, and Olympus
    make.remove("EASTMAN ",          Qt::CaseInsensitive);        // from Kodak
    make.remove(" COMPANY",          Qt::CaseInsensitive);        // from Kodak
    make.remove(" OPTICAL CO.,LTD",  Qt::CaseInsensitive);        // from Olympus
    make.remove(" Techwin co.,Ltd.", Qt::CaseInsensitive);        // from Samsung
    make.remove("  Co.,Ltd.",        Qt::CaseInsensitive);        // from Minolta
}

void ImagePropertiesTab::shortenedModelInfo(QString& model)
{
    model.remove("Canon ",           Qt::CaseInsensitive);
    model.remove("NIKON ",           Qt::CaseInsensitive);
    model.remove("PENTAX ",          Qt::CaseInsensitive);
    model.remove(" DIGITAL",         Qt::CaseInsensitive);        // from Canon
}

/**
 * Find rational approximation to given real number
 *
 *   val    : double value to convert as humain readable fraction
 *   num    : fraction numerator
 *   den    : fraction denominator
 *   maxden : the maximum denominator allowed
 *
 * This function return approximation error of the fraction
 *
 * Based on the theory of continued fractions
 * if x = a1 + 1/(a2 + 1/(a3 + 1/(a4 + ...)))
 * Then best approximation is found by truncating this series
 * wwith some adjustments in the last term.
 *
 * Note the fraction can be recovered as the first column of the matrix
 *  ( a1 1 ) ( a2 1 ) ( a3 1 ) ...
 *  ( 1  0 ) ( 1  0 ) ( 1  0 )
 * Instead of keeping the sequence of continued fraction terms,
 * we just keep the last partial product of these matrices.
 *
 * Details: http://stackoverflow.com/questions/95727/how-to-convert-floats-to-human-readable-fractions
 *
 */
double ImagePropertiesTab::doubleToHumanReadableFraction(double val, long* num, long* den, long maxden)
{
    long   m[2][2];
    long   ai;
    double x = val;

    // Initialize matrix

    m[0][0] = m[1][1] = 1;
    m[0][1] = m[1][0] = 0;

    // Loop finding terms until denominator gets too big

    while (m[1][0] *  ( ai = (long)x ) + m[1][1] <= maxden)
    {
        long t  = m[0][0] * ai + m[0][1];
        m[0][1] = m[0][0];
        m[0][0] = t;
        t       = m[1][0] * ai + m[1][1];
        m[1][1] = m[1][0];
        m[1][0] = t;

        if (x == (double)ai)
            break;     // division by zero

        x       = 1/(x - (double) ai);

        if (x > (double)0x7FFFFFFF)
            break;     // representation failure
    }

    // Now remaining x is between 0 and 1/ai
    // Approx as either 0 or 1/m where m is max that will fit in maxden

    *num = m[0][0];
    *den = m[1][0];

    // Return approxiamtion error

    return (val - ((double) m[0][0] / (double) m[1][0]));
}

bool ImagePropertiesTab::aspectRatioToString(int width, int height, QString& arString)
{
    if ( (width == 0) || (height == 0) )
    {
        return false;
    }

    int nw = width;
    int nh = height;

    // Always show ratio with and image horizontally oriented, for better readability
    if (height > width)
    {
        nw = height;
        nh = width;
    }

    long   num=0, den=0;
    doubleToHumanReadableFraction((double)nw / (double)nh, &num, &den, 10);

    const QString awidth  = QString::number(num);
    const QString aheight = QString::number(den);
    const QString aratio  = KGlobal::locale()->formatNumber((double)num/(double)den, 2);

    arString = i18nc("width : height (Aspect Ratio)", "%1:%2 (%3)",
                     awidth, aheight, aratio);

    return true;
}

}  // namespace Digikam
