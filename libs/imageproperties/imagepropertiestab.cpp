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
        labelPhotoExposureTime = 0;
        labelPhotoFocalLenght  = 0;
        labelPhotoSensitivity  = 0;
        labelPhotoFlash        = 0;
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
    KSqueezedTextLabel *labelPhotoExposureTime;
    KSqueezedTextLabel *labelPhotoFocalLenght;
    KSqueezedTextLabel *labelPhotoSensitivity;
    KSqueezedTextLabel *labelPhotoFlash;
    
    NavigateBarWidget  *navigateBar;
};

ImagePropertiesTab::ImagePropertiesTab(QWidget* parent, bool navBar)
                  : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new ImagePropertiesTabPriv;
    QGridLayout *topLayout = new QGridLayout(this, 25, 1, KDialog::marginHint(), 0);

    d->navigateBar = new NavigateBarWidget(this, navBar);
    topLayout->addMultiCellWidget(d->navigateBar, 0, 0, 0, 1);

    QLabel *title             = new QLabel(i18n("<u><i>File Properties:</i></u>"), this);
    QLabel *folder            = new QLabel(i18n("<b>Folder</b>:"), this);
    QLabel *modifiedDate      = new QLabel(i18n("<b>Modified</b>:"), this);
    QLabel *size              = new QLabel(i18n("<b>Size</b>:"), this);
    QLabel *owner             = new QLabel(i18n("<b>Owner</b>:"), this);
    QLabel *permissions       = new QLabel(i18n("<b>Permissions</b>:"), this);

    KSeparator *line          = new KSeparator (Horizontal, this);
    QLabel *title2            = new QLabel(i18n("<u><i>Image Properties:</i></u>"), this);
    QLabel *mime              = new QLabel(i18n("<b>Mime</b>:"), this);
    QLabel *dimensions        = new QLabel(i18n("<b>Dimensions</b>:"), this);

    KSeparator *line2         = new KSeparator (Horizontal, this);
    QLabel *title3            = new QLabel(i18n("<u><i>Photograph Properties:</i></u>"), this);
    QLabel *make              = new QLabel(i18n("<b>Make</b>:"), this);
    QLabel *model             = new QLabel(i18n("<b>Model</b>:"), this);
    QLabel *photoDate         = new QLabel(i18n("<b>Created</b>:"), this);
    QLabel *aperture          = new QLabel(i18n("<b>Aperture</b>:"), this);
    QLabel *exposureTime      = new QLabel(i18n("<b>Exposure</b>:"), this);
    QLabel *focalLenght       = new QLabel(i18n("<b>Focal</b>:"), this);
    QLabel *sensitivity       = new QLabel(i18n("<b>Sensitivity</b>:"), this);
    QLabel *flash             = new QLabel(i18n("<b>Flash</b>:"), this);

    d->labelFolder            = new KSqueezedTextLabel(0, this);
    d->labelFileModifiedDate  = new KSqueezedTextLabel(0, this);
    d->labelFileSize          = new KSqueezedTextLabel(0, this);
    d->labelFileOwner         = new KSqueezedTextLabel(0, this);
    d->labelFilePermissions   = new KSqueezedTextLabel(0, this);

    d->labelImageMime         = new KSqueezedTextLabel(0, this);
    d->labelImageDimensions   = new KSqueezedTextLabel(0, this);

    d->labelPhotoMake         = new KSqueezedTextLabel(0, this);
    d->labelPhotoModel        = new KSqueezedTextLabel(0, this);
    d->labelPhotoDateTime     = new KSqueezedTextLabel(0, this);
    d->labelPhotoAperture     = new KSqueezedTextLabel(0, this);
    d->labelPhotoExposureTime = new KSqueezedTextLabel(0, this);
    d->labelPhotoFocalLenght  = new KSqueezedTextLabel(0, this);
    d->labelPhotoSensitivity  = new KSqueezedTextLabel(0, this);
    d->labelPhotoFlash        = new KSqueezedTextLabel(0, this);

    topLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                            QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 0, 0, 0, 1);
    topLayout->addMultiCellWidget(title, 1, 1, 0, 1);
    topLayout->addMultiCellWidget(folder, 2, 2, 0, 0);
    topLayout->addMultiCellWidget(d->labelFolder, 2, 2, 1, 1);
    topLayout->addMultiCellWidget(modifiedDate, 3, 3, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileModifiedDate, 3, 3, 1, 1);
    topLayout->addMultiCellWidget(size, 4, 4, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileSize, 4, 4, 1, 1);
    topLayout->addMultiCellWidget(owner, 5, 5, 0, 0);
    topLayout->addMultiCellWidget(d->labelFileOwner, 5, 5, 1, 1);
    topLayout->addMultiCellWidget(permissions, 6, 6, 0, 0);
    topLayout->addMultiCellWidget(d->labelFilePermissions, 6, 6, 1, 1);
    
    topLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                            QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 7, 7, 0, 1);    
    topLayout->addMultiCellWidget(line, 8, 8, 0, 1);
    topLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                            QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 9, 9, 0, 1); 
    
    topLayout->addMultiCellWidget(title2, 10, 10, 0, 1);
    topLayout->addMultiCellWidget(mime, 11, 11, 0, 0);
    topLayout->addMultiCellWidget(d->labelImageMime, 11, 11, 1, 1);
    topLayout->addMultiCellWidget(dimensions, 12, 12, 0, 0);
    topLayout->addMultiCellWidget(d->labelImageDimensions, 12, 12, 1, 1);
    
    topLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                            QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 13, 13, 0, 1);
    topLayout->addMultiCellWidget(line2, 14, 14, 0, 1);
    topLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                            QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 15, 15, 0, 1);  

    topLayout->addMultiCellWidget(title3, 16, 16, 0, 1);
    topLayout->addMultiCellWidget(make, 17, 17, 0, 0);
    topLayout->addMultiCellWidget(d->labelPhotoMake, 17, 17, 1, 1);
    topLayout->addMultiCellWidget(model, 18, 18, 0, 0);
    topLayout->addMultiCellWidget(d->labelPhotoModel, 18, 18, 1, 1);
    topLayout->addMultiCellWidget(photoDate, 19, 19, 0, 0);
    topLayout->addMultiCellWidget(d->labelPhotoDateTime, 19, 19, 1, 1);
    topLayout->addMultiCellWidget(aperture, 20, 20, 0, 0);
    topLayout->addMultiCellWidget(d->labelPhotoAperture, 20, 20, 1, 1);
    topLayout->addMultiCellWidget(exposureTime, 21, 21, 0, 0);
    topLayout->addMultiCellWidget(d->labelPhotoExposureTime, 21, 21, 1, 1);
    topLayout->addMultiCellWidget(focalLenght, 22, 22, 0, 0);
    topLayout->addMultiCellWidget(d->labelPhotoFocalLenght, 22, 22, 1, 1);
    topLayout->addMultiCellWidget(sensitivity, 23, 23, 0, 0);
    topLayout->addMultiCellWidget(d->labelPhotoSensitivity, 23, 23, 1, 1);
    topLayout->addMultiCellWidget(flash, 24, 24, 0, 0);
    topLayout->addMultiCellWidget(d->labelPhotoFlash, 24, 24, 1, 1);

    topLayout->setRowStretch(25, 10);
    topLayout->setColStretch(1, 10);
            
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
        d->labelPhotoExposureTime->setText(QString::null);
        d->labelPhotoFocalLenght->setText(QString::null);
        d->labelPhotoSensitivity->setText(QString::null);
        d->labelPhotoFlash->setText(QString::null);

        setEnabled(false);
        return;
    }

    // -- File system informations ------------------------------------------

    QString str;

    setEnabled(true);

    d->navigateBar->setFileName(url.filename());
    d->navigateBar->setButtonsState(itemType);

    d->labelFolder->setText(url.directory());

    KFileItem *fi = new KFileItem(KFileItem::Unknown, KFileItem::Unknown, url);
    QFileInfo fileInfo(url.path());
    
    QDateTime modifiedDate  = fileInfo.lastModified();
    str = KGlobal::locale()->formatDateTime(modifiedDate, true, true);
    d->labelFileModifiedDate->setText(str);

    str = i18n("%1 (%2)").arg(KIO::convertSize(fi->size()))
                         .arg(KGlobal::locale()->formatNumber(fi->size(), 0));
    d->labelFileSize->setText(str);

    d->labelFileOwner->setText( i18n("%1 - %2").arg(fi->user()).arg(fi->group()) );
    d->labelFilePermissions->setText( fi->permissionsString() );
    
    // -- Image Properties ------------------------------------------
    
    DMetadata metaData(url.path());

    QSize   dims;
    QString rawFilesExt(raw_file_extentions);

    if (rawFilesExt.upper().contains( fileInfo.extension().upper() ))
    {
        d->labelImageMime->setText(i18n("RAW Image"));
        dims = metaData.getImageDimensions();
    }
    else
    {
        d->labelImageMime->setText(fi->mimeComment());

        KFileMetaInfo meta = fi->metaInfo();
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

    QString unavialable(i18n("<i>unavailable</i>"));
    PhotoInfoContainer photoInfo = metaData.getPhotographInformations();

    d->labelPhotoMake->setText(photoInfo.make.isEmpty() ? unavialable : photoInfo.make);
    d->labelPhotoModel->setText(photoInfo.model.isEmpty() ? unavialable : photoInfo.model);

    if (photoInfo.dateTime.isValid())
    {
        str = KGlobal::locale()->formatDateTime(photoInfo.dateTime, true, true);
        d->labelPhotoDateTime->setText(str);
    }
    else
        d->labelPhotoDateTime->setText(unavialable);

    d->labelPhotoAperture->setText(photoInfo.aperture.isEmpty() ? unavialable : photoInfo.aperture);
    d->labelPhotoExposureTime->setText(photoInfo.exposureTime.isEmpty() ? unavialable : photoInfo.exposureTime);

    if (photoInfo.focalLenght35mm.isEmpty())
        d->labelPhotoFocalLenght->setText(photoInfo.focalLenght.isEmpty() ? 
                                          unavialable : photoInfo.focalLenght);
    else 
        d->labelPhotoFocalLenght->setText(i18n("%1 (35mm: %2)")
                                          .arg(photoInfo.focalLenght).arg(photoInfo.focalLenght35mm));

    d->labelPhotoSensitivity->setText(photoInfo.sensitivity.isEmpty() ? unavialable : photoInfo.sensitivity);
    d->labelPhotoFlash->setText(photoInfo.flash.isEmpty() ? unavialable : photoInfo.flash);
}
    
}  // NameSpace Digikam

#include "imagepropertiestab.moc"
