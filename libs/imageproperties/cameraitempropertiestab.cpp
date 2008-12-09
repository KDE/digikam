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

// Qt includes.

#include <qlayout.h>
#include <qstyle.h>
#include <qfile.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include <qwhatsthis.h>
#include <qframe.h>
#include <qscrollview.h>

// KDE includes.

#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <kfileitem.h>
#include <kmimetype.h>
#include <kseparator.h>
#include <ksqueezedtextlabel.h>

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "gpiteminfo.h"
#include "navigatebarwidget.h"
#include "cameraitempropertiestab.h"
#include "cameraitempropertiestab.moc"

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
        settingsArea           = 0;
        title2                 = 0;
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
    KSqueezedTextLabel *labelPhotoAperture;
    KSqueezedTextLabel *labelPhotoFocalLenght;
    KSqueezedTextLabel *labelPhotoExposureTime;
    KSqueezedTextLabel *labelPhotoSensitivity;
    KSqueezedTextLabel *labelPhotoExposureMode;
    KSqueezedTextLabel *labelPhotoFlash;
    KSqueezedTextLabel *labelPhotoWhiteBalance;
};

CameraItemPropertiesTab::CameraItemPropertiesTab(QWidget* parent, bool navBar)
                       : NavigateBarTab(parent)
{
    d = new CameraItemPropertiesTabPriv;

    setupNavigateBar(navBar);

    QScrollView *sv = new QScrollView(this);
    sv->viewport()->setBackgroundMode(Qt::PaletteBackground);
    sv->setResizePolicy(QScrollView::AutoOneFit);
    sv->setFrameStyle(QFrame::NoFrame);

    d->settingsArea = new QFrame(sv->viewport());
    d->settingsArea->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    d->settingsArea->setLineWidth( style().pixelMetric(QStyle::PM_DefaultFrameWidth, this) );

    sv->addChild(d->settingsArea);
    m_navigateBarLayout->addWidget(sv);

    // --------------------------------------------------

    QGridLayout *settingsLayout = new QGridLayout(d->settingsArea, 27, 1, KDialog::spacingHint(), 0);

    d->title                  = new QLabel(i18n("<big><b>Camera File Properties</b></big>"), d->settingsArea);
    d->file                   = new QLabel(i18n("<b>File</b>:"), d->settingsArea);
    d->folder                 = new QLabel(i18n("<b>Folder</b>:"), d->settingsArea);
    d->date                   = new QLabel(i18n("<b>Date</b>:"), d->settingsArea);
    d->size                   = new QLabel(i18n("<b>Size</b>:"), d->settingsArea);
    d->isReadable             = new QLabel(i18n("<b>Readable</b>:"), d->settingsArea);
    d->isWritable             = new QLabel(i18n("<b>Writable</b>:"), d->settingsArea);
    d->mime                   = new QLabel(i18n("<b>Type</b>:"), d->settingsArea);
    d->dimensions             = new QLabel(i18n("<b>Dimensions</b>:"), d->settingsArea);
    d->newFileName            = new QLabel(i18n("<nobr><b>New Name</b></nobr>:"), d->settingsArea);
    d->downloaded             = new QLabel(i18n("<b>Downloaded</b>:"), d->settingsArea);

    KSeparator *line          = new KSeparator (Horizontal, d->settingsArea);
    d->title2                 = new QLabel(i18n("<big><b>Photograph Properties</b></big>"), d->settingsArea);
    d->make                   = new QLabel(i18n("<b>Make</b>:"), d->settingsArea);
    d->model                  = new QLabel(i18n("<b>Model</b>:"), d->settingsArea);
    d->photoDate              = new QLabel(i18n("<b>Created</b>:"), d->settingsArea);
    d->aperture               = new QLabel(i18n("<b>Aperture</b>:"), d->settingsArea);
    d->focalLength            = new QLabel(i18n("<b>Focal</b>:"), d->settingsArea);
    d->exposureTime           = new QLabel(i18n("<b>Exposure</b>:"), d->settingsArea);
    d->sensitivity            = new QLabel(i18n("<b>Sensitivity</b>:"), d->settingsArea);
    d->exposureMode           = new QLabel(i18n("<nobr><b>Mode/Program</b></nobr>:"), d->settingsArea);
    d->flash                  = new QLabel(i18n("<b>Flash</b>:"), d->settingsArea);
    d->whiteBalance           = new QLabel(i18n("<nobr><b>White balance</b></nobr>:"), d->settingsArea);

    d->labelFile              = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelFolder            = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelFileDate          = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelFileSize          = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelFileIsReadable    = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelFileIsWritable    = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelImageMime         = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelImageDimensions   = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelNewFileName       = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelAlreadyDownloaded = new KSqueezedTextLabel(0, d->settingsArea);

    d->labelPhotoMake         = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoModel        = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoDateTime     = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoAperture     = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoFocalLenght  = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoExposureTime = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoSensitivity  = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoExposureMode = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoFlash        = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelPhotoWhiteBalance = new KSqueezedTextLabel(0, d->settingsArea);

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

    settingsLayout->addMultiCellWidget(d->title, 0, 0, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 1, 1, 0, 1);
    settingsLayout->addMultiCellWidget(d->file, 2, 2, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFile, 2, 2, 1, 1);
    settingsLayout->addMultiCellWidget(d->folder, 3, 3, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFolder, 3, 3, 1, 1);
    settingsLayout->addMultiCellWidget(d->date, 4, 4, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileDate, 4, 4, 1, 1);
    settingsLayout->addMultiCellWidget(d->size, 5, 5, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileSize, 5, 5, 1, 1);
    settingsLayout->addMultiCellWidget(d->isReadable, 6, 6, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileIsReadable, 6, 6, 1, 1);
    settingsLayout->addMultiCellWidget(d->isWritable, 7, 7, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileIsWritable, 7, 7, 1, 1);
    settingsLayout->addMultiCellWidget(d->mime, 8, 8, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelImageMime, 8, 8, 1, 1);
    settingsLayout->addMultiCellWidget(d->dimensions, 9, 9, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelImageDimensions, 9, 9, 1, 1);
    settingsLayout->addMultiCellWidget(d->newFileName, 10, 10, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelNewFileName, 10, 10, 1, 1);
    settingsLayout->addMultiCellWidget(d->downloaded, 11, 11, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelAlreadyDownloaded, 11, 11, 1, 1);

    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 12, 12, 0, 1);
    settingsLayout->addMultiCellWidget(line, 13, 13, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 14, 14, 0, 1);

    settingsLayout->addMultiCellWidget(d->title2, 15, 15, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 16, 16, 0, 1);
    settingsLayout->addMultiCellWidget(d->make, 17, 17, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoMake, 17, 17, 1, 1);
    settingsLayout->addMultiCellWidget(d->model, 18, 18, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoModel, 18, 18, 1, 1);
    settingsLayout->addMultiCellWidget(d->photoDate, 19, 19, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoDateTime, 19, 19, 1, 1);
    settingsLayout->addMultiCellWidget(d->aperture, 20, 20, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoAperture, 20, 20, 1, 1);
    settingsLayout->addMultiCellWidget(d->focalLength, 21, 21, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoFocalLenght, 21, 21, 1, 1);
    settingsLayout->addMultiCellWidget(d->exposureTime, 22, 22, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoExposureTime, 22, 22, 1, 1);
    settingsLayout->addMultiCellWidget(d->sensitivity, 23, 23, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoSensitivity, 23, 23, 1, 1);
    settingsLayout->addMultiCellWidget(d->exposureMode, 24, 24, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoExposureMode, 24, 24, 1, 1);
    settingsLayout->addMultiCellWidget(d->flash, 25, 25, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoFlash, 25, 25, 1, 1);
    settingsLayout->addMultiCellWidget(d->whiteBalance, 26, 26, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoWhiteBalance, 26, 26, 1, 1);
    settingsLayout->setRowStretch(27, 10);
    settingsLayout->setColStretch(1, 10);
}

CameraItemPropertiesTab::~CameraItemPropertiesTab()
{
    delete d;
}

void CameraItemPropertiesTab::setCurrentItem(const GPItemInfo* itemInfo,
                                             const QString &newFileName, const QByteArray& exifData,
                                             const KURL &currentURL)
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

    QDateTime date;
    date.setTime_t(itemInfo->mtime);
    d->labelFileDate->setText(KGlobal::locale()->formatDateTime(date, true, true));

    str = i18n("%1 (%2)").arg(KIO::convertSize(itemInfo->size))
                         .arg(KGlobal::locale()->formatNumber(itemInfo->size, 0));
    d->labelFileSize->setText(str);

    // -- Image Properties --------------------------------------------------

    d->labelImageMime->setText( (itemInfo->mime == QString("image/x-raw")) ? 
                               i18n("RAW Image") : KMimeType::mimeType(itemInfo->mime)->comment() );

    QString mpixels;
    QSize dims;
    if (itemInfo->width == -1 && itemInfo->height == -1 && !currentURL.isEmpty())
    {
        // delayed loading to list faster from UMSCamera
        if (itemInfo->mime == QString("image/x-raw"))
        {
            DMetadata metaData(currentURL.path());
            dims = metaData.getImageDimensions();
        }
        else
        {
            KFileMetaInfo meta(currentURL.path());
            if (meta.isValid())
            {
                if (meta.containsGroup("Jpeg EXIF Data"))
                    dims = meta.group("Jpeg EXIF Data").item("Dimensions").value().toSize();
                else if (meta.containsGroup("General"))
                    dims = meta.group("General").item("Dimensions").value().toSize();
                else if (meta.containsGroup("Technical"))
                    dims = meta.group("Technical").item("Dimensions").value().toSize();
            }
        }
    }
    else
    {
        // if available (GPCamera), take dimensions directly from itemInfo
        dims = QSize(itemInfo->width, itemInfo->height);
    }
    mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
    str = (!dims.isValid()) ? unknown : i18n("%1x%2 (%3Mpx)")
          .arg(dims.width()).arg(dims.height()).arg(mpixels);
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
    // NOTA: If something is changed here, please updated albumfiletip section too.

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
        str = KGlobal::locale()->formatDateTime(photoInfo.dateTime, true, true);
        d->labelPhotoDateTime->setText(str);
    }
    else
        d->labelPhotoDateTime->setText(unavailable);

    d->labelPhotoAperture->setText(photoInfo.aperture.isEmpty() ? unavailable : photoInfo.aperture);

    if (photoInfo.focalLength35mm.isEmpty())
        d->labelPhotoFocalLenght->setText(photoInfo.focalLength.isEmpty() ? unavailable : photoInfo.focalLength);
    else 
    {
        str = i18n("%1 (35mm: %2)").arg(photoInfo.focalLength).arg(photoInfo.focalLength35mm);
        d->labelPhotoFocalLenght->setText(str);
    }

    d->labelPhotoExposureTime->setText(photoInfo.exposureTime.isEmpty() ? unavailable : photoInfo.exposureTime);
    d->labelPhotoSensitivity->setText(photoInfo.sensitivity.isEmpty() ? unavailable : i18n("%1 ISO").arg(photoInfo.sensitivity));

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

}  // NameSpace Digikam
