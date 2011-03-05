/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : simple image properties side bar (without support
 *               of digiKam database).
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagepropertiessidebar.moc"

// Qt includes

#include <QRect>
#include <QSplitter>
#include <QFileInfo>

// KDE includes


#include <kfileitem.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <KDebug>

// LibKDcraw includes

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

// Local includes

#include "dimg.h"
#include "dmetadata.h"
#include "imagepropertiesgpstab.h"
#include "imagepropertiestab.h"
#include "imagepropertiesmetadatatab.h"
#include "imagepropertiescolorstab.h"
#include "imagepropertiesversionstab.h"

namespace Digikam
{

ImagePropertiesSideBar::ImagePropertiesSideBar(QWidget* parent,
        SidebarSplitter* splitter,
        KMultiTabBarPosition side,
        bool mimimizedDefault)
    : Sidebar(parent, splitter, side, mimimizedDefault)
{
    m_image              = 0;
    m_currentRect        = QRect();
    m_dirtyPropertiesTab = false;
    m_dirtyMetadataTab   = false;
    m_dirtyColorTab      = false;
    m_dirtyGpsTab        = false;
    m_dirtyHistoryTab    = false;

    m_propertiesTab      = new ImagePropertiesTab(parent);
    m_metadataTab        = new ImagePropertiesMetaDataTab(parent);
    m_colorTab           = new ImagePropertiesColorsTab(parent);
    m_gpsTab             = new ImagePropertiesGPSTab(parent);

    appendTab(m_propertiesTab, SmallIcon("document-properties"), i18n("Properties"));
    appendTab(m_metadataTab, SmallIcon("exifinfo"), i18n("Metadata"));
    appendTab(m_colorTab, SmallIcon("format-fill-color"), i18n("Colors"));
    appendTab(m_gpsTab, SmallIcon("applications-internet"), i18n("Geolocation"));

    connect(this, SIGNAL(signalChangedTab(QWidget*)),
            this, SLOT(slotChangedTab(QWidget*)));
}

ImagePropertiesSideBar::~ImagePropertiesSideBar()
{
}

void ImagePropertiesSideBar::itemChanged(const KUrl& url, const QRect& rect, DImg* img)
{
    if (!url.isValid())
    {
        return;
    }

    m_currentURL         = url;
    m_currentRect        = rect;
    m_image              = img;
    m_dirtyPropertiesTab = false;
    m_dirtyMetadataTab   = false;
    m_dirtyColorTab      = false;
    m_dirtyGpsTab        = false;
    m_dirtyHistoryTab    = false;

    slotChangedTab( getActiveTab() );
}

void ImagePropertiesSideBar::slotNoCurrentItem()
{
    m_currentURL = KUrl();

    m_propertiesTab->setCurrentURL();
    m_metadataTab->setCurrentURL();
    m_colorTab->setData();
    m_gpsTab->setCurrentURL();

    m_dirtyPropertiesTab = false;
    m_dirtyMetadataTab   = false;
    m_dirtyColorTab      = false;
    m_dirtyGpsTab        = false;
    m_dirtyHistoryTab    = false;
}

void ImagePropertiesSideBar::slotImageSelectionChanged(const QRect& rect)
{
    m_currentRect = rect;

    if (m_dirtyColorTab)
    {
        m_colorTab->setSelection(rect);
    }
    else
    {
        slotChangedTab(m_colorTab);
    }
}

void ImagePropertiesSideBar::slotChangedTab(QWidget* tab)
{
    if (!m_currentURL.isValid())
    {
        m_gpsTab->setActive(tab==m_gpsTab);

        return;
    }

    setCursor(Qt::WaitCursor);

    if (tab == m_propertiesTab && !m_dirtyPropertiesTab)
    {
        m_propertiesTab->setCurrentURL(m_currentURL);
        setImagePropertiesInformation(m_currentURL);
        m_dirtyPropertiesTab = true;
    }
    else if (tab == m_metadataTab && !m_dirtyMetadataTab)
    {
        m_metadataTab->setCurrentURL(m_currentURL);
        m_dirtyMetadataTab = true;
    }
    else if (tab == m_colorTab && !m_dirtyColorTab)
    {
        m_colorTab->setData(m_currentURL, m_currentRect, m_image);
        m_dirtyColorTab = true;
    }
    else if (tab == m_gpsTab && !m_dirtyGpsTab)
    {
        m_gpsTab->setCurrentURL(m_currentURL);
        m_dirtyGpsTab = true;
    }

    m_gpsTab->setActive(tab==m_gpsTab);

    unsetCursor();
}

void ImagePropertiesSideBar::setImagePropertiesInformation(const KUrl& url)
{
    if (!url.isValid())
    {
        return;
    }

    QString str;
    QString unavailable(i18n("<i>unavailable</i>"));

    KFileItem fi(KFileItem::Unknown, KFileItem::Unknown, url);
    QFileInfo fileInfo(url.toLocalFile());
    DMetadata metaData(url.toLocalFile());

    // -- File system information -----------------------------------------

    QDateTime modifiedDate = fileInfo.lastModified();
    str = KGlobal::locale()->formatDateTime(modifiedDate, KLocale::ShortDate, true);
    m_propertiesTab->setFileModifiedDate(str);

    str = QString("%1 (%2)").arg(KIO::convertSize(fi.size()))
          .arg(KGlobal::locale()->formatNumber(fi.size(), 0));
    m_propertiesTab->setFileSize(str);
    m_propertiesTab->setFileOwner(QString("%1 - %2").arg(fi.user()).arg(fi.group()));
    m_propertiesTab->setFilePermissions(fi.permissionsString());

    // -- Image Properties --------------------------------------------------

    QSize   dims;
    QString compression, bitDepth, colorMode;
    QString rawFilesExt(KDcrawIface::KDcraw::rawFiles());
    QString ext = fileInfo.suffix().toUpper();

    if (!ext.isEmpty() && rawFilesExt.toUpper().contains(ext))
    {
        m_propertiesTab->setImageMime(i18n("RAW Image"));
        compression = i18n("None");
        bitDepth    = "48";
        dims        = metaData.getImageDimensions();
        colorMode   = i18n("Uncalibrated");
    }
    else
    {
        m_propertiesTab->setImageMime(fi.mimeComment());
        KFileMetaInfo meta = fi.metaInfo();

        if (meta.isValid())
        {
            if (meta.item("Dimensions").isValid())
            {
                dims = meta.item("Dimensions").value().toSize();
            }

            if (meta.item("JPEG quality").isValid())
            {
                compression = i18n("JPEG quality %1", meta.item("JPEG quality").value().toString());
            }

            if (meta.item("Compression").isValid())
            {
                compression =  meta.item("Compression").value().toString();
            }

            if (meta.item("BitDepth").isValid())
            {
                bitDepth = meta.item("BitDepth").value().toString();
            }

            if (meta.item("ColorMode").isValid())
            {
                colorMode = meta.item("ColorMode").value().toString();
            }
        }

        /*          TODO: KDE4PORT: KFileMetaInfo API as Changed.
                                    Check if new method to search information is enough.

                if (meta.isValid())
                {
                    if (meta.containsGroup("Jpeg EXIF Data"))     // JPEG image ?
                    {
                        dims        = meta.group("Jpeg EXIF Data").item("Dimensions").value().toSize();

                        QString quality = meta.group("Jpeg EXIF Data").item("JPEG quality").value().toString();
                        quality.isEmpty() ? compression = unavailable :
                                            compression = i18n("JPEG quality %1",quality);
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
    m_propertiesTab->setImageDimensions(str);
    m_propertiesTab->setImageCompression(compression.isEmpty() ? unavailable : compression);
    m_propertiesTab->setImageBitDepth(bitDepth.isEmpty() ? unavailable : i18n("%1 bpp", bitDepth));
    m_propertiesTab->setImageColorMode(colorMode.isEmpty() ? unavailable : colorMode);

    // -- Photograph information ------------------------------------------

    PhotoInfoContainer photoInfo = metaData.getPhotographInformation();

    m_propertiesTab->setPhotoInfoDisable(photoInfo.isEmpty());

    m_propertiesTab->setPhotoMake(photoInfo.make.isEmpty() ? unavailable : photoInfo.make);
    m_propertiesTab->setPhotoModel(photoInfo.model.isEmpty() ? unavailable : photoInfo.model);

    if (photoInfo.dateTime.isValid())
    {
        str = KGlobal::locale()->formatDateTime(photoInfo.dateTime, KLocale::ShortDate, true);
        m_propertiesTab->setPhotoDateTime(str);
    }
    else
    {
        m_propertiesTab->setPhotoDateTime(unavailable);
    }

    m_propertiesTab->setPhotoLens(photoInfo.lens.isEmpty() ? unavailable : photoInfo.lens);
    m_propertiesTab->setPhotoAperture(photoInfo.aperture.isEmpty() ? unavailable : photoInfo.aperture);

    if (photoInfo.focalLength35mm.isEmpty())
    {
        m_propertiesTab->setPhotoFocalLength(photoInfo.focalLength.isEmpty() ? unavailable : photoInfo.focalLength);
    }
    else
    {
        str = i18n("%1 (35mm: %2)", photoInfo.focalLength, photoInfo.focalLength35mm);
        m_propertiesTab->setPhotoFocalLength(str);
    }

    m_propertiesTab->setPhotoExposureTime(photoInfo.exposureTime.isEmpty() ? unavailable : photoInfo.exposureTime);
    m_propertiesTab->setPhotoSensitivity(photoInfo.sensitivity.isEmpty() ? unavailable : i18n("%1 ISO", photoInfo.sensitivity));

    if (photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
    {
        m_propertiesTab->setPhotoExposureMode(unavailable);
    }
    else if (!photoInfo.exposureMode.isEmpty() && photoInfo.exposureProgram.isEmpty())
    {
        m_propertiesTab->setPhotoExposureMode(photoInfo.exposureMode);
    }
    else if (photoInfo.exposureMode.isEmpty() && !photoInfo.exposureProgram.isEmpty())
    {
        m_propertiesTab->setPhotoExposureMode(photoInfo.exposureProgram);
    }
    else
    {
        str = QString("%1 / %2").arg(photoInfo.exposureMode).arg(photoInfo.exposureProgram);
        m_propertiesTab->setPhotoExposureMode(str);
    }

    m_propertiesTab->setPhotoFlash(photoInfo.flash.isEmpty() ? unavailable : photoInfo.flash);
    m_propertiesTab->setPhotoWhiteBalance(photoInfo.whiteBalance.isEmpty() ? unavailable : photoInfo.whiteBalance);

    // -- Caption, ratings, tag information ---------------------

    CaptionsMap captions = metaData.getImageComments();
    QString caption;
    if (captions.contains("x-default"))
        caption = captions.value("x-default").caption;
    else if (!captions.isEmpty())
        caption = captions.begin().value().caption;
    m_propertiesTab->setCaption(caption);

    m_propertiesTab->setRating(metaData.getImageRating());

    QStringList tagPaths;
    metaData.getImageTagsPath(tagPaths);
    m_propertiesTab->setTags(tagPaths);
    m_propertiesTab->showOrHideCaptionAndTags();
}

void ImagePropertiesSideBar::doLoadState()
{
    Sidebar::doLoadState();

    /// @todo m_propertiesTab should load its settings from our group
    m_propertiesTab->setObjectName("Image Properties SideBar Expander");
    m_propertiesTab->readSettings();

    KConfigGroup group = getConfigGroup();

    const KConfigGroup groupGPSTab = KConfigGroup(&group, entryName("GPS Properties Tab"));
    m_gpsTab->readSettings(groupGPSTab);

    const KConfigGroup groupMetadataTab = KConfigGroup(&group, entryName("Metadata Properties Tab"));
    m_metadataTab->readSettings(groupMetadataTab);
}

void ImagePropertiesSideBar::doSaveState()
{
    Sidebar::doSaveState();

    KConfigGroup group = getConfigGroup();

    KConfigGroup groupGPSTab = KConfigGroup(&group, entryName("GPS Properties Tab"));
    m_gpsTab->writeSettings(groupGPSTab);

    KConfigGroup groupMetadataTab = KConfigGroup(&group, entryName("Metadata Properties Tab"));
    m_metadataTab->writeSettings(groupMetadataTab);
}

}  // namespace Digikam
