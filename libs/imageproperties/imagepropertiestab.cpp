/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-19
 * Description : A tab to display general image information
 *
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QFileInfo>
#include <QFrame>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kfileitem.h>
#include <ksqueezedtextlabel.h>
#include <kseparator.h>

// LibKDcraw includes.

#include <libkdcraw/dcrawbinary.h>

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "navigatebarwidget.h"
#include "imagepropertiestab.h"
#include "imagepropertiestab.moc"

namespace Digikam
{

class ImagePropertiesTabPriv
{
public:

    ImagePropertiesTabPriv()
    {
        settingsArea           = 0;
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

    QFrame             *settingsArea;

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

ImagePropertiesTab::ImagePropertiesTab(QWidget* parent, bool navBar)
                  : NavigateBarTab(parent)
{
    d = new ImagePropertiesTabPriv;

    setupNavigateBar(navBar);
    d->settingsArea = new QFrame(this);
    d->settingsArea->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    d->settingsArea->setLineWidth( style()->pixelMetric(QStyle::PM_DefaultFrameWidth) );

    QGridLayout *settingsLayout = new QGridLayout(d->settingsArea);

    // --------------------------------------------------

    d->title                    = new QLabel(i18n("<big><b>File Properties</b></big>"), d->settingsArea);
    d->file                     = new QLabel(i18n("<b>File</b>:"), d->settingsArea);
    d->folder                   = new QLabel(i18n("<b>Folder</b>:"), d->settingsArea);
    d->modifiedDate             = new QLabel(i18n("<b>Modified</b>:"), d->settingsArea);
    d->size                     = new QLabel(i18n("<b>Size</b>:"), d->settingsArea);
    d->owner                    = new QLabel(i18n("<b>Owner</b>:"), d->settingsArea);
    d->permissions              = new QLabel(i18n("<b>Permissions</b>:"), d->settingsArea);

    KSeparator *line            = new KSeparator (Qt::Horizontal, d->settingsArea);
    d->title2                   = new QLabel(i18n("<big><b>Image Properties</b></big>"), d->settingsArea);
    d->mime                     = new QLabel(i18n("<b>Type</b>:"), d->settingsArea);
    d->dimensions               = new QLabel(i18n("<b>Dimensions</b>:"), d->settingsArea);
    d->compression              = new QLabel(i18n("<b>Compression</b>:"), d->settingsArea);
    d->bitDepth                 = new QLabel(i18n("<nobr><b>Bit depth</b></nobr>:"), d->settingsArea);
    d->colorMode                = new QLabel(i18n("<nobr><b>Color mode</b></nobr>:"), d->settingsArea);

    KSeparator *line2           = new KSeparator (Qt::Horizontal, d->settingsArea);
    d->title3                   = new QLabel(i18n("<big><b>Photograph Properties</b></big>"), d->settingsArea);
    d->make                     = new QLabel(i18n("<b>Make</b>:"), d->settingsArea);
    d->model                    = new QLabel(i18n("<b>Model</b>:"), d->settingsArea);
    d->photoDate                = new QLabel(i18n("<b>Created</b>:"), d->settingsArea);
    d->aperture                 = new QLabel(i18n("<b>Aperture</b>:"), d->settingsArea);
    d->focalLength              = new QLabel(i18n("<b>Focal</b>:"), d->settingsArea);
    d->exposureTime             = new QLabel(i18n("<b>Exposure</b>:"), d->settingsArea);
    d->sensitivity              = new QLabel(i18n("<b>Sensitivity</b>:"), d->settingsArea);
    d->exposureMode             = new QLabel(i18n("<nobr><b>Mode/Program</b></nobr>:"), d->settingsArea);
    d->flash                    = new QLabel(i18n("<b>Flash</b>:"), d->settingsArea);
    d->whiteBalance             = new QLabel(i18n("<nobr><b>White balance</b></nobr>:"), d->settingsArea);

    d->labelFile                = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelFolder              = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelFileModifiedDate    = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelFileSize            = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelFileOwner           = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelFilePermissions     = new KSqueezedTextLabel(0, d->settingsArea);

    d->labelImageMime           = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelImageDimensions     = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelImageCompression    = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelImageBitDepth       = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelImageColorMode      = new KSqueezedTextLabel(0, d->settingsArea);

    d->labelPhotoMake           = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoModel          = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoDateTime       = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoAperture       = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoFocalLenght    = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoExposureTime   = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoSensitivity    = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoExposureMode   = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoFlash          = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoWhiteBalance   = new KSqueezedTextLabel(0, d->settingsArea);

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

    // --------------------------------------------------

    m_navigateBarLayout->addWidget(d->settingsArea);
    m_navigateBarLayout->setStretchFactor(d->settingsArea, 10);
}

ImagePropertiesTab::~ImagePropertiesTab()
{
    delete d;
}

void ImagePropertiesTab::setCurrentURL(const KUrl& url)
{
    if (url.isEmpty())
    {
        setNavigateBarFileName();

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

    QString str;
    QString unavailable(i18n("<i>unavailable</i>"));

    KFileItem fi(KFileItem::Unknown, KFileItem::Unknown, url);
    QFileInfo fileInfo(url.path());
    DMetadata metaData(url.path());

    // -- File system information ------------------------------------------

    d->labelFile->setText(url.fileName());
    d->labelFolder->setText(url.directory());

    QDateTime modifiedDate = fileInfo.lastModified();
    str = KGlobal::locale()->formatDateTime(modifiedDate, KLocale::ShortDate, true);
    d->labelFileModifiedDate->setText(str);

    str = QString("%1 (%2)").arg(KIO::convertSize(fi.size()))
                            .arg(KGlobal::locale()->formatNumber(fi.size(), 0));
    d->labelFileSize->setText(str);

    d->labelFileOwner->setText( QString("%1 - %2").arg(fi.user()).arg(fi.group()) );
    d->labelFilePermissions->setText( fi.permissionsString() );

    // -- Image Properties --------------------------------------------------

    QSize   dims;
    QString compression, bitDepth, colorMode;
    QString rawFilesExt(KDcrawIface::DcrawBinary::instance()->rawFiles());
    QString ext = fileInfo.suffix().toUpper();

    if (!ext.isEmpty() && rawFilesExt.toUpper().contains(ext))
    {
        d->labelImageMime->setText(i18n("RAW Image"));
        compression = i18n("None");
        bitDepth    = "48";
        dims        = metaData.getImageDimensions();
        colorMode   = i18n("Uncalibrated");
    }
    else
    {
        d->labelImageMime->setText(fi.mimeComment());

        KFileMetaInfo meta = fi.metaInfo();

        if (meta.isValid())
        {
            if (meta.item("Dimensions").isValid())
                dims = meta.item("Dimensions").value().toSize();

            if (meta.item("JPEG quality").isValid())
                compression = i18n("JPEG quality %1", meta.item("JPEG quality").value().toString());

            if (meta.item("Compression").isValid())
                compression =  meta.item("Compression").value().toString();

            if (meta.item("BitDepth").isValid())
                bitDepth = meta.item("BitDepth").value().toString();

            if (meta.item("ColorMode").isValid())
                colorMode = meta.item("ColorMode").value().toString();
        }

/*          TODO: KDE4PORT: KFileMetaInfo API as Changed.
                            Check if new method to search informations is enough.

        if (meta.isValid())
        {
            if (meta.containsGroup("Jpeg EXIF Data"))     // JPEG image ?
            {
                dims        = meta.group("Jpeg EXIF Data").item("Dimensions").value().toSize();

                QString quality = meta.group("Jpeg EXIF Data").item("JPEG quality").value().toString();
                quality.isEmpty() ? compression = unavailable :
                                    compression = i18n("JPEG quality %1").arg(quality);
                bitDepth    = meta.group("Jpeg EXIF Data").item("BitDepth").value().toString();
                colorMode   = meta.group("Jpeg EXIF Data").item("ColorMode").value().toString();
            }

            if (meta.containsGroup("General"))
            {
                if (dims.isEmpty() ) 
                    dims = meta.group("General").item("Dimensions").value().toSize();
                if (compression.isEmpty()) 
                    compression =  meta.group("General").item("Compression").value().toString();
                if (bitDepth.isEmpty()) 
                    bitDepth = meta.group("General").item("BitDepth").value().toString();
                if (colorMode.isEmpty()) 
                    colorMode = meta.group("General").item("ColorMode").value().toString();
            }

            if (meta.containsGroup("Technical"))
            {
                if (dims.isEmpty()) 
                    dims = meta.group("Technical").item("Dimensions").value().toSize();
                if (compression.isEmpty()) 
                    compression = meta.group("Technical").item("Compression").value().toString();
                if (bitDepth.isEmpty()) 
                    bitDepth = meta.group("Technical").item("BitDepth").value().toString();
                if (colorMode.isEmpty()) 
                    colorMode =  meta.group("Technical").item("ColorMode").value().toString();
            }
        }*/
    }

    QString mpixels;
    mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
    str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)",
           dims.width(), dims.height(), mpixels);
    d->labelImageDimensions->setText(str);
    d->labelImageCompression->setText(compression.isEmpty() ? unavailable : compression);
    d->labelImageBitDepth->setText(bitDepth.isEmpty() ? unavailable : i18n("%1 bpp", bitDepth));
    d->labelImageColorMode->setText(colorMode.isEmpty() ? unavailable : colorMode);

    // -- Photograph information ------------------------------------------
    // NOTA: If something is changed here, please updated albumfiletip section too.

    PhotoInfoContainer photoInfo = metaData.getPhotographInformations();

    if (photoInfo.isEmpty())
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

    d->labelPhotoMake->setText(photoInfo.make.isEmpty() ? unavailable : photoInfo.make);
    d->labelPhotoModel->setText(photoInfo.model.isEmpty() ? unavailable : photoInfo.model);

    if (photoInfo.dateTime.isValid())
    {
        str = KGlobal::locale()->formatDateTime(photoInfo.dateTime, KLocale::ShortDate, true);
        d->labelPhotoDateTime->setText(str);
    }
    else
        d->labelPhotoDateTime->setText(unavailable);

    d->labelPhotoAperture->setText(photoInfo.aperture.isEmpty() ? unavailable : photoInfo.aperture);

    if (photoInfo.focalLength35mm.isEmpty())
        d->labelPhotoFocalLenght->setText(photoInfo.focalLength.isEmpty() ? unavailable : photoInfo.focalLength);
    else 
    {
        str = i18n("%1 (35mm: %2)", photoInfo.focalLength, photoInfo.focalLength35mm);
        d->labelPhotoFocalLenght->setText(str);
    }

    d->labelPhotoExposureTime->setText(photoInfo.exposureTime.isEmpty() ? unavailable : photoInfo.exposureTime);
    d->labelPhotoSensitivity->setText(photoInfo.sensitivity.isEmpty() ? unavailable : i18n("%1 ISO", photoInfo.sensitivity));

    if (photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
        d->labelPhotoExposureMode->setText(unavailable);
    else if (!photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
        d->labelPhotoExposureMode->setText(photoInfo.exposureMode);
    else if (photoInfo.exposureMode.isEmpty() && !photoInfo.exposureProgram.isEmpty())
        d->labelPhotoExposureMode->setText(photoInfo.exposureProgram);
    else 
    {
        str = QString("%1 / %2").arg(photoInfo.exposureMode).arg(photoInfo.exposureProgram);
        d->labelPhotoExposureMode->setText(str);
    }

    d->labelPhotoFlash->setText(photoInfo.flash.isEmpty() ? unavailable : photoInfo.flash);
    d->labelPhotoWhiteBalance->setText(photoInfo.whiteBalance.isEmpty() ? unavailable : photoInfo.whiteBalance);
}

void ImagePropertiesTab::colorChanged(const QColor& back, const QColor& fore)
{
    QPalette plt(palette());

    plt.setColor(QPalette::Active, QPalette::Base, back);
    plt.setColor(QPalette::Active, QPalette::Text, fore);
    plt.setColor(QPalette::Inactive, QPalette::Base, back);
    plt.setColor(QPalette::Inactive, QPalette::Text, fore);

    setPalette(plt);

    d->settingsArea->setPalette(plt);

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

}  // NameSpace Digikam

