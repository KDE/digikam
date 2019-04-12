/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-19
 * Description : A tab to display general item information
 *
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "itempropertiestab.h"

// Qt includes

#include <QGridLayout>
#include <QStyle>
#include <QDir>
#include <QFile>
#include <QPixmap>
#include <QPainter>
#include <QPair>
#include <QVariant>
#include <QApplication>
#include <QCollator>
#include <QIcon>
#include <QLocale>
#include <QTime>
#include <QtMath>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "itempropertiestxtlabel.h"
#include "picklabelwidget.h"
#include "colorlabelwidget.h"
#include "tagscache.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemPropertiesTab::Private
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

    explicit Private()
      : caption(nullptr),
        pickLabel(nullptr),
        colorLabel(nullptr),
        rating(nullptr),
        tags(nullptr),
        labelFile(nullptr),
        labelFolder(nullptr),
        labelFileModifiedDate(nullptr),
        labelFileSize(nullptr),
        labelFileOwner(nullptr),
        labelFilePermissions(nullptr),
        labelImageMime(nullptr),
        labelImageDimensions(nullptr),
        labelImageRatio(nullptr),
        labelImageBitDepth(nullptr),
        labelImageColorMode(nullptr),
        labelPhotoMake(nullptr),
        labelPhotoModel(nullptr),
        labelPhotoDateTime(nullptr),
        labelPhotoLens(nullptr),
        labelPhotoAperture(nullptr),
        labelPhotoFocalLength(nullptr),
        labelPhotoExposureTime(nullptr),
        labelPhotoSensitivity(nullptr),
        labelPhotoExposureMode(nullptr),
        labelPhotoFlash(nullptr),
        labelPhotoWhiteBalance(nullptr),
        labelCaption(nullptr),
        labelTags(nullptr),
        labelPickLabel(nullptr),
        labelColorLabel(nullptr),
        labelRating(nullptr),
        labelVideoAspectRatio(nullptr),
        labelVideoDuration(nullptr),
        labelVideoFrameRate(nullptr),
        labelVideoVideoCodec(nullptr),
        labelVideoAudioBitRate(nullptr),
        labelVideoAudioChannelType(nullptr),
        labelVideoAudioCodec(nullptr)
    {
    }

    DTextLabelName* caption;
    DTextLabelName* pickLabel;
    DTextLabelName* colorLabel;
    DTextLabelName* rating;
    DTextLabelName* tags;

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

    DTextLabelValue* labelVideoAspectRatio;
    DTextLabelValue* labelVideoDuration;
    DTextLabelValue* labelVideoFrameRate;
    DTextLabelValue* labelVideoVideoCodec;
    DTextLabelValue* labelVideoAudioBitRate;
    DTextLabelValue* labelVideoAudioChannelType;
    DTextLabelValue* labelVideoAudioCodec;
};

ItemPropertiesTab::ItemPropertiesTab(QWidget* const parent)
    : DExpanderBox(parent),
      d(new Private)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(style()->pixelMetric(QStyle::PM_DefaultFrameWidth));

    // --------------------------------------------------

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QWidget* const w1                  = new QWidget(this);
    QGridLayout* const glay1           = new QGridLayout(w1);

    DTextLabelName* const file         = new DTextLabelName(i18n("File: "),        w1);
    DTextLabelName* const folder       = new DTextLabelName(i18n("Folder: "),      w1);
    DTextLabelName* const modifiedDate = new DTextLabelName(i18n("Date: "),        w1);
    DTextLabelName* const size         = new DTextLabelName(i18n("Size: "),        w1);
    DTextLabelName* const owner        = new DTextLabelName(i18n("Owner: "),       w1);
    DTextLabelName* const permissions  = new DTextLabelName(i18n("Permissions: "), w1);

    d->labelFile                       = new DTextLabelValue(QString(), w1);
    d->labelFolder                     = new DTextLabelValue(QString(), w1);
    d->labelFileModifiedDate           = new DTextLabelValue(QString(), w1);
    d->labelFileSize                   = new DTextLabelValue(QString(), w1);
    d->labelFileOwner                  = new DTextLabelValue(QString(), w1);
    d->labelFilePermissions            = new DTextLabelValue(QString(), w1);

    glay1->addWidget(file,                     0, 0, 1, 1);
    glay1->addWidget(d->labelFile,             0, 1, 1, 1);
    glay1->addWidget(folder,                   1, 0, 1, 1);
    glay1->addWidget(d->labelFolder,           1, 1, 1, 1);
    glay1->addWidget(modifiedDate,             2, 0, 1, 1);
    glay1->addWidget(d->labelFileModifiedDate, 2, 1, 1, 1);
    glay1->addWidget(size,                     3, 0, 1, 1);
    glay1->addWidget(d->labelFileSize,         3, 1, 1, 1);
    glay1->addWidget(owner,                    4, 0, 1, 1);
    glay1->addWidget(d->labelFileOwner,        4, 1, 1, 1);
    glay1->addWidget(permissions,              5, 0, 1, 1);
    glay1->addWidget(d->labelFilePermissions,  5, 1, 1, 1);
    glay1->setContentsMargins(spacing, spacing, spacing, spacing);
    glay1->setColumnStretch(0, 10);
    glay1->setColumnStretch(1, 10);
    glay1->setSpacing(0);

    insertItem(ItemPropertiesTab::Private::FileProperties,
               w1, QIcon::fromTheme(QLatin1String("dialog-information")),
               i18n("File Properties"), QLatin1String("FileProperties"), true);

    // --------------------------------------------------

    QWidget* const w2                = new QWidget(this);
    QGridLayout* const glay2         = new QGridLayout(w2);

    DTextLabelName* const mime       = new DTextLabelName(i18n("Type: "),         w2);
    DTextLabelName* const dimensions = new DTextLabelName(i18n("Dimensions: "),   w2);
    DTextLabelName* const ratio      = new DTextLabelName(i18n("Aspect Ratio: "), w2);
    DTextLabelName* const bitDepth   = new DTextLabelName(i18n("Bit depth: "),    w2);
    DTextLabelName* const colorMode  = new DTextLabelName(i18n("Color mode: "),   w2);

    d->labelImageMime                = new DTextLabelValue(QString(), w2);
    d->labelImageDimensions          = new DTextLabelValue(QString(), w2);
    d->labelImageRatio               = new DTextLabelValue(QString(), w2);
    d->labelImageBitDepth            = new DTextLabelValue(QString(), w2);
    d->labelImageColorMode           = new DTextLabelValue(QString(), w2);

    glay2->addWidget(mime,                    0, 0, 1, 1);
    glay2->addWidget(d->labelImageMime,       0, 1, 1, 1);
    glay2->addWidget(dimensions,              1, 0, 1, 1);
    glay2->addWidget(d->labelImageDimensions, 1, 1, 1, 1);
    glay2->addWidget(ratio,                   2, 0, 1, 1);
    glay2->addWidget(d->labelImageRatio,      2, 1, 1, 1);
    glay2->addWidget(bitDepth,                3, 0, 1, 1);
    glay2->addWidget(d->labelImageBitDepth,   3, 1, 1, 1);
    glay2->addWidget(colorMode,               4, 0, 1, 1);
    glay2->addWidget(d->labelImageColorMode,  4, 1, 1, 1);
    glay2->setContentsMargins(spacing, spacing, spacing, spacing);
    glay2->setColumnStretch(0, 10);
    glay2->setColumnStretch(1, 10);
    glay2->setSpacing(0);

    insertItem(ItemPropertiesTab::Private::ImageProperties,
               w2, QIcon::fromTheme(QLatin1String("view-preview")),
               i18n("Item Properties"), QLatin1String("ItemProperties"), true);

    // --------------------------------------------------

    QWidget* const w3                  = new QWidget(this);
    QGridLayout* const glay3           = new QGridLayout(w3);

    DTextLabelName* const make         = new DTextLabelName(i18n("Make: "),          w3);
    DTextLabelName* const model        = new DTextLabelName(i18n("Model: "),         w3);
    DTextLabelName* const photoDate    = new DTextLabelName(i18n("Created: "),       w3);
    DTextLabelName* const lens         = new DTextLabelName(i18n("Lens: "),          w3);
    DTextLabelName* const aperture     = new DTextLabelName(i18n("Aperture: "),      w3);
    DTextLabelName* const focalLength  = new DTextLabelName(i18n("Focal: "),         w3);
    DTextLabelName* const exposureTime = new DTextLabelName(i18n("Exposure: "),      w3);
    DTextLabelName* const sensitivity  = new DTextLabelName(i18n("Sensitivity: "),   w3);
    DTextLabelName* const exposureMode = new DTextLabelName(i18n("Mode/Program: "),  w3);
    DTextLabelName* const flash        = new DTextLabelName(i18n("Flash: "),         w3);
    DTextLabelName* const whiteBalance = new DTextLabelName(i18n("White balance: "), w3);

    d->labelPhotoMake                  = new DTextLabelValue(QString(), w3);
    d->labelPhotoModel                 = new DTextLabelValue(QString(), w3);
    d->labelPhotoDateTime              = new DTextLabelValue(QString(), w3);
    d->labelPhotoLens                  = new DTextLabelValue(QString(), w3);
    d->labelPhotoAperture              = new DTextLabelValue(QString(), w3);
    d->labelPhotoFocalLength           = new DTextLabelValue(QString(), w3);
    d->labelPhotoExposureTime          = new DTextLabelValue(QString(), w3);
    d->labelPhotoSensitivity           = new DTextLabelValue(QString(), w3);
    d->labelPhotoExposureMode          = new DTextLabelValue(QString(), w3);
    d->labelPhotoFlash                 = new DTextLabelValue(QString(), w3);
    d->labelPhotoWhiteBalance          = new DTextLabelValue(QString(), w3);

    glay3->addWidget(make,                      0,  0, 1, 1);
    glay3->addWidget(d->labelPhotoMake,         0,  1, 1, 1);
    glay3->addWidget(model,                     1,  0, 1, 1);
    glay3->addWidget(d->labelPhotoModel,        1,  1, 1, 1);
    glay3->addWidget(photoDate,                 2,  0, 1, 1);
    glay3->addWidget(d->labelPhotoDateTime,     2,  1, 1, 1);
    glay3->addWidget(lens,                      3,  0, 1, 1);
    glay3->addWidget(d->labelPhotoLens,         3,  1, 1, 1);
    glay3->addWidget(aperture,                  4,  0, 1, 1);
    glay3->addWidget(d->labelPhotoAperture,     4,  1, 1, 1);
    glay3->addWidget(focalLength,               5,  0, 1, 1);
    glay3->addWidget(d->labelPhotoFocalLength,  5,  1, 1, 1);
    glay3->addWidget(exposureTime,              6,  0, 1, 1);
    glay3->addWidget(d->labelPhotoExposureTime, 6,  1, 1, 1);
    glay3->addWidget(sensitivity,               7,  0, 1, 1);
    glay3->addWidget(d->labelPhotoSensitivity,  7,  1, 1, 1);
    glay3->addWidget(exposureMode,              8,  0, 1, 1);
    glay3->addWidget(d->labelPhotoExposureMode, 8,  1, 1, 1);
    glay3->addWidget(flash,                     9,  0, 1, 1);
    glay3->addWidget(d->labelPhotoFlash,        9,  1, 1, 1);
    glay3->addWidget(whiteBalance,              10, 0, 1, 1);
    glay3->addWidget(d->labelPhotoWhiteBalance, 10, 1, 1, 1);
    glay3->setContentsMargins(spacing, spacing, spacing, spacing);
    glay3->setColumnStretch(0, 10);
    glay3->setColumnStretch(1, 10);
    glay3->setSpacing(0);

    insertItem(ItemPropertiesTab::Private::PhotoProperties,
               w3, QIcon::fromTheme(QLatin1String("camera-photo")),
               i18n("Photograph Properties"), QLatin1String("PhotographProperties"), true);

    // --------------------------------------------------

    QWidget* const w4                      = new QWidget(this);
    QGridLayout* const glay4               = new QGridLayout(w4);

    DTextLabelName* const aspectRatio      = new DTextLabelName(i18n("Aspect Ratio: "),       w4);
    DTextLabelName* const duration         = new DTextLabelName(i18n("Duration: "),           w4);
    DTextLabelName* const frameRate        = new DTextLabelName(i18n("Frame Rate: "),         w4);
    DTextLabelName* const videoCodec       = new DTextLabelName(i18n("Video Codec: "),        w4);
    DTextLabelName* const audioBitRate     = new DTextLabelName(i18n("Audio Bit Rate: "),     w4);
    DTextLabelName* const audioChannelType = new DTextLabelName(i18n("Audio Channel Type: "), w4);
    DTextLabelName* const audioCodec       = new DTextLabelName(i18n("Audio Codec: "),        w4);

    d->labelVideoAspectRatio               = new DTextLabelValue(QString(), w4);
    d->labelVideoDuration                  = new DTextLabelValue(QString(), w4);
    d->labelVideoFrameRate                 = new DTextLabelValue(QString(), w4);
    d->labelVideoVideoCodec                = new DTextLabelValue(QString(), w4);
    d->labelVideoAudioBitRate              = new DTextLabelValue(QString(), w4);
    d->labelVideoAudioChannelType          = new DTextLabelValue(QString(), w4);
    d->labelVideoAudioCodec                = new DTextLabelValue(QString(), w4);

    glay4->addWidget(aspectRatio,                   0, 0, 1, 1);
    glay4->addWidget(d->labelVideoAspectRatio,      0, 1, 1, 1);
    glay4->addWidget(duration,                      1, 0, 1, 1);
    glay4->addWidget(d->labelVideoDuration,         1, 1, 1, 1);
    glay4->addWidget(frameRate,                     2, 0, 1, 1);
    glay4->addWidget(d->labelVideoFrameRate,        2, 1, 1, 1);
    glay4->addWidget(videoCodec,                    3, 0, 1, 1);
    glay4->addWidget(d->labelVideoVideoCodec,       3, 1, 1, 1);
    glay4->addWidget(audioBitRate,                  4, 0, 1, 1);
    glay4->addWidget(d->labelVideoAudioBitRate,     4, 1, 1, 1);
    glay4->addWidget(audioChannelType,              5, 0, 1, 1);
    glay4->addWidget(d->labelVideoAudioChannelType, 5, 1, 1, 1);
    glay4->addWidget(audioCodec,                    6, 0, 1, 1);
    glay4->addWidget(d->labelVideoAudioCodec,       6, 1, 1, 1);
    glay4->setContentsMargins(spacing, spacing, spacing, spacing);
    glay4->setColumnStretch(0, 10);
    glay4->setColumnStretch(1, 10);
    glay4->setSpacing(0);

    insertItem(ItemPropertiesTab::Private::VideoProperties,
               w4, QIcon::fromTheme(QLatin1String("video-x-generic")),
               i18n("Audio/Video Properties"), QLatin1String("VideoProperties"), true);

    // --------------------------------------------------

    QWidget* const w5        = new QWidget(this);
    QGridLayout* const glay5 = new QGridLayout(w5);

    d->caption               = new DTextLabelName(i18n("Caption: "),     w5);
    d->pickLabel             = new DTextLabelName(i18n("Pick label: "),  w5);
    d->colorLabel            = new DTextLabelName(i18n("Color label: "), w5);
    d->rating                = new DTextLabelName(i18n("Rating: "),      w5);
    d->tags                  = new DTextLabelName(i18n("Tags: "),        w5);

    d->labelCaption          = new DTextLabelValue(QString(), w5);
    d->labelPickLabel        = new DTextLabelValue(QString(), w5);
    d->labelColorLabel       = new DTextLabelValue(QString(), w5);
    d->labelRating           = new DTextLabelValue(QString(), w5);
    d->labelTags             = new DTextLabelValue(QString(), w5);
    d->labelTags->setElideMode(Qt::ElideLeft);

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
    glay5->setContentsMargins(spacing, spacing, spacing, spacing);
    glay5->setColumnStretch(0, 10);
    glay5->setColumnStretch(1, 10);
    glay5->setSpacing(0);

    insertItem(ItemPropertiesTab::Private::digiKamProperties,
               w5, QIcon::fromTheme(QLatin1String("edit-text-frame-update")),
               i18n("digiKam Properties"), QLatin1String("DigikamProperties"), true);

    // --------------------------------------------------

    addStretch();
}

ItemPropertiesTab::~ItemPropertiesTab()
{
    delete d;
}

void ItemPropertiesTab::setCurrentURL(const QUrl& url)
{
    if (url.isEmpty())
    {
        d->labelFile->setAdjustedText();
        d->labelFolder->setAdjustedText();
        d->labelFileModifiedDate->setAdjustedText();
        d->labelFileSize->setAdjustedText();
        d->labelFileOwner->setAdjustedText();
        d->labelFilePermissions->setAdjustedText();

        d->labelImageMime->setAdjustedText();
        d->labelImageDimensions->setAdjustedText();
        d->labelImageRatio->setAdjustedText();
        d->labelImageBitDepth->setAdjustedText();
        d->labelImageColorMode->setAdjustedText();

        d->labelPhotoMake->setAdjustedText();
        d->labelPhotoModel->setAdjustedText();
        d->labelPhotoDateTime->setAdjustedText();
        d->labelPhotoLens->setAdjustedText();
        d->labelPhotoAperture->setAdjustedText();
        d->labelPhotoFocalLength->setAdjustedText();
        d->labelPhotoExposureTime->setAdjustedText();
        d->labelPhotoSensitivity->setAdjustedText();
        d->labelPhotoExposureMode->setAdjustedText();
        d->labelPhotoFlash->setAdjustedText();
        d->labelPhotoWhiteBalance->setAdjustedText();

        d->labelCaption->setAdjustedText();
        d->labelPickLabel->setAdjustedText();
        d->labelColorLabel->setAdjustedText();
        d->labelRating->setAdjustedText();
        d->labelTags->setAdjustedText();

        d->labelVideoAspectRatio->setAdjustedText();
        d->labelVideoDuration->setAdjustedText();
        d->labelVideoFrameRate->setAdjustedText();
        d->labelVideoVideoCodec->setAdjustedText();
        d->labelVideoAudioBitRate->setAdjustedText();
        d->labelVideoAudioChannelType->setAdjustedText();
        d->labelVideoAudioCodec->setAdjustedText();

        setEnabled(false);
        return;
    }

    setEnabled(true);

    d->labelFile->setAdjustedText(url.fileName());
    d->labelFolder->setAdjustedText(QDir::toNativeSeparators(url.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).toLocalFile()));
}

void ItemPropertiesTab::setPhotoInfoDisable(const bool b)
{
    if (b)
    {
        widget(ItemPropertiesTab::Private::PhotoProperties)->hide();
    }
    else
    {
        widget(ItemPropertiesTab::Private::PhotoProperties)->show();
    }
}

void ItemPropertiesTab::setVideoInfoDisable(const bool b)
{
    if (b)
    {
        widget(ItemPropertiesTab::Private::VideoProperties)->hide();
    }
    else
    {
        widget(ItemPropertiesTab::Private::VideoProperties)->show();
    }
}

void ItemPropertiesTab::setFileModifiedDate(const QString& str)
{
    d->labelFileModifiedDate->setAdjustedText(str);
}

void ItemPropertiesTab::setFileSize(const QString& str)
{
    d->labelFileSize->setAdjustedText(str);
}

void ItemPropertiesTab::setFileOwner(const QString& str)
{
    d->labelFileOwner->setAdjustedText(str);
}

void ItemPropertiesTab::setFilePermissions(const QString& str)
{
    d->labelFilePermissions->setAdjustedText(str);
}

void ItemPropertiesTab::setImageMime(const QString& str)
{
    d->labelImageMime->setAdjustedText(str);
}

void ItemPropertiesTab::setItemDimensions(const QString& str)
{
    d->labelImageDimensions->setAdjustedText(str);
}

void ItemPropertiesTab::setImageRatio(const QString& str)
{
    d->labelImageRatio->setAdjustedText(str);
}

void ItemPropertiesTab::setImageBitDepth(const QString& str)
{
    d->labelImageBitDepth->setAdjustedText(str);
}

void ItemPropertiesTab::setImageColorMode(const QString& str)
{
    d->labelImageColorMode->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoMake(const QString& str)
{
    d->labelPhotoMake->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoModel(const QString& str)
{
    d->labelPhotoModel->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoDateTime(const QString& str)
{
    d->labelPhotoDateTime->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoLens(const QString& str)
{
    d->labelPhotoLens->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoAperture(const QString& str)
{
    d->labelPhotoAperture->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoFocalLength(const QString& str)
{
    d->labelPhotoFocalLength->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoExposureTime(const QString& str)
{
    d->labelPhotoExposureTime->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoSensitivity(const QString& str)
{
    d->labelPhotoSensitivity->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoExposureMode(const QString& str)
{
    d->labelPhotoExposureMode->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoFlash(const QString& str)
{
    d->labelPhotoFlash->setAdjustedText(str);
}

void ItemPropertiesTab::setPhotoWhiteBalance(const QString& str)
{
    d->labelPhotoWhiteBalance->setAdjustedText(str);
}

void ItemPropertiesTab::showOrHideCaptionAndTags()
{
    bool hasCaption    = !d->labelCaption->adjustedText().isEmpty();
    bool hasPickLabel  = !d->labelPickLabel->adjustedText().isEmpty();
    bool hasColorLabel = !d->labelColorLabel->adjustedText().isEmpty();
    bool hasRating     = !d->labelRating->adjustedText().isEmpty();
    bool hasTags       = !d->labelTags->adjustedText().isEmpty();

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

    widget(ItemPropertiesTab::Private::digiKamProperties)->setVisible(hasCaption || hasRating || hasTags || hasPickLabel || hasColorLabel);
}

void ItemPropertiesTab::setCaption(const QString& str)
{
    d->labelCaption->setAdjustedText(str);
}

void ItemPropertiesTab::setColorLabel(int colorId)
{
    if (colorId == NoColorLabel)
    {
        d->labelColorLabel->setAdjustedText(QString());
    }
    else
    {
        d->labelColorLabel->setAdjustedText(ColorLabelWidget::labelColorName((ColorLabel)colorId));
    }
}

void ItemPropertiesTab::setPickLabel(int pickId)
{
    if (pickId == NoPickLabel)
    {
        d->labelPickLabel->setAdjustedText(QString());
    }
    else
    {
        d->labelPickLabel->setAdjustedText(PickLabelWidget::labelPickName((PickLabel)pickId));
    }
}

void ItemPropertiesTab::setRating(int rating)
{
    QString str;

    if ((rating > RatingMin) && (rating <= RatingMax))
    {
        str = QLatin1Char(' ');

        for (int i = 0 ; i < rating ; ++i)
        {
            str += QChar(0x2730);
            str += QLatin1Char(' ');
        }
    }

    d->labelRating->setAdjustedText(str);
}

void ItemPropertiesTab::setVideoAspectRatio(const QString& str)
{
    d->labelVideoAspectRatio->setAdjustedText(str);
}

void ItemPropertiesTab::setVideoAudioBitRate(const QString& str)
{
    // use string given as parameter by default because it contains the value for "unavailable" if needed
    QString audioBitRateString = str;
    bool ok                    = false;
    const int audioBitRateInt  = str.toInt(&ok);

    if (ok)
    {
        audioBitRateString = QLocale().toString(audioBitRateInt);
    }

    d->labelVideoAudioBitRate->setAdjustedText(audioBitRateString);
}

void ItemPropertiesTab::setVideoAudioChannelType(const QString& str)
{
    d->labelVideoAudioChannelType->setAdjustedText(str);
}

void ItemPropertiesTab::setVideoAudioCodec(const QString& str)
{
    d->labelVideoAudioCodec->setAdjustedText(str);
}

void ItemPropertiesTab::setVideoDuration(const QString& str)
{
    // duration is given as a string in milliseconds
    // use string given as parameter by default because it contains the value for "unavailable" if needed
    QString durationString = str;
    bool ok                = false;
    const int durationVal  = str.toInt(&ok);

    if (ok)
    {
        unsigned int r, d, h, m, s, f;
        r = qAbs(durationVal);
        d = r / 86400000;
        r = r % 86400000;
        h = r / 3600000;
        r = r % 3600000;
        m = r / 60000;
        r = r % 60000;
        s = r / 1000;
        f = r % 1000;

        durationString = QString().sprintf("%d.%02d:%02d:%02d.%03d", d, h, m, s, f);
    }

    d->labelVideoDuration->setAdjustedText(durationString);
}

void ItemPropertiesTab::setVideoFrameRate(const QString& str)
{
    // use string given as parameter by default because it contains the value for "unavailable" if needed
    QString frameRateString = str;
    bool ok;
    const double frameRateDouble = str.toDouble(&ok);

    if (ok)
    {
        frameRateString = QLocale().toString(frameRateDouble) + i18n(" fps");
    }

    d->labelVideoFrameRate->setAdjustedText(frameRateString);
}

void ItemPropertiesTab::setVideoVideoCodec(const QString& str)
{
    d->labelVideoVideoCodec->setAdjustedText(str);
}

void ItemPropertiesTab::setTags(const QStringList& tagPaths, const QStringList& tagNames)
{
    Q_UNUSED(tagNames);
    d->labelTags->setAdjustedText(shortenedTagPaths(tagPaths).join(QLatin1Char('\n')));
}

typedef QPair<QString, QVariant> PathValuePair;

static bool naturalLessThan(const PathValuePair& a, const PathValuePair& b)
{
    return (QCollator().compare(a.first, b.first) < 0);
}

QStringList ItemPropertiesTab::shortenedTagPaths(const QStringList& tagPaths, QList<QVariant>* identifiers)
{
    QList<PathValuePair> tagsSorted;

    if (identifiers)
    {
        for (int i = 0 ; i < tagPaths.size() ; ++i)
        {
            tagsSorted << PathValuePair(tagPaths.at(i), (*identifiers).at(i));
        }
    }
    else
    {
        for (int i = 0 ; i < tagPaths.size() ; ++i)
        {
            tagsSorted << PathValuePair(tagPaths.at(i), QVariant());
        }
    }

    std::stable_sort(tagsSorted.begin(), tagsSorted.end(), naturalLessThan);

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
        QStringList currentPath  = tagPath.split(QLatin1Char('/'), QString::SkipEmptyParts);
        QStringList previousPath = previous.split(QLatin1Char('/'), QString::SkipEmptyParts);
        int depth;

        for (depth = 0 ; depth < currentPath.size() && depth < previousPath.size() ; ++depth)
        {
            if (currentPath.at(depth) != previousPath.at(depth))
                break;
        }

        if (depth)
        {
            QString indent;
            indent.fill(QLatin1Char(' '), qMin(depth, 5));
            //indent += QChar(0x2026);
            shortenedPath = indent + tagPath.section(QLatin1Char('/'), depth);
        }

        shortenedPath.replace(QLatin1Char('/'), QLatin1String(" / "));
        tagsShortened << shortenedPath;
        previous = tagPath;

        if (identifiers)
        {
            (*identifiers) << pair.second;
        }
    }

    return tagsShortened;
}

void ItemPropertiesTab::shortenedMakeInfo(QString& make)
{
    make.remove(QLatin1String(" CORPORATION"),       Qt::CaseInsensitive);        // from Nikon, Pentax, and Olympus
    make.remove(QLatin1String("EASTMAN "),           Qt::CaseInsensitive);        // from Kodak
    make.remove(QLatin1String(" COMPANY"),           Qt::CaseInsensitive);        // from Kodak
    make.remove(QLatin1String(" OPTICAL CO.,LTD"),   Qt::CaseInsensitive);        // from Olympus
    make.remove(QLatin1String(" IMAGING CORP."),     Qt::CaseInsensitive);        // from Olympus
    make.remove(QLatin1String(" Techwin co.,Ltd."),  Qt::CaseInsensitive);        // from Samsung
    make.remove(QLatin1String("  Co.,Ltd."),         Qt::CaseInsensitive);        // from Minolta
    make.remove(QLatin1String(" Electric Co.,Ltd."), Qt::CaseInsensitive);        // from Sanyo
    make.remove(QLatin1String(" Electric Co.,Ltd"),  Qt::CaseInsensitive);        // from Sanyo
}

void ItemPropertiesTab::shortenedModelInfo(QString& model)
{
    model.remove(QLatin1String("Canon "),           Qt::CaseInsensitive);
    model.remove(QLatin1String("NIKON "),           Qt::CaseInsensitive);
    model.remove(QLatin1String("PENTAX "),          Qt::CaseInsensitive);
    model.remove(QLatin1String(" DIGITAL"),         Qt::CaseInsensitive);        // from Canon
    model.remove(QLatin1String("KODAK "),           Qt::CaseInsensitive);
    model.remove(QLatin1String(" CAMERA"),          Qt::CaseInsensitive);        // from Kodak
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
double ItemPropertiesTab::doubleToHumanReadableFraction(double val, long* num, long* den, long maxden)
{
    double x = val;
    long   m[2][2];
    long   ai;

    // Initialize matrix

    m[0][0] = m[1][1] = 1;
    m[0][1] = m[1][0] = 0;

    // Loop finding terms until denominator gets too big

    while (m[1][0] * (ai = (long)x) + m[1][1] <= maxden)
    {
        long t  = m[0][0] * ai + m[0][1];
        m[0][1] = m[0][0];
        m[0][0] = t;
        t       = m[1][0] * ai + m[1][1];
        m[1][1] = m[1][0];
        m[1][0] = t;

        if (x == (double)ai)
        {
            break;     // division by zero
        }

        x       = 1 / (x - (double)ai);

        if (x > (double)0x7FFFFFFF)
        {
            break;     // representation failure
        }
    }

    // Now remaining x is between 0 and 1/ai
    // Approx as either 0 or 1/m where m is max that will fit in maxden

    *num = m[0][0];
    *den = m[1][0];

    // Return approximation error

    return (val - ((double)m[0][0] / (double)m[1][0]));
}

bool ItemPropertiesTab::aspectRatioToString(int width, int height, QString& arString)
{
    if ((width == 0) || (height == 0))
    {
        return false;
    }

    double ratio  = (double)qMax(width, height) / (double)qMin(width, height);
    long   num    = 0;
    long   den    = 0;

    doubleToHumanReadableFraction(ratio, &num, &den, 10);

    double aratio = (double)qMax(num, den) / (double)qMin(num, den);

    arString = i18nc("width : height (Aspect Ratio)", "%1:%2 (%3)",
                     (width > height) ? num : den,
                     (width > height) ? den : num,
                     QLocale().toString(aratio, 'g', 2));

    return true;
}

QString ItemPropertiesTab::permissionsString(const QFileInfo& fi)
{
    QString str;
    QFile::Permissions perms = fi.permissions();

    str.append(fi.isSymLink()                    ? QLatin1String("l") : QLatin1String("-"));

    str.append((perms & QFileDevice::ReadOwner)  ? QLatin1String("r") : QLatin1String("-"));
    str.append((perms & QFileDevice::WriteOwner) ? QLatin1String("w") : QLatin1String("-"));
    str.append((perms & QFileDevice::ExeOwner)   ? QLatin1String("x") : QLatin1String("-"));

    str.append((perms & QFileDevice::ReadGroup)  ? QLatin1String("r") : QLatin1String("-"));
    str.append((perms & QFileDevice::WriteGroup) ? QLatin1String("w") : QLatin1String("-"));
    str.append((perms & QFileDevice::ExeGroup)   ? QLatin1String("x") : QLatin1String("-"));

    str.append((perms & QFileDevice::ReadOther)  ? QLatin1String("r") : QLatin1String("-"));
    str.append((perms & QFileDevice::WriteOther) ? QLatin1String("w") : QLatin1String("-"));
    str.append((perms & QFileDevice::ExeOther)   ? QLatin1String("x") : QLatin1String("-"));

    return str;
}

QString ItemPropertiesTab::humanReadableBytesCount(qint64 bytes, bool si)
{
    int unit        = si ? 1000 : 1024;
    QString byteStr = i18nc("unit file size in bytes", "B");
    QString ret     = QString::number(bytes);

    if (bytes >= unit)
    {
        int exp     = (int)(qLn(bytes) / qLn(unit));
        QString pre = QString(si ? QLatin1String("kMGTPEZY") : QLatin1String("KMGTPEZY")).at(exp-1) + (si ? QLatin1String("") : QLatin1String("i"));
        ret.sprintf("%.1f %s", bytes / qPow(unit, exp), pre.toLatin1().constData());
    }

    return (QString::fromUtf8("%1%2").arg(ret).arg(byteStr));
}

} // namespace Digikam
