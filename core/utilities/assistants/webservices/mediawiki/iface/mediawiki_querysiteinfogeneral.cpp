/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "mediawiki_querysiteinfogeneral.h"

// Qt includes

#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

// Local includes

#include "mediawiki_iface.h"
#include "mediawiki_job_p.h"

namespace MediaWiki
{

class Q_DECL_HIDDEN QuerySiteInfoGeneralPrivate : public JobPrivate
{
public:

    explicit QuerySiteInfoGeneralPrivate(Iface& MediaWiki)
        : JobPrivate(MediaWiki)
    {
    }
};

QuerySiteInfoGeneral::QuerySiteInfoGeneral(Iface& MediaWiki, QObject* const /*parent*/)
    : Job(*new QuerySiteInfoGeneralPrivate(MediaWiki))
{
}

QuerySiteInfoGeneral::~QuerySiteInfoGeneral()
{
}

void QuerySiteInfoGeneral::start()
{
    QTimer::singleShot(0, this, SLOT(doWorkSendRequest()));
}

void QuerySiteInfoGeneral::doWorkSendRequest()
{
    Q_D(QuerySiteInfoGeneral);

    // Set the url
    QUrl url = d->MediaWiki.url();
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("xml"));
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("query"));
    query.addQueryItem(QStringLiteral("meta"),   QStringLiteral("siteinfo"));
    query.addQueryItem(QStringLiteral("siprop"), QStringLiteral("general"));
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

void QuerySiteInfoGeneral::doWorkProcessReply()
{
    Q_D(QuerySiteInfoGeneral);

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

    Generalinfo generalinfo;
    QXmlStreamReader reader(d->reply);

    while(!reader.atEnd() && !reader.hasError())
    {
        QXmlStreamReader::TokenType token = reader.readNext();

        if(token == QXmlStreamReader::StartElement)
        {
            if(reader.name() == QLatin1String("general"))
            {
                generalinfo.setMainPage(reader.attributes().value(QStringLiteral("mainpage")).toString());
                generalinfo.setUrl(QUrl::fromEncoded(reader.attributes().value(QStringLiteral("base")).toString().toLocal8Bit()));
                generalinfo.setSiteName(reader.attributes().value(QStringLiteral("sitename")).toString());
                generalinfo.setGenerator(reader.attributes().value(QStringLiteral("generator")).toString());
                generalinfo.setPhpVersion(reader.attributes().value(QStringLiteral("phpversion")).toString());
                generalinfo.setPhpApi(reader.attributes().value(QStringLiteral("phpsapi")).toString());
                generalinfo.setDataBaseType(reader.attributes().value(QStringLiteral("dbtype")).toString());
                generalinfo.setDataBaseVersion(reader.attributes().value(QStringLiteral("dbversion")).toString());
                generalinfo.setRev(reader.attributes().value(QStringLiteral("rev")).toString());
                generalinfo.setCas(reader.attributes().value(QStringLiteral("case")).toString());
                generalinfo.setLicense(reader.attributes().value(QStringLiteral("rights")).toString());
                generalinfo.setLanguage(reader.attributes().value(QStringLiteral("lang")).toString());
                generalinfo.setFallBack8bitEncoding(reader.attributes().value(QStringLiteral("fallback8bitEncoding")).toString());
                generalinfo.setWriteApi(reader.attributes().value(QStringLiteral("writeapi")).toString());
                generalinfo.setTimeZone(reader.attributes().value(QStringLiteral("timezone")).toString());
                generalinfo.setTimeOffset(reader.attributes().value(QStringLiteral("timeoffset")).toString());
                generalinfo.setArticlePath(reader.attributes().value(QStringLiteral("articlepath")).toString());
                generalinfo.setScriptPath(reader.attributes().value(QStringLiteral("scriptpath")).toString());
                generalinfo.setScript(reader.attributes().value(QStringLiteral("script")).toString());
                generalinfo.setVariantArticlePath(reader.attributes().value(QStringLiteral("variantarticlepath")).toString());
                generalinfo.setServerUrl(QUrl::fromEncoded(reader.attributes().value(QStringLiteral("server")).toString().toLocal8Bit()));
                generalinfo.setWikiId(reader.attributes().value(QStringLiteral("wikiid")).toString());
                generalinfo.setTime(QDateTime::fromString(reader.attributes().value(QStringLiteral("time")).toString(), QStringLiteral("yyyy-MM-dd'T'hh:mm:ss'Z'")));
            }
            else if(reader.name() == QLatin1String("error"))
            {
                this->setError(QuerySiteInfoGeneral::IncludeAllDenied);
                d->reply->close();
                d->reply->deleteLater();
                emitResult();
                return;
            }
        }
    }

    if (reader.hasError())
    {
        this->setError(Job::XmlError);
    }
    else
    {
        emit result(generalinfo);
        this->setError(Job::NoError);
    }

    d->reply->close();
    d->reply->deleteLater();
    emitResult();
}

} // namespace MediaWiki
