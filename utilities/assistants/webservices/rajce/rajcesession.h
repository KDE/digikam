/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * Copyright (C) 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef RAJCE_SESSION_H
#define RAJCE_SESSION_H

// Qt includes

#include <QString>
#include <QVector>

// Local includes

#include "rajcealbum.h"

namespace Digikam
{

enum RajceCommandType
{
    Login = 0,
    Logout,
    ListAlbums,
    CreateAlbum,
    OpenAlbum,
    CloseAlbum,
    AddPhoto
};

enum RajceErrorCode
{
/** Taken from the semi-official documentation:
 *  https://docs.google.com/View?id=ajkd99k8zcw6_120ctqvnjd5#Chybov_k_dy
 */
    UnknownError = 1,               //  1 Unknown error.
    InvalidCommand,                 //  2 Invalid command.
    InvalidCredentials,             //  3 Invalid credentials.
    InvalidSessionToken,            //  4 Invalid session token.
    InvalidOrRepeatedColumnName,    //  5 Unknown or repeated column name {colName}.
    InvalidAlbumId,                 //  6 Invalid album ID.
    AlbumDoesntExistOrNoPrivileges, //  7 The album doesn't exist or is not owned by the logged in user.
    InvalidAlbumToken,              //  8 Invalid album token.
    AlbumNameEmpty,                 //  9 Album can't have an empty name.
    FailedToCreateAlbum,            // 10 Failed to create an album (probably a serverside error).
    AlbumDoesntExist,               // 11 Album doesn't exist.
    UnknownApplication,             // 12 Nonexistent application.
    InvalidApplicationKey,          // 13 Invalid application key.
    FileNotAttached,                // 14 A file is not attached.
    NewerVersionExists,             // 15 A newer version already exists {version}.
    SavingFileFailed,               // 16 Failed to save the file.
    UnsupportedFileExtension,       // 17 Unsupported file extension {extension}.
    UnknownClientVersion,           // 18 Unknown client version.
    NonexistentTarget               // 19 Unknown target.
};

class RajceSession
{
public:

    explicit RajceSession();
    ~RajceSession();

    /** Copy constructor.
     */
    RajceSession(const RajceSession& other);

    /** Create a copy of container
     */
    RajceSession& operator=(const RajceSession& other);

public:

    QString& sessionToken();
    QString  const& sessionToken()      const;
    QString& nickname();
    QString  const& nickname()          const;
    QString& username();
    QString  const& username()          const;
    QString& openAlbumToken();
    QString  const& openAlbumToken()    const;
    QString& lastErrorMessage();
    QString  const& lastErrorMessage()  const;

    unsigned& maxWidth();
    unsigned  maxWidth()                const;
    unsigned& maxHeight();
    unsigned  maxHeight()               const;
    unsigned& imageQuality();
    unsigned  imageQuality()            const;
    unsigned& lastErrorCode();
    unsigned  lastErrorCode()           const;

    QVector<RajceAlbum>& albums();
    const QVector<RajceAlbum>& albums() const;

    RajceCommandType& lastCommand();
    RajceCommandType  lastCommand()     const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

QDebug operator<<(QDebug d, const Digikam::RajceSession& s);

#endif // RAJCE_SESSION_H
