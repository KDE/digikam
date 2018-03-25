/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "gallerygenerator.h"

// Qt includes

#include <QDir>
#include <QFutureWatcher>
#include <QRegExp>
#include <QStringList>
#include <QtConcurrentMap>
#include <QApplication>
#include <QUrl>
#include <QList>
#include <QTemporaryFile>

// KDE includes

#include <klocalizedstring.h>

// libxslt includes

#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/xslt.h>
#include <libexslt/exslt.h>

// Local includes

#include "digikam_debug.h"
#include "abstractthemeparameter.h"
#include "galleryelement.h"
#include "galleryelementfunctor.h"
#include "galleryinfo.h"
#include "gallerytheme.h"
#include "galleryxmlutils.h"
#include "htmlwizard.h"
#include "dprogresswdg.h"
#include "dhistoryview.h"
#include "dfileoperations.h"

namespace Digikam
{

typedef QMap<QByteArray, QByteArray> XsltParameterMap;

class GalleryGenerator::Private
{
public:

    // Url => local temp path
    typedef QHash<QUrl, QString> RemoteUrlHash;

public:

    explicit Private()
      : that(0),
        info(0),
        warnings(false),
        cancel(false),
        pview(0),
        pbar(0)
    {
    }

    GalleryGenerator* that;
    GalleryInfo*      info;
    GalleryTheme::Ptr theme;

    // State info
    bool              warnings;
    QString           xmlFileName;

    bool              cancel;
    DHistoryView*     pview;
    DProgressWdg*     pbar;

public:

    bool init()
    {
        cancel = false;
        theme  = GalleryTheme::findByInternalName(info->theme());

        if (!theme)
        {
            logError( i18n("Could not find theme in '%1'", info->theme()) );
            return false;
        }

        pview->setVisible(true);
        pbar->setVisible(true);

        return true;
    }

    bool createDir(const QString& dirName)
    {
        logInfo(i18n("Create directories"));

        if (!QDir().mkpath(dirName))
        {
            logError(i18n("Could not create folder '%1'", QDir::toNativeSeparators(dirName)));
            return false;
        }

        return true;
    }

    bool copyTheme()
    {
        logInfo(i18n("Copying theme"));

        QUrl srcUrl  = QUrl::fromLocalFile(theme->directory());
        QUrl destUrl = info->destUrl().adjusted(QUrl::StripTrailingSlash);
        QDir themeDir(destUrl.toLocalFile() + QLatin1Char('/') + srcUrl.fileName());

        if (themeDir.exists())
        {
            themeDir.removeRecursively();
        }

        bool ok = DFileOperations::copyFolderRecursively(srcUrl.toLocalFile(), destUrl.toLocalFile());

        if (!ok)
        {
            logError(i18n("Could not copy theme"));
            return false;
        }

        return true;
    }

    bool generateImagesAndXML()
    {
        logInfo(i18n("Generate images and XML files"));
        QString baseDestDir = info->destUrl().toLocalFile();

        if (!createDir(baseDestDir))
            return false;

        xmlFileName         = baseDestDir + QLatin1String("/gallery.xml");
        XMLWriter xmlWriter;

        if (!xmlWriter.open(xmlFileName))
        {
            logError(i18n("Could not create gallery.xml"));
            return false;
        }

        XMLElement collectionsX(xmlWriter, QLatin1String("collections"));

        if (info->m_getOption == GalleryInfo::ALBUMS)
        {
            // Loop over albums selection

            DInfoInterface::DAlbumIDs::ConstIterator albumIt  = info->m_albumList.constBegin();
            DInfoInterface::DAlbumIDs::ConstIterator albumEnd = info->m_albumList.constEnd();

            for (; albumIt != albumEnd ; ++albumIt)
            {
                int id                     = *albumIt;
                DInfoInterface::DInfoMap   inf;

                if (info->m_iface)
                    inf = info->m_iface->albumInfo(id);

                DAlbumInfo anf(inf);
                QString title              = anf.title();
                QString collectionFileName = webifyFileName(title);
                QString destDir            = baseDestDir + QLatin1Char('/') + collectionFileName;

                if (!createDir(destDir))
                {
                    return false;
                }

                XMLElement collectionX(xmlWriter,  QLatin1String("collection"));
                xmlWriter.writeElement("name",     title);
                xmlWriter.writeElement("fileName", collectionFileName);
                xmlWriter.writeElement("comment",  anf.caption());

                // Gather image element list
                QList<QUrl> imageList;

                if (info->m_iface)
                {
                    imageList = info->m_iface->albumsItems(DInfoInterface::DAlbumIDs() << id);
                }

                if (!processImages(xmlWriter, imageList, title, destDir))
                    return false;
            }
        }
        else
        {
            QString title              = info->imageSelectionTitle();
            QString collectionFileName = webifyFileName(title);
            QString destDir            = baseDestDir + QLatin1Char('/') + collectionFileName;

            if (!createDir(destDir))
            {
                return false;
            }

            XMLElement collectionX(xmlWriter,  QLatin1String("collection"));
            xmlWriter.writeElement("name",     title);
            xmlWriter.writeElement("fileName", collectionFileName);

            if (!processImages(xmlWriter, info->m_imageList, title, destDir))
                return false;
        }

        return true;
    }

    bool processImages(XMLWriter& xmlWriter, const QList<QUrl>& imageList,
                       const QString& title, const QString& destDir)
    {
        RemoteUrlHash remoteUrlHash;

        if (!downloadRemoteUrls(title, imageList, &remoteUrlHash))
        {
            return false;
        }

        QList<GalleryElement> imageElementList;

        foreach(const QUrl& url, imageList)
        {
            const QString path = remoteUrlHash.value(url, url.toLocalFile());

            if (path.isEmpty())
            {
                continue;
            }

            DInfoInterface::DInfoMap inf;

            if (info->m_iface)
                inf = info->m_iface->itemInfo(url);

            GalleryElement element = GalleryElement(inf);
            element.m_path         = remoteUrlHash.value(url, url.toLocalFile());
            imageElementList << element;
        }

        // Generate images
        logInfo(i18n("Generating files for \"%1\"", title));
        GalleryElementFunctor functor(that, info, destDir);
        QFuture<void> future = QtConcurrent::map(imageElementList, functor);
        QFutureWatcher<void> watcher;
        watcher.setFuture(future);

        connect(&watcher, SIGNAL(progressValueChanged(int)),
                pbar, SLOT(setValue(int)));

        pbar->setMaximum(imageElementList.count());

        while (!future.isFinished())
        {
            qApp->processEvents();

            if (cancel)
            {
                future.cancel();
                future.waitForFinished();
                return false;
            }
        }

        // Generate xml
        foreach(const GalleryElement& element, imageElementList)
        {
            element.appendToXML(xmlWriter, info->copyOriginalImage());
        }

        return true;
    }

    bool generateHTML()
    {
        logInfo(i18n("Generating HTML files"));

        QString xsltFileName                                 = theme->directory() + QLatin1String("/template.xsl");
        CWrapper<xsltStylesheetPtr, xsltFreeStylesheet> xslt = xsltParseStylesheetFile((const xmlChar*)
            QDir::toNativeSeparators(xsltFileName).toUtf8().data());

        if (!xslt)
        {
            logError(i18n("Could not load XSL file '%1'", xsltFileName));
            return false;
        }

        CWrapper<xmlDocPtr, xmlFreeDoc> xmlGallery =
            xmlParseFile(QDir::toNativeSeparators(xmlFileName).toUtf8().data() );

        if (!xmlGallery)
        {
            logError(i18n("Could not load XML file '%1'", xmlFileName));
            return false;
        }

        // Prepare parameters
        XsltParameterMap map;
        addI18nParameters(map);
        addThemeParameters(map);

        const char** params            = new const char*[map.size()*2+1];
        XsltParameterMap::Iterator it  = map.begin();
        XsltParameterMap::Iterator end = map.end();
        const char** ptr               = params;

        for ( ; it != end ; ++it)
        {
            *ptr = it.key().data();
            ++ptr;
            *ptr = it.value().data();
            ++ptr;
        }

        *ptr = 0;

        // Move to the destination dir, so that external documents get correctly
        // produced
        QString oldCD                             = QDir::currentPath();
        QDir::setCurrent(info->destUrl().toLocalFile());

        CWrapper<xmlDocPtr, xmlFreeDoc> xmlOutput = xsltApplyStylesheet(xslt, xmlGallery, params);

        QDir::setCurrent(oldCD);
        //delete []params;

        if (!xmlOutput)
        {
            logError(i18n("Error processing XML file"));
            return false;
        }

        QString destFileName = QDir::toNativeSeparators(info->destUrl().toLocalFile() + 
                                                        QLatin1String("/index.html"));

        if (xsltSaveResultToFilename(destFileName.toUtf8().data(), xmlOutput, xslt, 0) == -1)
        {
            logError(i18n("Could not open '%1' for writing", destFileName));
            return false;
        }

        return true;
    }

    bool downloadRemoteUrls(const QString& collectionName,
                            const QList<QUrl>& _list,
                            RemoteUrlHash* const hash)
    {
        Q_ASSERT(hash);
        QList<QUrl> list;

        foreach(const QUrl& url, _list)
        {
            if (!url.isLocalFile())
            {
                list << url;
            }
        }

        if (list.count() == 0)
        {
            return true;
        }

        logInfo(i18n("Downloading remote files for \"%1\"", collectionName));

        pbar->setMaximum(list.count());
        int count = 0;

        foreach(const QUrl& url, list)
        {
            if (cancel)
            {
                return false;
            }

            QTemporaryFile tempFile;
            tempFile.setFileTemplate(QLatin1String("htmlexport-"));

            if (!tempFile.open())
            {
                logError(i18n("Could not open temporary file"));
                return false;
            }

            QTemporaryFile tempPath;
            tempPath.setFileTemplate(tempFile.fileName());
            tempPath.setAutoRemove(false);

            if (tempPath.open() &&
                DFileOperations::copyFiles(QStringList() << url.toLocalFile(), tempPath.fileName()))
            {
                hash->insert(url, tempFile.fileName());
            }
            else
            {
                logWarning(i18n("Could not download %1", url.toDisplayString()));
                hash->insert(url, QString());
            }

            tempPath.close();
            tempFile.close();

            ++count;
            pbar->setValue(count);
        }

        return true;
    }

    /**
     * Add to map all the i18n parameters.
     */
    void addI18nParameters(XsltParameterMap& map)
    {
        map["i18nPrevious"]                   = makeXsltParam(i18n("Previous"));
        map["i18nNext"]                       = makeXsltParam(i18n("Next"));
        map["i18nCollectionList"]             = makeXsltParam(i18n("Album List"));
        map["i18nOriginalImage"]              = makeXsltParam(i18n("Original Image"));
        map["i18nUp"]                         = makeXsltParam(i18n("Go Up"));
        // Exif Tag
        map["i18nexifimagemake"]              = makeXsltParam(i18n("Make"));
        map["i18nexifimagemodel"]             = makeXsltParam(i18n("Model"));
        map["i18nexifimageorientation"]       = makeXsltParam(i18n("Image Orientation"));
        map["i18nexifimagexresolution"]       = makeXsltParam(i18n("Image X Resolution"));
        map["i18nexifimageyresolution"]       = makeXsltParam(i18n("Image Y Resolution"));
        map["i18nexifimageresolutionunit"]    = makeXsltParam(i18n("Image Resolution Unit"));
        map["i18nexifimagedatetime"]          = makeXsltParam(i18n("Image Date Time"));
        map["i18nexifimageycbcrpositioning"]  = makeXsltParam(i18n("YCBCR Positioning"));
        map["i18nexifphotoexposuretime"]      = makeXsltParam(i18n("Exposure Time"));
        map["i18nexifphotofnumber"]           = makeXsltParam(i18n("F Number"));
        map["i18nexifphotoexposureprogram"]   = makeXsltParam(i18n("Exposure Index"));
        map["i18nexifphotoisospeedratings"]   = makeXsltParam(i18n("ISO Speed Ratings"));
        map["i18nexifphotoshutterspeedvalue"] = makeXsltParam(i18n("Shutter Speed Value"));
        map["i18nexifphotoaperturevalue"]     = makeXsltParam(i18n("Aperture Value"));
        map["i18nexifphotofocallength"]       = makeXsltParam(i18n("Focal Length"));
        map["i18nexifgpsaltitude"]            = makeXsltParam(i18n("GPS Altitude"));
        map["i18nexifgpslatitude"]            = makeXsltParam(i18n("GPS Latitude"));
        map["i18nexifgpslongitude"]           = makeXsltParam(i18n("GPS Longitude"));
    }

    /**
     * Add to map all the theme parameters, as specified by the user.
     */
    void addThemeParameters(XsltParameterMap& map)
    {
        GalleryTheme::ParameterList parameterList      = theme->parameterList();
        QString themeInternalName                      = theme->internalName();
        GalleryTheme::ParameterList::ConstIterator it  = parameterList.constBegin();
        GalleryTheme::ParameterList::ConstIterator end = parameterList.constEnd();

        for (; it != end ; ++it)
        {
            AbstractThemeParameter* const themeParameter = *it;
            QByteArray internalName                      = themeParameter->internalName();
            QString value                                = info->getThemeParameterValue(themeInternalName,
                                                                QString::fromLatin1(internalName),
                                                                themeParameter->defaultValue());

            map[internalName]                            = makeXsltParam(value);
        }
    }

    /**
     * Prepare an XSLT param, managing quote mess.
     * abc   => 'abc'
     * a"bc  => 'a"bc'
     * a'bc  => "a'bc"
     * a"'bc => concat('a"', "'", 'bc')
     */
    QByteArray makeXsltParam(const QString& txt)
    {
        QString param;
        static const char apos  = '\'';
        static const char quote = '"';

        if (txt.indexOf(QLatin1Char(apos)) == -1)
        {
            // First or second case: no apos
            param = QLatin1Char(apos) + txt + QLatin1Char(apos);
        }
        else if (txt.indexOf(QLatin1Char(quote)) == -1)
        {
            // Third case: only apos, no quote
            param = QLatin1Char(quote) + txt + QLatin1Char(quote);
        }
        else
        {
            // Forth case: both apos and quote :-(
            const QStringList lst = txt.split(QLatin1Char(apos), QString::KeepEmptyParts);

            QStringList::ConstIterator it  = lst.constBegin();
            QStringList::ConstIterator end = lst.constEnd();
            param                          = QLatin1String("concat(");
            param                         += QLatin1Char(apos) + *it + QLatin1Char(apos);
            ++it;

            for (; it != end ; ++it)
            {
                param += QLatin1String(", \"'\", ");
                param += QLatin1Char(apos) + *it + QLatin1Char(apos);
            }

            param += QLatin1Char(')');
        }

        //qCDebug(DIGIKAM_GENERAL_LOG) << "param: " << txt << " => " << param;

        return param.toUtf8();
    }

    void logInfo(const QString& msg)
    {
        pview->addEntry(msg, DHistoryView::ProgressEntry);
    }

    void logError(const QString& msg)
    {
        pview->addEntry(msg, DHistoryView::ErrorEntry);
    }

    void logWarning(const QString& msg)
    {
        pview->addEntry(msg, DHistoryView::WarningEntry);
        warnings = true;
    }
};

// ----------------------------------------------------------------------

GalleryGenerator::GalleryGenerator(GalleryInfo* const info)
    : QObject(),
      d(new Private)
{
    d->that     = this;
    d->info     = info;
    d->warnings = false;

    connect(this, SIGNAL(logWarningRequested(QString)),
            SLOT(logWarning(QString)), Qt::QueuedConnection);
}

GalleryGenerator::~GalleryGenerator()
{
    delete d;
}

bool GalleryGenerator::run()
{
    if (!d->init())
        return false;

    QString destDir = d->info->destUrl().toLocalFile();
    qCDebug(DIGIKAM_GENERAL_LOG) << destDir;

    if (!d->createDir(destDir))
        return false;

    if (!d->copyTheme())
        return false;

    if (!d->generateImagesAndXML())
        return false;

    exsltRegisterAll();

    bool result = d->generateHTML();

    xsltCleanupGlobals();
    xmlCleanupParser();

    return result;
}

bool GalleryGenerator::warnings() const
{
    return d->warnings;
}

void GalleryGenerator::logWarning(const QString& text)
{
    d->logWarning(text);
}

void GalleryGenerator::slotCancel()
{
    d->cancel = true;
}

void GalleryGenerator::setProgressWidgets(DHistoryView* const pView, DProgressWdg* const pBar)
{
    d->pview = pView;
    d->pbar  = pBar;

    connect(d->pbar, SIGNAL(signalProgressCanceled()),
            this, SLOT(slotCancel()));
}

QString GalleryGenerator::webifyFileName(const QString& fname)
{
    QString fileName = fname.toLower();

    // Remove potentially troublesome chars
    return fileName.replace(QRegExp(QLatin1String("[^-0-9a-z]+")), QLatin1String("_"));
}

} // namespace Digikam
