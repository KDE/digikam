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
        title3                 = 0;
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

    QLabel             *title3;
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
    QGridLayout *settingsLayout = new QGridLayout(settingsArea, 29, 1, KDialog::marginHint(), 0);

    // --------------------------------------------------
    
    QLabel *title               = new QLabel(i18n("<big><b>File Properties</b></big>"), settingsArea);
    QLabel *folder              = new QLabel(i18n("<b>Folder</b>:"), settingsArea);
    QLabel *modifiedDate        = new QLabel(i18n("<b>Modified</b>:"), settingsArea);
    QLabel *size                = new QLabel(i18n("<b>Size</b>:"), settingsArea);
    QLabel *owner               = new QLabel(i18n("<b>Owner</b>:"), settingsArea);
    QLabel *permissions         = new QLabel(i18n("<b>Permissions</b>:"), settingsArea);

    KSeparator *line            = new KSeparator (Horizontal, settingsArea);
    QLabel *title2              = new QLabel(i18n("<big><b>Image Properties</b></big>"), settingsArea);
    QLabel *mime                = new QLabel(i18n("<b>Type</b>:"), settingsArea);
    QLabel *dimensions          = new QLabel(i18n("<b>Dimensions</b>:"), settingsArea);

    KSeparator *line2           = new KSeparator (Horizontal, settingsArea);
    d->title3                   = new QLabel(i18n("<big><b>Photograph Properties</b></big>"), settingsArea);
    d->make                     = new QLabel(i18n("<b>Make</b>:"), settingsArea);
    d->model                    = new QLabel(i18n("<b>Model</b>:"), settingsArea);
    d->photoDate                = new QLabel(i18n("<b>Created</b>:"), settingsArea);
    d->aperture                 = new QLabel(i18n("<b>Aperture</b>:"), settingsArea);
    d->focalLenght              = new QLabel(i18n("<b>Focal</b>:"), settingsArea);
    d->exposureTime             = new QLabel(i18n("<b>Exposure</b>:"), settingsArea);
    d->sensitivity              = new QLabel(i18n("<b>Sensitivity</b>:"), settingsArea);
    d->exposureMode             = new QLabel(i18n("<nobr><b>Mode/Program</b></nobr>:"), settingsArea);
    d->flash                    = new QLabel(i18n("<b>Flash</b>:"), settingsArea);
    d->whiteBalance             = new QLabel(i18n("<nobr><b>White balance</b></nobr>:"), settingsArea);

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

    title->setAlignment(Qt::AlignCenter);
    title2->setAlignment(Qt::AlignCenter);
    d->title3->setAlignment(Qt::AlignCenter);
    
    // --------------------------------------------------
    
    settingsLayout->addMultiCellWidget(title, 0, 0, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 1, 1, 0, 1);    
    settingsLayout->addMultiCellWidget(folder, 2, 2, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFolder, 2, 2, 1, 1);
    settingsLayout->addMultiCellWidget(modifiedDate, 3, 3, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileModifiedDate, 3, 3, 1, 1);
    settingsLayout->addMultiCellWidget(size, 4, 4, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileSize, 4, 4, 1, 1);
    settingsLayout->addMultiCellWidget(owner, 5, 5, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileOwner, 5, 5, 1, 1);
    settingsLayout->addMultiCellWidget(permissions, 6, 6, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFilePermissions, 6, 6, 1, 1);
    
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 7, 7, 0, 1);    
    settingsLayout->addMultiCellWidget(line, 8, 8, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 9, 9, 0, 1); 
    
    settingsLayout->addMultiCellWidget(title2, 10, 10, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 11, 11, 0, 1); 
    settingsLayout->addMultiCellWidget(mime, 12, 12, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelImageMime, 12, 12, 1, 1);
    settingsLayout->addMultiCellWidget(dimensions, 13, 13, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelImageDimensions, 13, 13, 1, 1);
    
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 14, 14, 0, 1);
    settingsLayout->addMultiCellWidget(line2, 15, 15, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 16, 16, 0, 1);  

    settingsLayout->addMultiCellWidget(d->title3, 17, 17, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 18, 18, 0, 1);  
    settingsLayout->addMultiCellWidget(d->make, 19, 19, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoMake, 19, 19, 1, 1);
    settingsLayout->addMultiCellWidget(d->model, 20, 20, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoModel, 20, 20, 1, 1);
    settingsLayout->addMultiCellWidget(d->photoDate, 21, 21, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoDateTime, 21, 21, 1, 1);
    settingsLayout->addMultiCellWidget(d->aperture, 22, 22, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoAperture, 22, 22, 1, 1);
    settingsLayout->addMultiCellWidget(d->focalLenght, 23, 23, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoFocalLenght, 23, 23, 1, 1);
    settingsLayout->addMultiCellWidget(d->exposureTime, 24, 24, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoExposureTime, 24, 24, 1, 1);
    settingsLayout->addMultiCellWidget(d->sensitivity, 25, 25, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoSensitivity, 25, 25, 1, 1);
    settingsLayout->addMultiCellWidget(d->exposureMode, 26, 26, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoExposureMode, 26, 26, 1, 1);
    settingsLayout->addMultiCellWidget(d->flash, 27, 27, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoFlash, 27, 27, 1, 1);
    settingsLayout->addMultiCellWidget(d->whiteBalance, 28, 28, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoWhiteBalance, 28, 28, 1, 1);
    
    settingsLayout->setRowStretch(29, 10);
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
    
    if (photoInfo.isEmpty())
    {
        d->title3->hide();
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
        d->title3->show();
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

#include "imagepropertiestab.moc"
