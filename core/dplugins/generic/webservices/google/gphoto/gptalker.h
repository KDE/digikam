/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-16-07
 * Description : a tool to export items to Google web services
 *
 * Copyright (C) 2007-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

#ifndef DIGIKAM_GP_TALKER_H
#define DIGIKAM_GP_TALKER_H

// Qt includes

#include <QUrl>
#include <QMap>
#include <QHash>
#include <QObject>
#include <QPointer>

// Local includes

#include "gsitem.h"
#include "gstalkerbase.h"

namespace Digikam
{

class GPTalker : public GSTalkerBase
{
    Q_OBJECT

public:

    explicit GPTalker(QWidget* const parent);
    ~GPTalker();

public:

    void    getLoggedInUser();
    
    void    listAlbums();
    void    listPhotos(const QString& albumId,
                       const QString& imgmax = QString());

    void    createAlbum(const GSFolder& newAlbum);

    bool    addPhoto(const QString& photoPath,
                     GSPhoto& info,
                     const QString& albumId,
                     bool rescale,
                     int maxDim,
                     int imageQuality);
    bool    updatePhoto(const QString& photoPath,
                        GSPhoto& info,
                        //const QString& albumId,
                        bool rescale,
                        int maxDim,
                        int imageQuality);
    
    void    getPhoto(const QString& imgPath);
    
    QStringList getUploadTokenList();

    void cancel();

Q_SIGNALS:

    void signalSetUserName(const QString& msg);
    void signalError(const QString& msg);
    void signalListAlbumsDone(int, const QString&, const QList <GSFolder>&);
    void signalListPhotosDone(int, const QString&, const QList <GSPhoto>&);
    void signalCreateAlbumDone(int, const QString&, const QString&);
    void signalAddPhotoDone(int, const QString&);
    void signalUploadPhotoDone(int, const QString&, const QStringList&);
    void signalGetPhotoDone(int errCode, const QString& errMsg,
                            const QByteArray& photoData);
    void signalReadyToUpload();

private:

    void parseResponseGetLoggedInUser(const QByteArray& data);
    void parseResponseListAlbums(const QByteArray& data);
    void parseResponseListPhotos(const QByteArray& data);
    void parseResponseCreateAlbum(const QByteArray& data);
    void parseResponseAddPhoto(const QByteArray& data);
    void parseResponseUploadPhoto(const QByteArray& data);

private Q_SLOTS:

    void slotError(const QString& msg);
    void slotFinished(QNetworkReply* reply);
    void slotUploadPhoto();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_GP_TALKER_H
