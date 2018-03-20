/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-14
 * Description : a tool to export items to YandexFotki web service
 *
 * Copyright (C) 2010      by Roman Tsisyk <roman at tsisyk dot com>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef YF_TALKER_H
#define YF_TALKER_H

// Qt includes

#include <QObject>
#include <QString>
#include <QDomElement>

// Local includes

#include "yfphoto.h"
#include "yfalbum.h"

class QNetworkReply;

namespace Digikam
{

class YFTalker : public QObject
{
    Q_OBJECT

public:

    // We use FSM for async I/O

    enum State
    {
        // FSM states
        STATE_UNAUTHENTICATED        = 0x0,
        STATE_AUTHENTICATED          = 0x80,
        STATE_ERROR                  = 0x40,

        STATE_GETSERVICE             = STATE_UNAUTHENTICATED | 0x1,
        STATE_GETSERVICE_ERROR       = STATE_UNAUTHENTICATED | STATE_ERROR | 0x2,
        STATE_GETSERVICE_DONE        = STATE_UNAUTHENTICATED | 0x3,
/*
        // for future use
        STATE_CHECKTOKEN             = STATE_UNAUTHENTICATED | 0x4,
        STATE_CHECKTOKEN_INVALID     = STATE_UNAUTHENTICATED | 0x5,
        STATE_CHECKTOKEN_DONE        = STATE_UNAUTHENTICATED | 0x6,
*/
        STATE_GETSESSION             = STATE_UNAUTHENTICATED | 0x7,
        STATE_GETSESSION_ERROR       = STATE_UNAUTHENTICATED | STATE_ERROR | 0x8,
        STATE_GETSESSION_DONE        = STATE_UNAUTHENTICATED | 0x9,

        STATE_GETTOKEN               = STATE_UNAUTHENTICATED | 0xa,
        STATE_INVALID_CREDENTIALS    = STATE_UNAUTHENTICATED | STATE_ERROR | 0xb,
        STATE_GETTOKEN_ERROR         = STATE_UNAUTHENTICATED | STATE_ERROR | 0xc,
        STATE_GETTOKEN_DONE          = STATE_AUTHENTICATED, // simple alias

        STATE_LISTALBUMS             = STATE_AUTHENTICATED   | 0x1,
        STATE_LISTALBUMS_ERROR       = STATE_AUTHENTICATED   | STATE_ERROR | 0x2,
        STATE_LISTALBUMS_DONE        = STATE_AUTHENTICATED,

        STATE_LISTPHOTOS             = STATE_AUTHENTICATED   | 0x4,
        STATE_LISTPHOTOS_ERROR       = STATE_AUTHENTICATED   | STATE_ERROR | 0x5,
        STATE_LISTPHOTOS_DONE        = STATE_AUTHENTICATED,

        STATE_UPDATEPHOTO_FILE       = STATE_AUTHENTICATED   | 0x7,
        STATE_UPDATEPHOTO_FILE_ERROR = STATE_AUTHENTICATED   | STATE_ERROR | 0x8,
        STATE_UPDATEPHOTO_INFO       = STATE_AUTHENTICATED   | 0x9,
        STATE_UPDATEPHOTO_INFO_ERROR = STATE_AUTHENTICATED   | STATE_ERROR | 0xa,
        STATE_UPDATEPHOTO_DONE       = STATE_AUTHENTICATED,

        STATE_UPDATEALBUM            = STATE_AUTHENTICATED   | 0xb,
        STATE_UPDATEALBUM_ERROR      = STATE_AUTHENTICATED   | STATE_ERROR | 0xc,
        STATE_UPDATEALBUM_DONE       = STATE_AUTHENTICATED
    };

public:

    explicit YFTalker(QObject* const parent = 0);
    ~YFTalker();

public:

    // Fields

    State state()                           const;
    const QString& sessionKey()             const;
    const QString& sessionId()              const;
    const QString& token()                  const;

    const QString& login()                  const;
    void  setLogin(const QString& login);

    const QString& password()               const;
    void  setPassword(const QString& password);

    bool  isAuthenticated()                 const;
    bool  isErrorState()                    const;
    const QList<YandexFotkiAlbum>& albums() const;
    const QList<YFPhoto>& photos()          const;

public:

    // Actions
    void getService();
    //void checkToken();
    void getSession();
    void getToken();

    void listAlbums();
    void listPhotos(const YandexFotkiAlbum& album);
    void updatePhoto(YFPhoto& photo, const YandexFotkiAlbum& album);
    void updateAlbum(YandexFotkiAlbum& album);

    void reset();
    void cancel();

public:

    // API-related public constants
    static const QString USERPAGE_URL;
    static const QString USERPAGE_DEFAULT_URL;

Q_SIGNALS:

    // emits than  than we are in a bad state
    void signalError();
    void signalGetSessionDone();
    void signalGetTokenDone();
    void signalGetServiceDone();

    void signalListAlbumsDone(const QList<YandexFotkiAlbum>&);
    void signalListPhotosDone(const QList<YFPhoto>&);
    void signalUpdatePhotoDone(YFPhoto& );
    void signalUpdateAlbumDone();

private Q_SLOTS:

    void slotFinished(QNetworkReply* reply);

    void slotParseResponseGetSession();
    //void slotParseResponseCheckToken();
    void slotParseResponseGetToken();
    void slotParseResponseGetService();
    void slotParseResponseListAlbums();

    void slotParseResponseListPhotos();
    bool slotParsePhotoXml(const QDomElement& entryElem,
                           YFPhoto& photo);

    void slotParseResponseUpdatePhotoFile();
    void slotParseResponseUpdatePhotoInfo();
    void slotParseResponseUpdateAlbum();

private:

    // for updatePhoto
    void updatePhotoFile(YFPhoto& photo);
    void updatePhotoInfo(YFPhoto& photo);

    // for updateAlbum
    void updateAlbumCreate(YandexFotkiAlbum& album);

    // return result from any place
    void setErrorState(State state);

    void listAlbumsNext(); // see listPhotos();

    // for photos pagination in listPhotos()
    void listPhotosNext(); // see listPhotos();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // YF_TALKER_H
