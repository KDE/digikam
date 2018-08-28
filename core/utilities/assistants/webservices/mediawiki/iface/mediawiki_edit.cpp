/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * Copyright (C) 2011      by Hormiere Guillaume <hormiere dot guillaume at gmail dot com>
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

#include "mediawiki_edit.h"

// Qt includes

#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>
#include <QCryptographicHash>
#include <QStringList>

#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkRequest>

// Local includes

#include "mediawiki_iface.h"
#include "mediawiki_queryinfo.h"
#include "mediawiki_job_p.h"

namespace MediaWiki
{

class Result
{
public:

    unsigned int m_captchaId;
    QVariant     m_captchaQuestion;
    QString      m_captchaAnswer;
};

class EditPrivate : public JobPrivate
{
public:

    EditPrivate(Iface& MediaWiki)
        : JobPrivate(MediaWiki)
    {
    }

    static int error(const QString& error)
    {
        QString temp = error;
        int ret      = 0;
        QStringList list;
        list    << QStringLiteral("notext")
                << QStringLiteral("invalidsection")
                << QStringLiteral("protectedtitle")
                << QStringLiteral("cantcreate")
                << QStringLiteral("cantcreateanon")
                << QStringLiteral("articleexists")
                << QStringLiteral("noimageredirectanon")
                << QStringLiteral("noimageredirect")
                << QStringLiteral("spamdetected")
                << QStringLiteral("filtered")
                << QStringLiteral("contenttoobig")
                << QStringLiteral("noeditanon")
                << QStringLiteral("noedit")
                << QStringLiteral("pagedeleted")
                << QStringLiteral("emptypage")
                << QStringLiteral("emptynewsection")
                << QStringLiteral("editconflict")
                << QStringLiteral("revwrongpage")
                << QStringLiteral("undofailure");

        ret = list.indexOf(temp.remove(QChar::fromLatin1('-')));

        if (ret == -1)
        {
            ret = 0;
        }

        return  ret + (int)Edit::TextMissing ;
    }

    QUrl                   baseUrl;
    QMap<QString, QString> requestParameter;
    Result                 result;
};

Edit::Edit(Iface& media, QObject* parent)
    : Job(*new EditPrivate(media), parent)
{
}

void Edit::setUndoAfter(int undoafter)
{
    Q_D(Edit);
    d->requestParameter[QStringLiteral("undoafter")] = QString::number(undoafter);
}

void Edit::setUndo(int undo)
{
    Q_D(Edit);
    d->requestParameter[QStringLiteral("undo")] = QString::number(undo);
}

void Edit::setPrependText(const QString& prependText)
{
    Q_D(Edit);
    d->requestParameter[QStringLiteral("prependtext")] = prependText;
    d->requestParameter[QStringLiteral("md5")] = QString();
}

void Edit::setAppendText(const QString& appendText)
{
    Q_D(Edit);
    d->requestParameter[QStringLiteral("appendtext")] = appendText;
    d->requestParameter[QStringLiteral("md5")] = QString();
}

void Edit::setPageName(const QString& pageName)
{
    Q_D(Edit);
    d->requestParameter[QStringLiteral("title")] = pageName;
}

void Edit::setToken(const QString& token)
{
    Q_D(Edit);
    d->requestParameter[QStringLiteral("token")] = token;
}

void Edit::setBaseTimestamp(const QDateTime& baseTimestamp)
{
    Q_D(Edit);
    d->requestParameter[QStringLiteral("basetimestamp")] = baseTimestamp.toString(QStringLiteral("yyyy-MM-ddThh:mm:ssZ"));
}

void Edit::setStartTimestamp(const QDateTime& startTimestamp)
{
    Q_D(Edit);
    d->requestParameter[QStringLiteral("starttimestamp")] = startTimestamp.toString(QStringLiteral("yyyy-MM-ddThh:mm:ssZ"));
}

void Edit::setText(const QString& text)
{
    Q_D(Edit);
    d->requestParameter[QStringLiteral("text")] = text;
    d->requestParameter[QStringLiteral("md5")]  = QString();
}

void Edit::setRecreate(bool recreate)
{
    Q_D(Edit);

    if (recreate)
    {
        d->requestParameter[QStringLiteral("recreate")] = QStringLiteral("on");
        d->requestParameter[QStringLiteral("md5")]      = QString();
    }
}

void Edit::setCreateonly(bool createonly)
{
    Q_D(Edit);

    if (createonly)
    {
        d->requestParameter[QStringLiteral("createonly")] = QStringLiteral("on");
        d->requestParameter[QStringLiteral("md5")]        = QString();
    }
}

void Edit::setNocreate(bool norecreate)
{
    Q_D(Edit);

    if (norecreate)
    {
        d->requestParameter[QStringLiteral("nocreate")] = QStringLiteral("on");
        d->requestParameter[QStringLiteral("md5")]      = QString();
    }
}

void Edit::setMinor(bool minor)
{
    Q_D(Edit);

    if (minor)
        d->requestParameter[QStringLiteral("minor")] = QStringLiteral("on");
    else
        d->requestParameter[QStringLiteral("notminor")] = QStringLiteral("on");
}

void Edit::setSection(const QString& section)
{
    Q_D(Edit);
    d->requestParameter[QStringLiteral("section")] = section;
}

void Edit::setSummary(const QString& summary)
{
    Q_D(Edit);
    d->requestParameter[QStringLiteral("summary")] = summary;
}

void Edit::setWatchList(Edit::Watchlist watchlist)
{
    Q_D(Edit);

    switch (watchlist)
    {
        case Edit::watch:
            d->requestParameter[QStringLiteral("watchlist")] = QString(QStringLiteral("watch"));
            break;
        case Edit::unwatch:
            d->requestParameter[QStringLiteral("watchlist")] = QString(QStringLiteral("unwatch"));
            break;
        case Edit::nochange:
            d->requestParameter[QStringLiteral("watchlist")] = QString(QStringLiteral("nochange"));
            break;
        case Edit::preferences:
            d->requestParameter[QStringLiteral("watchlist")] = QString(QStringLiteral("preferences"));
            break;
    }
}

Edit::~Edit()
{
}

void Edit::start()
{
    Q_D(Edit);
    QueryInfo* const info = new QueryInfo(d->MediaWiki,this);
    info->setPageName(d->requestParameter[QStringLiteral("title")]);
    info->setToken(QStringLiteral("edit"));

    connect(info, SIGNAL(page(Page)),
            this, SLOT(doWorkSendRequest(Page)));

    info->start();
}

void Edit::doWorkSendRequest(Page page)
{
    Q_D(Edit);
    d->requestParameter[QStringLiteral("token")] = page.pageEditToken();
    // Set the url
    QUrl    url = d->MediaWiki.url();
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("xml"));
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("edit"));

    // Add params
    if (d->requestParameter.contains(QStringLiteral("md5")))
    {
        QString text;

        if (d->requestParameter.contains(QStringLiteral("prependtext")))
            text += d->requestParameter[QStringLiteral("prependtext")];

        if (d->requestParameter.contains(QStringLiteral("appendtext")))
            text += d->requestParameter[QStringLiteral("appendtext")];

        if (d->requestParameter.contains(QStringLiteral("text")))
            text = d->requestParameter[QStringLiteral("text")];

        QByteArray hash = QCryptographicHash::hash(text.toUtf8(),QCryptographicHash::Md5);
        d->requestParameter[QStringLiteral("md5")] = QString::fromLatin1(hash.toHex());
    }

    QMapIterator<QString, QString> i(d->requestParameter);

    while (i.hasNext())
    {
        i.next();

        if (i.key() != QStringLiteral("token"))
            query.addQueryItem(i.key(),i.value());
    }

    QByteArray cookie;
    QList<QNetworkCookie> MediaWikiCookies = d->manager->cookieJar()->cookiesForUrl(d->MediaWiki.url());

    for(int i = 0 ; i < MediaWikiCookies.size(); ++i)
    {
        cookie += MediaWikiCookies.at(i).toRawForm(QNetworkCookie::NameAndValueOnly);
        cookie += ';';
    }

    // Add the token
    query.addQueryItem(QStringLiteral("token"), d->requestParameter[QStringLiteral("token")]);
    url.setQuery(query);
    d->baseUrl = url;

    // Set the request
    QNetworkRequest request( url );
    request.setRawHeader("User-Agent", d->MediaWiki.userAgent().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    request.setRawHeader( "Cookie", cookie );

    setPercent(25); // Request ready.

    // Send the request
    d->reply = d->manager->post( request, url.toString().toUtf8() );
    connectReply();

    connect( d->reply, SIGNAL(finished()),
             this, SLOT(finishedEdit()) );

    setPercent(50); // Request sent.
}

void Edit::finishedEdit()
{
    Q_D(Edit);

    disconnect(d->reply, SIGNAL(finished()),
               this, SLOT(finishedEdit()));

    setPercent(75); // Response received.

    if (d->reply->error() != QNetworkReply::NoError)
    {
        this->setError(this->NetworkError);
        d->reply->close();
        d->reply->deleteLater();
        emitResult();
        return;
    }

    QXmlStreamReader reader( d->reply );

    while (!reader.atEnd() && !reader.hasError())
    {
        QXmlStreamReader::TokenType token = reader.readNext();

        if (token == QXmlStreamReader::StartElement)
        {
            QXmlStreamAttributes attrs = reader.attributes();

            if (reader.name() == QStringLiteral("edit"))
            {
                if (attrs.value( QStringLiteral("result") ).toString() == QLatin1String("Success"))
                {
                    setPercent(100); // Response parsed successfully.
                    this->setError(KJob::NoError);
                    d->reply->close();
                    d->reply->deleteLater();
                    emitResult();
                    return;
                }
                else if (attrs.value( QStringLiteral("result") ).toString() == QLatin1String("Failure"))
                {
                    this->setError(KJob::NoError);
                    reader.readNext();
                    attrs = reader.attributes();
                    d->result.m_captchaId = attrs.value( QStringLiteral("id") ).toString().toUInt();

                    if (!attrs.value( QStringLiteral("question") ).isEmpty())
                        d->result.m_captchaQuestion = QVariant(attrs.value( QStringLiteral("question") ).toString()) ;
                    else if (!attrs.value( QStringLiteral("url") ).isEmpty())
                        d->result.m_captchaQuestion = QVariant(attrs.value( QStringLiteral("url") ).toString()) ;
                }
            }
            else if (reader.name() == QStringLiteral("error"))
            {
                this->setError(EditPrivate::error(attrs.value( QStringLiteral("code") ).toString()));
                d->reply->close();
                d->reply->deleteLater();
                emitResult();
                return;
            }
        }
        else if (token == QXmlStreamReader::Invalid && reader.error() != QXmlStreamReader::PrematureEndOfDocumentError)
        {
            this->setError(this->XmlError);
            d->reply->close();
            d->reply->deleteLater();
            emitResult();
            return;
        }
    }

    d->reply->close();
    d->reply->deleteLater();
    emit resultCaptcha(d->result.m_captchaQuestion);
}

void Edit::finishedCaptcha(const QString& captcha)
{
    Q_D(Edit);
    d->result.m_captchaAnswer = captcha;
    QUrl url                  = d->baseUrl;
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("CaptchaId"), QString::number(d->result.m_captchaId));
    query.addQueryItem(QStringLiteral("CaptchaAnswer"), d->result.m_captchaAnswer);
    url.setQuery(query);
    QString data      = url.toString();
    QByteArray cookie;
    QList<QNetworkCookie> MediaWikiCookies = d->manager->cookieJar()->cookiesForUrl(d->MediaWiki.url());

    for(int i = 0 ; i < MediaWikiCookies.size() ; ++i)
    {
        cookie += MediaWikiCookies.at(i).toRawForm(QNetworkCookie::NameAndValueOnly);
        cookie += ';';
    }

    // Set the request
    QNetworkRequest request( url );
    request.setRawHeader("User-Agent", d->MediaWiki.userAgent().toUtf8());
    request.setRawHeader( "Cookie", cookie );
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    // Send the request
    d->reply = d->manager->post(request, data.toUtf8());

    connect( d->reply, SIGNAL(finished()),
             this, SLOT(finishedEdit()) );
}

} // namespace MediaWiki
