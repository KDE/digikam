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

#include "cameraitempropertiestab.h"
#include "cameraitempropertiestab.moc"

// Qt includes.

#include <QStyle>
#include <QFile>
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

// Local includes.

#include "dmetadata.h"
#include "gpiteminfo.h"
#include "imagepropertiestxtlabel.h"

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
    DTextLabelName  *date;
    DTextLabelName  *size;
    DTextLabelName  *isReadable;
    DTextLabelName  *isWritable;
    DTextLabelName  *mime;
    DTextLabelName  *dimensions;
    DTextLabelName  *newFileName;
    DTextLabelName  *downloaded;

    QLabel          *title2;
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
    DTextLabelValue *labelFileIsReadable;
    DTextLabelValue *labelFileIsWritable;
    DTextLabelValue *labelFileDate;
    DTextLabelValue *labelFileSize;
    DTextLabelValue *labelImageMime;
    DTextLabelValue *labelImageDimensions;
    DTextLabelValue *labelNewFileName;
    DTextLabelValue *labelAlreadyDownloaded;

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

CameraItemPropertiesTab::CameraItemPropertiesTab(QWidget* parent)
                       : QScrollArea(parent), d(new CameraItemPropertiesTabPriv)
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

    d->title                  = new QLabel(i18n("<big><b>Camera File Properties</b></big>"), settingsArea);
    d->file                   = new DTextLabelName(i18n("File: "), settingsArea);
    d->folder                 = new DTextLabelName(i18n("Folder: "), settingsArea);
    d->date                   = new DTextLabelName(i18n("Date: "), settingsArea);
    d->size                   = new DTextLabelName(i18n("Size: "), settingsArea);
    d->isReadable             = new DTextLabelName(i18n("Readable: "), settingsArea);
    d->isWritable             = new DTextLabelName(i18n("Writable: "), settingsArea);
    d->mime                   = new DTextLabelName(i18n("Type: "), settingsArea);
    d->dimensions             = new DTextLabelName(i18n("Dimensions: "), settingsArea);
    d->newFileName            = new DTextLabelName(i18n("New Name: "), settingsArea);
    d->downloaded             = new DTextLabelName(i18n("Downloaded: "), settingsArea);

    KSeparator *line          = new KSeparator (Qt::Horizontal, settingsArea);
    d->title2                 = new QLabel(i18n("<big><b>Photograph Properties</b></big>"), settingsArea);
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
    d->labelFileDate          = new DTextLabelValue(0, settingsArea);
    d->labelFileSize          = new DTextLabelValue(0, settingsArea);
    d->labelFileIsReadable    = new DTextLabelValue(0, settingsArea);
    d->labelFileIsWritable    = new DTextLabelValue(0, settingsArea);
    d->labelImageMime         = new DTextLabelValue(0, settingsArea);
    d->labelImageDimensions   = new DTextLabelValue(0, settingsArea);
    d->labelNewFileName       = new DTextLabelValue(0, settingsArea);
    d->labelAlreadyDownloaded = new DTextLabelValue(0, settingsArea);

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
    settingsLayout->addWidget(d->labelPhotoFocalLength,  22, 1, 1, 1);
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

    if (itemInfo->downloaded == GPItemInfo::DownloadUnknow)
        str = unknown;
    else if (itemInfo->downloaded == GPItemInfo::DownloadedYes)
        str = i18n("Yes");
    else
        str = i18n("No");

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
        d->title2->show();
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
        d->labelPhotoFocalLength->setText(photoInfo.focalLength.isEmpty() ? unavailable : photoInfo.focalLength);
    else
    {
        str = i18n("%1 (35mm: %2)", photoInfo.focalLength,
                   photoInfo.focalLength35mm);
        d->labelPhotoFocalLength->setText(str);
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
