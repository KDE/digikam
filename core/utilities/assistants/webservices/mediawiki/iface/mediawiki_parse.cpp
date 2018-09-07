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
 * Copyright (C) 2011      by Vincent Garcia <xavier dot vincent dot garcia at gmail dot com>
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

#include "mediawiki_parse.h"

// Qt includes

#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>
#include <QNetworkReply>
#include <QNetworkRequest>

// Local includes

#include "mediawiki_job_p.h"
#include "mediawiki_iface.h"

namespace MediaWiki
{

class Q_DECL_HIDDEN ParsePrivate : public JobPrivate
{

public:

    explicit ParsePrivate(Iface& MediaWiki)
        : JobPrivate(MediaWiki)
    {
    }

    QMap<QString, QString> requestParameter;
};

Parse::Parse(Iface& MediaWiki, QObject* const parent)
    : Job(*new ParsePrivate(MediaWiki), parent)
{
}

Parse::~Parse()
{
}

void Parse::setText(const QString& param)
{
    Q_D(Parse);
    d->requestParameter[QStringLiteral("text")] = param;
}

void Parse::setTitle(const QString& param)
{
    Q_D(Parse);
    d->requestParameter[QStringLiteral("title")] = param;
}

void Parse::setPageName(const QString& param)
{
    Q_D(Parse);
    d->requestParameter[QStringLiteral("page")] = param;
}

void Parse::setUseLang(const QString& param)
{
    Q_D(Parse);
    d->requestParameter[QStringLiteral("uselang")] = param;
}

void Parse::start()
{
    QTimer::singleShot(0, this, SLOT(doWorkSendRequest()));
}

void Parse::doWorkSendRequest()
{
    Q_D(Parse);

    QUrl url = d->MediaWiki.url();
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("xml"));
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("parse"));

    QMapIterator<QString, QString> i(d->requestParameter);
    while (i.hasNext())
    {
        i.next();
        query.addQueryItem(i.key(), i.value());
    }
    url.setQuery(query);

    // Set the request
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", d->MediaWiki.userAgent().toUtf8());

    // Send the request
    d->reply = d->manager->get(request);
    connectReply();
    connect(d->reply, SIGNAL(finished()),
            this, SLOT(doWorkProcessReply()));
}

void Parse::doWorkProcessReply()
{
    Q_D(Parse);
    disconnect(d->reply, SIGNAL(finished()),
               this, SLOT(doWorkProcessReply()));

    if (d->reply->error() == QNetworkReply::NoError)
    {
        QXmlStreamReader reader(d->reply);
        QString text;

        while (!reader.atEnd() && !reader.hasError())
        {
            QXmlStreamReader::TokenType token = reader.readNext();

            if (token == QXmlStreamReader::StartElement)
            {
                if (reader.name() == QLatin1String("text"))
                {
                    text = reader.text().toString();
                    setError(Parse::NoError);
                }
                else if (reader.name() == QLatin1String("error"))
                {
                    if (reader.attributes().value(QStringLiteral("code")).toString() == QLatin1String("params"))
                        this->setError(this->TooManyParams);
                    else if (reader.attributes().value(QStringLiteral("code")).toString() == QLatin1String("missingtitle"))
                        this->setError(this->MissingPage);

                    d->reply->close();
                    d->reply->deleteLater();
                    emitResult();
                    return;
                }
            }
        }

        if (!reader.hasError())
        {
            emit result(text);
        }
        else
        {
            setError(Parse::XmlError);
        }
    }
    else
    {
        setError(Parse::NetworkError);
    }

    d->reply->close();
    d->reply->deleteLater();
    emitResult();
}

} // namespace MediaWiki
