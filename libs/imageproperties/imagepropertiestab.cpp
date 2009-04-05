/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-19
 * Description : A tab to display general image information
 *
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <kseparator.h>

// Local includes

#include "imagepropertiestxtlabel.h"

namespace Digikam
{

class ImagePropertiesTabPriv
{
public:

    ImagePropertiesTabPriv()
    {
        title                  = 0;
        file                   = 0;
        folder                 = 0;
        modifiedDate           = 0;
        size                   = 0;
        owner                  = 0;
        permissions            = 0;
        title2                 = 0;
        mime                   = 0;
        dimensions             = 0;
        compression            = 0;
        bitDepth               = 0;
        colorMode              = 0;
        title3                 = 0;
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

    QLabel          *title;
    DTextLabelName  *file;
    DTextLabelName  *folder;
    DTextLabelName  *modifiedDate;
    DTextLabelName  *size;
    DTextLabelName  *owner;
    DTextLabelName  *permissions;

    QLabel          *title2;
    DTextLabelName  *mime;
    DTextLabelName  *dimensions;
    DTextLabelName  *compression;
    DTextLabelName  *bitDepth;
    DTextLabelName  *colorMode;

    QLabel          *title3;
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
                  : QScrollArea(parent), d(new ImagePropertiesTabPriv)
{
    setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    setLineWidth( style()->pixelMetric(QStyle::PM_DefaultFrameWidth) );
    setWidgetResizable(true);

    QWidget *settingsArea       = new QWidget(viewport());
    QGridLayout *settingsLayout = new QGridLayout(settingsArea);
    setWidget(settingsArea);

    viewport()->setAutoFillBackground(false);
    settingsArea->setAutoFillBackground(false);

    // --------------------------------------------------

    d->title                  = new QLabel(i18n("<big><b>File Properties</b></big>"), settingsArea);
    d->file                   = new DTextLabelName(i18n("File: "), settingsArea);
    d->folder                 = new DTextLabelName(i18n("Folder: "), settingsArea);
    d->modifiedDate           = new DTextLabelName(i18n("Date: "), settingsArea);
    d->size                   = new DTextLabelName(i18n("Size: "), settingsArea);
    d->owner                  = new DTextLabelName(i18n("Owner: "), settingsArea);
    d->permissions            = new DTextLabelName(i18n("Permissions: "), settingsArea);

    KSeparator *line          = new KSeparator(Qt::Horizontal, settingsArea);
    d->title2                 = new QLabel(i18n("<big><b>Image Properties</b></big>"), settingsArea);
    d->mime                   = new DTextLabelName(i18n("Type: "), settingsArea);
    d->dimensions             = new DTextLabelName(i18n("Dimensions: "), settingsArea);
    d->compression            = new DTextLabelName(i18n("Compression: "), settingsArea);
    d->bitDepth               = new DTextLabelName(i18n("Bit depth: "), settingsArea);
    d->colorMode              = new DTextLabelName(i18n("Color mode: "), settingsArea);

    KSeparator *line2         = new KSeparator(Qt::Horizontal, settingsArea);
    d->title3                 = new QLabel(i18n("<big><b>Photograph Properties</b></big>"), settingsArea);
    d->make                   = new DTextLabelName(i18n("Make: "), settingsArea);
    d->model                  = new DTextLabelName(i18n("Model: "), settingsArea);
    d->photoDate              = new DTextLabelName(i18n("Created: "), settingsArea);
    d->lens                   = new DTextLabelName(i18n("Lens: "), settingsArea);
    d->aperture               = new DTextLabelName(i18n("Aperture: "), settingsArea);
    d->focalLength            = new DTextLabelName(i18n("Focal: "), settingsArea);
    d->exposureTime           = new DTextLabelName(i18n("Exposure: "), settingsArea);
    d->sensitivity            = new DTextLabelName(i18n("Sensitivity: "), settingsArea);
    d->exposureMode           = new DTextLabelName(i18n("Mode/Program: "), settingsArea);
    d->flash                  = new DTextLabelName(i18n("Flash: "), settingsArea);
    d->whiteBalance           = new DTextLabelName(i18n("White balance: "), settingsArea);

    d->labelFile              = new DTextLabelValue(0, settingsArea);
    d->labelFolder            = new DTextLabelValue(0, settingsArea);
    d->labelFileModifiedDate  = new DTextLabelValue(0, settingsArea);
    d->labelFileSize          = new DTextLabelValue(0, settingsArea);
    d->labelFileOwner         = new DTextLabelValue(0, settingsArea);
    d->labelFilePermissions   = new DTextLabelValue(0, settingsArea);

    d->labelImageMime         = new DTextLabelValue(0, settingsArea);
    d->labelImageDimensions   = new DTextLabelValue(0, settingsArea);
    d->labelImageCompression  = new DTextLabelValue(0, settingsArea);
    d->labelImageBitDepth     = new DTextLabelValue(0, settingsArea);
    d->labelImageColorMode    = new DTextLabelValue(0, settingsArea);

    d->labelPhotoMake         = new DTextLabelValue(0, settingsArea);
    d->labelPhotoModel        = new DTextLabelValue(0, settingsArea);
    d->labelPhotoDateTime     = new DTextLabelValue(0, settingsArea);
    d->labelPhotoLens         = new DTextLabelValue(0, settingsArea);
    d->labelPhotoAperture     = new DTextLabelValue(0, settingsArea);
    d->labelPhotoFocalLength  = new DTextLabelValue(0, settingsArea);
    d->labelPhotoExposureTime = new DTextLabelValue(0, settingsArea);
    d->labelPhotoSensitivity  = new DTextLabelValue(0, settingsArea);
    d->labelPhotoExposureMode = new DTextLabelValue(0, settingsArea);
    d->labelPhotoFlash        = new DTextLabelValue(0, settingsArea);
    d->labelPhotoWhiteBalance = new DTextLabelValue(0, settingsArea);

    d->title->setAlignment(Qt::AlignCenter);
    d->title2->setAlignment(Qt::AlignCenter);
    d->title3->setAlignment(Qt::AlignCenter);

    // --------------------------------------------------

    settingsLayout->addWidget(d->title,                   0, 0, 1, 2);
    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding),
                                                          1, 0, 1, 2);
    settingsLayout->addWidget(d->file,                    2, 0, 1, 1);
    settingsLayout->addWidget(d->labelFile,               2, 1, 1, 1);
    settingsLayout->addWidget(d->folder,                  3, 0, 1, 1);
    settingsLayout->addWidget(d->labelFolder,             3, 1, 1, 1);
    settingsLayout->addWidget(d->modifiedDate,            4, 0, 1, 1);
    settingsLayout->addWidget(d->labelFileModifiedDate,   4, 1, 1, 1);
    settingsLayout->addWidget(d->size,                    5, 0, 1, 1);
    settingsLayout->addWidget(d->labelFileSize,           5, 1, 1, 1);
    settingsLayout->addWidget(d->owner,                   6, 0, 1, 1);
    settingsLayout->addWidget(d->labelFileOwner,          6, 1, 1, 1);
    settingsLayout->addWidget(d->permissions,             7, 0, 1, 1);
    settingsLayout->addWidget(d->labelFilePermissions,    7, 1, 1, 1);

    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding),
                                                          8, 0, 1, 2);
    settingsLayout->addWidget(line,                       9, 0, 1, 2);
    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding),
                                                         10, 0, 1, 2);

    settingsLayout->addWidget(d->title2,                 11, 0, 1, 2);
    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding),
                                                         12, 0, 1, 2);
    settingsLayout->addWidget(d->mime,                   13, 0, 1, 1);
    settingsLayout->addWidget(d->labelImageMime,         13, 1, 1, 1);
    settingsLayout->addWidget(d->dimensions,             14, 0, 1, 1);
    settingsLayout->addWidget(d->labelImageDimensions,   14, 1, 1, 1);
    settingsLayout->addWidget(d->compression,            15, 0, 1, 1);
    settingsLayout->addWidget(d->labelImageCompression,  15, 1, 1, 1);
    settingsLayout->addWidget(d->bitDepth,               16, 0, 1, 1);
    settingsLayout->addWidget(d->labelImageBitDepth,     16, 1, 1, 1);
    settingsLayout->addWidget(d->colorMode,              17, 0, 1, 1);
    settingsLayout->addWidget(d->labelImageColorMode,    17, 1, 1, 1);

    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding),
                                                         18, 0, 1, 2);
    settingsLayout->addWidget(line2,                     19, 0, 1, 2);
    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding),
                                                         20, 0, 1, 2);

    settingsLayout->addWidget(d->title3,                 21, 0, 1, 2);
    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding),
                                                         22, 0, 1, 2);
    settingsLayout->addWidget(d->make,                   23, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoMake,         23, 1, 1, 1);
    settingsLayout->addWidget(d->model,                  24, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoModel,        24, 1, 1, 1);
    settingsLayout->addWidget(d->photoDate,              25, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoDateTime,     25, 1, 1, 1);
    settingsLayout->addWidget(d->lens,                   26, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoLens,         26, 1, 1, 1);
    settingsLayout->addWidget(d->aperture,               27, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoAperture,     27, 1, 1, 1);
    settingsLayout->addWidget(d->focalLength,            28, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoFocalLength,  28, 1, 1, 1);
    settingsLayout->addWidget(d->exposureTime,           29, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoExposureTime, 29, 1, 1, 1);
    settingsLayout->addWidget(d->sensitivity,            30, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoSensitivity,  30, 1, 1, 1);
    settingsLayout->addWidget(d->exposureMode,           31, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoExposureMode, 31, 1, 1, 1);
    settingsLayout->addWidget(d->flash,                  32, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoFlash,        32, 1, 1, 1);
    settingsLayout->addWidget(d->whiteBalance,           33, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoWhiteBalance, 33, 1, 1, 1);

    settingsLayout->setRowStretch(34, 10);
    settingsLayout->setColumnStretch(1, 10);
    settingsLayout->setMargin(KDialog::spacingHint());
    settingsLayout->setSpacing(0);
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
    {
        d->title3->hide();
        d->make->hide();
        d->model->hide();
        d->photoDate->hide();
        d->lens->hide();
        d->aperture->hide();
        d->focalLength->hide();
        d->exposureTime->hide();
        d->sensitivity->hide();
        d->exposureMode->hide();
        d->flash->hide();
        d->whiteBalance->hide();
        d->labelPhotoMake->hide();
        d->labelPhotoModel->hide();
        d->labelPhotoDateTime->hide();
        d->labelPhotoLens->hide();
        d->labelPhotoAperture->hide();
        d->labelPhotoFocalLength->hide();
        d->labelPhotoExposureTime->hide();
        d->labelPhotoSensitivity->hide();
        d->labelPhotoExposureMode->hide();
        d->labelPhotoFlash->hide();
        d->labelPhotoWhiteBalance->hide();
    }
    else
    {
        d->title3->show();
        d->make->show();
        d->model->show();
        d->photoDate->show();
        d->lens->show();
        d->aperture->show();
        d->focalLength->show();
        d->exposureTime->show();
        d->sensitivity->show();
        d->exposureMode->show();
        d->flash->show();
        d->whiteBalance->show();
        d->labelPhotoMake->show();
        d->labelPhotoModel->show();
        d->labelPhotoDateTime->show();
        d->labelPhotoLens->show();
        d->labelPhotoAperture->show();
        d->labelPhotoFocalLength->show();
        d->labelPhotoExposureTime->show();
        d->labelPhotoSensitivity->show();
        d->labelPhotoExposureMode->show();
        d->labelPhotoFlash->show();
        d->labelPhotoWhiteBalance->show();
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
