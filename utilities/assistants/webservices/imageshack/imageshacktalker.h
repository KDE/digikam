/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * Copyright (C) 2012 Dodon Victor <dodonvictor at gmail dot com>
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

#ifndef IMAGESHACK_TALKER_H
#define IMAGESHACK_TALKER_H

// Qt includes

#include <QObject>
#include <QString>
#include <QMap>
#include <QNetworkReply>
#include <QNetworkAccessManager>

class QDomElement;
class QByteArray;

namespace Digikam
{

class ImageShack;

struct ImageShackImageInfo
{
};

class ImageShackTalker : public QObject
{
    Q_OBJECT

public:

    explicit ImageShackTalker(ImageShack* const imghack);
    ~ImageShackTalker();

    void authenticate();
    void cancelLogIn();
    void cancel();
    void getGalleries();

    void uploadItem(const QString& path, const QMap<QString, QString>& opts);
    void uploadItemToGallery(const QString& path, const QString& gallery, const QMap<QString, QString>& opts);

Q_SIGNALS:

    void signalBusy(bool busy);
    void signalJobInProgress(int step, int maxStep = 0, const QString& label = QString());
    void signalLoginDone(int errCode,  const QString &errMsg);
    void signalGetGalleriesDone(int errCode, const QString &errMsg);

    void signalAddPhotoDone(int errCode, const QString& errMsg);
    void signalUpdateGalleries(const QStringList& gTexts, const QStringList& gNames);

private:

    enum State
    {
        IMGHCK_AUTHENTICATING,
        IMGHCK_DONOTHING,
        IMGHCK_GETGALLERIES,
        IMGHCK_ADDPHOTO,
        IMGHCK_ADDVIDEO,
        IMGHCK_ADDPHOTOGALLERY
    };

private Q_SLOTS:

    void slotFinished(QNetworkReply* reply);

private:

    QString getCallString(QMap<QString, QString>& args);
    void checkRegistrationCodeDone(int errCode, const QString& errMsg);
    void parseAccessToken(const QByteArray& data);
    void parseGetGalleries(const QByteArray& data);
    void authenticationDone(int errCode, const QString& errMsg);

    void logOut();

    int parseErrorResponse(QDomElement elem, QString& errMsg);

    void parseUploadPhotoDone(QByteArray data);
    void parseAddPhotoToGalleryDone(QByteArray data);

    QString mimeType(const QString& path);

private:

    ImageShack*            m_imageshack;

    QByteArray             m_buffer;

    QString                m_userAgent;
    QUrl                   m_photoApiUrl;
    QUrl                   m_videoApiUrl;
    QUrl                   m_loginApiUrl;
    QUrl                   m_galleryUrl;
    QString                m_appKey;

    bool                   m_loginInProgress;

    QNetworkAccessManager* m_netMngr;

    QNetworkReply*         m_reply;

    State                  m_state;
};

} // namespace Digikam

#endif // IMAGESHACK_TALKER_H
