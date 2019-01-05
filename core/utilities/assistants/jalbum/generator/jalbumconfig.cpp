/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2012-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

JalbumConfig::JalbumConfig()
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

JalbumConfig::~JalbumConfig()
{
}

void JalbumConfig::setDestUrl(const QUrl& v)
{
    if (!isImmutable(QLatin1String("destUrl")))
        m_destUrl = v;
}

QUrl JalbumConfig::destUrl() const
{
    return m_destUrl;
}

void JalbumConfig::setJarUrl(const QUrl& v)
{
    if (!isImmutable(QLatin1String("jarUrl")))
        m_jarUrl = v;
}

QUrl JalbumConfig::jarUrl() const
{
    return m_jarUrl;
}

void JalbumConfig::setImageSelectionTitle(const QString& v)
{
    if (!isImmutable(QLatin1String("imageSelectionTitle")))
        m_imageSelectionTitle = v;
}

QString JalbumConfig::imageSelectionTitle() const
{
    return m_imageSelectionTitle;
}

} // namespace Digikam
