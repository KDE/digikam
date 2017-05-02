/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "generator.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QFutureWatcher>
#include <QRegExp>
#include <QStringList>
#include <QtConcurrentMap>
#include <QApplication>
#include <QUrl>

// KDE includes

#include <klocalizedstring.h>
#include <kio/job.h>
#include <kio/netaccess.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>

// libxslt includes

#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/xslt.h>
#include <libexslt/exslt.h>

// Local includes

#include "digikam_debug.h"
#include "abstractthemeparameter.h"
#include "imageelement.h"
#include "imagegenerationfunctor.h"
#include "galleryinfo.h"
#include "theme.h"
#include "xmlutils.h"

namespace Digikam
{

typedef QMap<QByteArray, QByteArray> XsltParameterMap;

/**
 * Produce a web-friendly file name
 */
QString Generator::webifyFileName(const QString& _fileName)
{
    QString fileName = _fileName.toLower();

    // Remove potentially troublesome chars
    return fileName.replace(QRegExp("[^-0-9a-z]+"), "_");
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

    if (txt.indexOf(apos) == -1)
    {
        // First or second case: no apos
        param = apos + txt + apos;
    }
    else if (txt.indexOf(quote) == -1)
    {
        // Third case: only apos, no quote
        param = quote + txt + quote;
    }
    else
    {
        // Forth case: both apos and quote :-(
        const QStringList lst = txt.split(apos, QString::KeepEmptyParts);

        QStringList::ConstIterator it  = lst.constBegin();
        QStringList::ConstIterator end = lst.constEnd();
        param                          = "concat(";
        param                         += apos + *it + apos;
        ++it;

        for (; it != end ; ++it)
        {
            param += ", \"'\", ";
            param += apos + *it + apos;
        }

        param += ')';
    }

    //qCDebug(DIGIKAM_GENERAL_LOG) << "param: " << txt << " => " << param;
    return param.toUtf8();
}

// ----------------------------------------------------------------------

class Generator::Private
{
public:

    Generator*             that;
    Interface*             mInterface;
    GalleryInfo*           mInfo;
    KPBatchProgressDialog* mProgressDialog;
    Theme::Ptr             mTheme;

    // State info
    bool                   mWarnings;
    QString                mXMLFileName;

public:

    bool init()
    {
        mTheme = Theme::findByInternalName(mInfo->theme());

        if (!mTheme)
        {
            logError( i18n("Could not find theme in '%1'", mInfo->theme()) );
            return false;
        }

        return true;
    }

    bool copyTheme()
    {
        mProgressDialog->progressWidget()->addedAction(i18n("Copying theme"), ProgressMessage);

        QUrl srcUrl  = QUrl(mTheme->directory());
        QUrl destUrl = mInfo->destUrl();

        destUrl.addPath(srcUrl.fileName());

        if (QFile::exists(destUrl.toLocalFile()))
        {
            KIO::NetAccess::del(destUrl, mProgressDialog);
        }

        bool ok = KIO::NetAccess::dircopy(srcUrl, destUrl, mProgressDialog);

        if (!ok)
        {
            logError(i18n("Could not copy theme"));
            return false;
        }

        return true;
    }

    // Url => local temp path
    typedef QHash<QUrl, QString> RemoteUrlHash;

    bool downloadRemoteUrls(const QString& collectionName, const QUrl::List& _list, RemoteUrlHash* const hash)
    {
        Q_ASSERT(hash);
        QUrl::List list;

        Q_FOREACH(const QUrl& url, _list)
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

        logInfo( i18n("Downloading remote files for \"%1\"", collectionName) );

        mProgressDialog->progressWidget()->setTotal(list.count());
        int count = 0;

        Q_FOREACH(const QUrl& url, list)
        {
            if (mProgressDialog->isHidden())
            {
                return false;
            }

            KTemporaryFile* const tempFile = new KTemporaryFile;
            // Ensure the tempFile gets deleted when mProgressDialog is closed
            tempFile->setParent(mProgressDialog);
            tempFile->setPrefix("htmlexport-");

            if (!tempFile->open())
            {
                delete tempFile;
                logError(i18n("Could not open temporary file"));
                return false;
            }

            const QString tempPath = KStandardDirs::locate("tmp", tempFile->fileName());
            KIO::Job* job          = KIO::file_copy(url,
                                                    QUrl::fromLocalFile(tempPath),
                                                    -1 /* permissions */,
                                                    KIO::Overwrite);

            if (KIO::NetAccess::synchronousRun(job, mProgressDialog))
            {
                hash->insert(url, tempFile->fileName());
            }
            else
            {
                logWarning(i18n("Could not download %1", url.prettyUrl()));
                hash->insert(url, QString());
            }

            ++count;
            mProgressDialog->progressWidget()->setProgress(count);
        }

        return true;
    }

    bool generateImagesAndXML()
    {
        QString baseDestDir = mInfo->destUrl().toLocalFile();

        if (!createDir(baseDestDir))
            return false;

        mXMLFileName        = baseDestDir + QLatin1String("/gallery.xml");
        XMLWriter xmlWriter;

        if (!xmlWriter.open(mXMLFileName))
        {
            logError(i18n("Could not create gallery.xml"));
            return false;
        }

        XMLElement collectionsX(xmlWriter, "collections");

        // Loop on collections
        QList<ImageCollection>::ConstIterator collectionIt  = mInfo->mCollectionList.constBegin();
        QList<ImageCollection>::ConstIterator collectionEnd = mInfo->mCollectionList.constEnd();

        for (; collectionIt != collectionEnd ; ++collectionIt)
        {
            ImageCollection collection =* collectionIt;

            QString collectionFileName = webifyFileName(collection.name());
            QString destDir            = baseDestDir + QLatin1Char('/') + collectionFileName;

            if (!createDir(destDir))
                return false;

            XMLElement collectionX(xmlWriter,  "collection");
            xmlWriter.writeElement("name",     collection.name());
            xmlWriter.writeElement("fileName", collectionFileName);
            xmlWriter.writeElement("comment",  collection.comment());

            // Gather image element list
            QUrl::List imageList = collection.images();
            RemoteUrlHash remoteUrlHash;

            if (!downloadRemoteUrls(collection.name(), imageList, &remoteUrlHash))
            {
                return false;
            }

            QList<ImageElement> imageElementList;

            Q_FOREACH(const QUrl& url, imageList)
            {
                const QString path = remoteUrlHash.value(url, url.toLocalFile());

                if (path.isEmpty())
                {
                    continue;
                }

                KPImageInfo info(url);
                ImageElement element = ImageElement(info);
                element.mPath        = remoteUrlHash.value(url, url.toLocalFile());
                imageElementList << element;
            }

            // Generate images
            logInfo( i18n("Generating files for \"%1\"", collection.name()) );
            ImageGenerationFunctor functor(that, mInfo, destDir);
            QFuture<void> future = QtConcurrent::map(imageElementList, functor);
            QFutureWatcher<void> watcher;
            watcher.setFuture(future);

            connect(&watcher, SIGNAL(progressValueChanged(int)),
                    mProgressDialog->progressWidget(), SLOT(setProgress(int)));

            mProgressDialog->progressWidget()->setTotal(imageElementList.count());

            while (!future.isFinished())
            {
                qApp->processEvents();

                if (mProgressDialog->isHidden())
                {
                    future.cancel();
                    future.waitForFinished();
                    return false;
                }
            }

            // Generate xml
            Q_FOREACH(const ImageElement& element, imageElementList)
            {
                element.appendToXML(xmlWriter, mInfo->copyOriginalImage());
            }
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
        map["i18nCollectionList"]             = makeXsltParam(i18n("Collection List"));
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
        Theme::ParameterList parameterList      = mTheme->parameterList();
        QString themeInternalName               = mTheme->internalName();
        Theme::ParameterList::ConstIterator it  = parameterList.constBegin();
        Theme::ParameterList::ConstIterator end = parameterList.constEnd();

        for (; it!=end; ++it)
        {
            AbstractThemeParameter* const themeParameter = *it;
            QByteArray internalName                      = themeParameter->internalName();
            QString value                                = mInfo->getThemeParameterValue(themeInternalName,
                                                                                         internalName,
                                                                                         themeParameter->defaultValue());

            map[internalName]                            = makeXsltParam(value);
        }
    }

    bool generateHTML()
    {
        logInfo(i18n("Generating HTML files"));

        QString xsltFileName                                 = mTheme->directory() + QLatin1String("/template.xsl");
        CWrapper<xsltStylesheetPtr, xsltFreeStylesheet> xslt = xsltParseStylesheetFile((const xmlChar*)
            QDir::toNativeSeparators(xsltFileName).toLocal8Bit().data());

        if (!xslt)
        {
            logError(i18n("Could not load XSL file '%1'", xsltFileName));
            return false;
        }

        CWrapper<xmlDocPtr, xmlFreeDoc> xmlGallery =
            xmlParseFile(QDir::toNativeSeparators(mXMLFileName).toLocal8Bit().data() );

        if (!xmlGallery)
        {
            logError(i18n("Could not load XML file '%1'", mXMLFileName));
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
        QDir::setCurrent(mInfo->destUrl().toLocalFile());

        CWrapper<xmlDocPtr, xmlFreeDoc> xmlOutput = xsltApplyStylesheet(xslt, xmlGallery, params);

        QDir::setCurrent(oldCD);
        //delete []params;

        if (!xmlOutput)
        {
            logError(i18n("Error processing XML file"));
            return false;
        }

        QString destFileName = QDir::toNativeSeparators(mInfo->destUrl().toLocalFile() + "/index.html");

#ifdef Q_CC_MSVC
        if (-1 == xsltSaveResultToFilename(destFileName.toLocal8Bit().data(), xmlOutput, xslt, 0))
        {
            logError(i18n("Could not open '%1' for writing", destFileName));
            return false;
        }
#else
        FILE* const file = fopen(destFileName.toLocal8Bit().data(), "w");

        if (!file)
        {
            logError(i18n("Could not open '%1' for writing", destFileName));
            return false;
        }

        xsltSaveResultToFile(file, xmlOutput, xslt);
        fclose(file);
#endif

        return true;
    }

    bool createDir(const QString& dirName)
    {
        QStringList parts = dirName.split(QLatin1Char('/'), QString::SkipEmptyParts);
        QDir dir          = QDir::root();

        Q_FOREACH(const QString& part, parts)
        {
            if (!dir.exists(part))
            {
                if (!dir.mkdir(part))
                {
                    logError(i18n("Could not create folder '%1' in '%2'", part, dir.absolutePath()));
                    return false;
                }
            }

            dir.cd(part);
        }

        return true;
    }

    void logInfo(const QString& msg)
    {
        mProgressDialog->progressWidget()->addedAction(msg, ProgressMessage);
    }

    void logError(const QString& msg)
    {
        mProgressDialog->progressWidget()->addedAction(msg, ErrorMessage);
    }

    void logWarning(const QString& msg)
    {
        mProgressDialog->progressWidget()->addedAction(msg, WarningMessage);
        mWarnings = true;
    }
};

// ----------------------------------------------------------------------

Generator::Generator(Interface* const interface,
                     GalleryInfo* const info,
                     KPBatchProgressDialog* const progressDialog)
    : QObject(),
      d(new Private)
{
    d->that            = this;
    d->mInterface      = interface;
    d->mInfo           = info;
    d->mProgressDialog = progressDialog;
    d->mWarnings       = false;

    connect(this, SIGNAL(logWarningRequested(QString)),
            SLOT(logWarning(QString)), Qt::QueuedConnection);
}

Generator::~Generator()
{
    delete d;
}

bool Generator::run()
{
    if (!d->init())
        return false;

    QString destDir = d->mInfo->destUrl().toLocalFile();
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

bool Generator::warnings() const
{
    return d->mWarnings;
}

void Generator::logWarning(const QString& text)
{
    d->logWarning(text);
}

} // namespace Digikam
