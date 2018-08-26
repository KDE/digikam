/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a MediaWiki C++ interface
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

#include "mediawiki_queryrevision.h"

// Qt includes

#include <QDateTime>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

// Local includes

#include "mediawiki_mediawiki.h"
#include "mediawiki_job_p.h"

namespace mediawiki
{

class QueryRevisionPrivate : public JobPrivate
{

public:

    QueryRevisionPrivate(MediaWiki& mediawiki)
         : JobPrivate(mediawiki)
    {
    }

    QMap<QString, QString> requestParameter;
};

QueryRevision::QueryRevision(MediaWiki& mediawiki, QObject* const parent)
    : Job(*new QueryRevisionPrivate(mediawiki), parent)
{
}

QueryRevision::~QueryRevision()
{
}

void QueryRevision::start()
{
    QTimer::singleShot(0, this, SLOT(doWorkSendRequest()));
}

void QueryRevision::setPageName(const QString& pageName)
{
    Q_D(QueryRevision);
    d->requestParameter[QStringLiteral("titles")] = pageName;
}

void QueryRevision::setProperties(Properties properties)
{
    Q_D(QueryRevision);
    QString buff;

    if(properties & QueryRevision::Ids)
    {
        buff.append(QStringLiteral("ids"));
    }

    if(properties & QueryRevision::Flags)
    {
        if (buff.length())
            buff.append(QStringLiteral("|"));

        buff.append(QStringLiteral("flags"));
    }

    if(properties & QueryRevision::Timestamp)
    {
        if (buff.length())
            buff.append(QStringLiteral("|"));

        buff.append(QStringLiteral("timestamp"));
    }

    if(properties & QueryRevision::User)
    {
        if (buff.length())
            buff.append(QStringLiteral("|"));

        buff.append(QStringLiteral("user"));
    }

    if(properties & QueryRevision::Comment)
    {
        if (buff.length())
            buff.append(QStringLiteral("|"));

        buff.append(QStringLiteral("comment"));
    }

    if(properties & QueryRevision::Size)
    {
        if (buff.length())
            buff.append(QStringLiteral("|"));

        buff.append(QStringLiteral("size"));
    }

    if(properties & QueryRevision::Content)
    {
        if (buff.length())
            buff.append(QStringLiteral("|"));

        buff.append(QStringLiteral("content"));
    }

    d->requestParameter[QStringLiteral("rvprop")] = buff;
}

void QueryRevision::setPageId(unsigned int pageId)
{
    Q_D(QueryRevision);
    d->requestParameter[QStringLiteral("pageids")] = QString::number(pageId);
}

void QueryRevision::setRevisionId(unsigned int revisionId)
{
    Q_D(QueryRevision);
    d->requestParameter[QStringLiteral("revids")] = QString::number(revisionId);
}

void QueryRevision::setLimit(int limit)
{
    Q_D(QueryRevision);
    d->requestParameter[QStringLiteral("rvlimit")] = QString::number(limit);
}

void QueryRevision::setStartId(int startId)
{
    Q_D(QueryRevision);
    d->requestParameter[QStringLiteral("rvstartid")] = QString::number(startId);
}

void QueryRevision::setEndId(int endId)
{
    Q_D(QueryRevision);
    d->requestParameter[QStringLiteral("rvendid")] = QString::number(endId);
}

void QueryRevision::setStartTimestamp(const QDateTime& start)
{
    Q_D(QueryRevision);
    d->requestParameter[QStringLiteral("rvstart")] = start.toString(QStringLiteral("yyyy-MM-ddThh:mm:ssZ"));
}

void QueryRevision::setEndTimestamp(const QDateTime& end)
{
    Q_D(QueryRevision);
    d->requestParameter[QStringLiteral("rvend")] = end.toString(QStringLiteral("yyyy-MM-ddThh:mm:ssZ"));
}

void QueryRevision::setUser(const QString& user)
{
    Q_D(QueryRevision);
    d->requestParameter[QStringLiteral("rvuser")] = user;
}

void QueryRevision::setExcludeUser(const QString& excludeUser)
{
    Q_D(QueryRevision);
    d->requestParameter[QStringLiteral("rvexcludeuser")] = excludeUser;
}

void QueryRevision::setDirection(QueryRevision::Direction direction)
{
    Q_D(QueryRevision);

    if (direction == QueryRevision::Older)
    {
        d->requestParameter[QStringLiteral("rvdir")] = QStringLiteral("older");
    }
    else if (direction == QueryRevision::Newer)
    {
        d->requestParameter[QStringLiteral("rvdir")] = QStringLiteral("newer");
    }
}

void QueryRevision::setGenerateXML(bool generateXML)
{
    Q_D(QueryRevision);

    if (generateXML)
    {
        d->requestParameter[QStringLiteral("rvgeneratexml")] = QStringLiteral("on");
    }
}

void QueryRevision::setSection(int section)
{
    Q_D(QueryRevision);
    d->requestParameter[QStringLiteral("rvsection")] = QString::number(section);
}

void QueryRevision::setToken(QueryRevision::Token token)
{
    Q_D(QueryRevision);

    if (QueryRevision::Rollback == token)
    {
        d->requestParameter[QStringLiteral("rvtoken")] = QStringLiteral("rollback");
    }
}

void QueryRevision::setExpandTemplates(bool expandTemplates)
{
    Q_D(QueryRevision);

    if (expandTemplates)
    {
        d->requestParameter[QStringLiteral("rvexpandtemplates")] = QStringLiteral("on");
    }
}
void QueryRevision::doWorkSendRequest()
{
    Q_D(QueryRevision);

    // Set the url
    QUrl url = d->mediawiki.url();
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("format"), QStringLiteral("xml"));
    query.addQueryItem(QStringLiteral("action"), QStringLiteral("query"));
    query.addQueryItem(QStringLiteral("prop"),   QStringLiteral("revisions"));

    QMapIterator<QString, QString> i(d->requestParameter);

    while (i.hasNext())
    {
        i.next();
        query.addQueryItem(i.key(), i.value());
    }
    url.setQuery(query);

    // Set the request
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", d->mediawiki.userAgent().toUtf8());

    setPercent(25); // Request ready.

    // Send the request
    d->reply = d->manager->get(request);
    connectReply();

    connect(d->reply, SIGNAL(finished()), 
            this, SLOT(doWorkProcessReply()));

    setPercent(50); // Request sent.
}

void QueryRevision::doWorkProcessReply()
{
    Q_D(QueryRevision);

    disconnect(d->reply, SIGNAL(finished()),
               this, SLOT(doWorkProcessReply()));

    setPercent(75); // Response received.

    if (d->reply->error() == QNetworkReply::NoError)
    {
        QList<Revision> results;
        Revision        tempR;
        QString         replytmp = QString::fromUtf8(d->reply->readAll());

        if (d->requestParameter.contains(QStringLiteral("rvgeneratexml")))
        {
            for (int i = replytmp.indexOf(QStringLiteral("parsetree")); i != -1; i = replytmp.indexOf(QStringLiteral("parsetree"), i+1))
            {
                int count = 0;

                while (count < 2)
                {
                    if (replytmp[i] == QLatin1Char('"') && replytmp[i-1] != QLatin1Char('\\')) count++;
                    if (replytmp[i] == QLatin1Char('<'))                          replytmp[i] = char(255);
                    if (replytmp[i] == QLatin1Char('>'))                          replytmp[i] = char(254);
                    ++i;
                }
            }
        }

        QXmlStreamReader reader(replytmp);

        while (!reader.atEnd() && !reader.hasError())
        {
            QXmlStreamReader::TokenType token = reader.readNext();

            if (token == QXmlStreamReader::StartElement)
            {
                if (reader.name() == QLatin1String("page") && d->requestParameter.contains(QStringLiteral("rvtoken")))
                {
                    tempR.setRollback(reader.attributes().value(QStringLiteral("rollbacktoken")).toString());
                }

                if (reader.name() == QLatin1String("rev"))
                {
                    if (d->requestParameter.contains(QStringLiteral("rvprop")))
                    {
                        QString rvprop = d->requestParameter[QStringLiteral("rvprop")];

                        if (rvprop.contains(QStringLiteral("ids")))
                        {
                            tempR.setRevisionId(reader.attributes().value(QStringLiteral("revid")).toString().toInt());
                            tempR.setParentId(reader.attributes().value(QStringLiteral("parentid")).toString().toInt());}

                            if (rvprop.contains(QStringLiteral("size")))
                                tempR.setSize(reader.attributes().value(QStringLiteral("size")).toString().toInt());

                            if (rvprop.contains(QStringLiteral("minor")))
                                tempR.setMinorRevision(true);

                            if (rvprop.contains(QStringLiteral("user")))
                                tempR.setUser(reader.attributes().value(QStringLiteral("user")).toString());

                            if (rvprop.contains(QStringLiteral("timestamp")))
                                tempR.setTimestamp(QDateTime::fromString(reader.attributes().value(QStringLiteral("timestamp")).toString(),QStringLiteral("yyyy-MM-ddThh:mm:ssZ")));

                            if (rvprop.contains(QStringLiteral("comment")))
                                tempR.setComment(reader.attributes().value(QStringLiteral("comment")).toString());

                            if (d->requestParameter.contains(QStringLiteral("rvgeneratexml")))
                                tempR.setParseTree(reader.attributes().value(QStringLiteral("parsetree")).toString());

                            if (rvprop.contains(QStringLiteral("content")))
                                tempR.setContent(reader.readElementText());
                        }

                        results << tempR;
                    }
                    else if (reader.name() == QLatin1String("error"))
                    {
                        if (reader.attributes().value(QStringLiteral("code")).toString() == QLatin1String("rvrevids"))
                            this->setError(this->WrongRevisionId);
                        else if (reader.attributes().value(QStringLiteral("code")).toString() == QLatin1String("rvmultpages"))
                            this->setError(this->MultiPagesNotAllowed);
                        else if (reader.attributes().value(QStringLiteral("code")).toString() == QLatin1String("rvaccessdenied"))
                            this->setError(this->TitleAccessDenied);
                        else if (reader.attributes().value(QStringLiteral("code")).toString() == QLatin1String("rvbadparams"))
                            this->setError(this->TooManyParams);
                        else if (reader.attributes().value(QStringLiteral("code")).toString() == QLatin1String("rvnosuchsection"))
                            this->setError(this->SectionNotFound);

                        d->reply->close();
                        d->reply->deleteLater();
                        //emit revision(QList<Revision>());
                        emitResult();
                        return;
                    }
              }
        }
        if (!reader.hasError())
        {
            setError(KJob::NoError);

            for (int i = 0; i < results.length(); i++)
            {
                results[i].setParseTree(results[i].parseTree().replace(QChar(254), QStringLiteral(">")));
                results[i].setParseTree(results[i].parseTree().replace(QChar(255), QStringLiteral("<")));
            }

            emit revision(results);
            setPercent(100); // Response parsed successfully.
        }
        else
        {
            setError(XmlError);
            d->reply->close();
            d->reply->deleteLater();
            //emit revision(QList<Revision>());
        }
    }
    else
    {
        setError(NetworkError);
        d->reply->close();
        d->reply->deleteLater();
        //emit revision(QList<Revision>());
    }

    emitResult();
}

} // namespace mediawiki
