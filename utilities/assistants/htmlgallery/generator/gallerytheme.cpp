/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "gallerytheme.h"

// Qt includes

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QUrl>
#include <QDir>

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

static GalleryTheme::List sList;

class GalleryTheme::Private
{
public:

    explicit Private()
        : desktopFile(0)
    {
    }

    KDesktopFile* desktopFile;
    QUrl          url;
    ParameterList parameterList;

public:

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
        delete desktopFile;

        desktopFile                   = new KDesktopFile(desktopFileName);
        url                           = QUrl::fromLocalFile(desktopFileName);
        QStringList parameterNameList = readParameterNameList(desktopFileName);

        readParameters(parameterNameList);
    }

    void readParameters(const QStringList& list)
    {
        QStringList::ConstIterator it  = list.constBegin();
        QStringList::ConstIterator end = list.constEnd();

        for (; it != end ; ++it)
        {
            QString groupName                 = QLatin1String(PARAMETER_GROUP_PREFIX) + *it;
            QByteArray internalName           = it->toUtf8();
            KConfigGroup group                = desktopFile->group(groupName);
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
            parameterList << parameter;
        }
    }
};

GalleryTheme::GalleryTheme()
   : d(new Private)
{
}

GalleryTheme::~GalleryTheme()
{
    delete d->desktopFile;
    delete d;
}

const GalleryTheme::List& GalleryTheme::getList()
{
    if (sList.isEmpty())
    {
        QStringList list;
        QStringList internalNameList;
        const QStringList filter     = QStringList() << QLatin1String("*.desktop");
        const QStringList themesDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                                 QLatin1String("digikam/themes"),
                                                                 QStandardPaths::LocateDirectory);

        foreach(const QString& themeDir, themesDirs)
        {
            foreach(const QFileInfo& themeInfo, QDir(themeDir).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
            {
                foreach(const QFileInfo& deskFile, QDir(themeInfo.absoluteFilePath()).entryInfoList(filter))
                {
                    list << deskFile.absoluteFilePath();
                }
            }
        }

        QStringList::ConstIterator it  = list.constBegin();
        QStringList::ConstIterator end = list.constEnd();

        for (; it != end ; ++it)
        {
            GalleryTheme* const theme = new GalleryTheme;
            theme->d->init(*it);
            QString internalName      = theme->internalName();

            if (!internalNameList.contains(internalName))
            {
                sList << GalleryTheme::Ptr(theme);
                internalNameList << internalName;
            }
        }
    }

    return sList;
}

GalleryTheme::Ptr GalleryTheme::findByInternalName(const QString& internalName)
{
    const GalleryTheme::List& lst         = getList();
    GalleryTheme::List::ConstIterator it  = lst.constBegin();
    GalleryTheme::List::ConstIterator end = lst.constEnd();

    for (; it != end ; ++it)
    {
        GalleryTheme::Ptr theme = *it;

        if (theme->internalName() == internalName)
        {
            return theme;
        }
    }

    return GalleryTheme::Ptr(0);
}

QString GalleryTheme::internalName() const
{
    return d->url.fileName();
}

QString GalleryTheme::name() const
{
    return d->desktopFile->readName();
}

QString GalleryTheme::comment() const
{
    return d->desktopFile->readComment();
}

QString GalleryTheme::directory() const
{
    return d->url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).toLocalFile();
}

QString GalleryTheme::authorName() const
{
    return d->desktopFile->group(AUTHOR_GROUP).readEntry("Name");
}

QString GalleryTheme::authorUrl() const
{
    return d->desktopFile->group(AUTHOR_GROUP).readEntry("Url");
}

QString GalleryTheme::previewName() const
{
    return d->desktopFile->group(PREVIEW_GROUP).readEntry("Name");
}

QString GalleryTheme::previewUrl() const
{
    return d->desktopFile->group(PREVIEW_GROUP).readEntry("Url");
}

bool GalleryTheme::allowNonsquareThumbnails() const
{
    return d->desktopFile->group(OPTIONS_GROUP).readEntry("Allow non-square thumbnails", false);
}

GalleryTheme::ParameterList GalleryTheme::parameterList() const
{
    return d->parameterList;
}

} // namespace Digikam
