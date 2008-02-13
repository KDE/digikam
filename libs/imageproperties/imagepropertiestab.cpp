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

// Qt includes.

#include <QGridLayout>
#include <QStyle>
#include <QFile>
#include <QLabel>
#include <QPixmap>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <ksqueezedtextlabel.h>
#include <kseparator.h>

// Local includes.

#include "ddebug.h"
#include "imagepropertiestab.h"
#include "imagepropertiestab.moc"

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
        labelPhotoAperture     = 0;
        labelPhotoFocalLenght  = 0;
        labelPhotoExposureTime = 0;
        labelPhotoSensitivity  = 0;
        labelPhotoExposureMode = 0;
        labelPhotoFlash        = 0;
        labelPhotoWhiteBalance = 0;
    }

    QLabel             *title;
    QLabel             *file;
    QLabel             *folder;
    QLabel             *modifiedDate;
    QLabel             *size;
    QLabel             *owner;
    QLabel             *permissions;

    QLabel             *title2;
    QLabel             *mime;
    QLabel             *dimensions;
    QLabel             *compression;
    QLabel             *bitDepth;
    QLabel             *colorMode;

    QLabel             *title3;
    QLabel             *make;
    QLabel             *model;
    QLabel             *photoDate;
    QLabel             *aperture;
    QLabel             *focalLength;
    QLabel             *exposureTime;
    QLabel             *sensitivity;
    QLabel             *exposureMode;
    QLabel             *flash;
    QLabel             *whiteBalance;

    KSqueezedTextLabel *labelFile;
    KSqueezedTextLabel *labelFolder;
    KSqueezedTextLabel *labelFileModifiedDate;
    KSqueezedTextLabel *labelFileSize;
    KSqueezedTextLabel *labelFileOwner;
    KSqueezedTextLabel *labelFilePermissions;

    KSqueezedTextLabel *labelImageMime;
    KSqueezedTextLabel *labelImageDimensions;
    KSqueezedTextLabel *labelImageCompression;
    KSqueezedTextLabel *labelImageBitDepth;
    KSqueezedTextLabel *labelImageColorMode;

    KSqueezedTextLabel *labelPhotoMake;
    KSqueezedTextLabel *labelPhotoModel;
    KSqueezedTextLabel *labelPhotoDateTime;
    KSqueezedTextLabel *labelPhotoAperture;
    KSqueezedTextLabel *labelPhotoFocalLenght;
    KSqueezedTextLabel *labelPhotoExposureTime;
    KSqueezedTextLabel *labelPhotoSensitivity;
    KSqueezedTextLabel *labelPhotoExposureMode;
    KSqueezedTextLabel *labelPhotoFlash;
    KSqueezedTextLabel *labelPhotoWhiteBalance;
};

ImagePropertiesTab::ImagePropertiesTab(QWidget* parent)
                  : QFrame(parent)
{
    d = new ImagePropertiesTabPriv;

    setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    setLineWidth( style()->pixelMetric(QStyle::PM_DefaultFrameWidth) );

    QGridLayout *settingsLayout = new QGridLayout(this);

    // --------------------------------------------------

    d->title                    = new QLabel(i18n("<big><b>File Properties</b></big>"), this);
    d->file                     = new QLabel(i18n("<b>File</b>:"), this);
    d->folder                   = new QLabel(i18n("<b>Folder</b>:"), this);
    d->modifiedDate             = new QLabel(i18n("<b>Modified</b>:"), this);
    d->size                     = new QLabel(i18n("<b>Size</b>:"), this);
    d->owner                    = new QLabel(i18n("<b>Owner</b>:"), this);
    d->permissions              = new QLabel(i18n("<b>Permissions</b>:"), this);

    KSeparator *line            = new KSeparator (Qt::Horizontal, this);
    d->title2                   = new QLabel(i18n("<big><b>Image Properties</b></big>"), this);
    d->mime                     = new QLabel(i18n("<b>Type</b>:"), this);
    d->dimensions               = new QLabel(i18n("<b>Dimensions</b>:"), this);
    d->compression              = new QLabel(i18n("<b>Compression</b>:"), this);
    d->bitDepth                 = new QLabel(i18n("<nobr><b>Bit depth</b></nobr>:"), this);
    d->colorMode                = new QLabel(i18n("<nobr><b>Color mode</b></nobr>:"), this);

    KSeparator *line2           = new KSeparator (Qt::Horizontal, this);
    d->title3                   = new QLabel(i18n("<big><b>Photograph Properties</b></big>"), this);
    d->make                     = new QLabel(i18n("<b>Make</b>:"), this);
    d->model                    = new QLabel(i18n("<b>Model</b>:"), this);
    d->photoDate                = new QLabel(i18n("<b>Created</b>:"), this);
    d->aperture                 = new QLabel(i18n("<b>Aperture</b>:"), this);
    d->focalLength              = new QLabel(i18n("<b>Focal</b>:"), this);
    d->exposureTime             = new QLabel(i18n("<b>Exposure</b>:"), this);
    d->sensitivity              = new QLabel(i18n("<b>Sensitivity</b>:"), this);
    d->exposureMode             = new QLabel(i18n("<nobr><b>Mode/Program</b></nobr>:"), this);
    d->flash                    = new QLabel(i18n("<b>Flash</b>:"), this);
    d->whiteBalance             = new QLabel(i18n("<nobr><b>White balance</b></nobr>:"), this);

    d->labelFile                = new KSqueezedTextLabel(0, this);
    d->labelFolder              = new KSqueezedTextLabel(0, this);
    d->labelFileModifiedDate    = new KSqueezedTextLabel(0, this);
    d->labelFileSize            = new KSqueezedTextLabel(0, this);
    d->labelFileOwner           = new KSqueezedTextLabel(0, this);
    d->labelFilePermissions     = new KSqueezedTextLabel(0, this);

    d->labelImageMime           = new KSqueezedTextLabel(0, this);
    d->labelImageDimensions     = new KSqueezedTextLabel(0, this);
    d->labelImageCompression    = new KSqueezedTextLabel(0, this);
    d->labelImageBitDepth       = new KSqueezedTextLabel(0, this);
    d->labelImageColorMode      = new KSqueezedTextLabel(0, this);

    d->labelPhotoMake           = new KSqueezedTextLabel(0, this);
    d->labelPhotoModel          = new KSqueezedTextLabel(0, this);
    d->labelPhotoDateTime       = new KSqueezedTextLabel(0, this);
    d->labelPhotoAperture       = new KSqueezedTextLabel(0, this);
    d->labelPhotoFocalLenght    = new KSqueezedTextLabel(0, this);
    d->labelPhotoExposureTime   = new KSqueezedTextLabel(0, this);
    d->labelPhotoSensitivity    = new KSqueezedTextLabel(0, this);
    d->labelPhotoExposureMode   = new KSqueezedTextLabel(0, this);
    d->labelPhotoFlash          = new KSqueezedTextLabel(0, this);
    d->labelPhotoWhiteBalance   = new KSqueezedTextLabel(0, this);

    int hgt = fontMetrics().height()-2;
    d->title->setAlignment(Qt::AlignCenter);
    d->file->setMaximumHeight(hgt);
    d->folder->setMaximumHeight(hgt);
    d->modifiedDate->setMaximumHeight(hgt);
    d->size->setMaximumHeight(hgt);
    d->owner->setMaximumHeight(hgt);
    d->permissions->setMaximumHeight(hgt);
    d->labelFile->setMaximumHeight(hgt);
    d->labelFolder->setMaximumHeight(hgt);
    d->labelFileModifiedDate->setMaximumHeight(hgt);
    d->labelFileSize->setMaximumHeight(hgt);
    d->labelFileOwner->setMaximumHeight(hgt);
    d->labelFilePermissions->setMaximumHeight(hgt);

    d->title2->setAlignment(Qt::AlignCenter);
    d->mime->setMaximumHeight(hgt);
    d->dimensions->setMaximumHeight(hgt);
    d->compression->setMaximumHeight(hgt);
    d->bitDepth->setMaximumHeight(hgt);
    d->colorMode->setMaximumHeight(hgt);
    d->labelImageMime->setMaximumHeight(hgt);
    d->labelImageDimensions->setMaximumHeight(hgt);
    d->labelImageCompression->setMaximumHeight(hgt);
    d->labelImageBitDepth->setMaximumHeight(hgt);
    d->labelImageColorMode->setMaximumHeight(hgt);

    d->title3->setAlignment(Qt::AlignCenter);
    d->make->setMaximumHeight(hgt);
    d->model->setMaximumHeight(hgt);
    d->photoDate->setMaximumHeight(hgt);
    d->aperture->setMaximumHeight(hgt);
    d->focalLength->setMaximumHeight(hgt);
    d->exposureTime->setMaximumHeight(hgt);
    d->sensitivity->setMaximumHeight(hgt);
    d->exposureMode->setMaximumHeight(hgt);
    d->flash->setMaximumHeight(hgt);
    d->whiteBalance->setMaximumHeight(hgt);
    d->labelPhotoMake->setMaximumHeight(hgt);
    d->labelPhotoModel->setMaximumHeight(hgt);
    d->labelPhotoDateTime->setMaximumHeight(hgt);
    d->labelPhotoAperture->setMaximumHeight(hgt);
    d->labelPhotoFocalLenght->setMaximumHeight(hgt);
    d->labelPhotoExposureTime->setMaximumHeight(hgt);
    d->labelPhotoSensitivity->setMaximumHeight(hgt);
    d->labelPhotoExposureMode->setMaximumHeight(hgt);
    d->labelPhotoFlash->setMaximumHeight(hgt);
    d->labelPhotoWhiteBalance->setMaximumHeight(hgt);

    // --------------------------------------------------

    settingsLayout->addWidget(d->title, 0, 0, 1, 2 );
    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 1, 0, 1, 2);
    settingsLayout->addWidget(d->file, 2, 0, 1, 1);
    settingsLayout->addWidget(d->labelFile, 2, 1, 1, 1);
    settingsLayout->addWidget(d->folder, 3, 0, 1, 1);
    settingsLayout->addWidget(d->labelFolder, 3, 1, 1, 1);
    settingsLayout->addWidget(d->modifiedDate, 4, 0, 1, 1);
    settingsLayout->addWidget(d->labelFileModifiedDate, 4, 1, 1, 1);
    settingsLayout->addWidget(d->size, 5, 0, 1, 1);
    settingsLayout->addWidget(d->labelFileSize, 5, 1, 1, 1);
    settingsLayout->addWidget(d->owner, 6, 0, 1, 1);
    settingsLayout->addWidget(d->labelFileOwner, 6, 1, 1, 1);
    settingsLayout->addWidget(d->permissions, 7, 0, 1, 1);
    settingsLayout->addWidget(d->labelFilePermissions, 7, 1, 1, 1);

    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 8, 0, 1, 2);
    settingsLayout->addWidget(line, 9, 0, 1, 2 );
    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 10, 0, 1, 2);

    settingsLayout->addWidget(d->title2, 11, 0, 1, 2 );
    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 12, 0, 1, 2);
    settingsLayout->addWidget(d->mime, 13, 0, 1, 1);
    settingsLayout->addWidget(d->labelImageMime, 13, 1, 1, 1);
    settingsLayout->addWidget(d->dimensions, 14, 0, 1, 1);
    settingsLayout->addWidget(d->labelImageDimensions, 14, 1, 1, 1);
    settingsLayout->addWidget(d->compression, 15, 0, 1, 1);
    settingsLayout->addWidget(d->labelImageCompression, 15, 1, 1, 1);
    settingsLayout->addWidget(d->bitDepth, 16, 0, 1, 1);
    settingsLayout->addWidget(d->labelImageBitDepth, 16, 1, 1, 1);
    settingsLayout->addWidget(d->colorMode, 17, 0, 1, 1);
    settingsLayout->addWidget(d->labelImageColorMode, 17, 1, 1, 1);

    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 18, 0, 1, 2);
    settingsLayout->addWidget(line2, 19, 0, 1, 2 );
    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 20, 0, 1, 2);

    settingsLayout->addWidget(d->title3, 21, 0, 1, 2 );
    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 22, 0, 1, 2);
    settingsLayout->addWidget(d->make, 23, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoMake, 23, 1, 1, 1);
    settingsLayout->addWidget(d->model, 24, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoModel, 24, 1, 1, 1);
    settingsLayout->addWidget(d->photoDate, 25, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoDateTime, 25, 1, 1, 1);
    settingsLayout->addWidget(d->aperture, 26, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoAperture, 26, 1, 1, 1);
    settingsLayout->addWidget(d->focalLength, 27, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoFocalLenght, 27, 1, 1, 1);
    settingsLayout->addWidget(d->exposureTime, 28, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoExposureTime, 28, 1, 1, 1);
    settingsLayout->addWidget(d->sensitivity, 29, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoSensitivity, 29, 1, 1, 1);
    settingsLayout->addWidget(d->exposureMode, 30, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoExposureMode, 30, 1, 1, 1);
    settingsLayout->addWidget(d->flash, 31, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoFlash, 31, 1, 1, 1);
    settingsLayout->addWidget(d->whiteBalance, 32, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoWhiteBalance, 32, 1, 1, 1);

    settingsLayout->setRowStretch(33, 10);
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
        d->labelPhotoAperture->setText(QString());
        d->labelPhotoFocalLenght->setText(QString());
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

void ImagePropertiesTab::colorChanged(const QColor& back, const QColor& fore)
{
    QPalette plt(palette());

    plt.setColor(QPalette::Active, QPalette::Base, back);
    plt.setColor(QPalette::Active, QPalette::Text, fore);
    plt.setColor(QPalette::Inactive, QPalette::Base, back);
    plt.setColor(QPalette::Inactive, QPalette::Text, fore);

    setPalette(plt);

    d->title->setPalette(plt);
    d->file->setPalette(plt);
    d->folder->setPalette(plt);
    d->modifiedDate->setPalette(plt);
    d->size->setPalette(plt);
    d->owner->setPalette(plt);
    d->permissions->setPalette(plt);

    d->title2->setPalette(plt);
    d->mime->setPalette(plt);
    d->dimensions->setPalette(plt);
    d->compression->setPalette(plt);
    d->bitDepth->setPalette(plt);
    d->colorMode->setPalette(plt);

    d->title3->setPalette(plt);
    d->make->setPalette(plt);
    d->model->setPalette(plt);
    d->photoDate->setPalette(plt);
    d->aperture->setPalette(plt);
    d->focalLength->setPalette(plt);
    d->exposureTime->setPalette(plt);
    d->sensitivity->setPalette(plt);
    d->exposureMode->setPalette(plt);
    d->flash->setPalette(plt);
    d->whiteBalance->setPalette(plt);

    d->labelFile->setPalette(plt);
    d->labelFolder->setPalette(plt);
    d->labelFileModifiedDate->setPalette(plt);
    d->labelFileSize->setPalette(plt);
    d->labelFileOwner->setPalette(plt);
    d->labelFilePermissions->setPalette(plt);

    d->labelImageMime->setPalette(plt);
    d->labelImageDimensions->setPalette(plt);
    d->labelImageCompression->setPalette(plt);
    d->labelImageBitDepth->setPalette(plt);
    d->labelImageColorMode->setPalette(plt);

    d->labelPhotoMake->setPalette(plt);
    d->labelPhotoModel->setPalette(plt);
    d->labelPhotoDateTime->setPalette(plt);
    d->labelPhotoAperture->setPalette(plt);
    d->labelPhotoFocalLenght->setPalette(plt);
    d->labelPhotoExposureTime->setPalette(plt);
    d->labelPhotoSensitivity->setPalette(plt);
    d->labelPhotoExposureMode->setPalette(plt);
    d->labelPhotoFlash->setPalette(plt);
    d->labelPhotoWhiteBalance->setPalette(plt);
}

void ImagePropertiesTab::setPhotoInfoEnable(bool b)
{
    if (b)
    {
        d->title3->hide();
        d->make->hide();
        d->model->hide();
        d->photoDate->hide();
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
        d->labelPhotoAperture->hide();
        d->labelPhotoFocalLenght->hide();
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
        d->labelPhotoAperture->show();
        d->labelPhotoFocalLenght->show();
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
    d->labelImageCompression->setText(str);
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

void ImagePropertiesTab::setPhotoAperture(const QString& str)
{
    d->labelPhotoAperture->setText(str);
}

void ImagePropertiesTab::setPhotoFocalLength(const QString& str)
{
    d->labelPhotoFocalLenght->setText(str);
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

}  // NameSpace Digikam
