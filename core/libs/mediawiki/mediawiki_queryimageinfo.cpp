/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Ludovic Delfau <ludovicdelfau at gmail dot com>
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

#include "mediawiki_queryimageinfo.h"

// Qt includes

#include <QString>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>
#include <QNetworkReply>
#include <QNetworkRequest>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "mediawiki_job_p.h"
#include "mediawiki_iface.h"
#include "mediawiki_imageinfo.h"

namespace MediaWiki
{

class Q_DECL_HIDDEN QueryImageinfoPrivate : public JobPrivate
{
public:

    explicit QueryImageinfoPrivate(Iface& MediaWiki)
        : JobPrivate(MediaWiki)
    {
        onlyOneSignal = false;
    }

    static inline qint64 toQInt64(const QString& qstring)
    {
        bool   ok;
        qint64 result = qstring.toLongLong(&ok);
        return (ok ? result : -1);
    }

    static inline void addQueryItemIfNotNull(QUrlQuery& query, const QString& key, const QString& value)
    {
        if (!value.isNull())
        {
            query.addQueryItem(key, value);
        }
    }

public:

    bool    onlyOneSignal;

    QString title;
    QString iiprop;
    QString limit;
    QString begin;
    QString end;
    QString width;
    QString height;
};

QueryImageinfo::QueryImageinfo(Iface& MediaWiki, QObject* const parent)
    : Job(*new QueryImageinfoPrivate(MediaWiki), parent)
{
    Q_D(QueryImageinfo);
    Q_UNUSED(d);
}

QueryImageinfo::~QueryImageinfo()
{
}

void QueryImageinfo::setTitle(const QString& title)
{
    Q_D(QueryImageinfo);
    d->title = title;
}

void QueryImageinfo::setProperties(Properties properties)
{
    Q_D(QueryImageinfo);

    QString iiprop;

    if (properties & QueryImageinfo::Timestamp)
    {
        iiprop.append(QStringLiteral("timestamp|"));
    }
    if (properties & QueryImageinfo::User)
    {
        iiprop.append(QStringLiteral("user|"));
    }
    if (properties & QueryImageinfo::Comment)
    {
        iiprop.append(QStringLiteral("comment|"));
    }
    if (properties & QueryImageinfo::Url)
    {
        iiprop.append(QStringLiteral("url|"));
    }
    if (properties & QueryImageinfo::Size)
    {
        iiprop.append(QStringLiteral("size|"));
    }
    if (properties & QueryImageinfo::Sha1)
    {
        iiprop.append(QStringLiteral("sha1|"));
    }
    if (properties & QueryImageinfo::Mime)
    {
        iiprop.append(QStringLiteral("mime|"));
    }
    if (properties & QueryImageinfo::Metadata)
    {
        iiprop.append(QStringLiteral("metadata|"));
    }

    iiprop.chop(1);
    d->iiprop = iiprop;
}

void QueryImageinfo::setLimit(unsigned int limit)
{
    Q_D(QueryImageinfo);
    d->limit = (limit > 0u) ? QString::number(limit) : QString();
}

void QueryImageinfo::setOnlyOneSignal(bool onlyOneSignal)
{
    Q_D(QueryImageinfo);
    d->onlyOneSignal = onlyOneSignal;
}

void QueryImageinfo::setBeginTimestamp(const QDateTime& begin)
{
    Q_D(QueryImageinfo);
    d->begin = begin.toString(QStringLiteral("yyyy-MM-dd'T'hh:mm:ss'Z'"));
}

void QueryImageinfo::setEndTimestamp(const QDateTime& end)
{
    Q_D(QueryImageinfo);
    d->end = end.toString(QStringLiteral("yyyy-MM-dd'T'hh:mm:ss'Z'"));
}

void QueryImageinfo::setWidthScale(unsigned int width)
{
    Q_D(QueryImageinfo);
    d->width = (width > 0u) ? QString::number(width) : QString();
}

void QueryImageinfo::setHeightScale(unsigned int height)
{
    Q_D(QueryImageinfo);
    d->height = (height > 0u) ? QString::number(height) : QString();

    if (d->width.isNull())
    {
        d->width = d->height;
    }
}

void QueryImageinfo::start()
{
    QTimer::singleShot(0, this, SLOT(doWorkSendRequest()));
}

void QueryImageinfo::doWorkSendRequest()
{
    Q_D(QueryImageinfo);

    // Requirements.
    if (d->title.isEmpty())
    {
        setError(QueryImageinfo::MissingMandatoryParameter);
        setErrorText(i18n("You cannot query the information of an "
                          "image if you do not provide the title of that image."));
        emitResult();
        return;
    }

    // Set the url
    QUrl url = d->MediaWiki.url();
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("xml"));
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("query"));
    query.addQueryItem(QStringLiteral("titles"), d->title);
    query.addQueryItem(QStringLiteral("prop"),   QStringLiteral("imageinfo"));
    QueryImageinfoPrivate::addQueryItemIfNotNull(query, QStringLiteral("iiprop"),      d->iiprop);
    QueryImageinfoPrivate::addQueryItemIfNotNull(query, QStringLiteral("iilimit"),     d->limit);
    QueryImageinfoPrivate::addQueryItemIfNotNull(query, QStringLiteral("iistart"),     d->begin);
    QueryImageinfoPrivate::addQueryItemIfNotNull(query, QStringLiteral("iiend"),       d->end);
    QueryImageinfoPrivate::addQueryItemIfNotNull(query, QStringLiteral("iiurlwidth"),  d->width);
    QueryImageinfoPrivate::addQueryItemIfNotNull(query, QStringLiteral("iiurlheight"), d->height);
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

void QueryImageinfo::doWorkProcessReply()
{
    Q_D(QueryImageinfo);

    disconnect(d->reply, SIGNAL(finished()),
               this, SLOT(doWorkProcessReply()));

    d->begin.clear();

    if (d->reply->error() == QNetworkReply::NoError)
    {
        QXmlStreamReader reader(d->reply);
        QList<Imageinfo> imageinfos;
        Imageinfo imageinfo;

        while (!reader.atEnd() && !reader.hasError())
        {
            QXmlStreamReader::TokenType token = reader.readNext();

            if (token == QXmlStreamReader::StartElement)
            {
                if (reader.name() == QLatin1String("imageinfo"))
                {
                    if (!reader.attributes().value(QStringLiteral("iistart")).isNull())
                    {
                        d->begin = reader.attributes().value(QStringLiteral("iistart")).toString();
                    }
                }
                else if (reader.name() == QLatin1String("ii"))
                {
                    imageinfo.setTimestamp(QDateTime::fromString(reader.attributes().value(QStringLiteral("timestamp")).toString(), QStringLiteral("yyyy-MM-dd'T'hh:mm:ss'Z'")));
                    imageinfo.setUser(reader.attributes().value(QStringLiteral("user")).toString());
                    imageinfo.setComment(reader.attributes().value(QStringLiteral("comment")).toString());
                    imageinfo.setUrl(QUrl::fromEncoded(reader.attributes().value(QStringLiteral("url")).toString().toLocal8Bit()));
                    imageinfo.setDescriptionUrl(QUrl::fromEncoded(reader.attributes().value(QStringLiteral("descriptionurl")).toString().toLocal8Bit()));
                    imageinfo.setThumbUrl(QUrl::fromEncoded(reader.attributes().value(QStringLiteral("thumburl")).toString().toLocal8Bit()));
                    imageinfo.setThumbWidth(QueryImageinfoPrivate::toQInt64(reader.attributes().value(QStringLiteral("thumbwidth")).toString()));
                    imageinfo.setThumbHeight(QueryImageinfoPrivate::toQInt64(reader.attributes().value(QStringLiteral("thumbheight")).toString()));
                    imageinfo.setSize(QueryImageinfoPrivate::toQInt64(reader.attributes().value(QStringLiteral("size")).toString()));
                    imageinfo.setWidth(QueryImageinfoPrivate::toQInt64(reader.attributes().value(QStringLiteral("width")).toString()));
                    imageinfo.setHeight(QueryImageinfoPrivate::toQInt64(reader.attributes().value(QStringLiteral("height")).toString()));
                    imageinfo.setSha1(reader.attributes().value(QStringLiteral("sha1")).toString());
                    imageinfo.setMime(reader.attributes().value(QStringLiteral("mime")).toString());
                }
                else if (reader.name() == QLatin1String("metadata"))
                {
                    if (!reader.attributes().isEmpty())
                    {
                        imageinfo.metadata()[reader.attributes().value(QStringLiteral("name")).toString()] = reader.attributes().value(QStringLiteral("value")).toString();
                    }
                }
            }
            else if (token == QXmlStreamReader::EndElement)
            {
                if (reader.name() == QLatin1String("ii"))
                {
                    imageinfos.push_back(imageinfo);
                    imageinfo = Imageinfo();
                }
            }
        }

        if (!reader.hasError())
        {
            emit result(imageinfos);

            if (d->begin.isNull() || d->onlyOneSignal)
            {
                setError(KJob::NoError);
            }
            else
            {
                QTimer::singleShot(0, this, SLOT(doWorkSendRequest()));
                return;
            }
        }
        else
        {
            setError(QueryImageinfo::XmlError);
        }
    }
    else
    {
        setError(QueryImageinfo::NetworkError);
    }

    emitResult();
}

} // namespace MediaWiki
