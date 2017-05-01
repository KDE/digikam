/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "theme.h"

// Qt includes

#include <QFile>
#include <QTextStream>
#include <QUrl>

// KDE includes

#include <kconfiggroup.h>
#include <kdesktopfile.h>

// Local includes

#include "digikam_debug.h"
#include "colorthemeparameter.h"
#include "intthemeparameter.h"
#include "listthemeparameter.h"
#include "stringthemeparameter.h"

namespace Digikam
{

static const char* AUTHOR_GROUP           = "X-HTMLExport Author";
static const char* PARAMETER_GROUP_PREFIX = "X-HTMLExport Parameter ";
static const char* PARAMETER_TYPE_KEY     = "Type";
static const char* PREVIEW_GROUP          = "X-HTMLExport Preview";
static const char* OPTIONS_GROUP          = "X-HTMLExport Options";
static const char* STRING_PARAMETER_TYPE  = "string";
static const char* LIST_PARAMETER_TYPE    = "list";
static const char* COLOR_PARAMETER_TYPE   = "color";
static const char* INT_PARAMETER_TYPE     = "int";

static Theme::List sList;

struct Theme::Private
{
    Private()
        : mDesktopFile(0)
    {
    }

    KDesktopFile* mDesktopFile;
    QUrl          mUrl;
    ParameterList mParameterList;

    /**
     * Return the list of parameters defined in the desktop file. We need to
     * parse the file ourself to preserve parameter order.
     */
    QStringList readParameterNameList(const QString& desktopFileName)
    {
        QStringList list;
        QFile file(desktopFileName);

        if (!file.open(QIODevice::ReadOnly))
        {
            return QStringList();
        }

        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        QString prefix = QLatin1String("[") + QLatin1String(PARAMETER_GROUP_PREFIX);

        while (!stream.atEnd())
        {
            QString line = stream.readLine();
            line         = line.trimmed();

            if (!line.startsWith(prefix))
            {
                continue;
            }

            // Remove opening bracket and group prefix
            line         = line.mid(prefix.length());

            // Remove closing bracket
            line.truncate(line.length() - 1);

            list.append(line);
        }

        return list;
    }

    void init(const QString& desktopFileName)
    {
        delete mDesktopFile;
        mDesktopFile = new KDesktopFile(desktopFileName);
        mUrl.setPath(desktopFileName);

        QStringList parameterNameList = readParameterNameList(desktopFileName);
        readParameters(parameterNameList);
    }

    void readParameters(const QStringList& list)
    {
        QStringList::ConstIterator it=list.constBegin(), end=list.constEnd();

        for (;it!=end; ++it)
        {
            QString groupName                 = QLatin1String(PARAMETER_GROUP_PREFIX) + *it;
            QByteArray internalName           = it->toUtf8();
            KConfigGroup group                = mDesktopFile->group(groupName);
            QString type                      = group.readEntry(PARAMETER_TYPE_KEY);
            AbstractThemeParameter* parameter = 0;

            if (type == QLatin1String(STRING_PARAMETER_TYPE))
            {
                parameter = new StringThemeParameter();
            }
            else if (type == QLatin1String(LIST_PARAMETER_TYPE))
            {
                parameter = new ListThemeParameter();
            }
            else if (type == QLatin1String(COLOR_PARAMETER_TYPE))
            {
                parameter = new ColorThemeParameter();
            }
            else if (type == QLatin1String(INT_PARAMETER_TYPE))
            {
                parameter = new IntThemeParameter();
            }
            else
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Parameter '" << internalName
                                           << "' has unknown type '" << type
                                           << "'. Falling back to string type\n";
                parameter = new StringThemeParameter();
            }

            parameter->init(internalName, &group);
            mParameterList << parameter;
        }
    }
};

Theme::Theme()
   : d(new Private)
{
}

Theme::~Theme()
{
    delete d->mDesktopFile;
    delete d;
}

const Theme::List& Theme::getList()
{
    if (sList.isEmpty())
    {
        QStringList internalNameList;
        const QStringList list        = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                                  QLatin1String("kipiplugin_htmlexport/themes/*/*.desktop"));
        QStringList::ConstIterator it = list.constBegin(), end=list.constEnd();

        for (; it != end ; ++it)
        {
            Theme* const theme   = new Theme;
            theme->d->init(*it);
            QString internalName = theme->internalName();

            if (!internalNameList.contains(internalName))
            {
                sList << Theme::Ptr(theme);
                internalNameList << internalName;
            }
        }
    }

    return sList;
}

Theme::Ptr Theme::findByInternalName(const QString& internalName)
{
    const Theme::List& lst        = getList();
    Theme::List::ConstIterator it = lst.constBegin(), end=lst.constEnd();

    for (; it != end ; ++it)
    {
        Theme::Ptr theme = *it;

        if (theme->internalName() == internalName)
        {
            return theme;
        }
    }

    return Theme::Ptr(0);
}

QString Theme::internalName() const
{
    QUrl url = d->mUrl;
    url      = url.adjusted(QUrl::RemoveFilename);
    url.setPath(url.path() + QLatin1String(""));

    return url.fileName();
}

QString Theme::name() const
{
    return d->mDesktopFile->readName();
}

QString Theme::comment() const
{
    return d->mDesktopFile->readComment();
}

QString Theme::directory() const
{
    return d->mUrl.adjusted(QUrl::RemoveFilename).path();
}

QString Theme::authorName() const
{
    return d->mDesktopFile->group(AUTHOR_GROUP).readEntry("Name");
}

QString Theme::authorUrl() const
{
    return d->mDesktopFile->group(AUTHOR_GROUP).readEntry("Url");
}

QString Theme::previewName() const
{
        return d->mDesktopFile->group(PREVIEW_GROUP).readEntry("Name");
}

QString Theme::previewUrl() const
{
        return d->mDesktopFile->group(PREVIEW_GROUP).readEntry("Url");
}

bool Theme::allowNonsquareThumbnails() const
{
    return d->mDesktopFile->group(OPTIONS_GROUP).readEntry("Allow non-square thumbnails", false);
}

Theme::ParameterList Theme::parameterList() const
{
    return d->mParameterList;
}

} // namespace Digikam
