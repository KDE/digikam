/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-19
 * Description : A tab to display general image information
 *
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

class ImagePropertiesTab::ImagePropertiesTabPriv
{
public:

    ImagePropertiesTabPriv() :
        file(0),
        folder(0),
        modifiedDate(0),
        size(0),
        owner(0),
        permissions(0),
        mime(0),
        dimensions(0),
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
        labelRating(0)
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
};

ImagePropertiesTab::ImagePropertiesTab(QWidget* parent)
    : RExpanderBox(parent), d(new ImagePropertiesTabPriv)
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

    addItem(w1, SmallIcon("dialog-information"),
            i18n("File Properties"), QString("FileProperties"), true);

    // --------------------------------------------------

    QWidget* const w2         = new QWidget(this);
    QGridLayout* const glay2  = new QGridLayout(w2);

    d->mime                   = new DTextLabelName(i18n("Type: "),        w2);
    d->dimensions             = new DTextLabelName(i18n("Dimensions: "),  w2);
    d->bitDepth               = new DTextLabelName(i18n("Bit depth: "),   w2);
    d->colorMode              = new DTextLabelName(i18n("Color mode: "),  w2);

    d->labelImageMime         = new DTextLabelValue(0, w2);
    d->labelImageDimensions   = new DTextLabelValue(0, w2);
    d->labelImageBitDepth     = new DTextLabelValue(0, w2);
    d->labelImageColorMode    = new DTextLabelValue(0, w2);

    glay2->addWidget(d->mime,                   0, 0, 1, 1);
    glay2->addWidget(d->labelImageMime,         0, 1, 1, 1);
    glay2->addWidget(d->dimensions,             1, 0, 1, 1);
    glay2->addWidget(d->labelImageDimensions,   1, 1, 1, 1);
    glay2->addWidget(d->bitDepth,               2, 0, 1, 1);
    glay2->addWidget(d->labelImageBitDepth,     2, 1, 1, 1);
    glay2->addWidget(d->colorMode,              3, 0, 1, 1);
    glay2->addWidget(d->labelImageColorMode,    3, 1, 1, 1);
    glay2->setMargin(KDialog::spacingHint());
    glay2->setSpacing(0);
    glay2->setColumnStretch(1, 10);

    addItem(w2, SmallIcon("image-x-generic"),
            i18n("Image Properties"), QString("ImageProperties"), true);

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

    glay3->addWidget(d->make,                   23, 0, 1, 1);
    glay3->addWidget(d->labelPhotoMake,         23, 1, 1, 1);
    glay3->addWidget(d->model,                  24, 0, 1, 1);
    glay3->addWidget(d->labelPhotoModel,        24, 1, 1, 1);
    glay3->addWidget(d->photoDate,              25, 0, 1, 1);
    glay3->addWidget(d->labelPhotoDateTime,     25, 1, 1, 1);
    glay3->addWidget(d->lens,                   26, 0, 1, 1);
    glay3->addWidget(d->labelPhotoLens,         26, 1, 1, 1);
    glay3->addWidget(d->aperture,               27, 0, 1, 1);
    glay3->addWidget(d->labelPhotoAperture,     27, 1, 1, 1);
    glay3->addWidget(d->focalLength,            28, 0, 1, 1);
    glay3->addWidget(d->labelPhotoFocalLength,  28, 1, 1, 1);
    glay3->addWidget(d->exposureTime,           29, 0, 1, 1);
    glay3->addWidget(d->labelPhotoExposureTime, 29, 1, 1, 1);
    glay3->addWidget(d->sensitivity,            30, 0, 1, 1);
    glay3->addWidget(d->labelPhotoSensitivity,  30, 1, 1, 1);
    glay3->addWidget(d->exposureMode,           31, 0, 1, 1);
    glay3->addWidget(d->labelPhotoExposureMode, 31, 1, 1, 1);
    glay3->addWidget(d->flash,                  32, 0, 1, 1);
    glay3->addWidget(d->labelPhotoFlash,        32, 1, 1, 1);
    glay3->addWidget(d->whiteBalance,           33, 0, 1, 1);
    glay3->addWidget(d->labelPhotoWhiteBalance, 33, 1, 1, 1);
    glay3->setColumnStretch(1, 10);
    glay3->setMargin(KDialog::spacingHint());
    glay3->setSpacing(0);

    addItem(w3, SmallIcon("camera-photo"),
            i18n("Photograph Properties"), QString("PhotographProperties"), true);

    // --------------------------------------------------

    QWidget* const w4         = new QWidget(this);
    QGridLayout* const glay4  = new QGridLayout(w4);

    d->caption                = new DTextLabelName(i18n("Caption: "),     w4);
    d->pickLabel              = new DTextLabelName(i18n("Pick label: "),  w4);
    d->colorLabel             = new DTextLabelName(i18n("Color label: "), w4);
    d->rating                 = new DTextLabelName(i18n("Rating: "),      w4);
    d->tags                   = new DTextLabelName(i18n("Tags: "),        w4);

    d->labelCaption           = new DTextLabelValue(0, w4);
    d->labelPickLabel         = new DTextLabelValue(0, w4);
    d->labelColorLabel        = new DTextLabelValue(0, w4);
    d->labelRating            = new DTextLabelValue(0, w4);
    d->labelTags              = new DTextLabelValue(0, w4);
    d->labelTags->setTextElideMode(Qt::ElideLeft);

    glay4->addWidget(d->caption,         0, 0, 1, 1);
    glay4->addWidget(d->labelCaption,    0, 1, 1, 1);
    glay4->addWidget(d->tags,            1, 0, 1, 1);
    glay4->addWidget(d->labelTags,       1, 1, 1, 1);
    glay4->addWidget(d->pickLabel,       2, 0, 1, 1);
    glay4->addWidget(d->labelPickLabel,  2, 1, 1, 1);
    glay4->addWidget(d->colorLabel,      3, 0, 1, 1);
    glay4->addWidget(d->labelColorLabel, 3, 1, 1, 1);
    glay4->addWidget(d->rating,          4, 0, 1, 1);
    glay4->addWidget(d->labelRating,     4, 1, 1, 1);
    glay4->setMargin(KDialog::spacingHint());
    glay4->setSpacing(0);
    glay4->setColumnStretch(1, 10);

    addItem(w4, SmallIcon("imagecomment"),
            i18n("digiKam Properties"), QString("DigikamProperties"), true);

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

        setEnabled(false);
        return;
    }

    setEnabled(true);

    d->labelFile->setText(url.fileName());
    d->labelFolder->setText(url.directory());
}

void ImagePropertiesTab::setPhotoInfoDisable(const bool b)
{
    if (b)
    {
        widget(2)->hide();
    }
    else
    {
        widget(2)->show();
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

    widget(3)->setVisible(hasCaption || hasRating || hasTags || hasPickLabel || hasColorLabel);
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
        str = " ";
        for (int i=0; i<rating; ++i)
        {
            str += QChar(0x2730);
            str += ' ';
        }
    }
    d->labelRating->setText(str);
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
        for (int i=0; i<tagPaths.size(); ++i)
        {
            tagsSorted << PathValuePair(tagPaths.at(i), (*identifiers).at(i));
        }
    }
    else
    {
        for (int i=0; i<tagPaths.size(); ++i)
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
    foreach (const PathValuePair& pair, tagsSorted)
    {
        const QString& tagPath = pair.first;
        QString shortenedPath = tagPath;

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

        shortenedPath.replace("/", " / ");
        tagsShortened << shortenedPath;
        previous = tagPath;

        if (identifiers)
        {
            (*identifiers) << pair.second;
        }
    }

    return tagsShortened;
}

}  // namespace Digikam
