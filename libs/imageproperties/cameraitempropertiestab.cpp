/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-02-08
 * Description : A tab to display camera item information
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

// Local includes.

#include "cameraitempropertiestab.h"
#include "cameraitempropertiestab.moc"

// Qt includes.

#include <QStyle>
#include <QFile>
#include <QLabel>
#include <QGridLayout>

// KDE includes.

#include <kdebug.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kfileitem.h>
#include <kmimetype.h>
#include <kseparator.h>
#include <ksqueezedtextlabel.h>

// Digikam includes.

#include "dmetadata.h"
#include "gpiteminfo.h"

namespace Digikam
{

class CameraItemPropertiesTabPriv
{
public:

    CameraItemPropertiesTabPriv()
    {
        title                  = 0;
        file                   = 0;
        folder                 = 0;
        date                   = 0;
        size                   = 0;
        isReadable             = 0;
        isWritable             = 0;
        mime                   = 0;
        dimensions             = 0;
        newFileName            = 0;
        downloaded             = 0;
        title2                 = 0;
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
        labelFileIsReadable    = 0;
        labelFileIsWritable    = 0;
        labelFileDate          = 0;
        labelFileSize          = 0;
        labelImageMime         = 0;
        labelImageDimensions   = 0;
        labelNewFileName       = 0;
        labelAlreadyDownloaded = 0;
        labelPhotoMake         = 0;
        labelPhotoModel        = 0;
        labelPhotoDateTime     = 0;
        labelPhotoLens         = 0;
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
    QLabel             *date;
    QLabel             *size;
    QLabel             *isReadable;
    QLabel             *isWritable;
    QLabel             *mime;
    QLabel             *dimensions;
    QLabel             *newFileName;
    QLabel             *downloaded;

    QLabel             *title2;
    QLabel             *make;
    QLabel             *model;
    QLabel             *photoDate;
    QLabel             *lens;
    QLabel             *aperture;
    QLabel             *focalLength;
    QLabel             *exposureTime;
    QLabel             *sensitivity;
    QLabel             *exposureMode;
    QLabel             *flash;
    QLabel             *whiteBalance;

    KSqueezedTextLabel *labelFile;
    KSqueezedTextLabel *labelFolder;
    KSqueezedTextLabel *labelFileIsReadable;
    KSqueezedTextLabel *labelFileIsWritable;
    KSqueezedTextLabel *labelFileDate;
    KSqueezedTextLabel *labelFileSize;
    KSqueezedTextLabel *labelImageMime;
    KSqueezedTextLabel *labelImageDimensions;
    KSqueezedTextLabel *labelNewFileName;
    KSqueezedTextLabel *labelAlreadyDownloaded;

    KSqueezedTextLabel *labelPhotoMake;
    KSqueezedTextLabel *labelPhotoModel;
    KSqueezedTextLabel *labelPhotoDateTime;
    KSqueezedTextLabel *labelPhotoLens;
    KSqueezedTextLabel *labelPhotoAperture;
    KSqueezedTextLabel *labelPhotoFocalLenght;
    KSqueezedTextLabel *labelPhotoExposureTime;
    KSqueezedTextLabel *labelPhotoSensitivity;
    KSqueezedTextLabel *labelPhotoExposureMode;
    KSqueezedTextLabel *labelPhotoFlash;
    KSqueezedTextLabel *labelPhotoWhiteBalance;
};

CameraItemPropertiesTab::CameraItemPropertiesTab(QWidget* parent)
                       : QFrame(parent)
{
    d = new CameraItemPropertiesTabPriv;

    setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    setLineWidth( style()->pixelMetric(QStyle::PM_DefaultFrameWidth) );

    QGridLayout *settingsLayout = new QGridLayout(this);

    // --------------------------------------------------

    d->title                  = new QLabel(i18n("<big><b>Camera File Properties</b></big>"), this);
    d->file                   = new QLabel(i18n("<b>File</b>:"), this);
    d->folder                 = new QLabel(i18n("<b>Folder</b>:"), this);
    d->date                   = new QLabel(i18n("<b>Date</b>:"), this);
    d->size                   = new QLabel(i18n("<b>Size</b>:"), this);
    d->isReadable             = new QLabel(i18n("<b>Readable</b>:"), this);
    d->isWritable             = new QLabel(i18n("<b>Writable</b>:"), this);
    d->mime                   = new QLabel(i18n("<b>Type</b>:"), this);
    d->dimensions             = new QLabel(i18n("<b>Dimensions</b>:"), this);
    d->newFileName            = new QLabel(i18n("<nobr><b>New Name</b></nobr>:"), this);
    d->downloaded             = new QLabel(i18n("<b>Downloaded</b>:"), this);

    KSeparator *line          = new KSeparator (Qt::Horizontal, this);
    d->title2                 = new QLabel(i18n("<big><b>Photograph Properties</b></big>"), this);
    d->make                   = new QLabel(i18n("<b>Make</b>:"), this);
    d->model                  = new QLabel(i18n("<b>Model</b>:"), this);
    d->photoDate              = new QLabel(i18n("<b>Created</b>:"), this);
    d->lens                   = new QLabel(i18n("<b>Lens</b>:"), this);
    d->aperture               = new QLabel(i18n("<b>Aperture</b>:"), this);
    d->focalLength            = new QLabel(i18n("<b>Focal</b>:"), this);
    d->exposureTime           = new QLabel(i18n("<b>Exposure</b>:"), this);
    d->sensitivity            = new QLabel(i18n("<b>Sensitivity</b>:"), this);
    d->exposureMode           = new QLabel(i18n("<nobr><b>Mode/Program</b></nobr>:"), this);
    d->flash                  = new QLabel(i18n("<b>Flash</b>:"), this);
    d->whiteBalance           = new QLabel(i18n("<nobr><b>White balance</b></nobr>:"), this);

    d->labelFile              = new KSqueezedTextLabel(0, this);
    d->labelFolder            = new KSqueezedTextLabel(0, this);
    d->labelFileDate          = new KSqueezedTextLabel(0, this);
    d->labelFileSize          = new KSqueezedTextLabel(0, this);
    d->labelFileIsReadable    = new KSqueezedTextLabel(0, this);
    d->labelFileIsWritable    = new KSqueezedTextLabel(0, this);
    d->labelImageMime         = new KSqueezedTextLabel(0, this);
    d->labelImageDimensions   = new KSqueezedTextLabel(0, this);
    d->labelNewFileName       = new KSqueezedTextLabel(0, this);
    d->labelAlreadyDownloaded = new KSqueezedTextLabel(0, this);

    d->labelPhotoMake         = new KSqueezedTextLabel(0, this);
    d->labelPhotoModel        = new KSqueezedTextLabel(0, this);
    d->labelPhotoDateTime     = new KSqueezedTextLabel(0, this);
    d->labelPhotoLens         = new KSqueezedTextLabel(0, this);
    d->labelPhotoAperture     = new KSqueezedTextLabel(0, this);
    d->labelPhotoFocalLenght  = new KSqueezedTextLabel(0, this);
    d->labelPhotoExposureTime = new KSqueezedTextLabel(0, this);
    d->labelPhotoSensitivity  = new KSqueezedTextLabel(0, this);
    d->labelPhotoExposureMode = new KSqueezedTextLabel(0, this);
    d->labelPhotoFlash        = new KSqueezedTextLabel(0, this);
    d->labelPhotoWhiteBalance = new KSqueezedTextLabel(0, this);

    int hgt = fontMetrics().height()-2;
    d->title->setAlignment(Qt::AlignCenter);
    d->file->setMaximumHeight(hgt);
    d->folder->setMaximumHeight(hgt);
    d->date->setMaximumHeight(hgt);
    d->size->setMaximumHeight(hgt);
    d->isReadable->setMaximumHeight(hgt);
    d->isWritable->setMaximumHeight(hgt);
    d->mime->setMaximumHeight(hgt);
    d->dimensions->setMaximumHeight(hgt);
    d->newFileName->setMaximumHeight(hgt);
    d->downloaded->setMaximumHeight(hgt);
    d->labelFile->setMaximumHeight(hgt);
    d->labelFolder->setMaximumHeight(hgt);
    d->labelFileDate->setMaximumHeight(hgt);
    d->labelFileSize->setMaximumHeight(hgt);
    d->labelFileIsReadable->setMaximumHeight(hgt);
    d->labelFileIsWritable->setMaximumHeight(hgt);
    d->labelImageMime->setMaximumHeight(hgt);
    d->labelImageDimensions->setMaximumHeight(hgt);
    d->labelNewFileName->setMaximumHeight(hgt);
    d->labelAlreadyDownloaded->setMaximumHeight(hgt);

    d->title2->setAlignment(Qt::AlignCenter);
    d->make->setMaximumHeight(hgt);
    d->model->setMaximumHeight(hgt);
    d->photoDate->setMaximumHeight(hgt);
    d->lens->setMaximumHeight(hgt);
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
    d->labelPhotoLens->setMaximumHeight(hgt);
    d->labelPhotoAperture->setMaximumHeight(hgt);
    d->labelPhotoFocalLenght->setMaximumHeight(hgt);
    d->labelPhotoExposureTime->setMaximumHeight(hgt);
    d->labelPhotoSensitivity->setMaximumHeight(hgt);
    d->labelPhotoExposureMode->setMaximumHeight(hgt);
    d->labelPhotoFlash->setMaximumHeight(hgt);
    d->labelPhotoWhiteBalance->setMaximumHeight(hgt);

    // --------------------------------------------------

    settingsLayout->addWidget(d->title,                  0, 0, 1, 2);
    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding),
                                                          1, 1, 0, 1);
    settingsLayout->addWidget(d->file,                    2, 0, 1, 1);
    settingsLayout->addWidget(d->labelFile,               2, 1, 1, 1);
    settingsLayout->addWidget(d->folder,                  3, 0, 1, 1);
    settingsLayout->addWidget(d->labelFolder,             3, 1, 1, 1);
    settingsLayout->addWidget(d->date,                    4, 0, 1, 1);
    settingsLayout->addWidget(d->labelFileDate,           4, 1, 1, 1);
    settingsLayout->addWidget(d->size,                    5, 0, 1, 1);
    settingsLayout->addWidget(d->labelFileSize,           5, 1, 1, 1);
    settingsLayout->addWidget(d->isReadable,              6, 0, 1, 1);
    settingsLayout->addWidget(d->labelFileIsReadable,     6, 1, 1, 1);
    settingsLayout->addWidget(d->isWritable,              7, 0, 1, 1);
    settingsLayout->addWidget(d->labelFileIsWritable,     7, 1, 1, 1);
    settingsLayout->addWidget(d->mime,                    8, 0, 1, 1);
    settingsLayout->addWidget(d->labelImageMime,          8, 1, 1, 1);
    settingsLayout->addWidget(d->dimensions,              9, 0, 1, 1);
    settingsLayout->addWidget(d->labelImageDimensions,    9, 1, 1, 1);
    settingsLayout->addWidget(d->newFileName,            10, 0, 1, 1);
    settingsLayout->addWidget(d->labelNewFileName,       10, 1, 1, 1);
    settingsLayout->addWidget(d->downloaded,             11, 0, 1, 1);
    settingsLayout->addWidget(d->labelAlreadyDownloaded, 11, 1, 1, 1);

    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding),
                                                         12, 0, 0, 1);
    settingsLayout->addWidget(line,                      13, 0, 1, 2);
    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding),
                                                         14, 0, 0, 1);

    settingsLayout->addWidget(d->title2,                 15, 0, 1, 2);
    settingsLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                QSizePolicy::Minimum, QSizePolicy::MinimumExpanding),
                                                         16, 0, 0, 1);
    settingsLayout->addWidget(d->make,                   17, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoMake,         17, 1, 1, 1);
    settingsLayout->addWidget(d->model,                  18, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoModel,        18, 1, 1, 1);
    settingsLayout->addWidget(d->photoDate,              19, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoDateTime,     19, 1, 1, 1);
    settingsLayout->addWidget(d->lens,                   20, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoLens,         20, 1, 1, 1);
    settingsLayout->addWidget(d->aperture,               21, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoAperture,     21, 1, 1, 1);
    settingsLayout->addWidget(d->focalLength,            22, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoFocalLenght,  22, 1, 1, 1);
    settingsLayout->addWidget(d->exposureTime,           23, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoExposureTime, 23, 1, 1, 1);
    settingsLayout->addWidget(d->sensitivity,            24, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoSensitivity,  24, 1, 1, 1);
    settingsLayout->addWidget(d->exposureMode,           25, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoExposureMode, 25, 1, 1, 1);
    settingsLayout->addWidget(d->flash,                  26, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoFlash,        26, 1, 1, 1);
    settingsLayout->addWidget(d->whiteBalance,           27, 0, 1, 1);
    settingsLayout->addWidget(d->labelPhotoWhiteBalance, 27, 1, 1, 1);

    settingsLayout->setRowStretch(28, 10);
    settingsLayout->setColumnStretch(1, 10);
    settingsLayout->setMargin(KDialog::spacingHint());
    settingsLayout->setSpacing(0);
}

CameraItemPropertiesTab::~CameraItemPropertiesTab()
{
    delete d;
}

void CameraItemPropertiesTab::setCurrentItem(const GPItemInfo* itemInfo,
                                             const QString &newFileName, const QByteArray& exifData,
                                             const KUrl &currentURL)
{
    if (!itemInfo)
    {
        d->labelFile->setText(QString());
        d->labelFolder->setText(QString());
        d->labelFileIsReadable->setText(QString());
        d->labelFileIsWritable->setText(QString());
        d->labelFileDate->setText(QString());
        d->labelFileSize->setText(QString());
        d->labelImageMime->setText(QString());
        d->labelImageDimensions->setText(QString());
        d->labelNewFileName->setText(QString());
        d->labelAlreadyDownloaded->setText(QString());

        d->labelPhotoMake->setText(QString());
        d->labelPhotoModel->setText(QString());
        d->labelPhotoDateTime->setText(QString());
        d->labelPhotoLens->setText(QString());
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
    QString unknown(i18n("<i>unknown</i>"));

    // -- Camera file system information ------------------------------------------

    d->labelFile->setText(itemInfo->name);
    d->labelFolder->setText(itemInfo->folder);

    if (itemInfo->readPermissions < 0)
        str = unknown;
    else if (itemInfo->readPermissions == 0)
        str = i18n("No");
    else
        str = i18n("Yes");

    d->labelFileIsReadable->setText(str);

    if (itemInfo->writePermissions < 0)
        str = unknown;
    else if (itemInfo->writePermissions == 0)
        str = i18n("No");
    else
        str = i18n("Yes");

    d->labelFileIsWritable->setText(str);

    if (itemInfo->mtime.isValid())
        d->labelFileDate->setText(KGlobal::locale()->formatDateTime(itemInfo->mtime,
                                                                    KLocale::ShortDate, true));
    else
        d->labelFileDate->setText(unknown);

    str = i18n("%1 (%2)", KIO::convertSize(itemInfo->size),
                          KGlobal::locale()->formatNumber(itemInfo->size, 0));
    d->labelFileSize->setText(str);

    // -- Image Properties --------------------------------------------------

    if (itemInfo->mime == "image/x-raw")
        d->labelImageMime->setText(i18n("RAW Image"));
    else
    {
        KMimeType::Ptr mimeType = KMimeType::mimeType(itemInfo->mime, KMimeType::ResolveAliases);
        if (mimeType)
            d->labelImageMime->setText(mimeType->comment());
        else
            d->labelImageMime->setText(itemInfo->mime); // last fallback
    }

    QString mpixels;
    QSize dims;
    if (itemInfo->width == -1 && itemInfo->height == -1 && !currentURL.isEmpty())
    {
        // delayed loading to list faster from UMSCamera
        if (itemInfo->mime == "image/x-raw")
        {
            DMetadata metaData(currentURL.path());
            dims = metaData.getImageDimensions();
        }
        else
        {
            KFileMetaInfo meta(currentURL.path());

/*          TODO: KDE4PORT: KFileMetaInfo API as Changed.
                            Check if new method to search "Dimensions" information is enough.

            if (meta.isValid())
            {
                if (meta.containsGroup("Jpeg EXIF Data"))
                    dims = meta.group("Jpeg EXIF Data").item("Dimensions").value().toSize();
                else if (meta.containsGroup("General"))
                    dims = meta.group("General").item("Dimensions").value().toSize();
                else if (meta.containsGroup("Technical"))
                    dims = meta.group("Technical").item("Dimensions").value().toSize();
            }*/

            if (meta.isValid() && meta.item("Dimensions").isValid())
            {
                dims = meta.item("Dimensions").value().toSize();
            }
        }
    }
    else
    {
        // if available (GPCamera), take dimensions directly from itemInfo
        dims = QSize(itemInfo->width, itemInfo->height);
    }
    mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
    str = (!dims.isValid()) ? unknown : i18n("%1x%2 (%3Mpx)",
           dims.width(), dims.height(), mpixels);
    d->labelImageDimensions->setText(str);

    // -- Download information ------------------------------------------

    d->labelNewFileName->setText(newFileName.isEmpty() ? i18n("<i>unchanged</i>") : newFileName);

    if (itemInfo->downloaded < 0)
        str = unknown;
    else if (itemInfo->downloaded == 0)
        str = i18n("No");
    else
        str = i18n("Yes");

    d->labelAlreadyDownloaded->setText(str);

    // -- Photograph information ------------------------------------------
    // Note: If something is changed here, please updated albumfiletip section too.

    QString unavailable(i18n("<i>unavailable</i>"));
    DMetadata metaData;
    metaData.setExif(exifData);
    PhotoInfoContainer photoInfo = metaData.getPhotographInformations();

    if (photoInfo.isEmpty())
    {
        d->title2->hide();
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
        d->labelPhotoLens->hide();
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
        d->title2->show();
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
        d->labelPhotoLens->show();
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

    d->labelPhotoLens->setText(photoInfo.lens.isEmpty() ? unavailable : photoInfo.lens);
    d->labelPhotoAperture->setText(photoInfo.aperture.isEmpty() ? unavailable : photoInfo.aperture);

    if (photoInfo.focalLength35mm.isEmpty())
        d->labelPhotoFocalLenght->setText(photoInfo.focalLength.isEmpty() ? unavailable : photoInfo.focalLength);
    else
    {
        str = i18n("%1 (35mm: %2)", photoInfo.focalLength,
                   photoInfo.focalLength35mm);
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

}  // namespace Digikam
