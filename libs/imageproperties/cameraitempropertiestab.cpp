/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2006-02-08
 * Description : A tab to display camera item informations
 *
 * Copyright 2006 by Gilles Caulier
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
#include <qfile.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include <qwhatsthis.h>
#include <qframe.h>

// KDE includes.

#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <kfileitem.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <kseparator.h>
#include <ksqueezedtextlabel.h>

// Local includes.

#include "dmetadata.h"
#include "gpiteminfo.h"
#include "navigatebarwidget.h"
#include "cameraitempropertiestab.h"

namespace Digikam
{

class CameraItemPropertiesTabPriv
{
public:

    CameraItemPropertiesTabPriv()
    {
        navigateBar            = 0;
        settingsArea           = 0;
        title2                 = 0;
        make                   = 0;
        model                  = 0;
        photoDate              = 0;
        aperture               = 0;
        focalLenght            = 0;
        exposureTime           = 0;
        sensitivity            = 0;
        exposureMode           = 0;
        flash                  = 0;
        whiteBalance           = 0;
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

    QLabel             *title2;
    QLabel             *make;
    QLabel             *model;
    QLabel             *photoDate;
    QLabel             *aperture;
    QLabel             *focalLenght;
    QLabel             *exposureTime;
    QLabel             *sensitivity;
    QLabel             *exposureMode;
    QLabel             *flash;
    QLabel             *whiteBalance;

    QFrame             *settingsArea;

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

    NavigateBarWidget  *navigateBar;
};

CameraItemPropertiesTab::CameraItemPropertiesTab(QWidget* parent, bool navBar)
                       : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new CameraItemPropertiesTabPriv;

    QVBoxLayout *vLayout        = new QVBoxLayout(this);
    d->navigateBar              = new NavigateBarWidget(this, navBar);
    d->settingsArea             = new QFrame(this);
    d->settingsArea->setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain);
    d->settingsArea->setMargin(0);
    d->settingsArea->setLineWidth(1);
    QGridLayout *settingsLayout = new QGridLayout(d->settingsArea, 26, 1, KDialog::marginHint(), 0);

    // --------------------------------------------------

    QLabel *title               = new QLabel(i18n("<big><b>Camera File Properties</b></big>"), d->settingsArea);
    QLabel *folder              = new QLabel(i18n("<b>Folder</b>:"), d->settingsArea);
    QLabel *date                = new QLabel(i18n("<b>Date</b>:"), d->settingsArea);
    QLabel *size                = new QLabel(i18n("<b>Size</b>:"), d->settingsArea);
    QLabel *isReadable          = new QLabel(i18n("<b>Readable</b>:"), d->settingsArea);
    QLabel *isWritable          = new QLabel(i18n("<b>Writable</b>:"), d->settingsArea);
    QLabel *mime                = new QLabel(i18n("<b>Type</b>:"), d->settingsArea);
    QLabel *dimensions          = new QLabel(i18n("<b>Dimensions</b>:"), d->settingsArea);
    QLabel *newFileName         = new QLabel(i18n("<nobr><b>New Name</b></nobr>:"), d->settingsArea);
    QLabel *downloaded          = new QLabel(i18n("<b>Downloaded</b>:"), d->settingsArea);

    KSeparator *line            = new KSeparator (Horizontal, d->settingsArea);
    d->title2                   = new QLabel(i18n("<big><b>Photograph Properties</b></big>"), d->settingsArea);
    d->make                     = new QLabel(i18n("<b>Make</b>:"), d->settingsArea);
    d->model                    = new QLabel(i18n("<b>Model</b>:"), d->settingsArea);
    d->photoDate                = new QLabel(i18n("<b>Created</b>:"), d->settingsArea);
    d->aperture                 = new QLabel(i18n("<b>Aperture</b>:"), d->settingsArea);
    d->focalLenght              = new QLabel(i18n("<b>Focal</b>:"), d->settingsArea);
    d->exposureTime             = new QLabel(i18n("<b>Exposure</b>:"), d->settingsArea);
    d->sensitivity              = new QLabel(i18n("<b>Sensitivity</b>:"), d->settingsArea);
    d->exposureMode             = new QLabel(i18n("<nobr><b>Mode/Program</b></nobr>:"), d->settingsArea);
    d->flash                    = new QLabel(i18n("<b>Flash</b>:"), d->settingsArea);
    d->whiteBalance             = new QLabel(i18n("<nobr><b>White balance</b></nobr>:"), d->settingsArea);

    d->labelFolder              = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelFileDate            = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelFileSize            = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelFileIsReadable      = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelFileIsWritable      = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelImageMime           = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelImageDimensions     = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelNewFileName         = new KSqueezedTextLabel(0, d->settingsArea);
    d->labelAlreadyDownloaded   = new KSqueezedTextLabel(0, d->settingsArea);

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

    title->setAlignment(Qt::AlignCenter);
    d->title2->setAlignment(Qt::AlignCenter);

    // --------------------------------------------------

    settingsLayout->addMultiCellWidget(title, 0, 0, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 1, 1, 0, 1);
    settingsLayout->addMultiCellWidget(folder, 2, 2, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFolder, 2, 2, 1, 1);
    settingsLayout->addMultiCellWidget(date, 3, 3, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileDate, 3, 3, 1, 1);
    settingsLayout->addMultiCellWidget(size, 4, 4, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileSize, 4, 4, 1, 1);
    settingsLayout->addMultiCellWidget(isReadable, 5, 5, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileIsReadable, 5, 5, 1, 1);
    settingsLayout->addMultiCellWidget(isWritable, 6, 6, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileIsWritable, 6, 6, 1, 1);
    settingsLayout->addMultiCellWidget(mime, 7, 7, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelImageMime, 7, 7, 1, 1);
    settingsLayout->addMultiCellWidget(dimensions, 8, 8, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelImageDimensions, 8, 8, 1, 1);
    settingsLayout->addMultiCellWidget(newFileName, 9, 9, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelNewFileName, 9, 9, 1, 1);
    settingsLayout->addMultiCellWidget(downloaded, 10, 10, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelAlreadyDownloaded, 10, 10, 1, 1);

    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 11, 11, 0, 1);
    settingsLayout->addMultiCellWidget(line, 12, 12, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 13, 13, 0, 1);

    settingsLayout->addMultiCellWidget(d->title2, 14, 14, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 15, 15, 0, 1);
    settingsLayout->addMultiCellWidget(d->make, 16, 16, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoMake, 16, 16, 1, 1);
    settingsLayout->addMultiCellWidget(d->model, 17, 17, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoModel, 17, 17, 1, 1);
    settingsLayout->addMultiCellWidget(d->photoDate, 18, 18, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoDateTime, 18, 18, 1, 1);
    settingsLayout->addMultiCellWidget(d->aperture, 19, 19, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoAperture, 19, 19, 1, 1);
    settingsLayout->addMultiCellWidget(d->focalLenght, 20, 20, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoFocalLenght, 20, 20, 1, 1);
    settingsLayout->addMultiCellWidget(d->exposureTime, 21, 21, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoExposureTime, 21, 21, 1, 1);
    settingsLayout->addMultiCellWidget(d->sensitivity, 22, 22, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoSensitivity, 22, 22, 1, 1);
    settingsLayout->addMultiCellWidget(d->exposureMode, 23, 23, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoExposureMode, 23, 23, 1, 1);
    settingsLayout->addMultiCellWidget(d->flash, 24, 24, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoFlash, 24, 24, 1, 1);
    settingsLayout->addMultiCellWidget(d->whiteBalance, 25, 25, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoWhiteBalance, 25, 25, 1, 1);
    settingsLayout->setRowStretch(26, 10);
    settingsLayout->setColStretch(1, 10);

    // --------------------------------------------------

    vLayout->addWidget(d->navigateBar);
    vLayout->addWidget(d->settingsArea);

    // --------------------------------------------------

    connect(d->navigateBar, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));

    connect(d->navigateBar, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(d->navigateBar, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->navigateBar, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));
}

CameraItemPropertiesTab::~CameraItemPropertiesTab()
{
    delete d;
}

void CameraItemPropertiesTab::setCurrentItem(const GPItemInfo* itemInfo, int itemType, 
                                             const QString &newFileName, const QByteArray& exifData,
                                             const KURL &currentURL)
{
    if (!itemInfo)
    {
        d->navigateBar->setFileName();

        d->labelFolder->setText(QString::null);
        d->labelFileIsReadable->setText(QString::null);
        d->labelFileIsWritable->setText(QString::null);
        d->labelFileDate->setText(QString::null);
        d->labelFileSize->setText(QString::null);
        d->labelImageMime->setText(QString::null);
        d->labelImageDimensions->setText(QString::null);
        d->labelNewFileName->setText(QString::null);
        d->labelAlreadyDownloaded->setText(QString::null);

        d->labelPhotoMake->setText(QString::null);
        d->labelPhotoModel->setText(QString::null);
        d->labelPhotoDateTime->setText(QString::null);
        d->labelPhotoAperture->setText(QString::null);
        d->labelPhotoFocalLenght->setText(QString::null);
        d->labelPhotoExposureTime->setText(QString::null);
        d->labelPhotoSensitivity->setText(QString::null);
        d->labelPhotoExposureMode->setText(QString::null);
        d->labelPhotoFlash->setText(QString::null);
        d->labelPhotoWhiteBalance->setText(QString::null);

        setEnabled(false);
        return;
    }

    setEnabled(true);

    QString str;
    QString unknown(i18n("<i>unknown</i>"));

    d->navigateBar->setFileName(itemInfo->name);
    d->navigateBar->setButtonsState(itemType);

    // -- Camera file system informations ------------------------------------------

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

    // -- Download informations ------------------------------------------

    d->labelNewFileName->setText(newFileName.isEmpty() ? i18n("<i>unchanged</i>") : newFileName);

    if (itemInfo->downloaded < 0)
        str = unknown;
    else if (itemInfo->downloaded == 0)
        str = i18n("No");
    else
        str = i18n("Yes");

    d->labelAlreadyDownloaded->setText(str);

    // -- Photograph informations ------------------------------------------
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
        d->focalLenght->hide();
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
        d->focalLenght->show();
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

    if (photoInfo.focalLenght35mm.isEmpty())
        d->labelPhotoFocalLenght->setText(photoInfo.focalLenght.isEmpty() ? unavailable : photoInfo.focalLenght);
    else 
    {
        str = i18n("%1 (35mm: %2)").arg(photoInfo.focalLenght).arg(photoInfo.focalLenght35mm);
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

#include "cameraitempropertiestab.moc"
