/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-05-16
 * Description : a tool to export GPS data to KML file.
 *
 * Copyright (C) 2006-2007 by Stephane Pontier <shadow dot walker at free dot fr>
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "kmlexport.h"

// Qt includes

#include <QImageReader>
#include <QPainter>
#include <QRegExp>
#include <QTextStream>
#include <QStandardPaths>
#include <QApplication>
#include <QIODevice>
#include <QDir>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dmessagebox.h"

namespace Digikam
{

KmlExport::KmlExport(DInfoInterface* const iface)
{
    m_localTarget        = true;
    m_optimize_googlemap = false;
    m_GPXtracks          = false;
    m_iconSize           = 33;
    m_googlemapSize      = 32;
    m_size               = 320;
    m_altitudeMode       = 0;
    m_TimeZone           = 12;
    m_LineWidth          = 4;
    m_GPXOpacity         = 64;
    m_GPXAltitudeMode    = 0;
    m_kmlDocument        = 0;
    m_iface              = iface;
}

KmlExport::~KmlExport()
{
}

void KmlExport::setUrls(const QList<QUrl>& urls)
{
    m_urls = urls;
}

QString KmlExport::webifyFileName(const QString& fileName) const
{
    QString webFileName = fileName.toLower();

    // Remove potentially troublesome chars
    webFileName         = webFileName.replace(QRegExp(QLatin1String("[^-0-9a-z]+")), QLatin1String("_"));

    return webFileName;
}

QImage KmlExport::generateSquareThumbnail(const QImage& fullImage, int size) const
{
    QImage image = fullImage.scaled(size, size, Qt::KeepAspectRatioByExpanding);

    if (image.width() == size && image.height() == size)
    {
        return image;
    }

    QPixmap croppedPix(size, size);
    QPainter painter(&croppedPix);

    int sx = 0, sy = 0;

    if (image.width()>size)
    {
        sx = (image.width() - size)/2;
    }
    else
    {
        sy = (image.height() - size)/2;
    }

    painter.drawImage(0, 0, image, sx, sy, size, size);
    painter.end();

    return croppedPix.toImage();
}

QImage KmlExport::generateBorderedThumbnail(const QImage& fullImage, int size) const
{
    int image_border = 3;

    // getting an image minus the border
    QImage image     = fullImage.scaled(size -(2*image_border), size - (2*image_border), Qt::KeepAspectRatioByExpanding);

    QPixmap croppedPix(image.width() + (2*image_border), image.height() + (2*image_border));
    QPainter painter(&croppedPix);

    QColor BrushColor(255,255,255);
    painter.fillRect(0,0,image.width() + (2*image_border),image.height() + (2*image_border),BrushColor);
    /*! @todo add a corner to the thumbnail and a hotspot to the kml element */

    painter.drawImage(image_border, image_border, image);
    painter.end();

    return croppedPix.toImage();
}

void KmlExport::generateImagesthumb(const QUrl& imageURL, QDomElement& kmlAlbum)
{
    DItemInfo info(m_iface->itemInfo(imageURL));

    // Load image
    QString path = imageURL.toLocalFile();
    QFile imageFile(path);

    if (!imageFile.open(QIODevice::ReadOnly))
    {
        logError(i18n("Could not read image '%1'", path));
        return;
    }

    QImageReader reader(&imageFile);
    QString imageFormat = QString::fromUtf8(reader.format());

    if (imageFormat.isEmpty())
    {
        logError(i18n("Format of image '%1' is unknown", path));
        return;
    }

    imageFile.close();

    if (!imageFile.open(QIODevice::ReadOnly))
    {
        logError(i18n("Could not read image '%1'", path));
        return;
    }

    QByteArray imageData = imageFile.readAll();
    imageFile.close();
    QImage image;

    if (!image.loadFromData(imageData))
    {
        logError(i18n("Error loading image '%1'", path));
        return;
    }

    // Process images

    if (info.orientation() != DMetadata::ORIENTATION_UNSPECIFIED)
    {
         m_meta.rotateExifQImage(image, (MetaEngine::ImageOrientation)info.orientation());
    }

    image = image.scaled(m_size, m_size, Qt::KeepAspectRatioByExpanding);
    QImage icon;

    if (m_optimize_googlemap)
    {
        icon = generateSquareThumbnail(image,m_googlemapSize);
    }
    else
    {
    //    icon = image.smoothScale(m_iconSize, m_iconSize, QImage::ScaleMax);
        icon = generateBorderedThumbnail(image, m_iconSize);
    }

    // Save images
    /** @todo remove the extension of the file
     * it's appear with digikam but not with gwenview
     * which already seems to strip the extension
     */
    QString baseFileName = webifyFileName(info.name());
    //baseFileName       = mUniqueNameHelper.makeNameUnique(baseFileName);
    QString fullFileName;
    fullFileName         = baseFileName + QLatin1Char('.') + imageFormat.toLower();
    QString destPath     = m_imageDir.filePath(fullFileName);

    if (!image.save(destPath, imageFormat.toLatin1().constData(), 85))
    {
        // if not able to save the image, it's pointless to create a placemark
        logError(i18n("Could not save image '%1' to '%2'", path, destPath));
    }
    else
    {
        logInfo(i18n("Creation of picture '%1'", fullFileName));

        double alt = 0.0, lat = 0.0, lng = 0.0;

        if (info.hasGeolocationInfo())
        {
            lat = info.latitude();
            lng = info.longitude();
            alt = info.altitude();
        }
        else if (m_meta.load(imageURL.toLocalFile()))
        {
            m_meta.getGPSInfo(alt, lat, lng);
        }

        QDomElement kmlPlacemark = addKmlElement(kmlAlbum, QLatin1String("Placemark"));
        addKmlTextElement(kmlPlacemark, QLatin1String("name"), fullFileName);
        // location and altitude
        QDomElement kmlGeometry  = addKmlElement(kmlPlacemark, QLatin1String("Point"));

        if (alt)
        {
            addKmlTextElement(kmlGeometry, QLatin1String("coordinates"), QString::fromUtf8("%1,%2,%3 ")
                .arg(lng, 0, 'f', 8)
                .arg(lat, 0, 'f', 8)
                .arg(alt, 0, 'f', 8));
        }
        else
        {
            addKmlTextElement(kmlGeometry, QLatin1String("coordinates"), QString::fromUtf8("%1,%2 ")
                .arg(lng, 0, 'f', 8)
                .arg(lat, 0, 'f', 8));
        }

        if (m_altitudeMode == 2)
        {
            addKmlTextElement(kmlGeometry, QLatin1String("altitudeMode"), QLatin1String("absolute"));
        }
        else if (m_altitudeMode == 1)
        {
            addKmlTextElement(kmlGeometry, QLatin1String("altitudeMode"), QLatin1String("relativeToGround"));
        }
        else
        {
            addKmlTextElement(kmlGeometry, QLatin1String("altitudeMode"), QLatin1String("clampToGround"));
        }

        addKmlTextElement(kmlGeometry, QLatin1String("extrude"), QLatin1String("1"));

        // we try to load exif value if any otherwise, try the application db

        /** we need to take the DateTimeOriginal
          * if we refer to http://www.exif.org/Exif2-2.PDF
          * (standard)DateTime: is The date and time of image creation. In this standard it is the date and time the file was changed
          * DateTimeOriginal: The date and time when the original image data was generated.
          *                   For a DSC the date and time the picture was taken are recorded.
          * DateTimeDigitized: The date and time when the image was stored as digital data.
          * So for:
          * - a DSC: the right time is the DateTimeDigitized which is also DateTimeOriginal
          *          if the picture has been modified the (standard)DateTime should change.
          * - a scanned picture, the right time is the DateTimeOriginal which should also be the DateTime
          *          the (standard)DateTime should be the same except if the picture is modified
          * - a panorama created from several pictures, the right time is the DateTimeOriginal (average of DateTimeOriginal actually)
          *          The (standard)DateTime is the creation date of the panorama.
          * it's seems the time to take into acccount is the DateTimeOriginal.
          * but the MetadataProcessor::getImageDateTime() return the (standard)DateTime first
          * MetadataProcessor seems to take Original dateTime first so it shoul be alright now.
          */
        QDateTime datetime;

        m_meta.getImageDateTime();

        if (datetime.isValid())
        {
            QDomElement kmlTimeStamp = addKmlElement(kmlPlacemark, QLatin1String("TimeStamp"));
            addKmlTextElement(kmlTimeStamp, QLatin1String("when"), datetime.toString(QLatin1String("yyyy-MM-ddThh:mm:ssZ")));
        }
        else
        {
            QDomElement kmlTimeStamp = addKmlElement(kmlPlacemark, QLatin1String("TimeStamp"));
            addKmlTextElement(kmlTimeStamp, QLatin1String("when"), (info.dateTime()).toString(QLatin1String("yyyy-MM-ddThh:mm:ssZ")));
        }

        QString my_description;

        if (m_optimize_googlemap)
        {
            my_description = QLatin1String("<img src=\"") + m_UrlDestDir + m_imageDirBasename + QLatin1Char('/') + fullFileName + QLatin1String("\">");
        }
        else
        {
            my_description = QLatin1String("<img src=\"") + m_imageDirBasename + QLatin1Char('/') + fullFileName + QLatin1String("\">");
        }

        my_description += QLatin1String("<br/>") + info.comment() ;

        addKmlTextElement(kmlPlacemark, QLatin1String("description"), my_description);
        logInfo(i18n("Creation of placemark '%1'", fullFileName));

        // Save icon
        QString iconFileName = QLatin1String("thumb_") + baseFileName + QLatin1Char('.') + imageFormat.toLower();
        QString destPath     = m_imageDir.filePath(iconFileName);

        if (!icon.save(destPath, imageFormat.toLatin1().constData(), 85))
        {
            logWarning(i18n("Could not save icon for image '%1' to '%2'",path,destPath));
        }
        else
        {
            logInfo(i18n("Creation of icon '%1'", iconFileName));
            // style et icon
            QDomElement kmlStyle     = addKmlElement(kmlPlacemark, QLatin1String("Style"));
            QDomElement kmlIconStyle = addKmlElement(kmlStyle,     QLatin1String("IconStyle"));
            QDomElement kmlIcon      = addKmlElement(kmlIconStyle, QLatin1String("Icon"));

            if (m_optimize_googlemap)
            {
                addKmlTextElement(kmlIcon, QLatin1String("href"), m_UrlDestDir + m_imageDirBasename + QLatin1Char('/') + iconFileName);
            }
            else
            {
                addKmlTextElement(kmlIcon, QLatin1String("href"), m_imageDirBasename + QLatin1Char('/') + iconFileName);
            }

            QDomElement kmlBallonStyle = addKmlElement(kmlStyle, QLatin1String("BalloonStyle"));
            addKmlTextElement(kmlBallonStyle, QLatin1String("text"), QLatin1String("$[description]"));
        }
    }
}

void KmlExport::addTrack(QDomElement& kmlAlbum)
{
    if (m_GPXFile.isEmpty())
    {
        logWarning(i18n("No GPX file chosen."));
        return;
    }

    m_gpxParser.clear();
    bool ret = m_gpxParser.loadGPXFile(QUrl::fromLocalFile(m_GPXFile));

    if (!ret)
    {
        logError(i18n("Cannot parse %1 GPX file.",m_GPXFile));
        return;
    }

    if (m_gpxParser.numPoints() <= 0)
    {
        logError(i18n("The %1 GPX file do not have a date-time track to use.",
                      m_GPXFile));
        return;
    }

    // create a folder that will contain tracks and points
    QDomElement kmlFolder = addKmlElement(kmlAlbum, QLatin1String("Folder"));
    addKmlTextElement(kmlFolder, QLatin1String("name"), i18n("Tracks"));

    if (!m_optimize_googlemap)
    {
        // style of points and track
        QDomElement kmlTrackStyle = addKmlElement(kmlAlbum, QLatin1String("Style"));
        kmlTrackStyle.setAttribute(QLatin1String("id"), QLatin1String("track"));
        QDomElement kmlIconStyle  = addKmlElement(kmlTrackStyle, QLatin1String("IconStyle"));
        QDomElement kmlIcon       = addKmlElement(kmlIconStyle, QLatin1String("Icon"));
        //! FIXME is there a way to be sure of the location of the icon?
        addKmlTextElement(kmlIcon, QLatin1String("href"), QLatin1String("http://maps.google.com/mapfiles/kml/pal4/icon60.png"));

        m_gpxParser.CreateTrackPoints(kmlFolder, *m_kmlDocument, m_TimeZone - 12, m_GPXAltitudeMode);
    }

    // linetrack style
    QDomElement kmlLineTrackStyle = addKmlElement(kmlAlbum, QLatin1String("Style"));
    kmlLineTrackStyle.setAttribute(QLatin1String("id"), QLatin1String("linetrack"));
    QDomElement kmlLineStyle      = addKmlElement(kmlLineTrackStyle, QLatin1String("LineStyle"));

    // the KML color is not #RRGGBB but AABBGGRR
    QString KMLColorValue = QString::fromUtf8("%1%2%3%4")
        .arg((int)m_GPXOpacity*256/100, 2, 16)
        .arg((&m_GPXColor)->blue(), 2, 16)
        .arg((&m_GPXColor)->green(), 2, 16)
        .arg((&m_GPXColor)->red(), 2, 16);
    addKmlTextElement(kmlLineStyle, QLatin1String("color"), KMLColorValue);
    addKmlTextElement(kmlLineStyle, QLatin1String("width"), QString::fromUtf8("%1").arg(m_LineWidth));

    m_gpxParser.CreateTrackLine(kmlAlbum, *m_kmlDocument, m_GPXAltitudeMode);
}

void KmlExport::generate()
{
    getConfig();
    m_logData.clear();

    //! @todo perform a test here before continuing.
    QDir().mkpath(m_tempDestDir.absolutePath());
    QDir().mkpath(m_imageDir.absolutePath());

    // create the document, and it's root
    m_kmlDocument                   = new QDomDocument(QLatin1String(""));
    QDomImplementation impl;
    QDomProcessingInstruction instr = m_kmlDocument->createProcessingInstruction(QLatin1String("xml"), QLatin1String("version=\"1.0\" encoding=\"UTF-8\""));
    m_kmlDocument->appendChild(instr);
    QDomElement kmlRoot             = m_kmlDocument->createElementNS(QLatin1String("http://www.opengis.net/kml/2.2"), QLatin1String("kml"));
    m_kmlDocument->appendChild(kmlRoot);

    QDomElement kmlAlbum            = addKmlElement(kmlRoot, QLatin1String("Document"));
    QDomElement kmlName             = addKmlTextElement(kmlAlbum, QLatin1String("name"), m_KMLFileName);
    QDomElement kmlDescription      = addKmlHtmlElement(kmlAlbum, QLatin1String("description"),
                                                        QLatin1String("Created with Geolocation Editor from <a href=\"http://www.digikam.org/\">digiKam</a>"));

    if (m_GPXtracks)
    {
        addTrack(kmlAlbum);
    }

    QList<QUrl> images = m_urls;
    int pos            = 1;

    QList<QUrl>::ConstIterator imagesEnd (images.constEnd());

    for (QList<QUrl>::ConstIterator selIt = images.constBegin();
         selIt != imagesEnd; ++selIt, ++pos)
    {
        double alt, lat, lng;
        QUrl url        = *selIt;
        DItemInfo info(m_iface->itemInfo(url));
        bool hasGPSInfo = info.hasGeolocationInfo();

        if (hasGPSInfo)
        {
            lat = info.latitude();
            lng = info.longitude();
            alt = info.altitude();
        }
        else if (m_meta.load(url.toLocalFile()))
        {
            hasGPSInfo = m_meta.getGPSInfo(alt, lat, lng);
        }

        if (hasGPSInfo)
        {
            // generation de l'image et de l'icone
            generateImagesthumb(url, kmlAlbum);
        }
        else
        {
            logWarning(i18n("No position data for '%1'", info.name()));
        }

        emit(signalProgressChanged(pos));
        QApplication::processEvents();
    }

    /** @todo change to kml or kmz if compressed */
    QFile file(m_tempDestDir.filePath(m_KMLFileName + QLatin1String(".kml")));

    if (!file.open(QIODevice::WriteOnly))
    {
        logError(i18n("Cannot open file for writing"));
        delete m_kmlDocument;
        m_kmlDocument = 0;
        return;
    }

    QTextStream stream(&file); // we will serialize the data into the file
    stream << m_kmlDocument->toString();
    file.close();

    delete m_kmlDocument;
    m_kmlDocument = 0;

    logInfo(i18n("Move %1 to final directory %2", m_tempDestDir.absolutePath(), m_baseDestDir));

    if (!copyDir(m_tempDestDir.absolutePath(), m_baseDestDir))
    {
        logWarning(i18n("Cannot move data to destination directory"));
    }

    QDir(m_tempDestDir.absolutePath()).removeRecursively();

    if (!m_logData.isEmpty())
    {
        DMessageBox::showInformationList(QMessageBox::Information,
                                         qApp->activeWindow(),
                                         qApp->applicationName(),
                                         i18n("Report below have been generated while KML file processing:"),
                                         m_logData);
    }
}

bool KmlExport::copyDir(const QString& srcFilePath, const QString& dstFilePath)
{
    if (QFileInfo(srcFilePath).isDir())
    {
        QDir srcDir(srcFilePath);
        QDir dstDir(dstFilePath);

        if (!QDir().mkpath(dstDir.absolutePath()))
            return false;

        QStringList files = srcDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

        foreach(const QString& file, files)
        {
            const QString newSrcFilePath = srcDir.absolutePath() + QLatin1Char('/') + file;
            const QString newDstFilePath = dstDir.absolutePath() + QLatin1Char('/') + file;

            if (!copyDir(newSrcFilePath, newDstFilePath))
                return false;
        }
    }
    else
    {
        if (srcFilePath != dstFilePath && QFile::exists(srcFilePath) && QFile::exists(dstFilePath))
        {
            if (!QFile::remove(dstFilePath))
                return false;
        }

        if (!QFile::copy(srcFilePath, dstFilePath))
            return false;
    }

    return true;
}

void KmlExport::getConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("KMLExport Settings"));

    m_localTarget        = group.readEntry(QLatin1String("localTarget"), true);
    m_optimize_googlemap = group.readEntry(QLatin1String("optimize_googlemap"), false);
    m_iconSize           = group.readEntry(QLatin1String("iconSize"), 33);
    //    googlemapSize    = group.readNumEntry(QLatin1String("googlemapSize"));
    m_size               = group.readEntry(QLatin1String("size"), 320);

    // UrlDestDir have to have the trailing
    m_baseDestDir        = group.readEntry(QLatin1String("baseDestDir"),   QString::fromUtf8("/tmp/"));
    m_UrlDestDir         = group.readEntry(QLatin1String("UrlDestDir"),    QString::fromUtf8("http://www.example.com/"));
    m_KMLFileName        = group.readEntry(QLatin1String("KMLFileName"),   QString::fromUtf8("kmldocument"));
    m_altitudeMode       = group.readEntry(QLatin1String("Altitude Mode"), 0);

    m_GPXtracks          = group.readEntry(QLatin1String("UseGPXTracks"),      false);
    m_GPXFile            = group.readEntry(QLatin1String("GPXFile"),           QString());
    m_TimeZone           = group.readEntry(QLatin1String("Time Zone"),         12);
    m_LineWidth          = group.readEntry(QLatin1String("Line Width"),        4);
    m_GPXColor           = group.readEntry(QLatin1String("Track Color"),       QColor("#17eeee"));
    m_GPXOpacity         = group.readEntry(QLatin1String("Track Opacity"),     64);
    m_GPXAltitudeMode    = group.readEntry(QLatin1String("GPX Altitude Mode"), 0);

    m_tempDestDir        = QDir(QDir::temp().filePath(QString::fromLatin1("digiKam-kmlexport-%1").arg(qApp->applicationPid())));

    m_imageDirBasename   = QLatin1String("images");
    m_imageDir           = QDir(m_tempDestDir.filePath(m_imageDirBasename));

    m_googlemapSize      = 32;
}

void KmlExport::logInfo(const QString& msg)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << msg;
}

void KmlExport::logError(const QString& msg)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << msg;
    m_logData.append(msg);
}

void KmlExport::logWarning(const QString& msg)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << msg;
    m_logData.append(msg);
}

QDomElement KmlExport::addKmlElement(QDomElement& target,
                                     const QString& tag) const
{
    QDomElement kmlElement = m_kmlDocument->createElement( tag );
    target.appendChild( kmlElement );
    return kmlElement;
}

QDomElement KmlExport::addKmlTextElement(QDomElement& target,
                                         const QString& tag,
                                         const QString& text) const
{
    QDomElement kmlElement  = m_kmlDocument->createElement( tag );
    target.appendChild( kmlElement );
    QDomText kmlTextElement = m_kmlDocument->createTextNode( text );
    kmlElement.appendChild( kmlTextElement );
    return kmlElement;
}

QDomElement KmlExport::addKmlHtmlElement(QDomElement& target,
                                         const QString& tag,
                                         const QString& text) const
{
    QDomElement kmlElement  = m_kmlDocument->createElement( tag );
    target.appendChild( kmlElement );
    QDomText kmlTextElement = m_kmlDocument->createCDATASection( text );
    kmlElement.appendChild( kmlTextElement );
    return kmlElement;
}

} // namespace Digikam
