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
 * ============================================================ */

#ifndef DIGIKAMALBUMS_H
#define DIGIKAMALBUMS_H

// Qt includes

#include <QObject>
#include <QDateTime>
#include <QEventLoop>

// KDE includes

#include <kio/slavebase.h>
#include <kjob.h>
#include <kio/job.h>

class kio_digikamalbums : public QObject, public KIO::SlaveBase
{
    Q_OBJECT

public:

    kio_digikamalbums(const QByteArray& pool_socket, const QByteArray& app_socket);
    virtual ~kio_digikamalbums();

    void special(const QByteArray& data);

    void get(const KUrl& url);
    void put(const KUrl& url, int _mode, KIO::JobFlags _flags);
    void copy(const KUrl& src, const KUrl& dest, int mode, KIO::JobFlags flags);
    void rename(const KUrl& src, const KUrl& dest, KIO::JobFlags flags);

    void stat(const KUrl& url);
    void listDir(const KUrl& url);
    void mkdir(const KUrl& url, int permissions);
    void chmod(const KUrl& url, int permissions);
    void del(const KUrl& url, bool isfile);

private:

    void createDigikamPropsUDSEntry(KIO::UDSEntry& entry);
    bool createUDSEntry(const QString& path, KIO::UDSEntry& entry);

    void connectJob(KIO::Job* job);
    void connectSimpleJob(KIO::SimpleJob* job);
    void connectListJob(KIO::ListJob* job);
    void connectTransferJob(KIO::TransferJob* job);

private Q_SLOTS:

    // KIO::Job
    void slotResult(KJob* job);
    void slotWarning(KJob* job, const QString& msg);
    void slotInfoMessage(KJob* job, const QString& msg);
    void slotTotalSize(KJob* job, qulonglong size);
    void slotProcessedSize(KJob* job, qulonglong size);
    void slotSpeed(KJob* job, unsigned long bytesPerSecond);

    // KIO::SimpleJob subclasses
    void slotRedirection(KIO::Job* job, const KUrl& url);

    // KIO::ListJob
    void slotEntries(KIO::Job* job, const KIO::UDSEntryList& entries);

    // KIO::TransferJob
    void slotData(KIO::Job* job, const QByteArray& data);
    void slotDataReq(KIO::Job* job, QByteArray& data);
    void slotMimetype (KIO::Job* job, const QString& type);
    void slotCanResume (KIO::Job* job, KIO::filesize_t offset);

private:

    QEventLoop* m_eventLoop;
};

#endif /* DIGIKAMALBUMS_H */
