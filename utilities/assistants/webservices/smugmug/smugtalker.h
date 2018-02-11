/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-01
 * Description : a tool to export images to Smugmug web service
 *
 * Copyright (C) 2008-2009 by Luka Renko <lure at kubuntu dot org>
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

#ifndef SMUG_TALKER_H
#define SMUG_TALKER_H

// Qt includes

#include <QList>
#include <QString>
#include <QObject>

// local includes

#include "smugitem.h"
#include "dinfointerface.h"

class QNetworkReply;

namespace Digikam
{

class SmugTalker : public QObject
{
    Q_OBJECT

public:

    explicit SmugTalker(DInfoInterface* const iface, QWidget* const parent);
    ~SmugTalker();

public:

    SmugUser getUser() const;

    bool    loggedIn() const;
    void    cancel();
    void    login(const QString& email = QString(),
                  const QString& password = QString());
    void    logout();

    void    listAlbums(const QString& nickName = QString());
    void    listPhotos(qint64 albumID,
                       const QString& albumKey,
                       const QString& albumPassword = QString(),
                       const QString& sitePassword = QString());
    void    listAlbumTmpl();
    void    listCategories();
    void    listSubCategories(qint64 categoryID);

    void    createAlbum(const SmugAlbum& album);

    bool    addPhoto(const QString& imgPath,
                     qint64 albumID,
                     const QString& albumKey,
                     const QString& caption);

    void    getPhoto(const QString& imgPath);

Q_SIGNALS:

    void signalBusy(bool val);
    void signalLoginProgress(int step,
                             int maxStep = 0,
                             const QString& label = QString());
    void signalLoginDone(int errCode, const QString& errMsg);
    void signalAddPhotoDone(int errCode, const QString& errMsg);
    void signalGetPhotoDone(int errCode, const QString& errMsg,
                            const QByteArray& photoData);
    void signalCreateAlbumDone(int errCode, const QString& errMsg, qint64 newAlbumID,
                               const QString& newAlbumKey);
    void signalListAlbumsDone(int errCode, const QString& errMsg,
                              const QList <SmugAlbum>& albumsList);
    void signalListPhotosDone(int errCode, const QString& errMsg,
                              const QList <SmugPhoto>& photosList);
    void signalListAlbumTmplDone(int errCode, const QString& errMsg,
                                 const QList <SmugAlbumTmpl>& albumTList);
    void signalListCategoriesDone(int errCode, const QString& errMsg,
                                  const QList <SmugCategory>& categoriesList);
    void signalListSubCategoriesDone(int errCode, const QString& errMsg,
                                     const QList <SmugCategory>& categoriesList);

private:

    QString htmlToText(const QString& htmlText) const;
    QString errorToText(int errCode, const QString& errMsg) const;
    void parseResponseLogin(const QByteArray& data);
    void parseResponseLogout(const QByteArray& data);
    void parseResponseAddPhoto(const QByteArray& data);
    void parseResponseCreateAlbum(const QByteArray& data);
    void parseResponseListAlbums(const QByteArray& data);
    void parseResponseListPhotos(const QByteArray& data);
    void parseResponseListAlbumTmpl(const QByteArray& data);
    void parseResponseListCategories(const QByteArray& data);
    void parseResponseListSubCategories(const QByteArray& data);

private Q_SLOTS:

    void slotFinished(QNetworkReply* reply);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // SMUG_TALKER_H
