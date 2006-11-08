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
#include <qframe.h>

// KDE includes.

#include <klocale.h>
#include <kdialogbase.h>
#include <kfileitem.h>
#include <ksqueezedtextlabel.h>
#include <kseparator.h>

// Local includes.

#include "ddebug.h"
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
        settingsArea           = 0;
        title                  = 0;
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
    QLabel             *focalLenght;
    QLabel             *exposureTime;
    QLabel             *sensitivity;
    QLabel             *exposureMode;
    QLabel             *flash;
    QLabel             *whiteBalance;

    QFrame             *settingsArea;

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

    NavigateBarWidget  *navigateBar;
};

ImagePropertiesTab::ImagePropertiesTab(QWidget* parent, bool navBar)
                  : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new ImagePropertiesTabPriv;

    QVBoxLayout *vLayout        = new QVBoxLayout(this);
    d->navigateBar              = new NavigateBarWidget(this, navBar);
    d->settingsArea             = new QFrame(this);
    d->settingsArea->setFrameStyle(QFrame::GroupBoxPanel|QFrame::Plain);
    d->settingsArea->setMargin(0);
    d->settingsArea->setLineWidth(1);
    QGridLayout *settingsLayout = new QGridLayout(d->settingsArea, 32, 1, KDialog::marginHint(), 0);

    // --------------------------------------------------

    d->title                    = new QLabel(i18n("<big><b>File Properties</b></big>"), d->settingsArea);
    d->folder                   = new QLabel(i18n("<b>Folder</b>:"), d->settingsArea);
    d->modifiedDate             = new QLabel(i18n("<b>Modified</b>:"), d->settingsArea);
    d->size                     = new QLabel(i18n("<b>Size</b>:"), d->settingsArea);
    d->owner                    = new QLabel(i18n("<b>Owner</b>:"), d->settingsArea);
    d->permissions              = new QLabel(i18n("<b>Permissions</b>:"), d->settingsArea);

    KSeparator *line            = new KSeparator (Horizontal, d->settingsArea);
    d->title2                   = new QLabel(i18n("<big><b>Image Properties</b></big>"), d->settingsArea);
    d->mime                     = new QLabel(i18n("<b>Type</b>:"), d->settingsArea);
    d->dimensions               = new QLabel(i18n("<b>Dimensions</b>:"), d->settingsArea);
    d->compression              = new QLabel(i18n("<b>Compression</b>:"), d->settingsArea);
    d->bitDepth                 = new QLabel(i18n("<nobr><b>Bits Depth</b></nobr>:"), d->settingsArea);
    d->colorMode                = new QLabel(i18n("<nobr><b>Color Mode</b></nobr>:"), d->settingsArea);

    KSeparator *line2           = new KSeparator (Horizontal, d->settingsArea);
    d->title3                   = new QLabel(i18n("<big><b>Photograph Properties</b></big>"), d->settingsArea);
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

    d->title->setAlignment(Qt::AlignCenter);
    d->title2->setAlignment(Qt::AlignCenter);
    d->title3->setAlignment(Qt::AlignCenter);

    // --------------------------------------------------

    settingsLayout->addMultiCellWidget(d->title, 0, 0, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 1, 1, 0, 1);
    settingsLayout->addMultiCellWidget(d->folder, 2, 2, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFolder, 2, 2, 1, 1);
    settingsLayout->addMultiCellWidget(d->modifiedDate, 3, 3, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileModifiedDate, 3, 3, 1, 1);
    settingsLayout->addMultiCellWidget(d->size, 4, 4, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileSize, 4, 4, 1, 1);
    settingsLayout->addMultiCellWidget(d->owner, 5, 5, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFileOwner, 5, 5, 1, 1);
    settingsLayout->addMultiCellWidget(d->permissions, 6, 6, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelFilePermissions, 6, 6, 1, 1);

    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 7, 7, 0, 1);
    settingsLayout->addMultiCellWidget(line, 8, 8, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 9, 9, 0, 1);

    settingsLayout->addMultiCellWidget(d->title2, 10, 10, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 11, 11, 0, 1);
    settingsLayout->addMultiCellWidget(d->mime, 12, 12, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelImageMime, 12, 12, 1, 1);
    settingsLayout->addMultiCellWidget(d->dimensions, 13, 13, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelImageDimensions, 13, 13, 1, 1);
    settingsLayout->addMultiCellWidget(d->compression, 14, 14, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelImageCompression, 14, 14, 1, 1);
    settingsLayout->addMultiCellWidget(d->bitDepth, 15, 15, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelImageBitDepth, 15, 15, 1, 1);
    settingsLayout->addMultiCellWidget(d->colorMode, 16, 16, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelImageColorMode, 16, 16, 1, 1);

    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 17, 17, 0, 1);
    settingsLayout->addMultiCellWidget(line2, 18, 18, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 19, 19, 0, 1);

    settingsLayout->addMultiCellWidget(d->title3, 20, 20, 0, 1);
    settingsLayout->addMultiCell(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(), 
                                 QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 21, 21, 0, 1);
    settingsLayout->addMultiCellWidget(d->make, 22, 22, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoMake, 22, 22, 1, 1);
    settingsLayout->addMultiCellWidget(d->model, 23, 23, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoModel, 23, 23, 1, 1);
    settingsLayout->addMultiCellWidget(d->photoDate, 24, 24, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoDateTime, 24, 24, 1, 1);
    settingsLayout->addMultiCellWidget(d->aperture, 25, 25, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoAperture, 25, 25, 1, 1);
    settingsLayout->addMultiCellWidget(d->focalLenght, 26, 26, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoFocalLenght, 26, 26, 1, 1);
    settingsLayout->addMultiCellWidget(d->exposureTime, 27, 27, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoExposureTime, 27, 27, 1, 1);
    settingsLayout->addMultiCellWidget(d->sensitivity, 28, 28, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoSensitivity, 28, 28, 1, 1);
    settingsLayout->addMultiCellWidget(d->exposureMode, 29, 29, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoExposureMode, 29, 29, 1, 1);
    settingsLayout->addMultiCellWidget(d->flash, 30, 30, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoFlash, 30, 30, 1, 1);
    settingsLayout->addMultiCellWidget(d->whiteBalance, 31, 31, 0, 0);
    settingsLayout->addMultiCellWidget(d->labelPhotoWhiteBalance, 31, 31, 1, 1);

    settingsLayout->setRowStretch(32, 10);
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
        d->labelImageCompression->setText(QString::null);
        d->labelImageBitDepth->setText(QString::null);
        d->labelImageColorMode->setText(QString::null);

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
    QString compression, bitDepth, colorMode;
    QString rawFilesExt(raw_file_extentions);

    if (rawFilesExt.upper().contains( fileInfo.extension(false).upper() ))
    {
        d->labelImageMime->setText(i18n("RAW Image"));
        compression = i18n("None");
        bitDepth = "48";
        dims = metaData.getImageDimensions();
    }
    else
    {
        d->labelImageMime->setText(fi.mimeComment());

        KFileMetaInfo meta = fi.metaInfo();
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
        }
    }

    QString mpixels;
    mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
    str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)")
          .arg(dims.width()).arg(dims.height()).arg(mpixels);
    d->labelImageDimensions->setText(str);
    d->labelImageCompression->setText(compression.isEmpty() ? unavailable : compression);
    d->labelImageBitDepth->setText(bitDepth.isEmpty() ? unavailable : i18n("%1 bpp").arg(bitDepth));
    d->labelImageColorMode->setText(colorMode.isEmpty() ? unavailable : colorMode);

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

void ImagePropertiesTab::colorChanged(const QColor& back, const QColor& fore)
{
    d->settingsArea->setPaletteBackgroundColor(back);

    d->title->setPaletteBackgroundColor(back);
    d->folder->setPaletteBackgroundColor(back);
    d->modifiedDate->setPaletteBackgroundColor(back);
    d->size->setPaletteBackgroundColor(back);
    d->owner->setPaletteBackgroundColor(back);
    d->permissions->setPaletteBackgroundColor(back);

    d->title2->setPaletteBackgroundColor(back);
    d->mime->setPaletteBackgroundColor(back);
    d->dimensions->setPaletteBackgroundColor(back);
    d->compression->setPaletteBackgroundColor(back);
    d->bitDepth->setPaletteBackgroundColor(back);
    d->colorMode->setPaletteBackgroundColor(back);

    d->title3->setPaletteBackgroundColor(back);
    d->make->setPaletteBackgroundColor(back);
    d->model->setPaletteBackgroundColor(back);
    d->photoDate->setPaletteBackgroundColor(back);
    d->aperture->setPaletteBackgroundColor(back);
    d->focalLenght->setPaletteBackgroundColor(back);
    d->exposureTime->setPaletteBackgroundColor(back);
    d->sensitivity->setPaletteBackgroundColor(back);
    d->exposureMode->setPaletteBackgroundColor(back);
    d->flash->setPaletteBackgroundColor(back);
    d->whiteBalance->setPaletteBackgroundColor(back);

    d->labelFolder->setPaletteBackgroundColor(back);
    d->labelFileModifiedDate->setPaletteBackgroundColor(back);
    d->labelFileSize->setPaletteBackgroundColor(back);
    d->labelFileOwner->setPaletteBackgroundColor(back);
    d->labelFilePermissions->setPaletteBackgroundColor(back);

    d->labelImageMime->setPaletteBackgroundColor(back);
    d->labelImageDimensions->setPaletteBackgroundColor(back);
    d->labelImageCompression->setPaletteBackgroundColor(back);
    d->labelImageBitDepth->setPaletteBackgroundColor(back);
    d->labelImageColorMode->setPaletteBackgroundColor(back);

    d->labelPhotoMake->setPaletteBackgroundColor(back);
    d->labelPhotoModel->setPaletteBackgroundColor(back);
    d->labelPhotoDateTime->setPaletteBackgroundColor(back);
    d->labelPhotoAperture->setPaletteBackgroundColor(back);
    d->labelPhotoFocalLenght->setPaletteBackgroundColor(back);
    d->labelPhotoExposureTime->setPaletteBackgroundColor(back);
    d->labelPhotoSensitivity->setPaletteBackgroundColor(back);
    d->labelPhotoExposureMode->setPaletteBackgroundColor(back);
    d->labelPhotoFlash->setPaletteBackgroundColor(back);
    d->labelPhotoWhiteBalance->setPaletteBackgroundColor(back);

    d->title->setPaletteForegroundColor(fore);
    d->folder->setPaletteForegroundColor(fore);
    d->modifiedDate->setPaletteForegroundColor(fore);
    d->size->setPaletteForegroundColor(fore);
    d->owner->setPaletteForegroundColor(fore);
    d->permissions->setPaletteForegroundColor(fore);

    d->title2->setPaletteForegroundColor(fore);
    d->mime->setPaletteForegroundColor(fore);
    d->dimensions->setPaletteForegroundColor(fore);
    d->compression->setPaletteForegroundColor(fore);
    d->bitDepth->setPaletteForegroundColor(fore);
    d->colorMode->setPaletteForegroundColor(fore);

    d->title3->setPaletteForegroundColor(fore);
    d->make->setPaletteForegroundColor(fore);
    d->model->setPaletteForegroundColor(fore);
    d->photoDate->setPaletteForegroundColor(fore);
    d->aperture->setPaletteForegroundColor(fore);
    d->focalLenght->setPaletteForegroundColor(fore);
    d->exposureTime->setPaletteForegroundColor(fore);
    d->sensitivity->setPaletteForegroundColor(fore);
    d->exposureMode->setPaletteForegroundColor(fore);
    d->flash->setPaletteForegroundColor(fore);
    d->whiteBalance->setPaletteForegroundColor(fore);

    d->labelFolder->setPaletteForegroundColor(fore);
    d->labelFileModifiedDate->setPaletteForegroundColor(fore);
    d->labelFileSize->setPaletteForegroundColor(fore);
    d->labelFileOwner->setPaletteForegroundColor(fore);
    d->labelFilePermissions->setPaletteForegroundColor(fore);

    d->labelImageMime->setPaletteForegroundColor(fore);
    d->labelImageDimensions->setPaletteForegroundColor(fore);
    d->labelImageCompression->setPaletteForegroundColor(fore);
    d->labelImageBitDepth->setPaletteForegroundColor(fore);
    d->labelImageColorMode->setPaletteForegroundColor(fore);

    d->labelPhotoMake->setPaletteForegroundColor(fore);
    d->labelPhotoModel->setPaletteForegroundColor(fore);
    d->labelPhotoDateTime->setPaletteForegroundColor(fore);
    d->labelPhotoAperture->setPaletteForegroundColor(fore);
    d->labelPhotoFocalLenght->setPaletteForegroundColor(fore);
    d->labelPhotoExposureTime->setPaletteForegroundColor(fore);
    d->labelPhotoSensitivity->setPaletteForegroundColor(fore);
    d->labelPhotoExposureMode->setPaletteForegroundColor(fore);
    d->labelPhotoFlash->setPaletteForegroundColor(fore);
    d->labelPhotoWhiteBalance->setPaletteForegroundColor(fore);
}

}  // NameSpace Digikam

#include "imagepropertiestab.moc"
