/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-30
 * Description : a tool to export items to Piwigo web service
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006      by Colin Guthrie <kde at colin dot guthr dot ie>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008      by Andrea Diamantini <adjam7 at gmail dot com>
 * Copyright (C) 2010-2014 by Frederic Coiffier <frederic dot coiffier at free dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef PIWIGO_TALKER_H
#define PIWIGO_TALKER_H

// Qt includes

#include <QObject>
#include <QList>
#include <QDateTime>
#include <QTextStream>
#include <QFile>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkAccessManager>

// Local includes

#include "dinfointerface.h"

template <class T> class QList;

namespace Digikam
{

class PiwigoAlbum;

class PiwigoTalker : public QObject
{
    Q_OBJECT

public:

    enum State
    {
        GE_LOGOUT = -1,
        GE_LOGIN  = 0,
        GE_GETVERSION,
        GE_LISTALBUMS,
        GE_CHECKPHOTOEXIST,
        GE_GETINFO,
        GE_SETINFO,
        GE_ADDPHOTOCHUNK,
        GE_ADDPHOTOSUMMARY
    };

    enum
    {
        CHUNK_MAX_SIZE = 512*1024,
        PIWIGO_VER_2_4 = 24
    };

public:

    explicit PiwigoTalker(DInfoInterface* const iface,
                          QWidget* const parent);
    ~PiwigoTalker();

public:

    bool loggedIn() const;

    void login(const QUrl& url, const QString& name, const QString& passwd);
    void listAlbums();
    void listPhotos(const QString& albumName);

/* TODO Implement this function
    void createAlbum(const QString& parentAlbumName,
                     const QString& albumName,
                     const QString& albumTitle,
                     const QString& albumCaption);
*/

    bool addPhoto(int   albumId,
                  const QString& photoPath,
                  bool  rescale = false,
                  int   maxWidth = 1600,
                  int   maxHeight = 1600,
                  int   quality = 95);

    void cancel();

    static QString getAuthToken();

Q_SIGNALS:

    void signalProgressInfo(const QString& msg);
    void signalError(const QString& msg);
    void signalLoginFailed(const QString& msg);
    void signalBusy(bool val);
    void signalAlbums(const QList<PiwigoAlbum>& albumList);
    void signalAddPhotoSucceeded();
    void signalAddPhotoFailed(const QString& msg);

private:

    void parseResponseLogin(const QByteArray& data);
    void parseResponseGetVersion(const QByteArray& data);
    void parseResponseListAlbums(const QByteArray& data);
    void parseResponseDoesPhotoExist(const QByteArray& data);
    void parseResponseGetInfo(const QByteArray& data);
    void parseResponseSetInfo(const QByteArray& data);

    void addNextChunk();
    void parseResponseAddPhotoChunk(const QByteArray& data);
    void addPhotoSummary();
    void parseResponseAddPhotoSummary(const QByteArray& data);

    QByteArray computeMD5Sum(const QString& filepath);
    void deleteTemporaryFile();

private Q_SLOTS:

    void slotFinished(QNetworkReply* reply);

private:

    class Private;
    Private* const d;
    
    static QString s_authToken;
};

} // namespace Digikam

#endif // PIWIGOTALKER_H
