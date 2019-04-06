/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * Copyright (C) 2011      by Peter Potrowl <peter dot potrowl at gmail dot com>
 * Copyright (C) 2011      by Manuel Campomanes <campomanes dot manuel at gmail dot com>
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

#include "mediawiki_login.h"

// Qt includes

#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>
#include <QNetworkCookie>
#include <QNetworkReply>
#include <QNetworkRequest>

// Local includes

#include "mediawiki_iface.h"
#include "mediawiki_job_p.h"

namespace MediaWiki
{

class Q_DECL_HIDDEN LoginPrivate : public JobPrivate
{

public:

    LoginPrivate(Iface& MediaWiki, const QString& login, const QString& password)
        : JobPrivate(MediaWiki),
          login(login),
          password(password)
    {
    }

    static int error(const QString& error)
    {
        QStringList list;
        list << QStringLiteral("NoName")
             << QStringLiteral("Illegal")
             << QStringLiteral("NotExists")
             << QStringLiteral("EmptyPass")
             << QStringLiteral("WrongPass")
             << QStringLiteral("WrongPluginPass")
             << QStringLiteral("CreateBlocked")
             << QStringLiteral("Throttled")
             << QStringLiteral("Blocked")
             << QStringLiteral("NeedToken");

        int ret = list.indexOf(error);

        if (ret == -1)
        {
            ret = 0;
        }

        return (ret + (int)Login::LoginMissing);
    }

public:

    QUrl    baseUrl;
    QString login;
    QString password;
    QString lgsessionid;
    QString lgtoken;
};

Login::Login(Iface& MediaWiki, const QString& login, const QString& password, QObject* const parent)
    : Job(*new LoginPrivate(MediaWiki, login, password), parent)
{
}

Login::~Login()
{
}

void Login::start()
{
    QTimer::singleShot(0, this, SLOT(doWorkSendRequest()));
}

void Login::doWorkSendRequest()
{
    Q_D(Login);

    // Set the url
    QUrl url   = d->MediaWiki.url();
    d->baseUrl = url;

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("xml"));
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("login"));
    query.addQueryItem(QStringLiteral("lgname"), d->login);
    query.addQueryItem(QStringLiteral("lgpassword"), d->password);

    // Set the request
    QNetworkRequest request(url);
    request.setRawHeader(QByteArrayLiteral("User-Agent"), d->MediaWiki.userAgent().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    // Send the request
    d->reply = d->manager->post(request, query.toString().toUtf8());

    connect(d->reply, SIGNAL(finished()),
            this, SLOT(doWorkProcessReply()));
}

void Login::doWorkProcessReply()
{
    Q_D(Login);

    disconnect(d->reply, SIGNAL(finished()),
               this, SLOT(doWorkProcessReply()));

    if (d->reply->error() != QNetworkReply::NoError)
    {
        this->setError(Job::NetworkError);
        d->reply->close();
        d->reply->deleteLater();
        emitResult();
        return;
    }

    QXmlStreamReader reader(d->reply);

    while (!reader.atEnd() && !reader.hasError())
    {
        QXmlStreamReader::TokenType token = reader.readNext();

        if (token == QXmlStreamReader::StartElement)
        {
            QXmlStreamAttributes attrs = reader.attributes();

            if (reader.name() == QLatin1String("login"))
            {
                if (attrs.value(QStringLiteral("result")).toString() == QLatin1String("Success"))
                {
                    this->setError(Job::NoError);
                    d->lgtoken     = attrs.value(QStringLiteral("lgtoken")).toString();
                    d->lgsessionid = attrs.value(QStringLiteral("sessionid")).toString();

                    if (d->manager->cookieJar()->cookiesForUrl(d->MediaWiki.url()).isEmpty())
                    {
                        QList<QNetworkCookie> cookies;
                        QString prefix = attrs.value(QStringLiteral("cookieprefix")).toString();

                        QString prefixSession = prefix;
                        prefixSession.append(QStringLiteral("_session"));
                        QNetworkCookie cookie1(prefixSession.toUtf8(),attrs.value(QStringLiteral("sessionid")).toString().toUtf8());
                        cookies.append(cookie1);

                        QString prefixUserName = prefix;
                        prefixUserName.append(QStringLiteral("UserName"));
                        QNetworkCookie cookie2(prefixUserName.toUtf8(),attrs.value(QStringLiteral("lgusername")).toString().toUtf8());
                        cookies.append(cookie2);

                        QString prefixUserID = prefix;
                        prefixUserID.append(QStringLiteral("UserID"));
                        QNetworkCookie cookie3(prefixUserID.toUtf8(),attrs.value(QStringLiteral("lguserid")).toString().toUtf8());
                        cookies.append(cookie3);

                        QString prefixToken = prefix;
                        prefixToken.append(QStringLiteral("Token"));
                        QNetworkCookie cookie4(prefixToken.toUtf8(),attrs.value(QStringLiteral("lgtoken")).toString().toUtf8());
                        cookies.append(cookie4);

                        d->manager->cookieJar()->setCookiesFromUrl(cookies, d->MediaWiki.url());
                    }

                    d->reply->close();
                    d->reply->deleteLater();
                    emitResult();
                    return;
                }
                else if (attrs.value(QStringLiteral("result")).toString() == QLatin1String("NeedToken"))
                {
                    this->setError(Job::NoError);
                    d->lgtoken     = attrs.value(QStringLiteral("token")).toString();
                    d->lgsessionid = attrs.value(QStringLiteral("sessionid")).toString();

                    if (d->manager->cookieJar()->cookiesForUrl(d->MediaWiki.url()).isEmpty())
                    {
                        QString prefix = attrs.value(QStringLiteral("cookieprefix")).toString();
                        prefix.append(QStringLiteral("_session"));
                        QNetworkCookie cookie(prefix.toUtf8(),QString(d->lgsessionid).toUtf8());
                        QList<QNetworkCookie> cookies;
                        cookies.append(cookie);
                        d->manager->cookieJar()->setCookiesFromUrl(cookies, d->MediaWiki.url());
                    }
                }
                else if (attrs.value(QStringLiteral("result")).toString() == QLatin1String("WrongToken"))
                {
                    this->setError(LoginPrivate::error(attrs.value(QStringLiteral("result")).toString()));
                    d->reply->close();
                    d->reply->deleteLater();
                    emitResult();
                    return;
                }
                else if (attrs.value(QStringLiteral("result")).toString() == QLatin1String("Failed"))
                {
                    this->setError(LoginPrivate::error(attrs.value(QStringLiteral("result")).toString()));
                    d->reply->close();
                    d->reply->deleteLater();
                    emitResult();
                    return;
                }
            }
            else if (reader.name() == QLatin1String("error"))
            {
                this->setError(LoginPrivate::error(attrs.value(QStringLiteral("code")).toString()));
                d->reply->close();
                d->reply->deleteLater();
                emitResult();
                return;
            }
        }
        else if (token == QXmlStreamReader::Invalid && reader.error() != QXmlStreamReader::PrematureEndOfDocumentError)
        {
            this->setError(XmlError);
            d->reply->close();
            d->reply->deleteLater();
            emitResult();
            return;
        }
    }
    d->reply->close();
    d->reply->deleteLater();

    QUrl url = d->baseUrl;

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("xml"));
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("login"));
    query.addQueryItem(QStringLiteral("lgname"), d->login);
    query.addQueryItem(QStringLiteral("lgpassword"), d->password);
    query.addQueryItem(QStringLiteral("lgtoken"), (d->lgtoken).replace(QStringLiteral("+"), QStringLiteral("%2B")));

    // Set the request
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", d->MediaWiki.userAgent().toUtf8());
    request.setRawHeader("Cookie", d->manager->cookieJar()->cookiesForUrl(d->MediaWiki.url()).at(0).toRawForm());
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    // Send the request
    d->reply = d->manager->post(request, query.toString().toUtf8());
    connectReply();

    connect(d->reply, SIGNAL(finished()),
            this, SLOT(doWorkProcessReply()));
}

} // namespace MediaWiki
