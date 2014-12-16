/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : a kio-slave to process file operations on
 *               digiKam albums.
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2005 by Renchi Raju <renchi dot raju at gmail dot com>
 *
 * The forwarding code is copied from kdelibs' ForwardingSlavebase.
 * Copyright for the KDE file forwardingslavebase follows:
 * Copyright (C) 2004 Kevin Ottens <ervin@ipsquad.net>
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

#include "digikamalbums.moc"

// Qt includes

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDataStream>
#include <QFileInfo>

// KDE includes

#include <kcomponentdata.h>
#include <kglobal.h>
#include <kio/deletejob.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kdebug.h>

// LibKDcraw includes

#include <libkdcraw/rawfiles.h>

// Local includes

#include "albumdb.h"
#include "collectionscanner.h"
#include "databaseaccess.h"
#include "databaseurl.h"
#include "digikam_export.h"
#include "imagelister.h"


kio_digikamalbums::kio_digikamalbums(const QByteArray& pool_socket, const QByteArray& app_socket)
    : SlaveBase("kio_digikamalbums", pool_socket, app_socket)
{
    m_eventLoop = new QEventLoop(this);
}

kio_digikamalbums::~kio_digikamalbums()
{
}

// ------------------------ Listing and Scanning ------------------------ //

void kio_digikamalbums::special(const QByteArray& data)
{
    KUrl        kurl;
    QDataStream ds(data);
    ds >> kurl;

    kDebug() << "kio_digikamalbums::special " << kurl;

    Digikam::DatabaseParameters dbParameters(kurl);
    QDBusConnection::sessionBus().registerService(QString("org.kde.digikam.KIO-digikamtags-%1").arg(QString::number(QCoreApplication::instance()->applicationPid())));
    Digikam::DatabaseAccess::setParameters(dbParameters);

    bool folders = (metaData("folders") == "true");

    if (folders)
    {
        QMap<int, int> albumNumberMap = Digikam::DatabaseAccess().db()->getNumberOfImagesInAlbums();

        QByteArray  ba;
        QDataStream os(&ba, QIODevice::WriteOnly);
        os << albumNumberMap;
        SlaveBase::data(ba);
    }
    else
    {
        bool recursive               = (metaData("listAlbumsRecursively") == "true");
        bool listOnlyAvailableImages = (metaData("listOnlyAvailableImages") == "true");

        Digikam::ImageLister lister;
        lister.setRecursive(recursive);
        lister.setListOnlyAvailable(listOnlyAvailableImages);
        // send data every 200 images to be more responsive
        Digikam::ImageListerSlaveBaseGrowingPartsSendingReceiver receiver(this, 200, 2000, 100);
        lister.list(&receiver, kurl);
        receiver.sendData();
    }

    finished();
}

// ------------------------ Implementation of KIO::SlaveBase ------------------------ //

void kio_digikamalbums::get(const KUrl& url)
{
    kDebug() << " : " << url;

    // no need to open the db. we don't need to read/write to it

    Digikam::DatabaseUrl dbUrl(url);

    KIO::TransferJob* job = KIO::get(dbUrl.fileUrl(), KIO::NoReload, KIO::HideProgressInfo);
    connectTransferJob(job);

    if (m_eventLoop->exec() != 0)
    {
        return;
    }

    finished();
}

void kio_digikamalbums::put(const KUrl& url, int permissions, KIO::JobFlags flags)
{
    kDebug() << " : " << url.url();

    Digikam::DatabaseUrl dbUrl(url);
    Digikam::DatabaseAccess::setParameters((Digikam::DatabaseParameters)dbUrl);
    Digikam::DatabaseAccess access;

    // get the parent album
    int albumID = access.db()->getAlbumForPath(dbUrl.albumRootId(), dbUrl.album(), false);

    if (albumID == -1)
    {
        error(KIO::ERR_UNKNOWN, i18n("Destination album %1 not found in database.", url.directory()));
        return;
    }

    KIO::TransferJob* job = KIO::put(dbUrl.fileUrl(), permissions, flags | KIO::HideProgressInfo);
    connectTransferJob(job);

    if (m_eventLoop->exec() != 0)
    {
        return;
    }

    // Let CollectionScanner do the database part

    // We have done our job => finish
    finished();
}

void kio_digikamalbums::copy(const KUrl& src, const KUrl& dst, int mode, KIO::JobFlags flags)
{
    kDebug() << "Src: " << src.path() << ", Dst: " << dst.path();

    Digikam::DatabaseUrl dbUrlSrc(src);
    Digikam::DatabaseUrl dbUrlDst(dst);

    if (dbUrlSrc == dbUrlDst)
    {
        error( KIO::ERR_FILE_ALREADY_EXIST, dbUrlSrc.fileName() );
        return;
    }

    if (dbUrlSrc.parameters() != dbUrlDst.parameters())
    {
        error(KIO::ERR_UNKNOWN, i18n("Database parameters of source and destination do not match."));
        return;
    }

    Digikam::DatabaseAccess::setParameters((Digikam::DatabaseParameters)dbUrlSrc);
    Digikam::DatabaseAccess access;

    // find the src parent album - do not create
    int srcAlbumID = access.db()->getAlbumForPath(dbUrlSrc.albumRootId(), dbUrlSrc.album(), false);

    if (srcAlbumID == -1)
    {
        error(KIO::ERR_UNKNOWN, i18n("Source album %1 not found in database",
                                     dbUrlSrc.album()));
        return;
    }

    // find the dst parent album - do not create
    int dstAlbumID = access.db()->getAlbumForPath(dbUrlDst.albumRootId(), dbUrlDst.album(), false);

    if (dstAlbumID == -1)
    {
        error(KIO::ERR_UNKNOWN, i18n("Destination album %1 not found in database",dbUrlDst.album()));
        return;
    }

    if (access.db()->getImageId(srcAlbumID, dbUrlSrc.fileName()) == -1)
    {
        error(KIO::ERR_UNKNOWN, i18n("Source image %1 not found in database", dbUrlSrc.fileName()));
        return;
    }

    // if the filename is .digikam_properties, we have been asked to copy the
    // metadata of the src album to the dst album
    //NOTE: Feature old and currently unused
/*
    if (src.fileName() == ".digikam_properties")
    {
        access.db()->copyAlbumProperties(srcAlbumID, dstAlbumID);
        finished();
        return;
    }
*/

    KIO::Job* job = KIO::file_copy(dbUrlSrc.fileUrl(), dbUrlDst.fileUrl(), mode, flags | KIO::HideProgressInfo );
    connectJob(job);

    if (m_eventLoop->exec() != 0)
    {
        return;
    }

    // Let CollectionScanner do the database part
    //access.db()->copyItem(srcAlbumID, dbUrlSrc.fileName(), dstAlbumID, dbUrlSrc.fileName());

    finished();
}

void kio_digikamalbums::rename(const KUrl& src, const KUrl& dst, KIO::JobFlags flags)
{
    kDebug() << "Src: " << src << ", Dst: " << dst;

    // if the filename is .digikam_properties ignore it
    if (src.fileName() == ".digikam_properties")
    {
        finished();
        return;
    }

    Digikam::DatabaseUrl dbUrlSrc(src);
    Digikam::DatabaseUrl dbUrlDst(dst);

    if (dbUrlSrc.parameters() != dbUrlDst.parameters())
    {
        error(KIO::ERR_UNKNOWN, i18n("Database parameters of source and destination do not match."));
        return;
    }

    Digikam::DatabaseAccess::setParameters((Digikam::DatabaseParameters)dbUrlSrc);
    Digikam::DatabaseAccess access;

    // check if we are renaming an album or a image
    QFileInfo info(dbUrlSrc.fileUrl().toLocalFile());
    bool renamingAlbum = info.isDir();

    int srcAlbumID = -1;

    if (renamingAlbum)
    {
        srcAlbumID = access.db()->getAlbumForPath(dbUrlSrc.albumRootId(), dbUrlSrc.album(), false);

        if (srcAlbumID == -1)
        {
            error(KIO::ERR_UNKNOWN, i18n("Source album %1 not found in database", src.url()));
            return;
        }
    }
    else
    {
        srcAlbumID = access.db()->getAlbumForPath(dbUrlSrc.albumRootId(), dbUrlSrc.album(), false);

        if (srcAlbumID == -1)
        {
            error(KIO::ERR_UNKNOWN, i18n("Source album %1 not found in database", src.directory()));
            return;
        }

        int dstAlbumID = access.db()->getAlbumForPath(dbUrlDst.albumRootId(), dbUrlDst.album(), false);

        if (dstAlbumID == -1)
        {
            error(KIO::ERR_UNKNOWN, i18n("Destination album %1 not found in database.", dst.directory()));
            return;
        }
    }

    KIO::Job* job = KIO::rename(dbUrlSrc.fileUrl(), dbUrlDst.fileUrl(), flags);
    connectJob(job);

    if (m_eventLoop->exec() != 0)
    {
        return;
    }

    // Let CollectionScanner do the database part.
/*
    if (renamingAlbum)
    {
        // rename subalbums as well
        access.db()->renameAlbum(srcAlbumID, dbUrlDst.album(), true);
    }
    else
    {
        access.db()->moveItem(srcAlbumID, dbUrlSrc.fileName(),
                              dstAlbumID, dbUrlDst.fileName());
    }
*/

    finished();
}

void kio_digikamalbums::mkdir(const KUrl& url, int permissions)
{
    kDebug() << " : " << url.url();

    Digikam::DatabaseUrl dbUrl(url);
    // DatabaseUrl has a strong opinion there should be a slash, KDE does not
    dbUrl.adjustPath(KUrl::AddTrailingSlash);
    Digikam::DatabaseAccess::setParameters((Digikam::DatabaseParameters)dbUrl);
    Digikam::DatabaseAccess access;

    KIO::SimpleJob* job = KIO::mkdir(dbUrl.fileUrl(), permissions);
    connectSimpleJob(job);

    if (m_eventLoop->exec() != 0)
    {
        return;
    }

    // We need to do this here, and not let CollectionScanner do this,
    // because the scanner might take time and put() will be called before
    access.db()->addAlbum(dbUrl.albumRootId(), dbUrl.album(), QString(), QDate::currentDate(), QString());

    finished();
}

void kio_digikamalbums::chmod(const KUrl& url, int permissions)
{
    kDebug() << " : " << url.url();

    Digikam::DatabaseUrl dbUrl(url);

    KIO::SimpleJob* job = KIO::chmod(dbUrl.fileUrl(), permissions);
    connectSimpleJob(job);

    if (m_eventLoop->exec() != 0)
    {
        return;
    }

    finished();
}

void kio_digikamalbums::del(const KUrl& url, bool isFile)
{
    kDebug() << " : " << url.url();

    // if the filename is .digikam_properties ignore it
    if (isFile && url.fileName() == ".digikam_properties")
    {
        finished();
        return;
    }

    Digikam::DatabaseUrl dbUrl(url);
    Digikam::DatabaseAccess::setParameters((Digikam::DatabaseParameters)dbUrl);
    Digikam::DatabaseAccess access;

    int albumID;

    if (isFile)
    {
        // find the Album to which this file belongs.
        albumID = access.db()->getAlbumForPath(dbUrl.albumRootId(), dbUrl.album(), false);

        if (albumID == -1)
        {
            error(KIO::ERR_UNKNOWN, i18n("Source album %1 not found in database", url.directory()));
            return;
        }
    }
    else
    {
        // find the corresponding album entry
        albumID = access.db()->getAlbumForPath(dbUrl.albumRootId(), dbUrl.album(), false);

        if (albumID == -1)
        {
            error(KIO::ERR_UNKNOWN, i18n("Source album %1 not found in database", url.path()));
            return;
        }
    }

    if (isFile)
    {
        KIO::DeleteJob* job = KIO::del(dbUrl.fileUrl(), KIO::HideProgressInfo);
        connectJob(job);
    }
    else
    {
        KIO::SimpleJob* job = KIO::rmdir(dbUrl.fileUrl());
        connectSimpleJob(job);
    }

    if (m_eventLoop->exec() != 0)
    {
        return;
    }

    // Let CollectionScanner do the database part

/*
    if (isFile)
    {
        // successful deletion. now remove file entry from the database
        access.db()->deleteItem(albumID, url.fileName());
    }
    else
    {
        // successful deletion. now remove album entry from the database
        access.db()->deleteAlbum(albumID);
    }
*/

    finished();
}

void kio_digikamalbums::stat(const KUrl& url)
{
    Digikam::DatabaseUrl dbUrl(url);

    KIO::SimpleJob* job = KIO::stat(dbUrl.fileUrl(), KIO::HideProgressInfo);
    connectSimpleJob(job);

    if (m_eventLoop->exec() != 0)
    {
        return;
    }

    finished();
}

void kio_digikamalbums::listDir(const KUrl& url)
{
    kDebug() << " : " << url.path();

    Digikam::DatabaseUrl dbUrl(url);

    KIO::UDSEntry entry;
    //createDigikamPropsUDSEntry(entry);
    //listEntry(entry, false);

    KIO::ListJob* job = KIO::listDir(dbUrl.fileUrl(), KIO::HideProgressInfo);
    connectListJob(job);

    if (m_eventLoop->exec() != 0)
    {
        return;
    }

    finished();
}

/*
void kio_digikamalbums::createDigikamPropsUDSEntry(KIO::UDSEntry& entry)
{
    entry.clear();

    entry.insert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG);
    entry.insert(KIO::UDSEntry::UDS_ACCESS, 00666);
    entry.insert(KIO::UDSEntry::UDS_SIZE, 0);
    entry.insert(KIO::UDSEntry::UDS_MODIFICATION_TIME, QDateTime::currentDateTime().toTime_t());
    entry.insert(KIO::UDSEntry::UDS_ACCESS_TIME, QDateTime::currentDateTime().toTime_t());
    entry.insert(KIO::UDSEntry::UDS_NAME, QString(".digikam_properties"));
}
*/

// ------------------------ Job forwarding code ------------------------ //

void kio_digikamalbums::connectJob(KIO::Job* job)
{
    // We will forward the warning message, no need to let the job
    // display it itself
    job->setUiDelegate( 0 );

    // Forward metadata (e.g. modification time for put())
    job->setMetaData( allMetaData() );

    connect( job, SIGNAL(result(KJob*)),
             this, SLOT(slotResult(KJob*)) );

    connect( job, SIGNAL(warning(KJob*,QString,QString)),
             this, SLOT(slotWarning(KJob*,QString)) );

    connect( job, SIGNAL(infoMessage(KJob*,QString,QString)),
             this, SLOT(slotInfoMessage(KJob*,QString)) );

    connect( job, SIGNAL(totalSize(KJob*,qulonglong)),
             this, SLOT(slotTotalSize(KJob*,qulonglong)) );

    connect( job, SIGNAL(processedSize(KJob*,qulonglong)),
             this, SLOT(slotProcessedSize(KJob*,qulonglong)) );

    connect( job, SIGNAL(speed(KJob*,ulong)),
             this, SLOT(slotSpeed(KJob*,ulong)) );
}

void kio_digikamalbums::connectSimpleJob(KIO::SimpleJob* job)
{
    connectJob(job);

    connect( job, SIGNAL(redirection(KIO::Job*,KUrl)),
             this, SLOT(slotRedirection(KIO::Job*,KUrl)) );
}

void kio_digikamalbums::connectListJob(KIO::ListJob* job)
{
    connectSimpleJob(job);

    connect( job, SIGNAL(entries(KIO::Job*,KIO::UDSEntryList)),
             this, SLOT(slotEntries(KIO::Job*,KIO::UDSEntryList)) );
}

void kio_digikamalbums::connectTransferJob(KIO::TransferJob* job)
{
    connectSimpleJob(job);

    connect( job, SIGNAL(data(KIO::Job*,QByteArray)),
             this, SLOT(slotData(KIO::Job*,QByteArray)) );

    connect( job, SIGNAL(dataReq(KIO::Job*,QByteArray&)),
             this, SLOT(slotDataReq(KIO::Job*,QByteArray&)) );

    connect( job, SIGNAL(mimetype(KIO::Job*,QString)),
             this, SLOT(slotMimetype(KIO::Job*,QString)) );

    connect( job, SIGNAL(canResume(KIO::Job*,KIO::filesize_t)),
             this, SLOT(slotCanResume(KIO::Job*,KIO::filesize_t)) );
}

void kio_digikamalbums::slotResult(KJob* job)
{
    if ( job->error() != 0)
    {
        error( job->error(), job->errorText() );
        m_eventLoop->exit(1);
    }
    else
    {
        KIO::StatJob* stat_job = qobject_cast<KIO::StatJob*>(job);

        if ( stat_job!=0L )
        {
            KIO::UDSEntry entry = stat_job->statResult();
            //prepareUDSEntry(entry);
            statEntry( entry );
        }

        //finished();
        m_eventLoop->exit();
    }
}

void kio_digikamalbums::slotWarning(KJob* /*job*/, const QString& msg)
{
    warning(msg);
}

void kio_digikamalbums::slotInfoMessage(KJob* /*job*/, const QString& msg)
{
    infoMessage(msg);
}

void kio_digikamalbums::slotTotalSize(KJob* /*job*/, qulonglong size)
{
    totalSize(size);
}

void kio_digikamalbums::slotProcessedSize(KJob* /*job*/, qulonglong size)
{
    processedSize(size);
}

void kio_digikamalbums::slotSpeed(KJob* /*job*/, unsigned long bytesPerSecond)
{
    speed(bytesPerSecond);
}

void kio_digikamalbums::slotRedirection(KIO::Job* job, const KUrl& url)
{
    redirection(url);

    // We've been redirected stop everything.
    job->kill( KJob::Quietly );
    //finished();

    m_eventLoop->exit();
}

void kio_digikamalbums::slotEntries(KIO::Job* /*job*/, const KIO::UDSEntryList& entries)
{
/*
    KIO::UDSEntryList final_entries = entries;

    KIO::UDSEntryList::iterator it = final_entries.begin();
    const KIO::UDSEntryList::iterator end = final_entries.end();

    for(; it!=end; ++it)
    {
        prepareUDSEntry(*it, true);
    }

    listEntries( final_entries );
*/
    listEntries(entries);
}

void kio_digikamalbums::slotData(KIO::Job* /*job*/, const QByteArray& _data)
{
    data(_data);
}

void kio_digikamalbums::slotDataReq(KIO::Job* /*job*/, QByteArray& data)
{
    dataReq();
    readData(data);
}

void kio_digikamalbums::slotMimetype (KIO::Job* /*job*/, const QString& type)
{
    mimeType(type);
}

void kio_digikamalbums::slotCanResume (KIO::Job* /*job*/, KIO::filesize_t offset)
{
    canResume(offset);
}

// ------------------------ KIO slave registration ------------------------ //

extern "C"
{
    KDE_EXPORT int kdemain(int argc, char** argv)
    {
        // Needed to load SQL driver plugins
        QCoreApplication app(argc, argv);

        KLocale::setMainCatalog("digikam");
        KComponentData componentData( "kio_digikamalbums" );
        KGlobal::locale();

        kDebug() << "*** kio_digikamalbums started ***";

        if (argc != 4)
        {
            kDebug() << "Usage: kio_digikamalbums protocol domain-socket1 domain-socket2";
            exit(-1);
        }

        kio_digikamalbums slave(argv[2], argv[3]);
        slave.dispatchLoop();

        kDebug() << "*** kio_digikamalbums finished ***";
        return 0;
    }
}
