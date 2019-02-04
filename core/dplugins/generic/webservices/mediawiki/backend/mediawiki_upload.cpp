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

#include "mediawiki_upload.h"

// Qt includes

#include <QFile>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QNetworkReply>
#include <QNetworkRequest>

// Local includes

#include "mediawiki_job_p.h"
#include "mediawiki_iface.h"
#include "mediawiki_queryinfo.h"

namespace MediaWiki
{

class Q_DECL_HIDDEN UploadPrivate : public JobPrivate
{
public:

    explicit UploadPrivate(Iface& MediaWiki)
        : JobPrivate(MediaWiki)
    {
        file = 0;
    }

    static int error(const QString& error)
    {
        QString temp = error;
        int ret      = 0;
        QStringList list;
        list    << QStringLiteral("internalerror")
                << QStringLiteral("uploaddisabled")
                << QStringLiteral("invalidsessionkey")
                << QStringLiteral("badaccessgroups")
                << QStringLiteral("missingparam")
                << QStringLiteral("mustbeloggedin")
                << QStringLiteral("fetchfileerror")
                << QStringLiteral("nomodule")
                << QStringLiteral("emptyfile")
                << QStringLiteral("filetypemissing")
                << QStringLiteral("filenametooshort")
                << QStringLiteral("overwrite")
                << QStringLiteral("stashfailed");

        ret = list.indexOf(temp.remove(QChar::fromLatin1('-')));

        if (ret == -1)
        {
            ret = 0;
        }

        return  ret + (int)Upload::InternalError ;
    }

    QIODevice* file;
    QString    filename;
    QString    comment;
    QString    text;
    QString    token;
};

Upload::Upload(Iface& MediaWiki, QObject* const parent)
    : Job(*new UploadPrivate(MediaWiki), parent)
{
}

Upload::~Upload()
{
}

void Upload::setFilename(const QString& param)
{
    Q_D(Upload);
    d->filename = param;
}

void Upload::setFile(QIODevice* const file)
{
    Q_D(Upload);
    d->file = file;
}

void Upload::setComment(const QString& param)
{
    Q_D(Upload);
    d->comment = param;
}

void Upload::setText(const QString& text)
{
    Q_D(Upload);
    d->text = text;
}

void Upload::start()
{
    Q_D(Upload);

    QueryInfo* const info = new QueryInfo(d->MediaWiki, this);
    info->setPageName(QStringLiteral("File:") + d->filename);
    info->setToken(QStringLiteral("edit"));

    connect(info, SIGNAL(page(Page)),
            this, SLOT(doWorkSendRequest(Page)));

    info->start();
}

void Upload::doWorkSendRequest(Page page)
{
    Q_D(Upload);

    QString token        = page.pageEditToken();
    d->token             = token;

    // Get the extension
    QStringList filename = d->filename.split(QChar::fromLatin1('.'));
    QString extension    = filename.at(filename.size()-1);

    if (extension == QLatin1String("jpg"))
        extension = QStringLiteral("jpeg");
    else if (extension == QLatin1String("svg"))
        extension += QStringLiteral("+xml");

    QUrl url = d->MediaWiki.url();
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("upload"));
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("xml"));
    url.setQuery(query);

    // Add the cookies
    QByteArray cookie = "";
    QList<QNetworkCookie> MediaWikiCookies = d->manager->cookieJar()->cookiesForUrl(d->MediaWiki.url());

    for (int i = 0 ; i < MediaWikiCookies.size() ; ++i)
    {
        cookie += MediaWikiCookies.at(i).toRawForm(QNetworkCookie::NameAndValueOnly);
        cookie += ';';
    }

    // Set the request
    QNetworkRequest request( url );
    request.setRawHeader("User-Agent", d->MediaWiki.userAgent().toUtf8());
    request.setRawHeader("Accept-Charset", "utf-8");

    QByteArray boundary = "-----------------------------15827188141577679942014851228";
    request.setRawHeader("Content-Type", "multipart/form-data; boundary="  + boundary);
    request.setRawHeader("Cookie", cookie );

    // send data
    boundary = "--" + boundary + "\r\n";
    QByteArray out = boundary;

    // ignore warnings
    out += "Content-Disposition: form-data; name=\"ignorewarnings\"\r\n\r\n";
    out += "true\r\n";
    out += boundary;

    // filename
    out += "Content-Disposition: form-data; name=\"filename\"\r\n\r\n";
    out += d->filename.toUtf8();
    out += "\r\n";
    out += boundary;

    // comment
    if (!d->comment.isEmpty())
    {
        out += "Content-Disposition: form-data; name=\"comment\"\r\n\r\n";
        out += d->comment.toUtf8();
        out += "\r\n";
        out += boundary;
    }

    // token
    out += "Content-Disposition: form-data; name=\"token\"\r\n\r\n";
    out += d->token.toUtf8();
    out += "\r\n";
    out += boundary;

    // the actual file
    out += "Content-Disposition: form-data; name=\"file\"; filename=\"";
    out += d->filename.toUtf8();
    out += "\"\r\n";
    out += "Content-Type: image/";
    out += extension.toUtf8();
    out += "\r\n\r\n";
    out += d->file->readAll();
    out += "\r\n";
    out += boundary;

    // description page
    out += "Content-Disposition: form-data; name=\"text\"\r\n";
    out += "Content-Type: text/plain\r\n\r\n";
    out += d->text.toUtf8();
    out += "\r\n";
    out += boundary.mid(0, boundary.length() - 2);
    out += "--\r\n";

    d->reply = d->manager->post( request, out );
    connectReply();

    connect( d->reply, SIGNAL(finished()),
             this, SLOT(doWorkProcessReply()) );
}

void Upload::doWorkProcessReply()
{
    Q_D(Upload);

    disconnect( d->reply, SIGNAL(finished()),
                this, SLOT(doWorkProcessReply()) );

    if ( d->reply->error() != QNetworkReply::NoError )
    {
        this->setError(this->NetworkError);
        d->reply->close();
        d->reply->deleteLater();
        emitResult();
        return;
    }

    QXmlStreamReader reader( d->reply );

    while ( !reader.atEnd() && !reader.hasError() )
    {
        QXmlStreamReader::TokenType token = reader.readNext();

        if ( token == QXmlStreamReader::StartElement )
        {
            QXmlStreamAttributes attrs = reader.attributes();

            if ( reader.name() == QLatin1String( "upload" ) )
            {
                if ( attrs.value(QStringLiteral("result")).toString() == QLatin1String("Success") )
                {
                    this->setError(KJob::NoError);
                }
            }
            else if ( reader.name() == QLatin1String( "error" ) )
            {
                this->setErrorText(attrs.value( QStringLiteral("info")).toString());
                this->setError(UploadPrivate::error(attrs.value(QStringLiteral("code")).toString()));
            }
        }
        else if ( token == QXmlStreamReader::Invalid && reader.error() != 
                  QXmlStreamReader::PrematureEndOfDocumentError)
        {
            this->setError(this->XmlError);
        }
    }

    d->reply->close();
    d->reply->deleteLater();
    emitResult();
}

} // namespace MediaWiki
