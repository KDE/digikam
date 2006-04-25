/* ============================================================
 * Author: Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2006-04-19
 * Description : A tab to display general image informations
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
#include <qfileinfo.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kdialogbase.h>
#include <kfileitem.h>
#include <kdebug.h>
#include <ksqueezedtextlabel.h>
#include <kseparator.h>

// Local includes.

#include "dmetadata.h"
#include "rawfiles.h"
#include "navigatebarwidget.h"
#include "imagepropertiestab.h"

namespace Digikam
{

class ImagePropertiesTabPriv
{
public:

    ImagePropertiesTabPriv()
    {
        navigateBar            = 0;
        labelFolder            = 0;
        labelFileModifiedDate  = 0;
        labelFileSize          = 0;
        labelFileOwner         = 0;
        labelFilePermissions   = 0;
        labelImageMime         = 0;
        labelImageDimensions   = 0;
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

    KSqueezedTextLabel *labelFolder;
    KSqueezedTextLabel *labelFileModifiedDate;
    KSqueezedTextLabel *labelFileSize;
    KSqueezedTextLabel *labelFileOwner;
    KSqueezedTextLabel *labelFilePermissions;
    
    KSqueezedTextLabel *labelImageMime;
    KSqueezedTextLabel *labelImageDimensions;
    
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

ImagePropertiesTab::ImagePropertiesTab(QWidget* parent, bool navBar)
                  : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new ImagePropertiesTabPriv;
    
    QVBoxLayout *vLayout        = new QVBoxLayout(this);
    d->navigateBar              = new NavigateBarWidget(this, navBar);
    QWidget *settingsArea       = new QWidget(this);
    QGridLayout *settingsLayout = new QGridLayout(settingsArea, 26, 1, KDialog::marginHint(), 0);

    // --------------------------------------------------
    
    QLabel *title               = new QLabel(i18n("<u><i>File Properties:</i></u>"), settingsArea);
    QLabel *folder              = new QLabel(i18n("<b>Folder</b>:"), settingsArea);
    QLabel *modifiedDate        = new QLabel(i18n("<b>Modified</b>:"), settingsArea);
    QLabel *size                = new QLabel(i18n("<b>Size</b>:"), settingsArea);
    QLabel *owner               = new QLabel(i18n("<b>Owner</b>:"), settingsArea);
    QLabel *permissions         = new QLabel(i18n("<b>Permissions</b>:"), settingsArea);

    KSeparator *line            = new KSeparator (Horizontal, settingsArea);
    QLabel *title2              = new QLabel(i18n("<u><i>Image Properties:</i></u>"), settingsArea);
    QLabel *mime                = new QLabel(i18n("<b>Type</b>:"), settingsArea);
    QLabel *dimensions          = new QLabel(i18n("<b>Dimensions</b>:"), settingsArea);

    KSeparator *line2           = new KSeparator (Horizontal, settingsArea);
    QLabel *title3              = new QLabel(i18n("<u><i>Photograph Properties:</i></u>"), settingsArea);
    QLabel *make                = new QLabel(i18n("<b>Make</b>:"), settingsArea);
    QLabel *model               = new QLabel(i18n("<b>Model</b>:"), settingsArea);
    QLabel *photoDate           = new QLabel(i18n("<b>Created</b>:"), settingsArea);
    QLabel *aperture            = new QLabel(i18n("<b>Aperture</b>:"), settingsArea);
    QLabel *focalLenght         = new QLabel(i18n("<b>Focal</b>:"), settingsArea);
    QLabel *exposureTime        = new QLabel(i18n("<b>Exposure</b>:"), settingsArea);
    QLabel *sensitivity         = new QLabel(i18n("<b>Sensitivity</b>:"), settingsArea);
    QLabel *exposureMode        = new QLabel(i18n("<nobr><b>Mode/Program</b></nobr>:"), settingsArea);
    QLabel *flash               = new QLabel(i18n("<b>Flash</b>:"), settingsArea);
    QLabel *whiteBalance        = new QLabel(i18n("<nobr><b>White balance</b></nobr>:"), settingsArea);

    d->labelFolder              = new KSqueezedTextLabel(0, settingsArea);
    d->labelFileModifiedDate    = new KSqueezedTextLabel(0, settingsArea);
    d->labelFileSize            = new KSqueezedTextLabel(0, settingsArea);
    d->labelFileOwner           = new KSqueezedTextLabel(0, settingsArea);
    d->labelFilePermissions     = new KSqueezedTextLabel(0, settingsArea);

    d->labelImageMime           = new KSqueezedTextLabel(0, settingsArea);
    d->labelImageDimensions     = new KSqueezedTextLabel(0, settingsArea);

    d->labelPhotoMake           = new KSqueezedTextLabel(0, settingsArea);
    d->labelPhotoModel          = new KSqueezedTextLabel(0, settingsArea);
    d->labelPhotoDateTime       = new KSqueezedTextLabel(0, settingsArea);
    d->labelPhotoAperture       = new KSqueezedTextLabel(0, settingsArea);
    d->labelPhotoFocalLenght    = new KSqueezedTextLabel(0, settingsArea);
    d->labelPhotoExposureTime   = new KSqueezedTextLabel(0, settingsArea);
    d->labelPhotoSensitivity    = new KSqueezedTextLabel(0, settingsArea);
    d->labelPhotoExposureMode   = new KSqueezedTextLabel(0, settingsArea);
    d->labelPhotoFlash          = new KSqueezedTextLabel(0, settingsArea);
    d->labelPhotoWhiteBalance   = new KSqueezedTextLabel(0, settingsArea);

    // --------------------------------------------------
    
    settingsLayout->addMultiCellWidget(title, 0, 0, 0, 1);
    settingsLayout->addMultiCellWidget(folder, 1, 1, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFolder, 1, 1, 1, 1);
    settingsLayout->addMultiCellWidget(modifiedDate, 2, 2, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileModifiedDate, 2, 2, 1, 1);
    settingsLayout->addMultiCellWidget(size, 3, 3, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileSize, 3, 3, 1, 1);
    settingsLayout->addMultiCellWidget(owner, 4, 4, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileOwner, 4, 4, 1, 1);
    settingsLayout->addMultiCellWidget(permissions, 5, 5, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFilePermissions, 5, 5, 1, 1);
    
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 6, 6, 0, 1);    
    settingsLayout->addMultiCellWidget(line, 7, 7, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 8, 8, 0, 1); 
    
    settingsLayout->addMultiCellWidget(title2, 9, 9, 0, 1);
    settingsLayout->addMultiCellWidget(mime, 10, 10, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelImageMime, 10, 10, 1, 1);
    settingsLayout->addMultiCellWidget(dimensions, 11, 11, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelImageDimensions, 11, 11, 1, 1);
    
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 12, 12, 0, 1);
    settingsLayout->addMultiCellWidget(line2, 13, 13, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 14, 14, 0, 1);  

    settingsLayout->addMultiCellWidget(title3, 15, 15, 0, 1);
    settingsLayout->addMultiCellWidget(make, 16, 16, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoMake, 16, 16, 1, 1);
    settingsLayout->addMultiCellWidget(model, 17, 17, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoModel, 17, 17, 1, 1);
    settingsLayout->addMultiCellWidget(photoDate, 18, 18, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoDateTime, 18, 18, 1, 1);
    settingsLayout->addMultiCellWidget(aperture, 19, 19, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoAperture, 19, 19, 1, 1);
    settingsLayout->addMultiCellWidget(focalLenght, 20, 20, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoFocalLenght, 20, 20, 1, 1);
    settingsLayout->addMultiCellWidget(exposureTime, 21, 21, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoExposureTime, 21, 21, 1, 1);
    settingsLayout->addMultiCellWidget(sensitivity, 22, 22, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoSensitivity, 22, 22, 1, 1);
    settingsLayout->addMultiCellWidget(exposureMode, 23, 23, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoExposureMode, 23, 23, 1, 1);
    settingsLayout->addMultiCellWidget(flash, 24, 24, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoFlash, 24, 24, 1, 1);
    settingsLayout->addMultiCellWidget(whiteBalance, 25, 25, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoWhiteBalance, 25, 25, 1, 1);
    
    settingsLayout->setRowStretch(26, 10);
    settingsLayout->setColStretch(1, 10);
    
    // --------------------------------------------------
    
    vLayout->addWidget(d->navigateBar);
    vLayout->addWidget(settingsArea);    
    
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

ImagePropertiesTab::~ImagePropertiesTab()
{
    delete d;
}

void ImagePropertiesTab::setCurrentURL(const KURL& url, int itemType)
{
    if (url.isEmpty())
    {
        d->navigateBar->setFileName();
        
        d->labelFolder->setText(QString::null);
        d->labelFileModifiedDate->setText(QString::null);
        d->labelFileSize->setText(QString::null);
        d->labelFileOwner->setText(QString::null);
        d->labelFilePermissions->setText(QString::null);
        
        d->labelImageMime->setText(QString::null);
        d->labelImageDimensions->setText(QString::null);
        
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
    QString unavailable(i18n("<i>unavailable</i>"));
    
    KFileItem fi(KFileItem::Unknown, KFileItem::Unknown, url);
    QFileInfo fileInfo(url.path());
    DMetadata metaData(url.path());
    
    d->navigateBar->setFileName(url.filename());
    d->navigateBar->setButtonsState(itemType);

    // -- File system informations ------------------------------------------

    d->labelFolder->setText(url.directory());
    
    QDateTime modifiedDate = fileInfo.lastModified();
    str = KGlobal::locale()->formatDateTime(modifiedDate, true, true);
    d->labelFileModifiedDate->setText(str);

    str = QString("%1 (%2)").arg(KIO::convertSize(fi.size()))
                            .arg(KGlobal::locale()->formatNumber(fi.size(), 0));
    d->labelFileSize->setText(str);

    d->labelFileOwner->setText( QString("%1 - %2").arg(fi.user()).arg(fi.group()) );
    d->labelFilePermissions->setText( fi.permissionsString() );
    
    // -- Image Properties --------------------------------------------------
    
    QSize   dims;
    QString rawFilesExt(raw_file_extentions);

    if (rawFilesExt.upper().contains( fileInfo.extension().upper() ))
    {
        d->labelImageMime->setText(i18n("RAW Image"));
        dims = metaData.getImageDimensions();
    }
    else
    {
        d->labelImageMime->setText(fi.mimeComment());

        KFileMetaInfo meta = fi.metaInfo();
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

    QString mpixels;
    mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 1);
    str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)")
          .arg(dims.width()).arg(dims.height()).arg(mpixels);
    d->labelImageDimensions->setText(str);

    // -- Photograph informations ------------------------------------------
    // NOTA: If something is changed here, please updated albumfiletip section too.
    
    PhotoInfoContainer photoInfo = metaData.getPhotographInformations();

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

#include "imagepropertiestab.moc"
