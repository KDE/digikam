/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-19
 * Description : A tab to display general image information
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagepropertiestab.h"
#include "imagepropertiestab.moc"

// Qt includes

#include <QGridLayout>
#include <QStyle>
#include <QFile>
#include <QPixmap>

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kdialog.h>

// Local includes

#include "imagepropertiestxtlabel.h"

namespace Digikam
{

class ImagePropertiesTabPriv
{
public:

    ImagePropertiesTabPriv()
    {
        file                   = 0;
        folder                 = 0;
        modifiedDate           = 0;
        size                   = 0;
        owner                  = 0;
        permissions            = 0;
        mime                   = 0;
        dimensions             = 0;
        compression            = 0;
        bitDepth               = 0;
        colorMode              = 0;
        make                   = 0;
        model                  = 0;
        photoDate              = 0;
        lens                   = 0;
        aperture               = 0;
        focalLength            = 0;
        exposureTime           = 0;
        sensitivity            = 0;
        exposureMode           = 0;
        flash                  = 0;
        whiteBalance           = 0;
        labelFile              = 0;
        labelFolder            = 0;
        labelFileModifiedDate  = 0;
        labelFileSize          = 0;
        labelFileOwner         = 0;
        labelFilePermissions   = 0;
        labelImageMime         = 0;
        labelImageDimensions   = 0;
        labelImageCompression  = 0;
        labelImageBitDepth     = 0;
        labelImageColorMode    = 0;
        labelPhotoMake         = 0;
        labelPhotoModel        = 0;
        labelPhotoDateTime     = 0;
        labelPhotoLens         = 0;
        labelPhotoAperture     = 0;
        labelPhotoFocalLength  = 0;
        labelPhotoExposureTime = 0;
        labelPhotoSensitivity  = 0;
        labelPhotoExposureMode = 0;
        labelPhotoFlash        = 0;
        labelPhotoWhiteBalance = 0;
    }

    DTextLabelName  *file;
    DTextLabelName  *folder;
    DTextLabelName  *modifiedDate;
    DTextLabelName  *size;
    DTextLabelName  *owner;
    DTextLabelName  *permissions;

    DTextLabelName  *mime;
    DTextLabelName  *dimensions;
    DTextLabelName  *compression;
    DTextLabelName  *bitDepth;
    DTextLabelName  *colorMode;

    DTextLabelName  *make;
    DTextLabelName  *model;
    DTextLabelName  *photoDate;
    DTextLabelName  *lens;
    DTextLabelName  *aperture;
    DTextLabelName  *focalLength;
    DTextLabelName  *exposureTime;
    DTextLabelName  *sensitivity;
    DTextLabelName  *exposureMode;
    DTextLabelName  *flash;
    DTextLabelName  *whiteBalance;

    DTextLabelValue *labelFile;
    DTextLabelValue *labelFolder;
    DTextLabelValue *labelFileModifiedDate;
    DTextLabelValue *labelFileSize;
    DTextLabelValue *labelFileOwner;
    DTextLabelValue *labelFilePermissions;

    DTextLabelValue *labelImageMime;
    DTextLabelValue *labelImageDimensions;
    DTextLabelValue *labelImageCompression;
    DTextLabelValue *labelImageBitDepth;
    DTextLabelValue *labelImageColorMode;

    DTextLabelValue *labelPhotoMake;
    DTextLabelValue *labelPhotoModel;
    DTextLabelValue *labelPhotoDateTime;
    DTextLabelValue *labelPhotoLens;
    DTextLabelValue *labelPhotoAperture;
    DTextLabelValue *labelPhotoFocalLength;
    DTextLabelValue *labelPhotoExposureTime;
    DTextLabelValue *labelPhotoSensitivity;
    DTextLabelValue *labelPhotoExposureMode;
    DTextLabelValue *labelPhotoFlash;
    DTextLabelValue *labelPhotoWhiteBalance;
};

ImagePropertiesTab::ImagePropertiesTab(QWidget* parent)
                  : DExpanderBox(parent), d(new ImagePropertiesTabPriv)
{
    setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    setLineWidth( style()->pixelMetric(QStyle::PM_DefaultFrameWidth) );

    // --------------------------------------------------

    QWidget *w1               = new QWidget(this);
    QGridLayout *glay1        = new QGridLayout(w1);

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

    QWidget *w2               = new QWidget(this);
    QGridLayout *glay2        = new QGridLayout(w2);

    d->mime                   = new DTextLabelName(i18n("Type: "),        w2);
    d->dimensions             = new DTextLabelName(i18n("Dimensions: "),  w2);
    d->compression            = new DTextLabelName(i18n("Compression: "), w2);
    d->bitDepth               = new DTextLabelName(i18n("Bit depth: "),   w2);
    d->colorMode              = new DTextLabelName(i18n("Color mode: "),  w2);

    d->labelImageMime         = new DTextLabelValue(0, w2);
    d->labelImageDimensions   = new DTextLabelValue(0, w2);
    d->labelImageCompression  = new DTextLabelValue(0, w2);
    d->labelImageBitDepth     = new DTextLabelValue(0, w2);
    d->labelImageColorMode    = new DTextLabelValue(0, w2);

    glay2->addWidget(d->mime,                   0, 0, 1, 1);
    glay2->addWidget(d->labelImageMime,         0, 1, 1, 1);
    glay2->addWidget(d->dimensions,             1, 0, 1, 1);
    glay2->addWidget(d->labelImageDimensions,   1, 1, 1, 1);
    glay2->addWidget(d->compression,            2, 0, 1, 1);
    glay2->addWidget(d->labelImageCompression,  2, 1, 1, 1);
    glay2->addWidget(d->bitDepth,               3, 0, 1, 1);
    glay2->addWidget(d->labelImageBitDepth,     3, 1, 1, 1);
    glay2->addWidget(d->colorMode,              4, 0, 1, 1);
    glay2->addWidget(d->labelImageColorMode,    4, 1, 1, 1);
    glay2->setMargin(KDialog::spacingHint());
    glay2->setSpacing(0);
    glay2->setColumnStretch(1, 10);

    addItem(w2, SmallIcon("image-x-generic"),
            i18n("Image Properties"), QString("ImageProperties"), true);

    // --------------------------------------------------

    QWidget *w3               = new QWidget(this);
    QGridLayout *glay3        = new QGridLayout(w3);

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

    addItem(w3, SmallIcon("applications-graphics"),
            i18n("Photograph Properties"), QString("PhotographProperties"), true);

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
        d->labelFile->setText(QString());
        d->labelFolder->setText(QString());
        d->labelFileModifiedDate->setText(QString());
        d->labelFileSize->setText(QString());
        d->labelFileOwner->setText(QString());
        d->labelFilePermissions->setText(QString());

        d->labelImageMime->setText(QString());
        d->labelImageDimensions->setText(QString());
        d->labelImageCompression->setText(QString());
        d->labelImageBitDepth->setText(QString());
        d->labelImageColorMode->setText(QString());

        d->labelPhotoMake->setText(QString());
        d->labelPhotoModel->setText(QString());
        d->labelPhotoDateTime->setText(QString());
        d->labelPhotoLens->setText(QString());
        d->labelPhotoAperture->setText(QString());
        d->labelPhotoFocalLength->setText(QString());
        d->labelPhotoExposureTime->setText(QString());
        d->labelPhotoSensitivity->setText(QString());
        d->labelPhotoExposureMode->setText(QString());
        d->labelPhotoFlash->setText(QString());
        d->labelPhotoWhiteBalance->setText(QString());

        setEnabled(false);
        return;
    }

    setEnabled(true);

    d->labelFile->setText(url.fileName());
    d->labelFolder->setText(url.directory());
}

void ImagePropertiesTab::setPhotoInfoDisable(bool b)
{
    if (b)
        widget(2)->hide();
    else
        widget(2)->show();
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

void ImagePropertiesTab::setImageCompression(const QString& str)
{
    d->compression->show();
    d->labelImageCompression->show();

    d->labelImageCompression->setText(str);
}

void ImagePropertiesTab::hideImageCompression()
{
    d->compression->hide();
    d->labelImageCompression->hide();
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

}  // namespace Digikam
