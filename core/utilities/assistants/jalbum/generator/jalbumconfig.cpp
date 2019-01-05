/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate jAlbum image galleries
 *
 * Copyright (C) 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "jalbumconfig.h"

// Qt includes

#include <QCoreApplication>
#include <QDebug>

#define JALBUM_JAR_PATH "/usr/share/jalbum/JAlbum.jar"

namespace Digikam
{

JAlbumConfig::JAlbumConfig()
    : KConfigSkeleton(QLatin1String("digikamrc"))
{
    setCurrentGroup(QLatin1String("jAlbum tool"));

    // -------------------

    QString dfltAlbumPath;
    QUrl dfltAlbumQurl;

#ifdef Q_OS_WIN
    dfltAlbumPath = QLatin1String(qgetenv("HOMEDRIVE").constData());
    dfltAlbumPath.append(QLatin1String(qgetenv("HOMEPATH").constData()));
    dfltAlbumPath.append(QLatin1String("\\Documents\\My Albums"));
#else
    dfltAlbumPath = QLatin1String(qgetenv("HOME").constData());
    dfltAlbumPath.append(QLatin1String("/Documents/My Albums"));
#endif

    dfltAlbumQurl = QUrl::fromUserInput(dfltAlbumPath, QString(), QUrl::AssumeLocalFile);

    KConfigSkeleton::ItemUrl* const itemdestUrl
        = new KConfigSkeleton::ItemUrl(currentGroup(), QLatin1String("destUrl"), m_destUrl, dfltAlbumQurl);

    addItem(itemdestUrl, QLatin1String("destUrl"));

    // -------------------

    QString dfltJarPath(QLatin1String(JALBUM_JAR_PATH));
    QUrl dfltJarQurl;

#ifdef Q_OS_WIN
    dfltJarPath = QLatin1String(qgetenv("ProgramFiles").constData());
    dfltJarPath.append(QLatin1String("\\jAlbum\\JAlbum.jar"));
#endif

    dfltJarQurl = QUrl::fromUserInput(dfltJarPath, QString(), QUrl::AssumeLocalFile);

    KConfigSkeleton::ItemUrl* const itemjarUrl
        = new KConfigSkeleton::ItemUrl(currentGroup(), QLatin1String("jarUrl"), m_jarUrl, dfltJarQurl);

    addItem(itemjarUrl, QLatin1String("jarUrl"));

    // -------------------

    KConfigSkeleton::ItemString* const itemimageSelectionTitle
        = new KConfigSkeleton::ItemString(currentGroup(), QLatin1String("imageSelectionTitle"), m_imageSelectionTitle);

    addItem(itemimageSelectionTitle, QLatin1String("imageSelectionTitle"));
}

JAlbumConfig::~JAlbumConfig()
{
}

void JAlbumConfig::setDestUrl(const QUrl& v)
{
    if (!isImmutable(QLatin1String("destUrl")))
        m_destUrl = v;
}

QUrl JAlbumConfig::destUrl() const
{
    return m_destUrl;
}

void JAlbumConfig::setJarUrl(const QUrl& v)
{
    if (!isImmutable(QLatin1String("jarUrl")))
        m_jarUrl = v;
}

QUrl JAlbumConfig::jarUrl() const
{
    return m_jarUrl;
}

void JAlbumConfig::setImageSelectionTitle(const QString& v)
{
    if (!isImmutable(QLatin1String("imageSelectionTitle")))
        m_imageSelectionTitle = v;
}

QString JAlbumConfig::imageSelectionTitle() const
{
    return m_imageSelectionTitle;
}

} // namespace Digikam
